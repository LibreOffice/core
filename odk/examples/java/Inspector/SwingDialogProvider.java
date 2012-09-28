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

import com.sun.star.uno.XComponentContext;
import java.awt.Component;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.event.ActionListener;
import java.awt.event.ComponentAdapter;
import java.awt.event.ComponentEvent;
import java.awt.event.KeyEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import javax.swing.ButtonGroup;
import javax.swing.JDialog;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import javax.swing.JRadioButtonMenuItem;
import javax.swing.JTabbedPane;
import javax.swing.KeyStroke;



public class SwingDialogProvider implements XDialogProvider{

    private JPopupMenu m_jPopupMenu = new JPopupMenu();
    private XComponentContext m_xComponentContext;
    private Inspector._Inspector m_oInspector;
    private JDialog m_jInspectorDialog = new JDialog();
    private JTabbedPane m_jTabbedPane1 = new JTabbedPane();
    private Container cp;

    private JMenu jMnuOptions = new JMenu("Options");
    private JRadioButtonMenuItem jJavaMenuItem = null;
    private JRadioButtonMenuItem jCPlusPlusMenuItem = null;
    private JRadioButtonMenuItem jBasicMenuItem = null;

    /** Creates a new instance of SwingPopupMentuProvider */
    public SwingDialogProvider(Inspector._Inspector _oInspector, String _sTitle) {
        m_oInspector = _oInspector;
        m_xComponentContext = _oInspector.getXComponentContext();
        insertMenus();
        initializePopupMenu();
        cp = m_jInspectorDialog.getContentPane();
        cp.setLayout(new java.awt.BorderLayout(0, 10));
        m_jTabbedPane1.setTabLayoutPolicy(JTabbedPane.SCROLL_TAB_LAYOUT);
        m_jInspectorDialog.addWindowListener(new InspectorWindowAdapter());
        m_jInspectorDialog.addComponentListener(new InspectorComponentAdapter());
        m_jInspectorDialog.setTitle(_sTitle);
        m_jInspectorDialog.setLocation(100, 50);
        m_jInspectorDialog.getContentPane().add(m_jTabbedPane1);
    }



        public JDialog getDialog(){
            return m_jInspectorDialog;
        }


        public void addMenuBar(JMenuBar _jMenuBar){
            getDialog().setJMenuBar(_jMenuBar);
        }


        public void  removeTabPaneByIndex(int _nIndex){
            if (_nIndex > -1){
                String sSelInspectorPanelTitle = m_jTabbedPane1.getTitleAt(_nIndex);
                m_jTabbedPane1.remove(_nIndex);
                m_oInspector.getInspectorPages().remove(sSelInspectorPanelTitle);
            }
        }


        public void selectInspectorPageByIndex(int nTabIndex){
            m_jTabbedPane1.setSelectedIndex(nTabIndex);
        }


        public int getInspectorPageCount(){
            return  m_jTabbedPane1.getTabCount();
        }


        public JTabbedPane getTabbedPane(){
            return m_jTabbedPane1;
        }


        public InspectorPane getSelectedInspectorPage(){
            int nIndex = m_jTabbedPane1.getSelectedIndex();
            return getInspectorPage(nIndex);
        }


        public InspectorPane getInspectorPage(int _nIndex){
            InspectorPane oInspectorPane = null;
            if (_nIndex > -1){
                JPanel jPnlContainerInspectorPanel = (JPanel) m_jTabbedPane1.getComponentAt(_nIndex);
                String sInspectorPanelTitle = m_jTabbedPane1.getTitleAt(_nIndex);
                oInspectorPane = (InspectorPane) m_oInspector.getInspectorPages().get(sInspectorPanelTitle);
            }
            return oInspectorPane;
        }


        public void removeTabPanes(){
            int nCount = m_jTabbedPane1.getTabCount();
            if (nCount > 0){
                for (int i = nCount-1; i >= 0; i--){
                    removeTabPaneByIndex(i);
                }
            }
        }

        public void removeSelectedTabPane(){
            int nIndex = getTabbedPane().getSelectedIndex();
            removeTabPaneByIndex(nIndex);
        }


        private class InspectorComponentAdapter extends ComponentAdapter{
            public void componentHidden(ComponentEvent e){
                m_jInspectorDialog.pack();
                m_jInspectorDialog.invalidate();

            }

            public void componentShown(ComponentEvent e){
                m_jInspectorDialog.pack();
                m_jInspectorDialog.invalidate();
            }
        }

        private class InspectorWindowAdapter extends WindowAdapter{
            public void windowClosed(WindowEvent e){
                removeTabPanes();
                m_oInspector.disposeHiddenDocuments();
            }

            public void windowClosing(WindowEvent e){
                removeTabPanes();
                m_oInspector.disposeHiddenDocuments();
            }
        }



        private void initializePopupMenu(){
            m_jPopupMenu.add(getInspectMenuItem("Inspect"));
            m_jPopupMenu.add(getSourceCodeMenuItem(SADDTOSOURCECODE));
            m_jPopupMenu.add(getInvokeMenuItem(SINVOKE));
            m_jPopupMenu.addSeparator();
            m_jPopupMenu.add(getHelpMenuItem("Help"));
        }


        private void addOpenDocumentMenu(JMenu _jMnuRoot){
            ActionListener oActionListener = new ActionListener(){
                public void actionPerformed(java.awt.event.ActionEvent evt) {
                    String sTDocUrl = evt.getActionCommand();
                    m_oInspector.inspectOpenDocument(sTDocUrl);
                }
            };
            String[] sTDocUrls = m_oInspector.getTDocUrls();
            String[] sTDocTitles = m_oInspector.getTDocTitles(sTDocUrls);
            for (int i = 0; i < sTDocUrls.length; i++){
                addSingleMenuItem(_jMnuRoot, sTDocTitles[i], sTDocUrls[i], oActionListener);
            }
        }


        private void addApplicationDocumentMenu(JMenu _jMnuRoot){
            ActionListener oActionListener = new ActionListener(){
                public void actionPerformed(java.awt.event.ActionEvent evt) {
                    String sApplicationDocUrl = evt.getActionCommand();
                    m_oInspector.inspectOpenEmptyDocument(sApplicationDocUrl);
                }
            };
            String[][] sApplUrls = m_oInspector.getApplicationUrls();
            for (int i = 0; i < sApplUrls.length; i++){
                addSingleMenuItem(_jMnuRoot, sApplUrls[i][1], sApplUrls[i][0], oActionListener);
            }
        }


        private void addSingleMenuItem(JMenu _jMnuOpenDocs, String _sTitle, String _sActionCommand, ActionListener _oActionListener){
            javax.swing.JMenuItem jMnuItemOpenDoc = new javax.swing.JMenuItem(_sTitle);
            jMnuItemOpenDoc.setActionCommand(_sActionCommand);
            jMnuItemOpenDoc.addActionListener(_oActionListener);
            _jMnuOpenDocs.add(jMnuItemOpenDoc);
        }


        private void addHelpMenu(JMenuBar _jInspectMenuBar){
            JMenu jMnuHelp = new JMenu("Help");
            jMnuHelp.add(getHelpMenuItem("Idl-Help"));
            _jInspectMenuBar.add(jMnuHelp);
        }


        private JMenuItem getHelpMenuItem(String _sMenuTitle){
            JMenuItem jMnuHelpItem = new JMenuItem(_sMenuTitle);
            jMnuHelpItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_F1, 0));
            jMnuHelpItem.setMnemonic('H');
            jMnuHelpItem.addActionListener(new ActionListener(){
                public void actionPerformed(java.awt.event.ActionEvent evt) {
                    m_oInspector.openIdlFileforSelectedNode();
                }
            });
            return jMnuHelpItem;
        }

        private void addFileMenu(JMenuBar _jInspectMenuBar){
            JMenu jMnuFile = new JMenu("File");
            JMenuItem jMnuItemRemoveInspector = new JMenuItem("Remove");
            jMnuItemRemoveInspector.addActionListener(new ActionListener(){
                public void actionPerformed(java.awt.event.ActionEvent evt) {
                    removeSelectedTabPane();
                }
            });
            jMnuFile.add(jMnuItemRemoveInspector);
            JMenuItem jMnuItemExit = new JMenuItem("Exit");
            jMnuItemExit.addActionListener(new ActionListener(){
                public void actionPerformed(java.awt.event.ActionEvent evt) {
                    getDialog().dispose();
                }
            });
            jMnuFile.add(jMnuItemExit);
            _jInspectMenuBar.add(jMnuFile);
        }

        private JMenuItem getInspectMenuItem(String _sLabel){
            JMenuItem jMnuSelectedObject = new JMenuItem(_sLabel);
            jMnuSelectedObject.addActionListener(new ActionListener(){
                public void actionPerformed(java.awt.event.ActionEvent evt) {
                    m_oInspector.inspectSelectedNode();
                }
            });
            return jMnuSelectedObject;
        }


        private JMenuItem getSourceCodeMenuItem(String _sLabel){
            JMenuItem jMnuSelectedObject = new JMenuItem(_sLabel);
            jMnuSelectedObject.addActionListener(new ActionListener(){
                public void actionPerformed(java.awt.event.ActionEvent evt) {
                    m_oInspector.addSourceCodeOfSelectedNode();
                }
            });
            return jMnuSelectedObject;
        }

        private JMenuItem getInvokeMenuItem(String _sLabel){
            JMenuItem jMnuSelectedObject = new JMenuItem(_sLabel);
            jMnuSelectedObject.addActionListener(new ActionListener(){
                public void actionPerformed(java.awt.event.ActionEvent evt) {
                    m_oInspector.invokeSelectedMethod();
                }
            });
            return jMnuSelectedObject;
        }


        private void addInspectMenu(JMenuBar _jInspectMenuBar){
            JMenu jMnuInspect = new JMenu("Inspect");
            addApplicationDocumentMenu(jMnuInspect);
            jMnuInspect.addSeparator();
            addGlobalServiceManagerMenu(jMnuInspect);
            jMnuInspect.addSeparator();
            jMnuInspect.add(getInspectMenuItem("Selected Object"));
            jMnuInspect.addSeparator();
            addOpenDocumentMenu(jMnuInspect);
            _jInspectMenuBar.add(jMnuInspect);
        }

        public int getLanguage(){
            return XLanguageSourceCodeGenerator.nJAVA;
        }


        public void selectSourceCodeLanguage(int _nLanguage){
            switch (_nLanguage){
                case XLanguageSourceCodeGenerator.nJAVA:
                    jJavaMenuItem.setSelected(true);
                    break;
                case XLanguageSourceCodeGenerator.nCPLUSPLUS:
                    jCPlusPlusMenuItem.setSelected(true);
                    break;
                case XLanguageSourceCodeGenerator.nBASIC:
                    jBasicMenuItem.setSelected(true);
                    break;
                default:
                    System.out.println("Warning: Sourcecode language is not defined!");
            }
        }

        private JRadioButtonMenuItem addLanguageMenuItem(ButtonGroup _group, String _sLanguageTitle, boolean _bSelect, char _sMnemonic, final int _nLanguage){
            JRadioButtonMenuItem jMenuItem = new JRadioButtonMenuItem(_sLanguageTitle, _bSelect);
            jMenuItem.setMnemonic(_sMnemonic);
            _group.add(jMenuItem);
            jMenuItem.addActionListener(new ActionListener(){
                public void actionPerformed(java.awt.event.ActionEvent evt) {
                    m_oInspector.setSourceCodeLanguage(_nLanguage);
                }
            });
            return jMenuItem;
        }


        public String getIDLPath(){
            return this.m_oInspector.getIDLPath();
        }

        private void addOptionsMenu(JMenuBar _jInspectMenuBar){
            ButtonGroup oButtonGroup = new ButtonGroup();
            jJavaMenuItem = addLanguageMenuItem(oButtonGroup, "Generate Java Sourcecode", true, 'J', XLanguageSourceCodeGenerator.nJAVA);
            jMnuOptions.add(jJavaMenuItem);
            jCPlusPlusMenuItem = addLanguageMenuItem(oButtonGroup, "Generate C++ Sourcecode", false, 'C', XLanguageSourceCodeGenerator.nCPLUSPLUS);
            jMnuOptions.add(jCPlusPlusMenuItem);
            jBasicMenuItem = addLanguageMenuItem(oButtonGroup, "Generate OpenOffice.org Basic Sourcecode", false, 'B', XLanguageSourceCodeGenerator.nBASIC);
            jMnuOptions.add(jBasicMenuItem);
            jMnuOptions.addSeparator();
            JMenuItem jMenuItem = new JMenuItem("Path to SDK-Installation");
            jMenuItem.setMnemonic('I');
            jMenuItem.addActionListener(new ActionListener(){
                public void actionPerformed(java.awt.event.ActionEvent evt) {
                    m_oInspector.assignSDKPath();
                }
            });

            jMnuOptions.add(jMenuItem);
            _jInspectMenuBar.add(jMnuOptions);
        }


        private void insertMenus(){
            JMenuBar jMenuBar1 = new javax.swing.JMenuBar();
            addFileMenu(jMenuBar1);
            addInspectMenu(jMenuBar1);
            JMenu jMnuEdit = new JMenu("Edit");
            JMenu jMnuView = new JMenu("View");
            addOptionsMenu(jMenuBar1);
            jMenuBar1.setFont(new java.awt.Font("Dialog", 0, 12));
            jMenuBar1.add(jMnuEdit);
            jMenuBar1.add(jMnuView);
            addHelpMenu(jMenuBar1);
            addMenuBar(jMenuBar1);
        }

        private void addGlobalServiceManagerMenu(JMenu _jMnuRoot){
            JMenuItem jMnuGlobalServiceManager = new JMenuItem("Global Service Manager");
            jMnuGlobalServiceManager.addActionListener(new ActionListener(){
                public void actionPerformed(java.awt.event.ActionEvent evt) {
                    m_oInspector.inspect(m_xComponentContext.getServiceManager(), "Global ServiceManager");
                }
            });
            _jMnuRoot.add(jMnuGlobalServiceManager);
        }



        public void enablePopupMenuItem(String _sMenuTitle, boolean _bdoEnable){
            Component[] oComponents = m_jPopupMenu.getComponents();
            for (int i = 0; i < oComponents.length; i++){
                if (oComponents[i] instanceof JMenuItem){
                    JMenuItem jMenuItem = (JMenuItem) oComponents[i];
                    if (jMenuItem.getText().equals(_sMenuTitle)){
                        jMenuItem.setEnabled(_bdoEnable);
                    }
                }
            }
        }


        public void showPopUpMenu(Object _invoker, int x, int y) throws ClassCastException{
            if (_invoker instanceof Component){
                m_jPopupMenu.show((Component) _invoker, x, y);
            }
        }


    public void show(int _nPageIndex){
        Dimension aDimension = m_jInspectorDialog.getSize();
        selectInspectorPageByIndex(_nPageIndex);
        if (_nPageIndex > 0){
            m_jInspectorDialog.setSize(aDimension);
        }
        else{
            m_jInspectorDialog.pack();
        }
//            m_jInspectorDialog.paint(m_jInspectorDialog.getGraphics());
        m_jInspectorDialog.setVisible(true);
    }

        public void paint(){
            m_jTabbedPane1.paintImmediately(m_jTabbedPane1.getBounds());
        }


        public void addInspectorPage(String _sTitle, Object _oContainer) throws ClassCastException{
            if (_oContainer instanceof Component){
                m_jTabbedPane1.addTab(_sTitle, (Component) _oContainer);
            }
        }
}
