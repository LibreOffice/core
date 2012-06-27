/************************************************************************
 *
 * Licensed Materials - Property of IBM.
 * (C) Copyright IBM Corporation 2003, 2012.  All Rights Reserved.
 * U.S. Government Users Restricted Rights:
 * Use, duplication or disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 ************************************************************************/

package org.openoffice.test.assistant.views;

import java.io.File;
import java.lang.reflect.InvocationTargetException;
import java.util.ArrayList;

import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.core.runtime.IStatus;
import org.eclipse.jdt.core.JavaConventions;
import org.eclipse.jface.action.Action;
import org.eclipse.jface.action.IMenuListener;
import org.eclipse.jface.action.IMenuManager;
import org.eclipse.jface.action.IToolBarManager;
import org.eclipse.jface.action.MenuManager;
import org.eclipse.jface.action.Separator;
import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.jface.operation.IRunnableWithProgress;
import org.eclipse.jface.preference.IPreferenceStore;
import org.eclipse.jface.text.IDocument;
import org.eclipse.jface.text.ITextSelection;
import org.eclipse.jface.viewers.CellEditor;
import org.eclipse.jface.viewers.ColumnViewerEditor;
import org.eclipse.jface.viewers.ColumnViewerEditorActivationEvent;
import org.eclipse.jface.viewers.ColumnViewerEditorActivationStrategy;
import org.eclipse.jface.viewers.DoubleClickEvent;
import org.eclipse.jface.viewers.ICellModifier;
import org.eclipse.jface.viewers.IDoubleClickListener;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.viewers.TableViewer;
import org.eclipse.jface.viewers.TableViewerEditor;
import org.eclipse.jface.viewers.TextCellEditor;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.ModifyEvent;
import org.eclipse.swt.events.ModifyListener;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Menu;
import org.eclipse.swt.widgets.Table;
import org.eclipse.swt.widgets.TableColumn;
import org.eclipse.swt.widgets.TableItem;
import org.eclipse.swt.widgets.Text;
import org.eclipse.ui.IActionBars;
import org.eclipse.ui.IEditorPart;
import org.eclipse.ui.IWorkbench;
import org.eclipse.ui.IWorkbenchActionConstants;
import org.eclipse.ui.IWorkbenchPage;
import org.eclipse.ui.IWorkbenchWindow;
import org.eclipse.ui.PlatformUI;
import org.eclipse.ui.part.ViewPart;
import org.eclipse.ui.texteditor.AbstractTextEditor;
import org.eclipse.ui.texteditor.IDocumentProvider;
import org.openoffice.test.assistant.Activator;
import org.openoffice.test.assistant.preferences.PreferenceConstants;
import org.openoffice.test.vcl.client.CommandCaller.WinInfoReceiver;
import org.openoffice.test.vcl.client.Constant;
import org.openoffice.test.vcl.client.SmartId;
import org.openoffice.test.vcl.client.VclHook;
import org.openoffice.test.vcl.widgets.VclApp;

public class VclExplorer extends ViewPart implements WinInfoReceiver {
    public static final String ID = "org.vclauto.assistant.views.VclExplorer";
    private TableViewer viewer;
    private Label messageLabel;
    private Action inspectAction;
    private Action launchAction;
    private Action insertCodeAction;
    private UIMapOp uiMapOp = new UIMapOp();
    private ArrayList<ControlInfo> controlInfos = new ArrayList<ControlInfo>();
    private Display display;
    private Color duplicatedNameColor;
    /**
     * The constructor.
     */
    public VclExplorer() {
        VclHook.getCommandCaller().setWinInfoReceiver(this);
    }

    public void createPartControl(Composite parent) {
        GridLayout layout = new GridLayout();
        layout.numColumns = 1;
        layout.marginLeft = 0;
        layout.marginRight = 0;
        layout.marginTop = 0;
        layout.marginBottom = 0;
        layout.marginHeight = 0;
        layout.marginWidth = 0;
        parent.setLayout(layout);

        messageLabel = new Label(parent, SWT.NONE);
        GridData gridData = new GridData(GridData.FILL_HORIZONTAL);
        gridData.minimumHeight = 14;
        gridData.heightHint = 14;
        messageLabel.setLayoutData(gridData);

        viewer = new TableViewer(parent, SWT.H_SCROLL | SWT.V_SCROLL | SWT.BORDER);
        gridData = new GridData(GridData.FILL_BOTH);
        viewer.getTable().setLayoutData(gridData);

        viewer.setContentProvider(new VclExplorerContentProvider());
        viewer.setLabelProvider(new VclExplorerLabelProvider());
        viewer.setInput(controlInfos);

        display = parent.getDisplay();
        duplicatedNameColor = new Color(display, 255, 160, 160);
        final Table table = viewer.getTable();
        table.setHeaderVisible(true);
        table.setLinesVisible(true);
        TableColumn column = new TableColumn(table, SWT.NONE);
        column.setText("ID");
        column.setResizable(true);
        column.setMoveable(false);
        column.setWidth(100);
        column = new TableColumn(table, SWT.NONE);
        column.setText("Name");
        column.setResizable(true);
        column.setMoveable(false);
        column.setWidth(150);

        column = new TableColumn(table, SWT.NONE);
        column.setText("Type");
        column.setResizable(true);
        column.setMoveable(false);
        column.setWidth(100);
        column = new TableColumn(table, SWT.NONE);
        column.setText("Tips");
        column.setResizable(true);
        column.setMoveable(false);
        column.setWidth(200);

        viewer.setColumnProperties(new String[] { "ID", "Name", "Type", "Tips" });
        TextCellEditor nameCellEditor = new TextCellEditor(viewer.getTable());
        final Text nameText = ((Text) nameCellEditor.getControl());
        nameText.addModifyListener(new ModifyListener() {
                    public void modifyText(ModifyEvent me) {
                        String message = validateName(nameText.getText());
                        if (message != null) {
                            messageLabel.setText(message);
                            nameText.setBackground(duplicatedNameColor);
                        } else {
                            messageLabel.setText("");
                            nameText.setBackground(null);
                        }
                    }
                });

        viewer.setCellEditors(new CellEditor[] { null, nameCellEditor, null, null});
        viewer.setCellModifier(new ICellModifier() {

            public boolean canModify(Object element, String property) {
                if (!"Name".equals(property))
                    return false;

                ControlInfo controlInfo = (ControlInfo) element;
                if (controlInfo.name != null)
                    return false;

                return uiMapOp.scan();
            }

            public Object getValue(Object element, String property) {
//              if (!"Name".equals(property))
//                  return false;
                ControlInfo controlInfo = (ControlInfo) element;
                return controlInfo.name == null ? "" : controlInfo.name;
            }

            public void modify(Object element, String property, Object value) {
//              if (!"Name".equals(property))
//                  return;
                TableItem item = (TableItem) element;
                ControlInfo controlInfo = (ControlInfo) item.getData();
                String newName = (String) value;
                String msg = validateName(newName);
                if (msg == null) {
                    controlInfo.name = newName;
                    if (doDefineName(controlInfo)) {
                        item.setText(1, newName);
                        messageLabel.setText("");
                    } else {
                        controlInfo.name = null;
                        messageLabel.setText("Can't define the control in UIMap. UIMap maybe is broken!");
                    }
                } else {
                    messageLabel.setText(msg);
                }
            }

        });

        ColumnViewerEditorActivationStrategy actSupport = new ColumnViewerEditorActivationStrategy(
                viewer) {
            protected boolean isEditorActivationEvent(
                    ColumnViewerEditorActivationEvent event) {
                return event.eventType == ColumnViewerEditorActivationEvent.TRAVERSAL
                        || event.eventType == ColumnViewerEditorActivationEvent.MOUSE_DOUBLE_CLICK_SELECTION
                        || event.eventType == ColumnViewerEditorActivationEvent.PROGRAMMATIC;
            }
        };

        TableViewerEditor.create(viewer, actSupport,
                ColumnViewerEditor.TABBING_HORIZONTAL
                        | ColumnViewerEditor.TABBING_MOVE_TO_ROW_NEIGHBOR
                        | ColumnViewerEditor.TABBING_VERTICAL
                        | ColumnViewerEditor.KEYBOARD_ACTIVATION);

        // Create the help context id for the viewer's control
        PlatformUI.getWorkbench().getHelpSystem().setHelp(viewer.getControl(), "org.vclauto.assistant.viewer");
        makeActions();
        hookContextMenu();
        hookDoubleClickAction();
        contributeToActionBars();
    }

    private void hookContextMenu() {
        MenuManager menuMgr = new MenuManager("#PopupMenu");
        menuMgr.setRemoveAllWhenShown(true);
        menuMgr.addMenuListener(new IMenuListener() {
            public void menuAboutToShow(IMenuManager manager) {
                VclExplorer.this.fillContextMenu(manager);
            }
        });
        Menu menu = menuMgr.createContextMenu(viewer.getControl());
        viewer.getControl().setMenu(menu);
        getSite().registerContextMenu(menuMgr, viewer);
    }

    private void contributeToActionBars() {
        IActionBars bars = getViewSite().getActionBars();
        fillLocalPullDown(bars.getMenuManager());
        fillLocalToolBar(bars.getToolBarManager());
    }

    private void fillLocalPullDown(IMenuManager manager) {
        manager.add(inspectAction);
        manager.add(new Separator());
        manager.add(launchAction);
    }

    private void fillContextMenu(IMenuManager manager) {
        manager.add(inspectAction);
        manager.add(launchAction);
        // Other plug-ins can contribute there actions here
        manager.add(new Separator(IWorkbenchActionConstants.MB_ADDITIONS));
    }

    private void fillLocalToolBar(IToolBarManager manager) {
        manager.add(inspectAction);
        manager.add(launchAction);
    }

    private void makeActions() {
        inspectAction = new Action() {
            public void run() {
                if (!VclHook.available()) {
                    showMessage("OpenOffice is not launched or initializing. Click \"Launch\" to start OpenOffice or wait it to finish initializing.");
                    return;
                }
                try {
                    VclHook.invokeCommand(Constant.RC_DisplayHid, new Object[]{Boolean.TRUE});
                } catch (Exception e) {
                    showMessage("OpenOffice disappeared! It maybe crashed or freezed. Please re-launch it.");
                }


            }
        };
        inspectAction.setText("Inspect");
        inspectAction.setToolTipText("Inspect VCL controls.");
//      action1.setImageDescriptor(PlatformUI.getWorkbench().getSharedImages().
//          getImageDescriptor(ISharedImages.IMG_OBJS_INFO_TSK));

        launchAction = new Action() {
            public void run() {
                IPreferenceStore store = Activator.getDefault().getPreferenceStore();
                final String ooHome = store.getString(PreferenceConstants.P_OPENOFFICE_HOME);
                if (ooHome == null || ooHome.length() == 0) {
                    showMessage("Please set OpenOffice home in the Vclauto assistant preference page.");
                    return;
                }
                File ooHomeFile = new File(ooHome, "soffice.bin");
                if (!ooHomeFile.exists()) {
                    showMessage("OpenOffice home is not set correctly in the Vclauto assistant preference page.");
                    return;
                }

                IRunnableWithProgress op = new IRunnableWithProgress() {
                    @Override
                    public void run(IProgressMonitor arg0) throws InvocationTargetException, InterruptedException {
                        VclApp app = new VclApp(ooHome);
                        if (!app.exists()) {
                            app.kill();
                            app.start();
                        }
                    }
                };

                IWorkbench wb = PlatformUI.getWorkbench();

                try {
                    wb.getProgressService().run(true, false, op);
                } catch (InvocationTargetException e) {
                    e.printStackTrace();
                } catch (InterruptedException e) {

                    e.printStackTrace();
                }
            }
        };
        launchAction.setText("Launch");
        launchAction.setToolTipText("Launch OpenOffice");
//      action2.setImageDescriptor(PlatformUI.getWorkbench().getSharedImages().
//              getImageDescriptor(ISharedImages.IMG_OBJS_INFO_TSK));
        insertCodeAction = new Action() {
            public void run() {
                ISelection selection = viewer.getSelection();
                if (selection == null)
                    return;

                final ControlInfo controlInfo = (ControlInfo)((IStructuredSelection) selection).getFirstElement();
                if (controlInfo.name != null) {
                    doInsertCode(controlInfo.name);
                }
            }
        };
    }

    private String validateName(String name) {
         if (uiMapOp.names.contains(name))
             return "Duplicated name.";
         IStatus status = JavaConventions.validateFieldName(name, "1.5", "1.5");
         return status.isOK() ? null : "It's an invalidate java field name.";
    }
    private boolean doDefineName(ControlInfo controlInfo) {
        try {
            uiMapOp.define(controlInfo);
        } catch (Exception e) {
            controlInfo.name = null;
            e.printStackTrace();
        }

        return true;
    }

    private void doInsertCode(String code) {
        IWorkbench wb = PlatformUI.getWorkbench();
        IWorkbenchWindow window = wb.getActiveWorkbenchWindow();
        IWorkbenchPage page = window.getActivePage();

        IEditorPart editorPart = page.getActiveEditor();
        if (!(editorPart instanceof AbstractTextEditor))
            return;

        AbstractTextEditor editor = (AbstractTextEditor) editorPart;
        IDocumentProvider dp = editor.getDocumentProvider();
        IDocument doc = dp.getDocument(editor.getEditorInput());
        try {
            ITextSelection textSelection = (ITextSelection) editorPart.getSite().getSelectionProvider().getSelection();
            int offset = textSelection.getOffset();

            doc.replace(offset, textSelection.getLength(), code);
        } catch (Exception e) {
            // TODO: handle exception
            e.printStackTrace();
        }
    }

    private void hookDoubleClickAction() {
        viewer.addDoubleClickListener(new IDoubleClickListener() {
            public void doubleClick(DoubleClickEvent event) {

                insertCodeAction.run();
            }
        });
    }
    private void showMessage(String message) {
        MessageDialog.openInformation(
            viewer.getControl().getShell(),
            "Vcl Explorer",
            message);
    }

    /**
     * Passing the focus request to the viewer's control.
     */
    public void setFocus() {
        viewer.getControl().setFocus();
    }


    @Override
    public void addWinInfo(final SmartId id, final long type, final String t) {
        final String tooltip = t.replaceAll("%.*%.*:", "");
        ControlInfo info = new ControlInfo(id, type, tooltip);
        //info.shot();
        controlInfos.add(info);
    }

    @Override
    public void onFinishReceiving() {
//      for (ControlInfo info : controlInfos) {
//          info.shot();
//      }
//
        display.asyncExec(new Runnable() {
            public void run() {
                if (uiMapOp.scan()) {
                    uiMapOp.populateName(controlInfos);
                } else {
                    messageLabel.setText("Can not find UIMap and code generating will not work.");
                }
                viewer.setInput(controlInfos);
            }
        });
    }

    @Override
    public void onStartReceiving() {
        controlInfos = new ArrayList<ControlInfo>();
    }
}