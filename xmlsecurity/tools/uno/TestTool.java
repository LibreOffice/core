/*************************************************************************
 *
 *  $RCSfile: TestTool.java,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: mt $ $Date: 2004-07-12 13:15:24 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package com.sun.star.xml.security.uno;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import java.io.File;
import java.io.IOException;
import java.io.FileOutputStream;
import java.io.FileInputStream;
import java.io.InputStream;
import java.io.ByteArrayInputStream;
import java.io.UnsupportedEncodingException;

import org.w3c.dom.Document;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

/* Basic GUI components */
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTree;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.JFileChooser;
import javax.swing.ToolTipManager;
import javax.swing.JTable;
import javax.swing.JLabel;
import javax.swing.BorderFactory;

/* GUI components for right-hand side */
import javax.swing.JSplitPane;
import javax.swing.JOptionPane;
import javax.swing.JTabbedPane;


/* GUI support classes */
import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.Container;
import java.awt.Toolkit;
import java.awt.event.WindowEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

/* For creating borders */
import javax.swing.border.EmptyBorder;
import javax.swing.border.BevelBorder;
import javax.swing.border.CompoundBorder;

/* For creating a TreeModel */
import javax.swing.tree.TreePath;
import java.util.Vector;

/* UNO classes */
import com.sun.star.uno.UnoRuntime;
import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.beans.XPropertySet;
import com.sun.star.uno.XComponentContext;
import com.sun.star.xml.sax.XDocumentHandler;

import com.sun.star.xml.crypto.*;
import com.sun.star.xml.crypto.sax.*;

public class TestTool extends JFrame implements ActionListener
{
    /*
     * xml security framewrok component names
     */
    public static String SIGNATURECREATOR_COMPONENT = "com.sun.star.xml.crypto.sax.SignatureCreator";
    public static String SIGNATUREVERIFIER_COMPONENT = "com.sun.star.xml.crypto.sax.SignatureVerifier";
    public static String ENCRYPTOR_COMPONENT = "com.sun.star.xml.crypto.sax.Encryptor";
    public static String DECRYPTOR_COMPONENT = "com.sun.star.xml.crypto.sax.Decryptor";
    public static String SAXEVENTKEEPER_COMPONENT = "com.sun.star.xml.crypto.sax.SAXEventKeeper";

    /*
     * Java-based component names
     */
    public static String SEINITIALIZER_COMPONENT_JAVA = "com.sun.star.xml.security.bridge.jxsec.SEInitializer_JxsecImpl";
    public static String XMLSIGNATURE_COMPONENT_JAVA = "com.sun.star.xml.security.bridge.jxsec.XMLSignature_JxsecImpl";
    public static String XMLENCRYPTION_COMPONENT_JAVA = "com.sun.star.xml.security.bridge.jxsec.XMLEncryption_JxsecImpl";
    public static String XMLDOCUMENTWRAPPER_COMPONENT_JAVA = "com.sun.star.xml.security.bridge.jxsec.XMLDocumentWrapper_JxsecImpl";

    /*
     * C-based component names
     */
    public static String SEINITIALIZER_COMPONENT_C = "com.sun.star.xml.security.bridge.xmlsec.SEInitializer_NssImpl";
    public static String XMLSIGNATURE_COMPONENT_C = "com.sun.star.xml.security.bridge.xmlsec.XMLSignature_NssImpl";
    public static String XMLENCRYPTION_COMPONENT_C = "com.sun.star.xml.security.bridge.xmlsec.XMLEncryption_NssImpl";
    public static String XMLDOCUMENTWRAPPER_COMPONENT_C = "com.sun.star.xml.security.bridge.xmlsec.XMLDocumentWrapper_XmlSecImpl";

    /* url resolver name */
    public static String UNOURLRESOLVER = "com.sun.star.bridge.UnoUrlResolver";

    /*
     * connection URL
     */
    private String m_unoURL = "uno:socket,host=localhost,port=2002;urp;StarOffice.ServiceManager";

    /* key file */
    private String m_javaTokenFile = null;
    private String m_nssTokenPath = null;

    /* User Interfaces */
    private JButton m_goButton;
    private JButton m_stepButton;
    private JButton m_startButton;
    private JButton m_openButton;
    private JCheckBox m_isExportingButton;
    private JCheckBox m_isJavaComponentButton;
    private JButton m_saveButton;
    private JButton m_batchButton;
    private JTree m_leftTree;
    private JTextArea m_leftTextArea;
    private JTree m_middleTree;
    private JTree m_rightTree;
    private JTabbedPane m_leftTabPane;
    private JTextArea m_bufferNodeTextArea;
    private JLabel m_saxChainLabel;
    private JTextField m_saxEventText;
    private JTable m_unsolvedReferenceTable;

    /*
     * whether a batch file is running,
     * if so, no message box is popped up
     */
    private boolean m_bIsBatchRunning = false;

    /*
     * whether the UI needs to be updated.
     * when user click the "go" button, the UI needs
     * not to be updated step by step for performance
     * reason
     */
    private boolean m_bIsUIUpdateSuppressed = false;

    /*
     * three DOM tree adapter
     */
    private DomToTreeModelAdapter m_leftTreeModelAdapter;
    private DomToTreeModelAdapter m_middleTreeModelAdapter;
    private DomToTreeModelAdapter m_rightTreeModelAdapter;

    /*
     * the current directory, which reserves the default
     * location when user open/save a file.
     */
    private File m_currentDirectory = null;

    /*
     * the log file
     */
    private FileOutputStream m_logFileOutputStream = null;

    /*
     * the thread which is parsing the current XML
     * file
     */
    private ParsingThread m_parsingThread;

    /*
     * whether is exporting or importing
     */
    private boolean m_bIsExporting;

    /*
     * whether java based component or c based component
     * is used now
     */
    private boolean m_bIsJavaBased;

    /*
     * XML security component interface
     */
    private XComponentContext m_xRemoteContext = null;
    private XMultiComponentFactory m_xRemoteServiceManager = null;
    private XXMLSecurityContext m_xXMLSecurityContext = null;
    private XXMLSignature m_xXMLSignature = null;
    private XXMLEncryption m_xXMLEncryption = null;
    private XSEInitializer m_xSEInitializer = null;

    /*
     * SAX event collector for the middle tree and the right tree
     */
    private SAXEventCollector m_rightTreeEventCollector = null;
    private SAXEventCollector m_middleTreeEventCollector = null;

    /*
     * security framework controller
     */
    private XMLSecurityFrameworkController m_xmlSecurityFrameworkController = null;

    /* org.w3c.dom.Document */
    private Document m_document;

    /* represents whether "Go" or "Step" */
    private boolean stepMode = true;

/**************************************************************************************
 * private methods
 **************************************************************************************/


    /******************************************************************************
     * UI related methods
     ******************************************************************************/

    /*
     * initalizes the UI.
     */
    private void initUI()
    {
        m_leftTreeModelAdapter = new DomToTreeModelAdapter(m_document);
        m_middleTreeModelAdapter = new DomToTreeModelAdapter(m_document);
        m_rightTreeModelAdapter = new DomToTreeModelAdapter(m_document);

        m_parsingThread = null;

        m_leftTree.setModel(m_leftTreeModelAdapter);
        m_middleTree.setModel(m_middleTreeModelAdapter);
        m_rightTree.setModel(m_rightTreeModelAdapter);
    }

    /*
     * constructs the user interface.
     */
    private Container buildUI(int width, int height)
    {
        JPanel mainPanel = new JPanel();

        int frameHeight = height-40;
        int leftWindowWidth = (width-40)/3;
        int middleWindowWidth = leftWindowWidth;
        int rightWindowWidth = leftWindowWidth;
        int leftPaneWidth = leftWindowWidth+middleWindowWidth;
        int frameWidth = leftPaneWidth + rightWindowWidth;

        /* Make a nice border */
        EmptyBorder emptyBorder = new EmptyBorder(5,5,5,5);
        BevelBorder bevelBorder = new BevelBorder(BevelBorder.LOWERED);
        CompoundBorder compoundBorder = new CompoundBorder(emptyBorder,bevelBorder);
        mainPanel.setBorder(new CompoundBorder(compoundBorder,emptyBorder));

        /* Set up the tree */
        m_leftTreeModelAdapter = new DomToTreeModelAdapter(m_document);
        m_middleTreeModelAdapter = new DomToTreeModelAdapter(m_document);
        m_rightTreeModelAdapter = new DomToTreeModelAdapter(m_document);

        m_leftTree = new JTree(m_leftTreeModelAdapter);
        m_leftTextArea = new JTextArea();
        m_middleTree = new JTree(m_middleTreeModelAdapter);
        m_rightTree = new JTree(m_rightTreeModelAdapter);

        ToolTipManager.sharedInstance().registerComponent(m_leftTree);
        ToolTipManager.sharedInstance().registerComponent(m_middleTree);
        ToolTipManager.sharedInstance().registerComponent(m_rightTree);

        /* Builds left tab pane */
        JScrollPane leftTreePane = new JScrollPane(m_leftTree);
        JScrollPane leftTextPane = new JScrollPane(m_leftTextArea);
        m_leftTabPane= new JTabbedPane();
        m_leftTabPane.add("Tree View",leftTreePane);
        m_leftTabPane.add("Text View",leftTextPane);

        /* Builds middle tree pane */
        JScrollPane middleTreePane = new JScrollPane(m_middleTree);

        /* Builds right tree pane */
        JScrollPane rightTreePane = new JScrollPane(m_rightTree);
        rightTreePane.setBorder(BorderFactory.createCompoundBorder(
                        BorderFactory.createTitledBorder("Result"),
                        BorderFactory.createEmptyBorder(8,8,8,8)));

        m_leftTabPane.setPreferredSize(
            new Dimension( leftWindowWidth, frameHeight ));
        middleTreePane.setPreferredSize(
            new Dimension( middleWindowWidth, frameHeight ));
        rightTreePane.setPreferredSize(
            new Dimension( rightWindowWidth, frameHeight ));

        /* Builds the SAX event text box */
        m_saxEventText = new JTextField();

        /* Builds the unsolved reference table */
        m_unsolvedReferenceTable = new JTable(
                new UnsolvedReferenceTableModel(this));

        /* Builds the BufferNode information text area */
        m_bufferNodeTextArea = new JTextArea();

        /* Builds the SAX chain information label */
        m_saxChainLabel = new JLabel();

        /* Builds the left pane */
        JPanel tabPaneWithSaxEventPane = new JPanel();
        tabPaneWithSaxEventPane.setLayout(new BorderLayout());
        tabPaneWithSaxEventPane.add("Center",m_leftTabPane);
        tabPaneWithSaxEventPane.add("South",new JScrollPane(m_saxEventText));

        JSplitPane leftPane =
            new JSplitPane( JSplitPane.VERTICAL_SPLIT,
                tabPaneWithSaxEventPane,
                new JScrollPane(m_unsolvedReferenceTable));
        leftPane.setBorder(BorderFactory.createCompoundBorder(
                        BorderFactory.createTitledBorder("Original"),
                        BorderFactory.createEmptyBorder(8,8,8,8)));

        leftPane.setContinuousLayout( true );
        leftPane.setDividerLocation( frameHeight*2/3 );
        leftPane.setPreferredSize(
            new Dimension( leftWindowWidth, frameHeight ));

        /* Builds the middle pane */
        JPanel bufferNodeWithSaxChainPane = new JPanel();
        bufferNodeWithSaxChainPane.setLayout(new BorderLayout());
        bufferNodeWithSaxChainPane.add("Center",m_bufferNodeTextArea);
        bufferNodeWithSaxChainPane.add("South",new JScrollPane(m_saxChainLabel));

        JSplitPane middlePane =
            new JSplitPane( JSplitPane.VERTICAL_SPLIT,
                middleTreePane,
                new JScrollPane(bufferNodeWithSaxChainPane));

        middlePane.setBorder(BorderFactory.createCompoundBorder(
                        BorderFactory.createTitledBorder("Insight SAXEventKeeper"),
                        BorderFactory.createEmptyBorder(8,8,8,8)));

        middlePane.setContinuousLayout( true );
        middlePane.setDividerLocation( frameHeight/2+5 );
        middlePane.setPreferredSize(
            new Dimension( middleWindowWidth, frameHeight ));

        /* Builds the whole frame pane */
        JSplitPane leftWithMiddlePane =
            new JSplitPane( JSplitPane.HORIZONTAL_SPLIT,
                leftPane,
                middlePane );
        leftWithMiddlePane.setContinuousLayout( true );
        leftWithMiddlePane.setDividerLocation( leftWindowWidth );
        leftWithMiddlePane.setPreferredSize(
            new Dimension( leftPaneWidth + 10, frameHeight+10 ));

        JSplitPane framePane =
            new JSplitPane( JSplitPane.HORIZONTAL_SPLIT,
                leftWithMiddlePane,
                rightTreePane );


        framePane.setContinuousLayout( true );
        framePane.setDividerLocation(leftPaneWidth+10 );
        framePane.setPreferredSize(
            new Dimension( frameWidth + 20, frameHeight+10 ));

        /* Adds all GUI components to the main panel */
        mainPanel.setLayout(new BorderLayout());
        mainPanel.add("Center", framePane );

        m_openButton = new JButton("Open...");
        m_openButton.addActionListener(this);

        m_goButton = new JButton("Go!");
        m_goButton.addActionListener(this);

        m_stepButton = new JButton("Step");
        m_stepButton.addActionListener(this);

        m_startButton = new JButton("Start");
        m_startButton.addActionListener(this);
        m_startButton.setEnabled(false);

        m_isExportingButton = new JCheckBox("export, not import", true);
        m_isJavaComponentButton = new JCheckBox("java, not C++", false);

        m_saveButton = new JButton("Save...");
        m_saveButton.addActionListener(this);

        m_batchButton = new JButton("Batch...");
        m_batchButton.addActionListener(this);

        JPanel buttonPanel = new JPanel();
        buttonPanel.add(m_batchButton);
        buttonPanel.add(m_openButton);
        buttonPanel.add(m_startButton);
        buttonPanel.add(m_goButton);
        buttonPanel.add(m_stepButton);
        buttonPanel.add(m_isExportingButton);
        buttonPanel.add(m_isJavaComponentButton);
        buttonPanel.add(m_saveButton);

        mainPanel.add("South", buttonPanel);

        enableGoButton(false);

        return mainPanel;
    }

    /*
     * enables/disables the Go(and Step) button.
     */
    private void enableGoButton(boolean enabled)
    {
        m_goButton.setEnabled(enabled);
        m_stepButton.setEnabled(enabled);
    }

    /*
     * updates the unsolved reference information.
     */
    private void updatesUnsolvedReferencesInformation()
    {
        m_unsolvedReferenceTable.setModel(new UnsolvedReferenceTableModel(this));
    }

    /*
     * adjusts the view of the tree in order to make the
     * particular Node into the focus tree leaf.
     */
    private void updatesTree(Node node, JTree tree)
    {
        int i=0;
        int currentLine = 0;

        while (i<tree.getRowCount())
        {
            TreePath treePath = tree.getPathForRow(i);
            tree.expandPath(treePath);

            AdapterNode adapterNode = (AdapterNode)treePath.getLastPathComponent();

            if (node == adapterNode.getNode())
            {
                tree.addSelectionPath(treePath);
                currentLine = i;
            }

            ++i;
        }

        tree.setCellRenderer(new XMLTreeCellRanderer(node));
        tree.scrollRowToVisible(currentLine);
    }

    /******************************************************************************
     * action listener related methods.
     ******************************************************************************/

    /*
     * reads in a document, either the document is a file or
     * is a text paragraph.
     */
    private void openDocument()
    {
        if (m_leftTabPane.getSelectedIndex() == 0)
        {
            File f = openFile();
            if (f != null)
            {
                parseFile(f);
            }
        }
        else
        {
            String text = m_leftTextArea.getText();

            try
            {
                parseStream(new ByteArrayInputStream(text.getBytes("UTF-8")));
            }
            catch(UnsupportedEncodingException e)
            {
                e.printStackTrace();
            }

            m_leftTabPane.setSelectedIndex(0);
        }
    }

    /*
     * save the result tree to a file.
     */
    private void saveResult()
    {
        saveFile();
    }

    /*
     * selects a batch file to excute.
     */
    private void openBatch()
    {
        File f = openFile();
        if (f != null)
        {
            runBatch(f);
        }
    }

    /*
     * makes the current operation to an end.
     */
    private void endMission()
    {
        enableGoButton(false);
        m_parsingThread = null;

        if (m_xmlSecurityFrameworkController != null)
        {
            m_xmlSecurityFrameworkController.endMission();
        }

        updatesUIs();

        m_xmlSecurityFrameworkController = null;
        freeComponents();

        System.gc();
    }


    /******************************************************************************
     * UNO component related methods
     ******************************************************************************/

    /*
     * connects the SO server.
     */
    private void connectSO(String unoUrlString)
    {
        if (unoUrlString != null)
        {
            m_unoURL = new String(unoUrlString);
        }

        try
        {
            m_xRemoteServiceManager = getRemoteServiceManager(m_unoURL);
        }
        catch(Exception e)
        {
            e.printStackTrace();
        }
    }

    /*
     * creates UNO components.
     */
    private boolean createComponents()
    {
        try
        {
            String SEInitializer_comp;
            String XMLSignature_comp;
            String XMLEncryption_comp;
            String tokenPath;

            if (m_bIsJavaBased)
            {
                SEInitializer_comp = SEINITIALIZER_COMPONENT_JAVA;
                XMLSignature_comp = XMLSIGNATURE_COMPONENT_JAVA;
                XMLEncryption_comp = XMLENCRYPTION_COMPONENT_JAVA;
                tokenPath = m_javaTokenFile;
            }
            else
            {
                SEInitializer_comp = SEINITIALIZER_COMPONENT_C;
                XMLSignature_comp = XMLSIGNATURE_COMPONENT_C;
                XMLEncryption_comp = XMLENCRYPTION_COMPONENT_C;
                tokenPath = m_nssTokenPath;
            }

            Object seInitializerObj = m_xRemoteServiceManager.createInstanceWithContext(
                SEInitializer_comp, m_xRemoteContext);

            if (seInitializerObj == null)
            {
                freeComponents();
                return false;
            }

            m_xSEInitializer = (XSEInitializer)UnoRuntime.queryInterface(
                        XSEInitializer.class, seInitializerObj);

            m_xXMLSecurityContext = m_xSEInitializer.createSecurityContext(tokenPath);

            Object xmlSignatureObj = m_xRemoteServiceManager.createInstanceWithContext(
                XMLSignature_comp, m_xRemoteContext);

            if (xmlSignatureObj == null)
            {
                freeComponents();
                return false;
            }

            m_xXMLSignature = (XXMLSignature)UnoRuntime.queryInterface(
                        XXMLSignature.class, xmlSignatureObj);

            Object xmlEncryptionObj = m_xRemoteServiceManager.createInstanceWithContext(
                XMLEncryption_comp, m_xRemoteContext);

            if (xmlEncryptionObj == null)
            {
                freeComponents();
                return false;
            }

            m_xXMLEncryption = (XXMLEncryption)UnoRuntime.queryInterface(
                        XXMLEncryption.class, xmlEncryptionObj);

            return true;
        }
        catch(Exception e)
        {
            freeComponents();
            e.printStackTrace();
            return false;
        }
    }

        /*
         * frees UNO components.
         */
    private void freeComponents()
    {
        try
        {
            if (m_xXMLSecurityContext != null)
            {
                m_xSEInitializer.freeSecurityContext(m_xXMLSecurityContext);
                m_xXMLSecurityContext = null;
            }

            m_xXMLSignature = null;
            m_xXMLEncryption = null;
            m_xSEInitializer = null;
        }
        catch(Exception e)
        {
            e.printStackTrace();
        }
    }

    /*
     * getRemoteServiceManager
     */
    private XMultiComponentFactory getRemoteServiceManager(String unoUrl) throws java.lang.Exception
    {
        if (m_xRemoteContext == null)
        {
            /*
             * First step: create local component context, get local servicemanager and
             * ask it to create a UnoUrlResolver object with an XUnoUrlResolver interface
             */
            XComponentContext xLocalContext =
                com.sun.star.comp.helper.Bootstrap.createInitialComponentContext(null);
            XMultiComponentFactory xLocalServiceManager = xLocalContext.getServiceManager();
            Object urlResolver = xLocalServiceManager.createInstanceWithContext(
                UNOURLRESOLVER, xLocalContext );
            /*
             * query XUnoUrlResolver interface from urlResolver object
             */
            XUnoUrlResolver xUnoUrlResolver = (XUnoUrlResolver) UnoRuntime.queryInterface(
                XUnoUrlResolver.class, urlResolver );

            /*
             * Second step: use xUrlResolver interface to import the remote StarOffice.ServiceManager,
             * retrieve its property DefaultContext and get the remote servicemanager
             */
            Object initialObject = xUnoUrlResolver.resolve( unoUrl );
            XPropertySet xPropertySet = (XPropertySet)UnoRuntime.queryInterface(
            XPropertySet.class, initialObject);
            Object context = xPropertySet.getPropertyValue("DefaultContext");
            m_xRemoteContext = (XComponentContext)UnoRuntime.queryInterface(
                XComponentContext.class, context);
        }
        return m_xRemoteContext.getServiceManager();
    }


    /******************************************************************************
     * XML related methods
     ******************************************************************************/

    /*
     * removes all empty text node inside the particular element
     */
    private void removeEmptyText(Node node)
    {
        int type = node.getNodeType();
        NodeList children;
        int i;

        switch (type)
        {
        case Node.DOCUMENT_NODE:
        case Node.ELEMENT_NODE:
            Node child = node.getFirstChild();
            while (child!= null)
            {
                Node nextSibling = child.getNextSibling();
                int childType = child.getNodeType();

                if (childType==Node.TEXT_NODE)
                {
                    String message = child.getNodeValue().trim();
                    if (message == null || message.length()<=0)
                    {
                        node.removeChild(child);
                    }
                }
                else if (childType == Node.ELEMENT_NODE)
                {
                    removeEmptyText(child);
                }

                child = nextSibling;
            }
            break;
        }
    }

    /*
     * reads a stream, and parses it into the original tree.
     */
    private void parseStream(InputStream is)
    {
        try
        {
            DocumentBuilderFactory factory =
                DocumentBuilderFactory.newInstance();
            m_document = null;
            m_startButton.setEnabled(false);
            initUI();

            /* factory.setValidating(true); */
            /* factory.setNamespaceAware(true); */

            try
            {
                DocumentBuilder builder = factory.newDocumentBuilder();
                m_document = builder.parse(is);
                m_startButton.setEnabled(true);
                initUI();
            }
            catch (ParserConfigurationException pce)
            {
                pce.printStackTrace();
            }
            catch (IOException ioe)
            {
                ioe.printStackTrace();
            }
        }
        catch(Exception exce)
        {
            System.out.println("input stream Exception");
        }
    }


    /******************************************************************************
     * file operation related methods
     ******************************************************************************/

    /*
     * opens a file, and parses it into the original tree.
     */
    private void parseFile(File name)
    {
        try
        {
            FileInputStream fis = new FileInputStream(name);
            parseStream(fis);
            fis.close();
        }
        catch(Exception exce)
        {
            System.out.println("open file Exception");
        }
    }


    /*
     * selects a file to open
     */
    private File openFile()
    {
        File rc = null;

        JFileChooser fileChooser= new JFileChooser();

        fileChooser.setDialogTitle("Select File To Open");
        fileChooser.setDialogType(JFileChooser.OPEN_DIALOG);

        fileChooser.setApproveButtonText("Ok");

        if (m_currentDirectory == null)
        {
            fileChooser.rescanCurrentDirectory();
        }
        else
        {
            fileChooser.setCurrentDirectory(m_currentDirectory);
        }

        fileChooser.setFileFilter(new XMLFileFilter());

        int result = fileChooser.showDialog(this,null);
        if (result==fileChooser.APPROVE_OPTION)
        {
            m_currentDirectory = fileChooser.getCurrentDirectory();
            rc = fileChooser.getSelectedFile();
        }

        return rc;
    }

    private void saveFile()
    {
        JFileChooser fileChooser= new JFileChooser();

        fileChooser.setDialogTitle("Select File To Save");
        fileChooser.setDialogType(JFileChooser.SAVE_DIALOG);

        fileChooser.setApproveButtonText("Ok");

        if (m_currentDirectory == null)
        {
            fileChooser.rescanCurrentDirectory();
        }
        else
        {
            fileChooser.setCurrentDirectory(m_currentDirectory);
        }

        fileChooser.setFileFilter(new XMLFileFilter());

        int result = fileChooser.showDialog(this,null);
        if (result==fileChooser.APPROVE_OPTION)
        {
            try
            {
                m_currentDirectory = fileChooser.getCurrentDirectory();
                saveFile(fileChooser.getSelectedFile());
            }
            catch(Exception exce)
            {
                System.out.println("save file Exception");
                exce.printStackTrace();
            }
        }
    }

    /*
     * excutes a batch file.
     */
    private void runBatch(File f)
    {
        FileInputStream fis = null;

        try
        {
            fis = new FileInputStream(f);
            StringBuffer commandBuffer = new StringBuffer();

            m_logFileOutputStream = new FileOutputStream("TestTool-log.txt");
            m_bIsBatchRunning = true;
            int ch = 0;

            while (ch != -1)
            {
                ch = fis.read();

                if (ch != 0x0a && ch != -1)
                {
                    if (ch != 0x0d)
                    {
                        commandBuffer.append((char)ch);
                    }
                }
                else
                {
                    String command = new String(commandBuffer);
                    if (command.startsWith("Open "))
                    {
                        m_logFileOutputStream.write(("start \""+command+"\" ...\n").getBytes());
                        String fileName = command.substring(5);
                        parseFile(new File(fileName));
                        m_logFileOutputStream.write("command end \n\n".getBytes());
                    }
                    else if (command.startsWith("Use Java Component"))
                    {
                        m_logFileOutputStream.write(("start \""+command+"\" ...\n").getBytes());
                        m_isJavaComponentButton.setSelected(true);
                        m_logFileOutputStream.write("command end \n\n".getBytes());
                    }
                    else if (command.startsWith("Use C++ Component"))
                    {
                        m_logFileOutputStream.write(("start \""+command+"\" ...\n").getBytes());
                        m_isJavaComponentButton.setSelected(false);
                        m_logFileOutputStream.write("command end \n\n".getBytes());
                    }
                    else if (command.startsWith("Go "))
                    {
                        m_logFileOutputStream.write(("start \""+command+"\" ...\n").getBytes());
                        String opera = command.substring(3);
                        if (opera.equals("Sign") || opera.equals("Encrypt"))
                        {
                            m_isExportingButton.setSelected(true);
                        }
                        else
                        {
                            m_isExportingButton.setSelected(false);
                        }

                        startsUp();
                        if (m_parsingThread != null)
                        {
                            m_bIsUIUpdateSuppressed = true;
                            try{
                                while (m_parsingThread.nextStep());
                                endMission();
                            }
                            catch(Exception e)
                            {
                                System.out.println("exception happen during batch:"+e);
                                e.printStackTrace();
                            }

                            m_bIsUIUpdateSuppressed = false;
                            updatesUIs();
                        }
                        m_logFileOutputStream.write("command end \n\n".getBytes());
                    }
                    else if (command.startsWith("Save "))
                    {
                        m_logFileOutputStream.write(("start \""+command+"\" ...\n").getBytes());
                        String fileName = command.substring(5);
                        saveFile(new File(fileName));
                        m_logFileOutputStream.write("command end \n\n".getBytes());
                    }

                    commandBuffer = new StringBuffer();
                }
            }

            m_bIsBatchRunning = false;
            m_logFileOutputStream.close();
            m_logFileOutputStream = null;

            fis.close();
            fis = null;
        }
        catch(java.io.IOException e)
        {
            e.printStackTrace();
        }
    }

    /*
     * save the current result tree to a particular file.
     */
    private void saveFile(File f)
    {
        try{
            FileOutputStream fos = new FileOutputStream(f);
            SAXEventPrinter.display((Document)m_rightTreeEventCollector.getDocument(),
                0,fos,false);
            fos.close();
        }catch(Exception e)
        {
            e.printStackTrace();
        }
    }

    /******************************************************************************
     * others
     ******************************************************************************/

    /*
     * starts up the operation.
     */
    private void startsUp()
    {
        if (m_parsingThread != null)
        {
            m_parsingThread = null;
        }

        m_bIsExporting = m_isExportingButton.isSelected();
        m_bIsJavaBased = m_isJavaComponentButton.isSelected();

        if (createComponents())
        {
            m_rightTreeEventCollector = new SAXEventCollector(this);

            m_parsingThread = new ParsingThread(
                m_document,
                null,
                this);

            m_xmlSecurityFrameworkController =
                new XMLSecurityFrameworkController(
                    this,
                    m_bIsExporting,
                    m_bIsJavaBased,
                    m_rightTreeEventCollector,
                    m_parsingThread,
                    m_xXMLSecurityContext,
                    m_xXMLSignature,
                    m_xXMLEncryption,
                    m_xRemoteServiceManager,
                    m_xRemoteContext);

            enableGoButton(true);
        }
        else
        {
            showMessage("Error in creating XML Security Components!");
        }
    }

/**************************************************************************************
 * protected methods
 **************************************************************************************/

    /******************************************************************************
     * UI related methods
     ******************************************************************************/

    /*
     * updates the sax chain information.
     */
    protected void updatesSAXChainInformation(String chainStr)
    {
        m_saxChainLabel.setText(chainStr);
    }

    /*
     * update the current SAX event information.
     */
    protected void updatesCurrentSAXEventInformation(String event)
    {
        m_saxEventText.setText(event);
    }

    /*
     * updates all information in the UI.
     */
    protected void updatesUIs()
    {
        if (!m_bIsUIUpdateSuppressed)
        {
            m_leftTree.clearSelection();
            updatesTree(null, m_leftTree);

            if (m_xmlSecurityFrameworkController != null)
            {
                String bufferNodeTreeText = m_xmlSecurityFrameworkController.getBufferNodeTreeInformation();
                if (bufferNodeTreeText == null)
                {
                    m_middleTree.setVisible(false);
                    m_bufferNodeTextArea.setText("No XML Security Related");
                }
                else
                {
                    m_middleTreeEventCollector = new SAXEventCollector(null);
                    m_xmlSecurityFrameworkController.getDocument(m_middleTreeEventCollector);

                    m_middleTreeModelAdapter = new DomToTreeModelAdapter(m_middleTreeEventCollector.getDocument());
                    m_middleTree.setModel(m_middleTreeModelAdapter);
                    updatesTree(null, m_middleTree);
                    m_middleTree.setVisible(true);
                    m_bufferNodeTextArea.setText(bufferNodeTreeText);
                }
            }
            else
            {
                m_middleTree.setVisible(false);
                m_bufferNodeTextArea.setText("No XMLImporter/XMLExporter");
            }

            if (m_rightTreeEventCollector != null)
            {
                m_rightTreeModelAdapter = new DomToTreeModelAdapter((Document)m_rightTreeEventCollector.getDocument());
                m_rightTree.setModel(m_rightTreeModelAdapter);
                updatesTree((Node)m_rightTreeEventCollector.getCurrentElement(), m_rightTree);
            }

            updatesUnsolvedReferencesInformation();
        }
    }

    /*
     * shows a message.
     */
    protected void showMessage(String msg)
    {
        if (m_bIsBatchRunning)
        {
            try
            {
                if (!msg.startsWith("Message from : SAXEventKeeper"))
                {
                    byte [] b = msg.getBytes();
                    m_logFileOutputStream.write("        ".getBytes());

                    for (int i=0; i<b.length; ++i)
                    {
                        m_logFileOutputStream.write(b[i]);
                        if (b[i] == '\n')
                        {
                            m_logFileOutputStream.write("        ".getBytes());
                        }
                    }
                    m_logFileOutputStream.write("\n        ==============================\n".getBytes());
                }
            }
            catch(IOException e)
            {
                e.printStackTrace();
            }
        }
        else
        {
            if (stepMode)
            {
                JOptionPane optionPane = new JOptionPane();
                optionPane.showMessageDialog(this, msg, "TestTool Notification", JOptionPane.INFORMATION_MESSAGE);
            }
            else
            {
                Object[] options = { "OK", "Go back to step mode" };
                if (1 == JOptionPane.showOptionDialog(this, msg, "TestTool Notification",
                    JOptionPane.DEFAULT_OPTION, JOptionPane.PLAIN_MESSAGE,
                    null, options, options[0]))
                {
                    stepMode = true;
                }
            }
        }
    }


    /******************************************************************************
     * information retrieving
     ******************************************************************************/

    /*
     * gets all unsolved reference ids.
     * a reference id is the value of the id attribute of an
     * referenced element.
     */
    protected Vector getUnsolvedReferenceIds()
    {
        Vector rc;

        if (m_xmlSecurityFrameworkController == null)
        {
            rc = new Vector();
        }
        else
        {
            rc = ((XMLSecurityFrameworkController)m_xmlSecurityFrameworkController).
                getUnsolvedReferenceIds();
        }

        return rc;
    }

    /*
     * gets all unsolved reference keeper ids.
     * a reference keeper id is the id which the SAXEventKeeper uses
     * to identify the corresponding BufferNode.
     */
    protected Vector getUnsolvedReferenceKeeperIds()
    {
        Vector rc;

        if (m_xmlSecurityFrameworkController == null)
        {
            rc = new Vector();
        }
        else
        {
            rc = ((XMLSecurityFrameworkController)m_xmlSecurityFrameworkController).
                getUnsolvedReferenceKeeperIds();
        }

        return rc;
    }

    /*
     * gets all unsolved references' remaining numbers.
     * a remaining number is that how many claims have not been found for
     * a unsolved reference.
     */
    protected Vector getUnsolvedReferenceRefNum()
    {
        Vector rc;

        if (m_xmlSecurityFrameworkController == null)
        {
            rc = new Vector();
        }
        else
        {
            rc = ((XMLSecurityFrameworkController)m_xmlSecurityFrameworkController).
                getUnsolvedReferenceRefNum();
        }

        return rc;
    }


/**************************************************************************************
 * public methods
 **************************************************************************************/

    /******************************************************************************
     * action listener related methods.
     ******************************************************************************/

    /*
     * action listening method.
     */
    public void actionPerformed(ActionEvent e)
    {
        if (e.getSource().equals(m_startButton))
        {
            endMission();
            startsUp();
        }
        if (e.getSource().equals(m_goButton))
        {
            if (m_parsingThread != null)
            {
                stepMode = false;
                boolean notOver;
                while ( notOver = m_parsingThread.nextStep())
                {
                    if (stepMode) break;
                }

                if (!notOver) endMission();
            }
        }
        if (e.getSource().equals(m_stepButton))
        {
            if (m_parsingThread != null)
            {
                if (!m_parsingThread.nextStep())
                {
                    endMission();
                }
            }
        }
        if (e.getSource().equals(m_openButton))
        {
            openDocument();
        }
        if (e.getSource().equals(m_saveButton))
        {
            saveResult();
        }
        if (e.getSource().equals(m_batchButton))
        {
            openBatch();
        }
    }

    /*
     * void-consturctor method
     */
    public TestTool()
    {
            getRootPane().putClientProperty("defeatSystemEventQueueCheck", Boolean.TRUE);

            try
            {
                m_currentDirectory = new File(System.getProperty("user.dir"));
        }
        catch(Exception e)
        {
            System.out.println("getProperty error :"+e);
        }
    }

    /*
     * consturctor method with a specific connection URL
     */
    public TestTool(String connecturl)
    {
        this();
        m_unoURL = new String(connecturl);
    }

    public static void main(String argv[])
    {
        Dimension screenSize =
            Toolkit.getDefaultToolkit().getScreenSize();

        TestTool tt;

        if (argv.length < 1)
        {
            System.out.println("Usage: java TestTool [javaTokenFile] [nssTokenPath] [xml file]?");
            return;
        }

        boolean hasFile = false;
        boolean hasBatch = false;
        String fileName = null;

        if (argv.length >= 3)
        {
            if (argv[2].startsWith("-b"))
            {
                fileName = argv[2].substring(2);
                hasBatch = true;
            }
            else
            {
                fileName = argv[2];
                hasFile = true;
            }
        }

        tt = new TestTool();
        tt.m_javaTokenFile = new String(argv[0]);
        tt.m_nssTokenPath = new String(argv[1]);
        tt.connectSO(null);

        /* Set up a GUI framework */
        JFrame myFrame = new JFrame("XML Security Components Tester");
        myFrame.addWindowListener(
            new WindowAdapter() {
                public void windowClosing(WindowEvent e) {System.exit(0);}
            }
            );

        myFrame.setContentPane(tt.buildUI(screenSize.width, screenSize.height));
        myFrame.pack();
        int w = screenSize.width-30;
        int h = screenSize.height-30;
        myFrame.setLocation(screenSize.width/2 - w/2,
        screenSize.height/2 - h/2);
        myFrame.setSize(w, h);
        myFrame.setVisible(true);

        if (hasFile)
        {
            tt.parseFile(new File(fileName));
        }
        else if (hasBatch)
        {
            tt.runBatch(new File(fileName));
        }
    }
}

