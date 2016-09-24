/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

import com.sun.star.beans.Property;
import com.sun.star.beans.PropertyValue;
import com.sun.star.lang.NullPointerException;
import com.sun.star.reflection.XConstantTypeDescription;
import com.sun.star.reflection.XIdlMethod;
import com.sun.star.uno.Type;
import java.awt.BorderLayout;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import javax.swing.JPanel;
import javax.swing.JScrollBar;
import javax.swing.JScrollPane;
import javax.swing.JSplitPane;
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



public class SwingTreeControlProvider implements XTreeControlProvider{
    private JTextArea jtxtGeneratedSourceCode = new JTextArea();
    private JTextField jtxtFilter = new JTextField();
    private javax.swing.JLabel jlblFilter = new javax.swing.JLabel("Set a filter");
    private JTree jTree = new javax.swing.JTree();
    private XDialogProvider m_xDialogProvider;
    private HideableTreeModel treeModel;
    private InspectorPane m_oInspectorPane;
    private Object oUserDefinedObject = null;
    private boolean bIsUserDefined = false;
    private SwingUnoNode oRootNode;

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
        oRootNode = new SwingUnoNode(_oRootObject);
        oRootNode.setLabel(_sRootTreeNodeName);
        treeModel = new HideableTreeModel(oRootNode);
        jTree.setModel(treeModel);
        jTree.setRootVisible(true);
        jTree.setVisible(true);
        oRootNode.setFoldable(true);
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


    private void insertBottomPanel(JSplitPane _jSplitPane){ //JPanel _jPnlCenter){
        jtxtGeneratedSourceCode.setTabSize(4);
        jtxtGeneratedSourceCode.getAccessibleContext().setAccessibleName("generated SourceCode");
        JScrollPane jScrollPane = new JScrollPane(jtxtGeneratedSourceCode);
        jScrollPane.setPreferredSize(new Dimension(nDIALOGWIDTH,205));
        jtxtGeneratedSourceCode.setEditable(false);
        _jSplitPane.setBottomComponent(jScrollPane);
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


        public String enableFilterElements(XUnoNode _oUnoNode){
            String sFilter ="";
            boolean bIsFacetteNode = isFacetteNode(_oUnoNode);
            this.jtxtFilter.setEnabled(bIsFacetteNode);
            this.jlblFilter.setEnabled(bIsFacetteNode);
            sFilter = m_oInspectorPane.getFilter(_oUnoNode);
            jtxtFilter.setText(sFilter);
            return sFilter;
        }


        private class InspectorKeyFilterAdapter extends KeyAdapter{
            @Override
            public void keyReleased(KeyEvent e){
                String sFilter = jtxtFilter.getText();
                SwingTreePathProvider oSwingTreePathProvider = new SwingTreePathProvider(jTree.getSelectionPath());
                XUnoNode oUnoNode = oSwingTreePathProvider.getLastPathComponent();
                if (oUnoNode instanceof XUnoFacetteNode){
                    m_oInspectorPane.applyFilter((XUnoFacetteNode) oUnoNode, sFilter);
                }
            }
        }



        /** Inspect the given object for methods, properties, interfaces, and
         * services.
         * @param _oUserDefinedObject The object to inspect
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
            jPnlCenter.getAccessibleContext().setAccessibleName("inspection tab view");
            insertTopPanel(jPnlCenter);
            jScrollPane1.setViewportView(jTree);
            jScrollPane1.setPreferredSize(new java.awt.Dimension(600, 600));
            jScrollPane1.getAccessibleContext().setAccessibleName("API view scroll pane");

            JScrollBar jHScrollBar = jScrollPane1.createHorizontalScrollBar();
            jHScrollBar.getAccessibleContext().setAccessibleName("API view horizontal scroll bar");
            jScrollPane1.setHorizontalScrollBar(jHScrollBar);

            JScrollBar jVScrollBar = jScrollPane1.createVerticalScrollBar();
            jVScrollBar.getAccessibleContext().setAccessibleName("API view vertical scroll bar");
            jScrollPane1.setVerticalScrollBar(jVScrollBar);

            JSplitPane jSplitPane = new JSplitPane(JSplitPane.VERTICAL_SPLIT);
            jSplitPane.setTopComponent(jScrollPane1);
            jPnlCenter.add(jSplitPane, java.awt.BorderLayout.CENTER);
            jSplitPane.setDividerLocation(500);
            insertBottomPanel(jSplitPane);
            UnoTreeRenderer oUnoTreeRenderer = new UnoTreeRenderer();
            jTree.setCellRenderer(oUnoTreeRenderer);
            jTree.addTreeSelectionListener(
                new TreeSelectionListener() {
                    public void valueChanged(TreeSelectionEvent event) {
                        TreePath tp = event.getNewLeadSelectionPath();
                        if (tp != null){
                            XUnoNode oUnoNode = getSelectedNode();
                            String sFilter = enableFilterElements(oUnoNode);
                        }
                    }
                }
                );
            //  Add KeyListener for help
            jTree.addKeyListener( new java.awt.event.KeyAdapter() {
                @Override
                public void keyReleased(java.awt.event.KeyEvent event) {
                    if ( event.getKeyCode() == KeyEvent.VK_F1 ) {
                        //  function key F1 pressed
                        TreePath aTreePath = jTree.getSelectionPath();
                        SwingUnoNode oUnoNode = (SwingUnoNode) aTreePath.getLastPathComponent();
                        oUnoNode.openIdlDescription(m_xDialogProvider.getIDLPath());
                    }
                }
            });
            jTree.addMouseListener(new MouseAdapter() {
                @Override
                public void mousePressed (MouseEvent e) {
                    if (e.isPopupTrigger()){
                        m_oInspectorPane.showPopUpMenu(e.getComponent(), e.getX(), e.getY());
                    }
                    //unfortunately under Windows the method "isPopupTrigger" always returns false
                    else if ((e.getModifiersEx() & MouseEvent.BUTTON3_DOWN_MASK) == MouseEvent.BUTTON3_DOWN_MASK){
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
                exception.printStackTrace(System.err);
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
                        XUnoNode oUnoNode = oSwingTreePathProvider.getLastPathComponent();
                        if (!m_oInspectorPane.expandNode(oUnoNode)){
                            throw new ExpandVetoException(event);
                        }
                    }

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


    public XUnoNode getSelectedNode(){
        XUnoNode oUnoNode = null;
        TreePath aTreePath = jTree.getSelectionPath();
        Object oNode = aTreePath.getLastPathComponent();
        if (oNode instanceof XUnoNode){
            oUnoNode = (XUnoNode) oNode;
        }
        return oUnoNode;
    }


    public void nodeInserted(XUnoNode _oParentNode, XUnoNode _oChildNode, int index) {
        getModel().nodeInserted(_oParentNode, _oChildNode, _oParentNode.getChildCount()-1);
    }


    public void nodeChanged(XUnoNode _oNode) {
        getModel().nodeChanged(_oNode);
    }


    public boolean setNodeVisible(Object node, boolean v) {
       return getModel().setNodeVisible(node, v);
    }


    public boolean isPropertyNode(XUnoNode _oUnoNode){
        return (_oUnoNode instanceof SwingUnoPropertyNode);
    }


    public boolean isMethodNode(XUnoNode _oUnoNode){
        return (_oUnoNode instanceof SwingUnoMethodNode);
    }


    public boolean isFacetteNode(XUnoNode _oUnoNode){
        return (_oUnoNode instanceof SwingUnoFacetteNode);
    }


    public XUnoNode addUnoNode(Object _oUnoObject){
        return new SwingUnoNode(_oUnoObject);
    }


    public XUnoNode addUnoNode(Object _oUnoObject, Type _aType){
        return new SwingUnoNode(_oUnoObject, _aType);
    }


    public XUnoFacetteNode addUnoFacetteNode(XUnoNode _oParentNode, String _sNodeDescription, Object _oUnoObject){
        SwingUnoFacetteNode oSwingUnoFacetteNode = new SwingUnoFacetteNode(_oUnoObject);
        oSwingUnoFacetteNode.setLabel(_sNodeDescription);
        if (_oParentNode != null){
            ((SwingUnoNode) _oParentNode).addChildNode(oSwingUnoFacetteNode);
        }
        return oSwingUnoFacetteNode;
    }


    public XUnoMethodNode addMethodNode(Object _objectElement, XIdlMethod _xIdlMethod){
        SwingUnoMethodNode oSwingUnoMethodNode = new SwingUnoMethodNode(_xIdlMethod, _objectElement, m_xDialogProvider);
        return oSwingUnoMethodNode;
    }


    public XUnoPropertyNode addUnoPropertyNodeWithName(Property _aProperty){
        SwingUnoPropertyNode oUnoPropertyNode = new SwingUnoPropertyNode(_aProperty);
        oUnoPropertyNode.setLabel("Name: " + _aProperty.Name);
        return oUnoPropertyNode;
    }


    public XUnoPropertyNode addUnoPropertyNodeWithHandle(Property _aProperty){
        SwingUnoPropertyNode oUnoPropertyNode = new SwingUnoPropertyNode(_aProperty);
        oUnoPropertyNode.setLabel("Handle: " + _aProperty.Handle);
        return oUnoPropertyNode;
    }


    public XUnoPropertyNode addUnoPropertyNodeWithType(Property _aProperty){
        SwingUnoPropertyNode oUnoPropertyNode = new SwingUnoPropertyNode(_aProperty);
        oUnoPropertyNode.setLabel("Type: " + _aProperty.Type.getTypeName());
        return oUnoPropertyNode;
    }


    public XUnoPropertyNode addUnoPropertyNodeWithAttributesDescription(Property _aProperty){
        SwingUnoPropertyNode oUnoPropertyNode = new SwingUnoPropertyNode(_aProperty);
        XConstantTypeDescription[] xPropertyAttributesTypeDescriptions = Introspector.getIntrospector().getFieldsOfConstantGroup("com.sun.star.beans.PropertyAttribute");
        String sDisplay = Introspector.getIntrospector().getConstantDisplayString(_aProperty.Attributes, xPropertyAttributesTypeDescriptions, "Attributes: ");
        oUnoPropertyNode.setLabel(sDisplay);
        return oUnoPropertyNode;
    }


    public XUnoPropertyNode addUnoPropertyNode(Object _oUnoObject, Property _aProperty){
        SwingUnoPropertyNode oUnoPropertyNode = new SwingUnoPropertyNode(_aProperty, _oUnoObject, null);
        oUnoPropertyNode.setPropertyNodeType(XUnoPropertyNode.nPROPERTYSETINFOTYPE);
        oUnoPropertyNode.setLabel(UnoPropertyNode.getStandardPropertyDescription(_aProperty, null));
        return oUnoPropertyNode;
    }


    public XUnoPropertyNode addUnoPropertyNode(Object _oUnoObject, Property _aProperty, Object _oUnoReturnObject){
        SwingUnoPropertyNode oUnoPropertyNode = new SwingUnoPropertyNode(_aProperty, _oUnoObject, _oUnoReturnObject);
        oUnoPropertyNode.setLabel(UnoPropertyNode.getStandardPropertyDescription(_aProperty, _oUnoReturnObject));
        return oUnoPropertyNode;
    }


    public XUnoPropertyNode addUnoPropertyNode(Object _oUnoObject, PropertyValue _aPropertyValue, Object _oReturnObject){
        SwingUnoPropertyNode oUnoPropertyNode = new SwingUnoPropertyNode(_aPropertyValue, _oUnoObject, _oReturnObject);
        oUnoPropertyNode.setLabel(UnoPropertyNode.getStandardPropertyValueDescription(_aPropertyValue));
        return oUnoPropertyNode;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
