import com.sun.star.lang.NullPointerException;
import com.sun.star.uno.AnyConverter;
import java.awt.BorderLayout;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JProgressBar;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.JTree;
import javax.swing.event.TreeSelectionEvent;
import javax.swing.event.TreeSelectionListener;
import javax.swing.event.TreeWillExpandListener;
import javax.swing.tree.DefaultTreeSelectionModel;
import javax.swing.tree.ExpandVetoException;
import javax.swing.tree.TreePath;
import javax.swing.tree.TreeSelectionModel;
/*
 * SwingTreeControlProvider.java
 *
 * Created on 12. Dezember 2006, 11:43
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */

/**
 *
 * @author bc93774
 */
public class SwingTreeControlProvider implements XTreeControlProvider{
    private JTextArea jtxtGeneratedSourceCode = new JTextArea();
    private JTextField jtxtFilter = new JTextField();
    private javax.swing.JLabel jlblFilter = new javax.swing.JLabel("Set a filter");
    private JPanel jPnlBottom = new JPanel(new BorderLayout());
    private JPanel jPanelProgress = new JPanel(new BorderLayout());
    private JPanel jPnlPath = new JPanel(new BorderLayout());
    private JLabel jLblPath = new JLabel("Generated source code");
    private JProgressBar jProgressBar1 = new JProgressBar();
    private JTree jTree = new javax.swing.JTree();
    private XDialogProvider m_xDialogProvider;
    private HideableTreeModel treeModel;
    private UnoTreeRenderer oUnoTreeRenderer;
    private InspectorPane m_oInspectorPane;
    private Object oUserDefinedObject = null;
    private boolean bIsUserDefined = false;
    private UnoObjectNode root;

    private final int nDIALOGWIDTH = 800;



    public SwingTreeControlProvider(XDialogProvider _xDialogProvider){
        m_xDialogProvider = _xDialogProvider;
    }


    public void addInspectorPane(InspectorPane _oInspectorPane){
        m_oInspectorPane = _oInspectorPane;
    }

    public InspectorPane getInspectorPane() throws NullPointerException{
        if (m_oInspectorPane == null){
            throw new NullPointerException("InspectorPage has not been added to TreeControl");
        }
        else{
            return m_oInspectorPane;
        }
    }

    private void ComponentSelector(Object _oRootObject, String _sRootTreeNodeName) {
        String sTreeNodeName =  _sRootTreeNodeName;
        root = new UnoObjectNode(_oRootObject, _sRootTreeNodeName);
        treeModel = new HideableTreeModel(root);
        jTree.setModel(treeModel);
        jTree.setRootVisible(true);
        jTree.setVisible(true);
        root.addDummyNode();
        enableFilterElements(null);
    }



    private void insertTopPanel(JPanel _jPnlCenter){
        javax.swing.JPanel jPnlTop = new javax.swing.JPanel(new BorderLayout(10, 10));
        jPnlTop.setPreferredSize(new java.awt.Dimension(nDIALOGWIDTH, 20));
        jlblFilter.setHorizontalAlignment(javax.swing.SwingConstants.RIGHT);
        jlblFilter.setHorizontalTextPosition(javax.swing.SwingConstants.RIGHT);
        jPnlTop.add(jlblFilter, java.awt.BorderLayout.WEST);
        jtxtFilter.setHorizontalAlignment(javax.swing.JTextField.LEFT);
        jtxtFilter.setPreferredSize(new java.awt.Dimension(200, 10));
        jtxtFilter.addKeyListener(new InspectorKeyFilterAdapter());
        jPnlTop.add(jtxtFilter, java.awt.BorderLayout.CENTER);
        _jPnlCenter.add(jPnlTop, java.awt.BorderLayout.NORTH);
    }


    private void insertBottomPanel(JPanel _jPnlCenter){
        jPnlPath.setPreferredSize(new Dimension(nDIALOGWIDTH,220));
        jPnlPath.add(jLblPath, java.awt.BorderLayout.NORTH);
        jtxtGeneratedSourceCode.setTabSize(4);
        JScrollPane jScrollPane = new JScrollPane(jtxtGeneratedSourceCode);
        jScrollPane.setPreferredSize(new Dimension(nDIALOGWIDTH, 205));
        jtxtGeneratedSourceCode.setEditable(false);
        jPnlPath.add(jScrollPane, java.awt.BorderLayout.SOUTH);
        jPnlBottom.add(jPnlPath, java.awt.BorderLayout.SOUTH);
//        initializeProgressPanel();
        _jPnlCenter.add(jPnlBottom, java.awt.BorderLayout.SOUTH);
    }

        private void insertBorderPanes(Container _cp){
            JPanel jPnlEast  = new JPanel(new BorderLayout());
            JPanel jPnlNorth = new JPanel(new BorderLayout());
            JPanel jPnlSouth = new JPanel(new BorderLayout());
            JPanel jPnlWest = new JPanel(new BorderLayout());
            _cp.add(jPnlNorth, java.awt.BorderLayout.NORTH);
            jPnlWest.setPreferredSize(new java.awt.Dimension(10, 10));
            _cp.add(jPnlWest, java.awt.BorderLayout.WEST);
            jPnlEast.setPreferredSize(new java.awt.Dimension(10, 10));
            _cp.add(jPnlEast, java.awt.BorderLayout.EAST);
            jPnlSouth.setPreferredSize(new java.awt.Dimension(10, 10));
            _cp.add(jPnlSouth, java.awt.BorderLayout.SOUTH);
        }


        public String enableFilterElements(UnoNode _oUnoNode){
            String sFilter ="";
            boolean bIsFacetteNode = m_oInspectorPane.isFacetteNode(_oUnoNode);
            this.jtxtFilter.setEnabled(bIsFacetteNode);
            this.jlblFilter.setEnabled(bIsFacetteNode);
            sFilter = m_oInspectorPane.getFilter(_oUnoNode);
            jtxtFilter.setText(sFilter);
            return sFilter;
        }


        private class InspectorKeyFilterAdapter extends KeyAdapter{
            public void keyReleased(KeyEvent e){
                String sFilter = jtxtFilter.getText();
                SwingTreePathProvider oSwingTreePathProvider = new SwingTreePathProvider(jTree.getSelectionPath());
                UnoNode oUnoNode = oSwingTreePathProvider.getLastPathComponent();
                if (oUnoNode instanceof UnoFacetteNode){
                    m_oInspectorPane.applyFilter((UnoFacetteNode) oUnoNode, sFilter);
                }
            }
        }



        /** Inspect the given object for methods, properties, interfaces, and
         * services.
         * @param a The object to inspect
         * @throws RuntimeException If
         */
        public Object inspect(java.lang.Object _oUserDefinedObject, String _sTitle) throws com.sun.star.uno.RuntimeException {
        JPanel jPnlContainer = new javax.swing.JPanel(new BorderLayout(10, 10));
        try {
            javax.swing.JPanel jPnlCenter = new javax.swing.JPanel();
            bIsUserDefined = (_oUserDefinedObject != null);
            if (bIsUserDefined){
                oUserDefinedObject = _oUserDefinedObject;
                m_oInspectorPane.setTitle(_sTitle);
            }
            javax.swing.JScrollPane jScrollPane1 = new javax.swing.JScrollPane();
            TreeSelectionModel tsm = new DefaultTreeSelectionModel();
            tsm.setSelectionMode(TreeSelectionModel.SINGLE_TREE_SELECTION);
            jTree.setSelectionModel(tsm);
            jTree.setVisible(false);
            jPnlCenter.setLayout(new java.awt.BorderLayout(10, 10));
            insertTopPanel(jPnlCenter);
            jScrollPane1.setViewportView(jTree);
            jScrollPane1.setPreferredSize(new java.awt.Dimension(600, 600));
            jPnlCenter.add(jScrollPane1, java.awt.BorderLayout.CENTER);
            insertBottomPanel(jPnlCenter);
            UnoTreeRenderer oUnoTreeRenderer = new UnoTreeRenderer();
            jTree.setCellRenderer(oUnoTreeRenderer);
            jTree.addTreeSelectionListener(
                new TreeSelectionListener() {
                    public void valueChanged(TreeSelectionEvent event) {
                        TreePath tp = event.getNewLeadSelectionPath();
                        if (tp != null){
                            UnoNode oUnoNode = getSelectedNode();
                            String sFilter = enableFilterElements(oUnoNode);
                        }
                    }
                }
                );
            //  Add KeyListener for help
            jTree.addKeyListener( new java.awt.event.KeyAdapter() {
                public void keyReleased(java.awt.event.KeyEvent event) {
                    if ( event.getKeyCode() == KeyEvent.VK_F1 ) {
                        //  function key F1 pressed
                        TreePath aTreePath = jTree.getSelectionPath();
                        UnoNode oUnoNode = (UnoNode) aTreePath.getLastPathComponent();
                        oUnoNode.openIdlDescription();
                    }
                }
            });
            jTree.addMouseListener(new MouseAdapter() {
                public void mousePressed (MouseEvent e) {
                    if (e.isPopupTrigger()) {
                        m_oInspectorPane.showPopUpMenu(e.getComponent(), e.getX(), e.getY());

                    }
                }
            });
            jPnlContainer.add(jPnlCenter, java.awt.BorderLayout.CENTER);
            insertBorderPanes(jPnlContainer);
            if (this.bIsUserDefined){
                ComponentSelector(oUserDefinedObject, m_oInspectorPane.getTitle());
            }
            m_xDialogProvider.addInspectorPage(m_oInspectorPane.getTitle(), jPnlContainer);
            addTreeExpandListener();
            }
            catch( Exception exception ) {
                exception.printStackTrace(System.out);
            }
            return jPnlContainer;
        }

        public HideableTreeModel getModel(){
            return  treeModel;
        }


        public void addTreeExpandListener(){
            jTree.addTreeWillExpandListener(
                new TreeWillExpandListener() {
                    public void treeWillExpand(javax.swing.event.TreeExpansionEvent event) throws javax.swing.tree.ExpandVetoException {
                    SwingTreePathProvider oSwingTreePathProvider = new SwingTreePathProvider(event.getPath());
                    UnoNode oUnoNode = oSwingTreePathProvider.getLastPathComponent();
                    try{
                        if (!m_oInspectorPane.expandNode(oUnoNode, oSwingTreePathProvider)){
                            throw new ExpandVetoException(event);
                        }
                    }finally{
//                        hideProgressPanel();
                    }}

                    public void treeWillCollapse( javax.swing.event.TreeExpansionEvent evt) throws javax.swing.tree.ExpandVetoException {
                    }
            });
        }


    public void setSourceCode(String _sSourceCode){
        jtxtGeneratedSourceCode.setText(_sSourceCode);
    }


    public XTreePathProvider getSelectedPath(){
        return new SwingTreePathProvider(jTree.getSelectionPath());
    }


    public void expandPath(XTreePathProvider _xTreePathProvider) throws ClassCastException{
        SwingTreePathProvider oSwingTreePathProvider = (SwingTreePathProvider) _xTreePathProvider;
        jTree.expandPath(oSwingTreePathProvider.getSwingTreePath());
    }


    public UnoNode getSelectedNode(){
        UnoNode oUnoNode = null;
        TreePath aTreePath = jTree.getSelectionPath();
        Object oNode = aTreePath.getLastPathComponent();
        if (oNode instanceof UnoNode){
            oUnoNode = (UnoNode) oNode;
        }
        return oUnoNode;
    }


    public void nodeInserted(UnoNode _oParentNode, UnoNode _oChildNode, int index) {
        getModel().nodeInserted(_oParentNode, _oChildNode, _oParentNode.getChildCount()-1);
    }


    public void nodeChanged(UnoNode _oNode) {
        getModel().nodeChanged(_oNode);
    }


    public boolean setNodeVisible(Object node, boolean v) {
       return getModel().setNodeVisible(node, v);
    }


    public void setMaximumOfProgressBar(){
        jProgressBar1.setMaximum(5);
    }


    public boolean isPropertyNode(UnoNode _oUnoNode){
        return (_oUnoNode instanceof UnoPropertyNode);
    }


    public boolean isMethodNode(UnoNode _oUnoNode){
        return (_oUnoNode instanceof UnoMethodNode);
    }


    public boolean isObjectNode(UnoNode _oUnoNode){
        return (_oUnoNode instanceof UnoObjectNode);
    }


    public boolean isFacetteNode(UnoNode _oUnoNode){
        return (_oUnoNode instanceof UnoFacetteNode);
    }

    public boolean isServiceNode(UnoNode _oUnoNode){
        return (_oUnoNode instanceof UnoServiceNode);
    }


    public boolean isInterfaceNode(UnoNode _oUnoNode){
        return (_oUnoNode instanceof UnoInterfaceNode);
    }
}


//        public ProgressDisplay getProgressDisplay(){
//            return new ProgressDisplay();
//        }
//
//
//    public class ProgressDisplay extends Thread{
//        public ProgressDisplay(){
//        }
//
//        public void  run ()   {
//  try {
//            synchronized(jProgressBar1){
//                jProgressBar1.setVisible(true);
//                jPanelProgress.setVisible(true);
//                m_xDialogProvider.paint();
//                jProgressBar1.paintImmediately(jProgressBar1.getBounds());
//                jProgressBar1.setStringPainted(true);
//            }
//  } catch (RuntimeException e) {
//  }}
//    }
//
//
//    public void setProgressValue(int _nValue, String _sTitle){
//        this.jProgressBar1.setValue(_nValue);
//        this.jProgressBar1.setString(_sTitle);
//        this.jProgressBar1.setStringPainted(true);
//        jProgressBar1.paint(jProgressBar1.getGraphics());
//    }


//        private void initializeProgressPanel(){
//            JPanel jPnlProgressSouth = new JPanel(new BorderLayout());
//            jPnlProgressSouth.setPreferredSize(new Dimension(nDIALOGWIDTH, 10));
//            jProgressBar1 = new javax.swing.JProgressBar();
//            jProgressBar1.setStringPainted(true);
//            jPanelProgress.add(jProgressBar1, java.awt.BorderLayout.NORTH);
//            jPanelProgress.add(jPnlProgressSouth, java.awt.BorderLayout.SOUTH);
//            jPnlBottom.add(jPanelProgress, java.awt.BorderLayout.NORTH);
//            hideProgressPanel();
//        }
//
//        public JPanel getBottomPanel(){
//            return jPnlBottom;
//        }
//
//
//        public void hideProgressPanel(){
//            jProgressBar1.setValue(0);
//            jProgressBar1.setString("");
//            jProgressBar1.setVisible(false);
//            jPanelProgress.setVisible(false);
//            jPanelProgress.invalidate();
//            jProgressBar1.invalidate();
//            this.m_xDialogProvider.paint();
//            jTree.paintImmediately(jTree.getBounds());
//        }


