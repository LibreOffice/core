/*************************************************************************
 *
 *  $RCSfile: Inspector.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-01-31 17:10:18 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright (c) 2003 by Sun Microsystems, Inc.
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

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lib.uno.helper.Factory;
import com.sun.star.lang.XSingleComponentFactory;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.lib.uno.helper.WeakBase;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.Type;
import com.sun.star.uno.TypeClass;
import com.sun.star.uno.XComponentContext;
import com.sun.star.beans.XIntrospection;
import com.sun.star.beans.XIntrospectionAccess;
import com.sun.star.beans.Property;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XPropertySetInfo;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.container.XEnumeration;
import com.sun.star.container.XEnumerationAccess;
import com.sun.star.container.XIndexAccess;
import com.sun.star.reflection.XIdlMethod;
import com.sun.star.reflection.ParamInfo;
import com.sun.star.reflection.XIdlClass;
import com.sun.star.reflection.ParamMode;

import org.openoffice.XInstanceInspector;

import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.JTree;
import javax.swing.tree.TreeSelectionModel;
import javax.swing.tree.DefaultTreeSelectionModel;
import javax.swing.JScrollPane;
import javax.swing.JTextField;
import javax.swing.JFrame;
import javax.swing.tree.TreePath;
import javax.swing.event.TreeWillExpandListener;
import javax.swing.tree.DefaultTreeModel;
import javax.swing.event.TreeSelectionEvent;
import javax.swing.event.TreeSelectionListener;
import javax.swing.JButton;
import java.awt.BorderLayout;
import java.awt.Container;
import java.awt.event.KeyEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;

import java.util.Hashtable;
import java.util.Vector;

/** The purpose of this class is to provide a factory for creating the service
 * (<CODE>__getServiceFactory</CODE>) and writes the information into the given
 * registry key (<CODE>__writeRegistryServiceInfo</CODE>).
 */
public class Inspector {
    /** This class implements the method of the interface XInstanceInspector.
     * Also the class implements the interfaces XServiceInfo, and XTypeProvider.
     */
    static public class _Inspector extends WeakBase implements
           XInstanceInspector, XServiceInfo
    {
        static private final String __serviceName =
        "org.openoffice.InstanceInspector";

        private XMultiComponentFactory m_multiCmpFactory;
        private XComponentContext m_cmpCtx;

        static private XIntrospection m_introspection;

        private Hashtable hashtableObjects;
        private DefaultTreeModel treeModel;
        private JTree tree;

        /** The constructor of the inner class has a XMultiServiceFactory parameter.
         * @param xMultiServiceFactory XMultiServiceFactory
         */
        public _Inspector( XComponentContext cmpContext ) {
            try {
                m_cmpCtx = cmpContext;
                m_multiCmpFactory = m_cmpCtx.getServiceManager();
            }
            catch( Exception e ) {
                e.printStackTrace();
            }
        }

        /** Inspect the given object for methods, properties, interfaces, and
         * services.
         * @param a The object to inspect
         * @throws RuntimeException If
         */
        public void inspect(java.lang.Object a)
            throws com.sun.star.uno.RuntimeException {
            try {
            final JTextField jTextFieldInformation =
                new javax.swing.JTextField();

            //  get the frame where to display the object tree
            JFrame jframe = new JFrame( "Instance Inspector" );

            //  add a WindowListener for closing
            jframe.addWindowListener(
                new WindowAdapter() {
                    public void windowClosing(WindowEvent event) {
                        event.getWindow().setVisible(false);
                        event.getWindow().dispose();
                    }
                }
                );

            //  Get the interface XIntrospection from the MultiServiceFactory
            Object o = m_multiCmpFactory.createInstanceWithContext(
                "com.sun.star.beans.Introspection", m_cmpCtx );
            m_introspection = ( XIntrospection ) UnoRuntime.queryInterface(
                XIntrospection.class, o );

            //  Get the interface XServiceInfo because of the implementation
            //  name
            XServiceInfo xServiceInfoRoot = ( XServiceInfo )
                UnoRuntime.queryInterface(XServiceInfo.class, a);

            //  Get the implementation name and the implemented class
            String sTreeNodeName =
                xServiceInfoRoot.getImplementationName() + " = " + a.toString();

            //  set the root node of the object tree
            DefaultMutableTreeNode root;
            root = new DefaultMutableTreeNode( sTreeNodeName );

            //  Create a simple TreeModel
            treeModel = new DefaultTreeModel( root );

            //  Create a data structure to store the name of the node and the
            //  connected object
            hashtableObjects = new Hashtable();
            hashtableObjects.put( "[" + sTreeNodeName + "]", a );

            //  activate simple selection mechanism
            TreeSelectionModel tsm = new DefaultTreeSelectionModel();
            tsm.setSelectionMode(TreeSelectionModel.SINGLE_TREE_SELECTION);

            //  Create a tree to show the object dependencies
            tree = new JTree( treeModel );
            tree.setSelectionModel(tsm);
            tree.setRootVisible(true);

            //  inspect the root object
            this.inspectChild(a, root, m_introspection, tree.getPathForRow(0));

            // Add JTree to JFrame
            Container cp = jframe.getContentPane();
            cp.add( new JScrollPane( tree ), BorderLayout.NORTH );
            cp.add( jTextFieldInformation, BorderLayout.CENTER );

            //  Add TreeSelectionListener
            tree.addTreeSelectionListener(
                new TreeSelectionListener() {
                    public void valueChanged(TreeSelectionEvent event) {
                        TreePath tp = event.getNewLeadSelectionPath();
                        if (tp != null) {
                            jTextFieldInformation.setText( "Selected: " +
                                                           tp.toString() );

                        } else {
                            jTextFieldInformation.setText( "" );
                        }
                    }
                }

                );

            //  Add KeyListener for help
            tree.addKeyListener(
                new java.awt.event.KeyAdapter() {
                    public void keyReleased(java.awt.event.KeyEvent event) {
                        try {
                        if ( event.getKeyCode() == KeyEvent.VK_F1 ) {
                            //  function key F1 pressed

                            XInterface oInterface;
                            XDesktop oDesktop;
                            XComponentLoader oCLoader;
                            XComponent aDoc = null;
                            String sParentNode = "";
                            String sURLDocument = "";

                            //  Path to the selected node
                            TreePath treePath = tree.getSelectionPath();

                            //  get the parent node
                            if ( treePath.getParentPath() != null ) {
                                sParentNode = treePath.getParentPath().
                                    getLastPathComponent().toString();
                            }

                            //  get the stored object
                            Object object = hashtableObjects.get( treePath.
                                                                  toString() );

                            //  set the context dependend part of the URL path
                            //  for the documentation
                            if ( treePath.getPathCount() % 2 <= 0 ) {
                                //  Category is selected
                                sParentNode = treePath.getParentPath().
                                    getLastPathComponent().toString();
                                int intBegin = sParentNode.indexOf(
                                    " = (" ) + 4;
                                int intEnd = sParentNode.indexOf( ")" );
                                sURLDocument = sParentNode.
                                    substring( intBegin, intEnd ).
                                    replace( '.', '/' ) + ".html";
                            }
                            else if ( object != null ) {
                                //  Complex property selected
                                String sNode =
                                    treePath.getLastPathComponent().toString();
                                int intBegin = sNode.indexOf( " = (" ) + 4;
                                int intEnd = sNode.indexOf( ")" );
                                sURLDocument =
                                    sNode.substring( intBegin, intEnd ).
                                    replace( '.', '/' ) + ".html";
                            }
                            else if ( sParentNode.equals( "Methods" ) ) {
                                //  Method selected
                                String sNode =
                                    treePath.getLastPathComponent().toString();
                                int intBegin = sNode.indexOf( " " ) + 1;
                                int intEnd = sNode.indexOf( " (" );
                                String sMethod =
                                    sNode.substring( intBegin, intEnd );

                                Object objectGrandParent =
                                    hashtableObjects.get(treePath.getParentPath().
                                                         getParentPath().toString());
                                XIntrospectionAccess xIntrospectionAccess =
                                    m_introspection.inspect(objectGrandParent);
                                XIdlMethod xIdlMethod = xIntrospectionAccess.
                                    getMethod(sMethod,
                                              com.sun.star.beans.MethodConcept.ALL );
                                String sDeclaringClass = xIdlMethod.
                                    getDeclaringClass().getName().replace('.', '/');

                                sURLDocument = sDeclaringClass +
                                        ".html#" + sMethod;
                            }
                            else if ( ( sParentNode.equals( "Services" )
                                        || ( sParentNode.equals( "Interfaces" ) ) ) ) {
                                //  Service or interface selected
                                sURLDocument = treePath.getLastPathComponent().
                                    toString().replace( '.', '/' ) + ".html";
                            }
                            else if (((DefaultMutableTreeNode) treePath.
                                      getLastPathComponent() ).getChildCount() == 0)
                            {
                                //  Simple property selected
                                String sGrandParentNode = treePath.
                                    getParentPath().getParentPath().
                                    getLastPathComponent().toString();
                                int intBegin =
                                    sGrandParentNode.indexOf( " = (" ) + 4;
                                int intEnd = sGrandParentNode.indexOf( ")" );
                                sURLDocument =
                                    sGrandParentNode.substring( intBegin, intEnd ).
                                    replace( '.', '/' ) + ".html";
                            }

                            //  get the component loader
                            oInterface = (XInterface)
                                m_multiCmpFactory.createInstanceWithContext(
                                    "com.sun.star.frame.Desktop", m_cmpCtx );
                            oDesktop = ( XDesktop ) UnoRuntime.queryInterface(
                                XDesktop.class, oInterface );
                            oCLoader = ( XComponentLoader )
                                UnoRuntime.queryInterface( XComponentLoader.class,
                                                           oDesktop );
                            PropertyValue [] szEmptyArgs = new PropertyValue [0];

                            //  open a HTML document from OpenOffice.org
                            aDoc = oCLoader.loadComponentFromURL(
                                "http://api.openoffice.org/common/ref/" +
                                sURLDocument, "_blank", 0, szEmptyArgs );
                        }
                        }
                        catch ( Exception exception ) {
                            System.err.println( exception );
                        }
                    }
                }
                );

            //  Add TreeWillExpandListener
            tree.addTreeWillExpandListener(
                new TreeWillExpandListener() {
                    public void treeWillExpand(
                        javax.swing.event.TreeExpansionEvent event)
                    throws javax.swing.tree.ExpandVetoException {
                        TreePath treePath = event.getPath();
                        if ( treePath != null ) {
                            if ( treePath.getPathCount() % 2 > 0 ) {
                                //  get the selected node
                                DefaultMutableTreeNode node;
                                node = ( DefaultMutableTreeNode ) treePath.
                                    getLastPathComponent();
                                node.removeAllChildren();

                                //  get the pertinent object from the hashtable
                                Object object = hashtableObjects.get(
                                    event.getPath().toString() );

                                if ( object != null ) {
                                    //  the object is able to be inspected

                                    //  inspect the object
                                    inspectChild( object,
                                    ( DefaultMutableTreeNode ) node,
                                    m_introspection, event.getPath() );
                                }
                            }
                        }

                    }
                    public void treeWillCollapse(
                        javax.swing.event.TreeExpansionEvent evt)
                    throws javax.swing.tree.ExpandVetoException {
                    }
                } );

            //  show the frame with the tree
            jframe.setSize( 200, 200 );
            jframe.pack();
            jframe.show();
            }
            catch( Exception exception ) {
                System.err.println( exception );
            }
        }

        //  return the parameter mode (IN, OUT, INOUT)
        private String getParamMode(ParamMode paramMode) {
            String toReturn = "";
            if ( paramMode == ParamMode.IN ) {
                toReturn = "IN";
            }
            if ( paramMode == ParamMode.OUT ) {
                toReturn = "OUT";
            }
            if ( paramMode == ParamMode.INOUT ) {
                toReturn = "INOUT";
            }

            return( toReturn );
        }

        //  add all methods for the given object to the tree under the node parent
        private void addMethodsToTree( Object a,
                                       DefaultMutableTreeNode parent,
                                       XIntrospection xIntrospection ) {
            try {
                //  get the introspection access for the given object
                XIntrospectionAccess xIntrospectionAccess =
                    xIntrospection.inspect( a );

                //  get all the methods
                XIdlMethod[] mMethods = xIntrospectionAccess.getMethods(
                    com.sun.star.beans.MethodConcept.ALL );

                for ( int n = 0; n < mMethods.length; n++ ) {
                    ParamInfo[] paramInfo = mMethods[ n ].getParameterInfos();
                    String sParameters = "";
                    if ( paramInfo != null ) {
                        //  get all parameters with type and mode
                        for ( int i = 0; i < paramInfo.length; i++ ) {
                            XIdlClass xIdlClass = paramInfo[ i ].aType;
                            if ( i == 0 ) {
                                //  the first parameter has no leading comma
                                sParameters += "[" + this.getParamMode(
                                    paramInfo[ i ].aMode ) + "] " +
                                    xIdlClass.getName();
                            }
                            else {
                                //  all other parameters are separated with comma
                                sParameters += ", [" + this.getParamMode(
                                    paramInfo[ i ].aMode ) + "] " +
                                    xIdlClass.getName();
                            }
                        }
                    }

                    //  add the method with name, return type, and parameters to
                    //  the tree
                    DefaultMutableTreeNode child = new DefaultMutableTreeNode(
                        mMethods[ n ].getReturnType().getName() + " " +
                        mMethods[ n ].getName() + " (" + sParameters + " )" );
                    parent.add( child );
                }
            }
            catch( Exception e ) {
                System.err.println( e );
            }
        }

        //  add all containers for the given object to the tree under the node
        //  parent
        private void addContainerToTree( Object a,
                                         DefaultMutableTreeNode parent,
                                         XIntrospection xIntrospection ) {
            try {
                //  get the introspection access for the given object
                XIntrospectionAccess xIntrospectionAccessObject =
                    xIntrospection.inspect( a );

                if ( xIntrospectionAccessObject != null ) {

                    //  get the enumeration access
                    XEnumerationAccess xEnumerationAccess = (XEnumerationAccess)
                        UnoRuntime.queryInterface( XEnumerationAccess.class,
                            xIntrospectionAccessObject.queryAdapter(
                                new Type( XEnumerationAccess.class ) ) );
                    if ( xEnumerationAccess != null ) {
                        //  get the enumeration
                        XEnumeration xEnumeration =
                            xEnumerationAccess.createEnumeration();

                        //  create a node for the enumeration category
                        DefaultMutableTreeNode childEnumeration =
                            new DefaultMutableTreeNode( "Enumeration" );

                        while ( xEnumeration.hasMoreElements() ) {
                            //  the enumeration has more objects

                            //  get the next object from the enumeration
                            Object objectElement = xEnumeration.nextElement();

                            //  create a node for the object and add it to
                            //  enumeration category
                            DefaultMutableTreeNode childEnumerationElement =
                                new DefaultMutableTreeNode(
                                    objectElement.toString() );
                            childEnumeration.add( childEnumerationElement );
                        }

                        if ( childEnumeration.getChildCount() > 0 ) {
                            //  there are some objects so that the enumeration
                            //  root could be added to the tree
                            parent.add( childEnumeration );
                        }
                    }

                    //  get the index access
                    XIndexAccess xIndexAccess = (XIndexAccess)
                        UnoRuntime.queryInterface( XIndexAccess.class,
                            xIntrospectionAccessObject.queryAdapter(
                                new Type( XIndexAccess.class ) ) );
                    if ( xIndexAccess != null ) {

                        //  create a node for the index category
                        DefaultMutableTreeNode childIndex =
                            new DefaultMutableTreeNode( "Index" );

                        //  add all elements in the index to the index category
                        for ( int i = 0; i < xIndexAccess.getCount(); i++ ) {
                            //  get the object at the current index
                            Object objectElement = xIndexAccess.getByIndex( i );

                            //  create a node for the object
                            DefaultMutableTreeNode childIndexElement =
                                new DefaultMutableTreeNode(
                                    objectElement.toString() );
                            childIndex.add( childIndexElement );

                            DefaultMutableTreeNode root =
                                (DefaultMutableTreeNode)parent.getRoot();
                            root.add( childIndexElement );
                        }

                        if ( childIndex.getChildCount() > 0 ) {
                            //  there are some objects to be added to the tree
                            parent.add( childIndex );
                        }
                    }
                }
            }
            catch( Exception exception ) {
                System.err.println( exception );
            }
        }

        //  add all interfaces for the given object to the tree under the node
        //  parent
        private void addInterfacesToTree( Object a,
                                          DefaultMutableTreeNode parent ) {
            try {
                //  get the type provider for the given object
                XTypeProvider xTypeProvider = ( XTypeProvider )
                    UnoRuntime.queryInterface( XTypeProvider.class, a );
                if ( xTypeProvider != null ) {
                    //  get all interfaces
                    Type[] type = xTypeProvider.getTypes();

                    for ( int m = 0; m < type.length; m++ ) {
                        //  create a node for the current interface and add it to
                        //  the tree
                        DefaultMutableTreeNode child =
                            new DefaultMutableTreeNode( type[ m ].getTypeName() );
                        parent.add( child );
                    }
                }
            }
            catch( Exception e ) {
                System.err.println( e );
            }
        }

        //  add all services for the given object to the tree under the node parent
        private void addServicesToTree(Object a,
                                       DefaultMutableTreeNode parent) {
            try {
                //  get the service info for the given object
                XServiceInfo xServiceInfo = ( XServiceInfo )
                    UnoRuntime.queryInterface( XServiceInfo.class, a );

                if ( xServiceInfo != null ) {
                    //  get all supported services
                    String[] sSupportedServiceNames =
                        xServiceInfo.getSupportedServiceNames();

                    for ( int m = 0; m < sSupportedServiceNames.length; m++ ) {
                        //  create a node for the service and add it to the tree
                        DefaultMutableTreeNode child =
                            new DefaultMutableTreeNode(sSupportedServiceNames[ m ]);
                        parent.add( child );
                    }
                }
            }
            catch( Exception e ) {
                System.err.println( e );
            }

        }

        //  add all properties for the given object to the tree under the
        //  node parent
        private void addPropertiesToTree( Object a,
                                          DefaultMutableTreeNode parentAttributes,
                                          DefaultMutableTreeNode parentContent,
                                          XIntrospection xIntrospection,
                                          TreePath treePath ) {
            try {
                //  get the introspection access for the given object
                XIntrospectionAccess xIntrospectionAccess =
                    xIntrospection.inspect( a );

                //  get all properties for the given object
                Property[] myProperties =xIntrospectionAccess.getProperties(
                    com.sun.star.beans.PropertyConcept.ALL );

                for ( int n = 0; n < myProperties.length; n++ ) {
                    //  get the type of class
                    Type type = myProperties[ n ].Type;
                    TypeClass typeClass = type.getTypeClass();

                    //  get the property set with help of the introspection access
                    XPropertySet xPropertySet = ( XPropertySet )
                        UnoRuntime.queryInterface( XPropertySet.class,
                            xIntrospectionAccess.queryAdapter(
                                new Type( XPropertySet.class ) ) );

                    if ( xPropertySet != null ) {
                        //  get the value of the property
                        Object object =
                            xPropertySet.getPropertyValue(myProperties[ n ].Name);

                        if ( object != null ) {
                            //  creates a node for the property with his name,
                            //  type, and value
                            String sTreeNodeName = myProperties[n].Name +
                                " = (" + myProperties[n].Type.getTypeName() +
                                ") " + object.toString();
                            DefaultMutableTreeNode child =
                                new DefaultMutableTreeNode( sTreeNodeName );

                            if ( ( !object.getClass().isPrimitive() )
                                 && ( typeClass != TypeClass.ARRAY )
                                 && ( typeClass != TypeClass.BOOLEAN )
                                 && ( typeClass != TypeClass.BYTE )
                                 && ( typeClass != TypeClass.CHAR )
                                 && ( typeClass != TypeClass.DOUBLE )
                                 && ( typeClass != TypeClass.ENUM )
                                 && ( typeClass != TypeClass.FLOAT )
                                 && ( typeClass != TypeClass.HYPER )
                                 && ( typeClass != TypeClass.LONG )
                                 && ( typeClass != TypeClass.SHORT )
                                 && ( typeClass != TypeClass.STRING )
                                 && ( typeClass != TypeClass.UNSIGNED_HYPER )
                                 && ( typeClass != TypeClass.UNSIGNED_LONG )
                                 && ( typeClass != TypeClass.UNSIGNED_SHORT ) ) {
                                //  it's not a primitive object

                                //  add the object to the category content
                                parentContent.add(child);

                                //  add the object to the hashtable for a possible
                                //  access in the tree
                                String sPath = treePath.toString();
                                sPath = sPath.substring( 0,
                                sPath.length() - 1 ) + ", Content, " +
                                    child.getUserObject().toString() + "]";
                                hashtableObjects.put( sPath, object );

                                //  create a dummy node and add it to the tree
                                DefaultMutableTreeNode subchild =
                                    new DefaultMutableTreeNode( "Dummy" );
                                child.add( subchild );
                            }
                            else {
                                parentAttributes.add(child);
                            }
                        }
                    }

                }
            }
            catch( Exception e ) {
                System.err.println( e );
            }
        }

        /** Inspect a given object and show its methods, properties, interfaces,
         * and services.
         * @param treePath
         * @param a The object to inspect
         * @param parent The parent node of the selected node
         * @param xIntrospection Is needed to get the interface XIntrospectionAccess
         */
        public void inspectChild(Object a,
                                 DefaultMutableTreeNode parent,
                                 XIntrospection xIntrospection,
                                 TreePath treePath ) {
            try {
                DefaultMutableTreeNode child;

                if ( !( a.getClass().isArray() ) ) {
                    //  the object is not an array

                    //  Get all methods
                    child = new DefaultMutableTreeNode("Methods");
                    this.addMethodsToTree( a, child, xIntrospection );
                    if ( child.getChildCount() > 0 ) {
                        parent.add(child);
                    }

                    //  Get all properties
                    DefaultMutableTreeNode childContent =
                        new DefaultMutableTreeNode("Content");
                    DefaultMutableTreeNode childAttributes =
                        new DefaultMutableTreeNode("Attributes");
                    this.addPropertiesToTree( a, childAttributes, childContent,
                                              xIntrospection, treePath );
                    if ( childAttributes.getChildCount() > 0 ) {
                        parent.add(childAttributes);
                    }
                    if ( childContent.getChildCount() > 0 ) {
                        parent.add(childContent);
                    }

                    //  Get all interfaces
                    child = new DefaultMutableTreeNode("Interfaces");
                    this.addInterfacesToTree( a, child );
                    if ( child.getChildCount() > 0 ) {
                        parent.add(child);
                    }

                    // Get all services
                    child = new DefaultMutableTreeNode("Services");
                    this.addServicesToTree( a, child );
                    if ( child.getChildCount() > 0 ) {
                        parent.add(child);
                    }

                    // Get all containers
                    child = new DefaultMutableTreeNode("Container");
                    this.addContainerToTree( a, child, xIntrospection );
                    if ( child.getChildCount() > 0 ) {
                        parent.add(child);
                    }
                }
                else {
                    //  the object is an array, so get only the properties

                    DefaultMutableTreeNode childContent =
                        new DefaultMutableTreeNode("Content");
                    DefaultMutableTreeNode childAttributes =
                        new DefaultMutableTreeNode("Attributes");

                    if ( a.getClass().getComponentType().isPrimitive() ) {
                        byte[] object = (byte[]) a;
                        for ( int i = 0; i < object.length; i++ ) {
                            DefaultMutableTreeNode defaultMutableTreeNodeElement =
                                new DefaultMutableTreeNode( Byte.toString(
                                                                object[ i ] ) );
                            childContent.add( defaultMutableTreeNodeElement );
                        }
                    }
                    else {

                        Object[] object = ( Object[] ) a;
                        for ( int i = 0; i < object.length; i++ ) {
                            DefaultMutableTreeNode defaultMutableTreeNodeElement =
                                new DefaultMutableTreeNode(object[i].toString());
                            childContent.add( defaultMutableTreeNodeElement );
                            String sPath = treePath.toString();
                            sPath = sPath.substring( 0, sPath.length() - 1 ) +
                                ", Content, " +
                                defaultMutableTreeNodeElement.getUserObject().
                                toString() + "]";
                            hashtableObjects.put( sPath, object );
                            DefaultMutableTreeNode subchild =
                                new DefaultMutableTreeNode( "Methods" );
                            defaultMutableTreeNodeElement.add( subchild );
                        }
                    }

                    if ( childAttributes.getChildCount() > 0 ) {
                        parent.add(childAttributes);
                    }
                    if ( childContent.getChildCount() > 0 ) {
                        parent.add(childContent);
                    }
                }
            }
            catch( Exception e ) {
                System.err.println( e );
            }
        }


        public static String[] getServiceNames() {
            String[] sSupportedServiceNames = { __serviceName };
            return sSupportedServiceNames;
        }

        // Implement the interface XServiceInfo
        /** Get all supported service names.
         * @return Supported service names.
         */
        public String[] getSupportedServiceNames() {
            return getServiceNames();
        }

        // Implement the interface XServiceInfo
        /** Test, if the given service will be supported.
         * @param sService Service name.
         * @return Return true, if the service will be supported.
         */
        public boolean supportsService( String sServiceName ) {
            return sServiceName.equals( __serviceName );
        }

        // Implement the interface XServiceInfo
        /** Get the implementation name of the component.
         * @return Implementation name of the component.
         */
        public String getImplementationName() {
            return _Inspector.class.getName();
        }

    }


    /**
     * Gives a factory for creating the service.
     * This method is called by the <code>JavaLoader</code>
     * <p>
     * @return  returns a <code>XSingleComponentFactory</code> for creating
     *          the component
     * @param   sImplName the name of the implementation for which a
     *          service is desired
     * @see     com.sun.star.comp.loader.JavaLoader
     */
    public static XSingleComponentFactory __getComponentFactory( String sImplName )
    {
        XSingleComponentFactory xFactory = null;

        if ( sImplName.equals( _Inspector.class.getName() ) )
            xFactory = Factory.createComponentFactory(_Inspector.class,
                                                      _Inspector.getServiceNames());

        return xFactory;
    }

    /**
     * Writes the service information into the given registry key.
     * This method is called by the <code>JavaLoader</code>
     * <p>
     * @return  returns true if the operation succeeded
     * @param   regKey the registryKey
     * @see     com.sun.star.comp.loader.JavaLoader
     */
    public static boolean __writeRegistryServiceInfo(XRegistryKey regKey) {
        return Factory.writeRegistryServiceInfo(_Inspector.class.getName(),
                                                _Inspector.getServiceNames(),
                                                regKey);
    }
}
