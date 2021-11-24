#include "sources_form.h"

#include <QFileInfo>
#include <QMenu>
#include <QTextStream>

#include "ui_sources_form.h"

using namespace FOEDAG;

SourcesForm::SourcesForm(QString strproject, QWidget *parent)
    : QWidget(parent), ui(new Ui::SourcesForm) {
  ui->setupUi(this);

  m_treeSrcHierachy = new QTreeWidget(ui->m_tabHierarchy);
  m_treeSrcHierachy->setSelectionMode(
      QAbstractItemView::SelectionMode::SingleSelection);
  QVBoxLayout *vbox = new QVBoxLayout();
  vbox->addWidget(m_treeSrcHierachy);
  vbox->setContentsMargins(0, 0, 0, 0);
  vbox->setSpacing(0);
  ui->m_tabHierarchy->setLayout(vbox);

  CreateActions();

  m_projManager = new ProjectManager(this);
  m_projManager->StartProject(strproject + PROJECT_FILE_FORMAT);

  UpdateSrcHierachyTree();

  connect(m_treeSrcHierachy, SIGNAL(itemPressed(QTreeWidgetItem *, int)), this,
          SLOT(SlotItempressed(QTreeWidgetItem *, int)));

  ui->m_tabWidget->removeTab(ui->m_tabWidget->indexOf(ui->tab_2));
}

SourcesForm::~SourcesForm() { delete ui; }

void SourcesForm::TestOpenProject(int argc, const char *argv[]) {
  QTextStream out(stdout);
  if (argc < 3 || "--file" != QString(argv[1])) {
    out << "-----------open_project ------------\n";
    out << " \n";
    out << " Description: \n";
    out << " Open a project. Show the source file categories and hierarchies. "
           "\n";
    out << " \n";
    out << " Syntax: \n";
    out << " open_project --file <project.ospr> \n";
    out << " \n";
    out << "--------------------------------------\n";
    return;
  }

  QFileInfo fileInfo;
  fileInfo.setFile(QString(argv[2]));
  if (fileInfo.exists()) {
    m_projManager->StartProject(QString(argv[2]));
    UpdateSrcHierachyTree();
  } else {
    out << " Warning : This file <" << QString(argv[2]) << "> is not exist! \n";
  }
}

void SourcesForm::SlotItempressed(QTreeWidgetItem *item, int column) {
  Q_UNUSED(column);
  if (qApp->mouseButtons() == Qt::RightButton) {
    QMenu *menu = new QMenu(m_treeSrcHierachy);
    menu->addAction(m_actRefresh);
    QString strPropertyRole =
        (item->data(0, Qt::WhatsThisPropertyRole)).toString();
    if (SOURCE_TREE_TOPITEM == strPropertyRole) {
      menu->addAction(m_actAddFileSet);
    } else if (SOURCE_TREE_DESFILESETITEM == strPropertyRole) {
      menu->addAction(m_actRemoveFileSet);
      menu->addAction(m_actAddFile);
      menu->addAction(m_actMakeActive);
    } else if (SOURCE_TREE_DESFILEITEM == strPropertyRole) {
      menu->addAction(m_actOpenFile);
      menu->addAction(m_actRemoveFile);
      menu->addAction(m_actSetAsTop);
    } else if (SOURCE_TREE_CONSTRFSETITEM == strPropertyRole) {
      menu->addAction(m_actRemoveFileSet);
      menu->addAction(m_actAddFile);
      menu->addAction(m_actMakeActive);
    } else if (SOURCE_TREE_CONSTRFILEITEM == strPropertyRole) {
      menu->addAction(m_actOpenFile);
      menu->addAction(m_actRemoveFile);
      menu->addAction(m_actSetAsTarget);
    } else if (SOURCE_TREE_SIMFILESETITEM == strPropertyRole) {
      menu->addAction(m_actRemoveFileSet);
      menu->addAction(m_actAddFile);
      menu->addAction(m_actMakeActive);
    } else if (SOURCE_TREE_SIMFILEITEM == strPropertyRole) {
      menu->addAction(m_actOpenFile);
      menu->addAction(m_actRemoveFile);
      menu->addAction(m_actSetAsTop);
    }
    QPoint p = QCursor::pos();
    menu->exec(QPoint(p.rx(), p.ry() + 3));
  }
}

void SourcesForm::SlotRefreshSourceTree() { UpdateSrcHierachyTree(); }

void SourcesForm::CreateActions() {
  m_actRefresh = new QAction(tr("Refresh Hierarchy"), m_treeSrcHierachy);
  connect(m_actRefresh, SIGNAL(triggered()), this,
          SLOT(SlotRefreshSourceTree()));

  m_actAddFileSet = new QAction(tr("Add FileSet"), m_treeSrcHierachy);
  connect(m_actAddFileSet, SIGNAL(triggered()), this, SLOT(SlotAddFileSet()));

  m_actAddFile = new QAction(tr("Add Sources"), m_treeSrcHierachy);
  connect(m_actAddFile, SIGNAL(triggered()), this, SLOT(SlotAddFile()));

  m_actOpenFile = new QAction(tr("Open File"), m_treeSrcHierachy);
  connect(m_actOpenFile, SIGNAL(triggered()), this, SLOT(SlotOpenFile()));

  m_actRemoveFileSet = new QAction(tr("Remove FileSet"), m_treeSrcHierachy);
  connect(m_actRemoveFileSet, SIGNAL(triggered()), this,
          SLOT(SlotRemoveFileSet()));

  m_actRemoveFile = new QAction(tr("Remove File"), m_treeSrcHierachy);
  connect(m_actRemoveFile, SIGNAL(triggered()), this, SLOT(SlotRemoveFile()));

  m_actSetAsTop = new QAction(tr("Set As TopModule"), m_treeSrcHierachy);
  connect(m_actSetAsTop, SIGNAL(triggered()), this, SLOT(SlotSetAsTop()));

  m_actSetAsTarget =
      new QAction(tr("Set as Target Constraint File"), m_treeSrcHierachy);
  connect(m_actSetAsTarget, SIGNAL(triggered()), this, SLOT(SlotSetAsTarget()));

  m_actMakeActive = new QAction(tr("Make Active"), m_treeSrcHierachy);
  connect(m_actMakeActive, SIGNAL(triggered()), this, SLOT(SlotSetActive()));
}

void SourcesForm::UpdateSrcHierachyTree() {
  if (nullptr == m_projManager) {
    return;
  }

  m_treeSrcHierachy->clear();
  QTreeWidgetItem *topitemDS = new QTreeWidgetItem(m_treeSrcHierachy);
  topitemDS->setText(0, tr("Design Sources"));
  topitemDS->setData(0, Qt::WhatsThisPropertyRole, SOURCE_TREE_TOPITEM);
  m_treeSrcHierachy->addTopLevelItem(topitemDS);

  QStringList listDesFset = m_projManager->getDesignFileSets();
  QString strDesAct = m_projManager->getDesignActiveFileSet();
  foreach (auto str, listDesFset) {
    QTreeWidgetItem *itemfolder = new QTreeWidgetItem(topitemDS);
    if (str == strDesAct) {
      itemfolder->setText(0, str + tr("(Active)"));
    } else {
      itemfolder->setText(0, str);
    }
    itemfolder->setData(0, Qt::WhatsThisPropertyRole,
                        SOURCE_TREE_DESFILESETITEM);

    QStringList listDesFile = m_projManager->getDesignFiles(str);
    QString strTop = m_projManager->getDesignTopModule(str);
    foreach (auto strfile, listDesFile) {
      QString filename =
          strfile.right(strfile.size() - (strfile.lastIndexOf("/") + 1));
      QTreeWidgetItem *itemf = new QTreeWidgetItem(itemfolder);
      if (filename == strTop) {
        itemf->setText(0, filename + tr("(Top)"));
      } else {
        itemf->setText(0, filename);
      }
      itemf->setData(0, Qt::WhatsThisPropertyRole, SOURCE_TREE_DESFILEITEM);
    }
  }

  QTreeWidgetItem *topitemCS = new QTreeWidgetItem(m_treeSrcHierachy);
  m_treeSrcHierachy->addTopLevelItem(topitemCS);
  topitemCS->setText(0, tr("Constraints"));
  topitemCS->setData(0, Qt::WhatsThisPropertyRole, SOURCE_TREE_TOPITEM);
  QStringList listConstrFset = m_projManager->getConstrFileSets();
  QString strConstrAct = m_projManager->getConstrActiveFileSet();
  foreach (auto str, listConstrFset) {
    QTreeWidgetItem *itemfolder = new QTreeWidgetItem(topitemCS);
    if (str == strConstrAct) {
      itemfolder->setText(0, str + tr("(Active)"));
    } else {
      itemfolder->setText(0, str);
    }
    itemfolder->setData(0, Qt::WhatsThisPropertyRole,
                        SOURCE_TREE_CONSTRFSETITEM);

    QStringList listConstrFile = m_projManager->getConstrFiles(str);
    QString strTarget = m_projManager->getConstrTargetFile(str);
    foreach (auto strfile, listConstrFile) {
      QString filename =
          strfile.right(strfile.size() - (strfile.lastIndexOf("/") + 1));
      QTreeWidgetItem *itemf = new QTreeWidgetItem(itemfolder);
      if (filename == strTarget) {
        itemf->setText(0, filename + tr("(Target)"));
      } else {
        itemf->setText(0, filename);
      }
      itemf->setData(0, Qt::WhatsThisPropertyRole, SOURCE_TREE_CONSTRFILEITEM);
    }
  }

  QTreeWidgetItem *topitemSS = new QTreeWidgetItem(m_treeSrcHierachy);
  m_treeSrcHierachy->addTopLevelItem(topitemSS);
  topitemSS->setText(0, tr("Simulation Sources"));
  topitemSS->setData(0, Qt::WhatsThisPropertyRole, SOURCE_TREE_TOPITEM);
  QStringList listSimFset = m_projManager->getSimulationFileSets();
  QString strSimAct = m_projManager->getSimulationActiveFileSet();
  foreach (auto str, listSimFset) {
    QTreeWidgetItem *itemfolder = new QTreeWidgetItem(topitemSS);
    if (str == strSimAct) {
      itemfolder->setText(0, str + tr("(Active)"));
    } else {
      itemfolder->setText(0, str);
    }
    itemfolder->setData(0, Qt::WhatsThisPropertyRole,
                        SOURCE_TREE_SIMFILESETITEM);

    QStringList listSimFile = m_projManager->getSimulationFiles(str);
    QString strTop = m_projManager->getSimulationTopModule(str);
    foreach (auto strfile, listSimFile) {
      QString filename =
          strfile.right(strfile.size() - (strfile.lastIndexOf("/") + 1));
      QTreeWidgetItem *itemf = new QTreeWidgetItem(itemfolder);
      if (filename == strTop) {
        itemf->setText(0, filename + tr("(Top)"));
      } else {
        itemf->setText(0, filename);
      }
      itemf->setData(0, Qt::WhatsThisPropertyRole, SOURCE_TREE_SIMFILEITEM);
    }
  }

  m_treeSrcHierachy->setHeaderHidden(true);
  m_treeSrcHierachy->expandAll();
}