/*************************************************************************
 *
 *  $RCSfile: UnoApp.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kr $ $Date: 2000-09-28 17:10:02 $
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

package com.sun.star.tools.uno;


import java.util.Enumeration;
import java.util.Hashtable;
import java.util.Vector;


import com.sun.star.bridge.XBridge;
import com.sun.star.bridge.XBridgeFactory;
import com.sun.star.bridge.XInstanceProvider;
import com.sun.star.bridge.XUnoUrlResolver;

import com.sun.star.comp.helper.RegistryServiceFactory;

import com.sun.star.comp.loader.JavaLoader;

import com.sun.star.connection.XAcceptor;
import com.sun.star.connection.XConnection;

import com.sun.star.container.XSet;

import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XSingleServiceFactory;

import com.sun.star.loader.XImplementationLoader;

import com.sun.star.registry.XRegistryKey;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.Type;


/**
 * <code>UnoApp</code> is the generic UNO application for java.
 * It removes the need for writing UNO applications in Java.
 * <p>
 */
public class UnoApp {
    /**
     * Bootstraps a servicemanager with some base components registered.
     * <p>
     * @return     a freshly boostrapped service manager
     * @see        com.sun.star.lang.ServiceManager
     */
    static public XMultiServiceFactory createSimpleServiceManager() throws Exception {
        JavaLoader loader = new JavaLoader();

        XImplementationLoader xImpLoader = (XImplementationLoader)UnoRuntime.queryInterface(XImplementationLoader.class, loader);

        // Get the factory for the ServiceManager
        Object loaderobj = xImpLoader.activate("com.sun.star.comp.servicemanager.ServiceManager", null, null, null);

        // Ensure that we have got a factory
        XSingleServiceFactory xManagerFac = (XSingleServiceFactory) UnoRuntime.queryInterface(XSingleServiceFactory.class, loaderobj);
        // Create an instance of the ServiceManager
        XMultiServiceFactory xMultiFac = (XMultiServiceFactory) UnoRuntime.queryInterface(XMultiServiceFactory.class,
                                                                                          xManagerFac.createInstance());

        // set the ServiceManager at the JavaLoader with the XInitialization interface
        XInitialization xInit = (XInitialization) UnoRuntime.queryInterface(XInitialization.class, xImpLoader);
        Object[] iniargs = { xMultiFac };
        xInit.initialize( iniargs );


        // now use the XSet interface at the ServiceManager to add the factory of the loader
        XSet xSet = (XSet) UnoRuntime.queryInterface(XSet.class, xMultiFac);

        // Get the factory of the loader
        XSingleServiceFactory xSingleServiceFactory = (XSingleServiceFactory) UnoRuntime.queryInterface(XSingleServiceFactory.class,
                                                                                                        xImpLoader.activate("com.sun.star.comp.loader.JavaLoader", null, null, null));

        // add the javaloader
        xSet.insert(xSingleServiceFactory);

        // add the service manager
        xSet.insert(xManagerFac);

        // Get the factory of the URLResolver
        xSingleServiceFactory = (XSingleServiceFactory)UnoRuntime.queryInterface(XSingleServiceFactory.class,
                                                                                 xImpLoader.activate("com.sun.star.comp.urlresolver.UrlResolver", null, null, null));
        xSet.insert(xSingleServiceFactory);

        // add the bridgefactory
        xSingleServiceFactory = (XSingleServiceFactory)UnoRuntime.queryInterface(XSingleServiceFactory.class,
                                                                                 xImpLoader.activate("com.sun.star.comp.bridgefactory.BridgeFactory", null, null, null));
        xSet.insert(xSingleServiceFactory);

        // add the connector
        xSingleServiceFactory = (XSingleServiceFactory)UnoRuntime.queryInterface(XSingleServiceFactory.class,
                                                                                 xImpLoader.activate("com.sun.star.comp.connections.Connector", null, null, null));
        xSet.insert(xSingleServiceFactory);

        // add the acceptor
        xSingleServiceFactory = (XSingleServiceFactory)UnoRuntime.queryInterface(XSingleServiceFactory.class,
                                                                                 xImpLoader.activate("com.sun.star.comp.connections.Acceptor", null, null, null));
        xSet.insert(xSingleServiceFactory);

        return xMultiFac;
    }


    /**
     * An XInstanceProvider implementation, which allows simple object export.
     * <p>
     * @see        com.sun.star.bridge.XBridgeFactory
     * @see        com.sun.star.bridge.XInstanceProvider
     */
    static class InstanceProvider implements XInstanceProvider {
        private String _name;
        private Object _component;

        InstanceProvider(String name, Object component) {
            _name = name;
            _component = component;
        }

        public Object getInstance( /*IN*/String sInstanceName ) throws com.sun.star.container.NoSuchElementException, com.sun.star.uno.RuntimeException {
            Object object = null;

            if(sInstanceName.equals(_name))
                object = _component;

            return object;
        }
    }

    /**
     * Exports the given object via the given url while the using
     * the <code>xMultiServiceFactory</code>.
     * <p>
     * @param xMultiServiceFactory   the service manager to use
     * @param dcp                    a uno url, which describes how to export
     * @param object                 the object to export
     * @see        com.sun.star.bridge.XBridge
     * @see        com.sun.star.bridge.XBridgeFactory
     * @see        com.sun.star.bridge.XInstanceProvider
     */
    static public void export(XMultiServiceFactory xMultiServiceFactory, String dcp, Object object) throws Exception {
        String conDcp = null;
        String protDcp = null;
        String rootOid = null;

        // split the description into tokens
        int index = dcp.indexOf(':');
        String url = dcp.substring(0, index).trim();
        dcp = dcp.substring(index + 1).trim();

        index = dcp.indexOf(';');
        conDcp = dcp.substring(0, index).trim();
        dcp = dcp.substring(index + 1).trim();

        index = dcp.indexOf(';');
        protDcp = dcp.substring(0, index).trim();
        dcp = dcp.substring(index + 1).trim();

        rootOid = dcp.trim().trim();

        // get an acceptor
        XAcceptor xAcceptor = (XAcceptor)UnoRuntime.queryInterface(XAcceptor.class,
                                                                   xMultiServiceFactory.createInstance("com.sun.star.connection.Acceptor"));

        System.err.println("waiting for connect...");
        XConnection xConnection = xAcceptor.accept(conDcp);


        // get a bridgefactory
        XBridgeFactory xBridgeFactory = (XBridgeFactory)UnoRuntime.queryInterface(XBridgeFactory.class,
                                                                                  xMultiServiceFactory.createInstance("com.sun.star.bridge.BridgeFactory"));

        // create the bridge
        XBridge xBridge = xBridgeFactory.createBridge(conDcp + ";" + protDcp, protDcp, xConnection, new InstanceProvider(rootOid, object));
    }



    /**
     * Parses the given string a makes words of according to the following rules:
     * - words embraced by " are recognized as one word
     * - charachters preceded by \ are quoted (e.g. " or spaces)
     * <p>
     * @return         an array of words
     * @param string   the string to parse
     */
    static String []parseString(String string) {
        Vector vector = new Vector();

        boolean inString = false;
        boolean quote = false;
        String word = "";
        int i = 0;
        while(i < string.length()) {
            if(string.charAt(i) == '\"' && !quote) {
                inString = !inString;
            }
            else if(string.charAt(i) == '\\' && !quote) {
                quote = true;
            }
            else if(Character.isSpace(string.charAt(i)) && !quote && !inString) {
                if(word.length() > 0) {
                    vector.addElement(word);
                    word = "";
                }
            }
            else {
                word += string.charAt(i);
                quote = false;
            }

            ++ i;
        }

        vector.addElement(word);

        String args[] = new String[vector.size()];
        for(i = 0; i < args.length; ++ i)
            args[i] = (String)vector.elementAt(i);

        return args;
    }

    /**
     * Takes a <code>String</code> array and concatenates the words
     * to one string.
     * <p>
     * @return         the concatenated string
     * @param args     the words to concatenate
     */
    static String mergeString(String args[]) {
        String string = "";

        for(int i = 0; i < args.length; ++ i)
            string += " " + args[i];

        return string;
    }

    /**
     * This is the base class for options for <code>UnoApp</code>.
     * Every option has to be derived from this class and has to implement
     * the <code>set</code> method, which is called while parsing the arguments.
     * <p>
     * Options which create the result object should also overwrite the
     * create method.
     */
    static abstract class Option {
        String _key;
        String _help;

        /**
         * Constructor, which has to be called with option key (e.g. -l)
         * and a description of the option.
         * <p>
         * @param key   the key of this option
         * @param help  a description of the option, which is printed when calling -h
         */
        protected Option(String key, String help) {
            _key  = key;
            _help = help;
        }

        /**
         * <code>set</code> is called while parsing the arguments.
         * <p>
         * @param unoApp   the <code>unoApp</code> to use for this option
         * @param args     an <code>String</code> array with the arguments
         * @param index    the index of the current argument
         */
        abstract void set(UnoApp unoApp, String args[], int index[]) throws Exception;

        /**
         * Create is called while trying to get the result object.
         * <p>
         * @param context               the context for this create, should have been set by set
         * @param xMultiServiceFactory  the service manager to use
         * @param args                  the args for the object instantiation
         */
        Object create(Object context, XMultiServiceFactory xMultiServiceFactory, Object args[]) throws Exception {
            throw new Exception("not implemented");
        }
    }

    /**
     * The result of applying this option is an instance of
     * the named service.
     */
    static class Service_Option extends Option {
        static final String __key = "-s";

        Service_Option() {
            super(__key, "the service to be instantiated");
        }

        void set(UnoApp unoApp, String args[], int index[]) throws Exception {
            String serviceName = args[index[0] ++];

            unoApp._context = serviceName;
            unoApp._creator = this;
        }

        Object create(Object context, XMultiServiceFactory xMultiServiceFactory, Object args[]) throws Exception {
            Object object = null;

            if(args != null && args.length != 0)
                object = xMultiServiceFactory.createInstanceWithArguments((String)context, args);

            else
                object = xMultiServiceFactory.createInstance((String)context);

            return object;
        }
    }

    /**
     * The result of applying this option is a registry
     * servicemanager.
     */
    static class Registry_Option extends Option {
        static final String __key = "-r";

        Registry_Option() {
            super(__key, "the registry to use");
        }

        void set(UnoApp unoApp, String args[], int index[]) throws Exception {
            unoApp._context = RegistryServiceFactory.create(args[index[0] ++]);
            unoApp._creator = this;

            System.err.println("got RegistryServiceFactory:" + unoApp._context);
        }

        Object create(Object context, XMultiServiceFactory xMultiServiceFactory, Object args[]) throws Exception {
            return context;
        }

    }

    /**
     * The result of applying this option is the import
     * of the described object.
     */
    static class Import_Option extends Option {
        static final String __key = "-u";

        Import_Option() {
            super(__key, "import object from this url");
        }

        void set(UnoApp unoApp, String args[], int index[]) throws Exception {
            unoApp._context = args[index[0] ++];
            unoApp._uno_url = (String)unoApp._context;
            unoApp._creator = this;
        }

        Object create(Object context, XMultiServiceFactory xMultiServiceFactory, Object args[]) throws Exception {
            XMultiServiceFactory smgr = createSimpleServiceManager();

            XUnoUrlResolver urlResolver = (XUnoUrlResolver)UnoRuntime.queryInterface(XUnoUrlResolver.class, smgr.createInstance("com.sun.star.bridge.UnoUrlResolver"));

            return urlResolver.resolve((String)context);
        }
    }

    /**
     * The result of applying this option is a multiservicefactory
     * for the given component.
     */
    static class Component_Option extends Option {
        static final String __key = "-c";

        Component_Option() {
            super(__key, "add the a component");
        }

        void set(UnoApp unoApp, String args[], int index[]) throws Exception {
            unoApp._context = args[index[0] ++];
            unoApp._creator = this;
        }

        Object create(Object context, XMultiServiceFactory xMultiServiceFactory, Object args[]) throws Exception {
            String componentName = (String)context;

            XImplementationLoader loader = (XImplementationLoader)UnoRuntime.queryInterface(XImplementationLoader.class,
                                                                                            xMultiServiceFactory.createInstance("com.sun.star.loader.Java"));

            Object serviceManager = xMultiServiceFactory.createInstance("com.sun.star.lang.ServiceManager");
            XSet serviceManager_xSet = (XSet)UnoRuntime.queryInterface(XSet.class, serviceManager);

            XRegistryKey xRegistryKey = new RegistryKey("ROOT");

            loader.writeRegistryInfo(xRegistryKey, null, componentName);

            String keys[] = xRegistryKey.getKeyNames();
            for(int j = 0; j < keys.length; ++ j) {
                String implName = keys[j].substring(1);
                implName = implName.substring(0, implName.length() - "/UNO/SERVICES".length());

                Object factory = loader.activate(implName, null, componentName, xRegistryKey);
                XSingleServiceFactory xSingleServiceFactory = (XSingleServiceFactory)UnoRuntime.queryInterface(XSingleServiceFactory.class,
                                                                                                               factory);

                if (xSingleServiceFactory == null)
                    throw new com.sun.star.loader.CannotActivateFactoryException("Can not get factory for " +  implName);

                serviceManager_xSet.insert(xSingleServiceFactory);
            }

            return serviceManager;
        }
    }

    /**
     * The -smgr option takes the given objects as <code>XSingleServiceFactory</code>
     * or as <code>XMultiServiceFactory</code> and inserts them into the given service manager.
     */
    static class ServiceManager_Option extends Option {
        static final String __key = "-smgr";

        class MySingleServiceFactory implements XSingleServiceFactory, XServiceInfo {
            XMultiServiceFactory _xMultiServiceFactory;
            String _serviceName;

            MySingleServiceFactory(XMultiServiceFactory xMultiServiceFactory, String serviceName) {
                _xMultiServiceFactory = xMultiServiceFactory;
                _serviceName = serviceName;
            }

            public Object createInstance( ) throws com.sun.star.uno.Exception, com.sun.star.uno.RuntimeException {
                return _xMultiServiceFactory.createInstance(_serviceName);
            }
            public Object createInstanceWithArguments( /*IN*/java.lang.Object[] aArguments ) throws com.sun.star.uno.Exception, com.sun.star.uno.RuntimeException {
                return _xMultiServiceFactory.createInstanceWithArguments(_serviceName, aArguments);
            }

            // Methods
            public String getImplementationName(  ) throws com.sun.star.uno.RuntimeException {
                return getClass().getName() + _serviceName;
            }

            public boolean supportsService( /*IN*/String ServiceName ) throws com.sun.star.uno.RuntimeException {
                return _serviceName.equals(ServiceName);
            }

            public String[] getSupportedServiceNames(  ) throws com.sun.star.uno.RuntimeException {
                return new String[]{_serviceName};
            }
        }

        ServiceManager_Option() {
            super(__key, "\"[comp name]*\" the service manager to use");
        }

        void set(UnoApp unoApp, String args[], int index[]) throws Exception {
            String arg = args[index[0] ++];

            if(arg.charAt(0) == '"')
                arg = arg.substring(0, arg.length() - 1).substring(1);

            String comps[] = parseString(arg);


            XMultiServiceFactory xMultiServiceFactory = unoApp._xMultiServiceFactory;

            // now use the XSet interface at the ServiceManager to add the factory of the loader
              XSet xSet = (XSet) UnoRuntime.queryInterface(XSet.class, xMultiServiceFactory);

            for(int i = 0; i < comps.length; ++ i) {
                Object object = new UnoApp(comps[i]).getObject();
                XSingleServiceFactory xSingleServiceFactory = (XSingleServiceFactory)UnoRuntime.queryInterface(XSingleServiceFactory.class,
                                                                                                               object);
                if(xSingleServiceFactory == null) {
                    XMultiServiceFactory blaxMultiServiceFactory = (XMultiServiceFactory)UnoRuntime.queryInterface(XMultiServiceFactory.class,
                                                                                                                   object);

                    String services[] = blaxMultiServiceFactory.getAvailableServiceNames();

                    for(int j = 0; j < services.length; ++ j)
                        xSet.insert(new MySingleServiceFactory(blaxMultiServiceFactory, services[j]));
                }
                else
                    xSet.insert(xSingleServiceFactory);
            }

            unoApp._xMultiServiceFactory = xMultiServiceFactory;
        }
    }

    /**
     * The Args option put the given objects into arg array,
     * which is used when instantiating the result object.
     */
    static class Args_Option extends Option {
        static final String __key = "--";

        Args_Option() {
            super(__key, "the args for XInitialization");
        }

        void set(UnoApp unoApp, String args[], int index[]) throws Exception {
            Object obj_args[] = new Object[args.length - index[0]];

            int i = 0;
            while(index[0] < args.length)
                obj_args[i ++] = new UnoApp(args[index[0] ++]).getObject();

            unoApp._args = obj_args;
        }
    }

    /**
     * The help option prints a help message.
     */
    static class Help_Option extends Option {
        static final String __key = "-h";

        Help_Option() {
            super(__key, "this help");
        }

        void set(UnoApp unoApp, String args[], int index[]) {
            System.out.println("usage: UnoApp [option]*");

            Enumeration elements = __options.elements();
            while(elements.hasMoreElements()) {
                Option option = (Option)elements.nextElement();
                System.out.println("\t" + option._key + "\t" + option._help);
            }
        }
    }


    // All options have to be inserted into this table
    static final Hashtable __options = new Hashtable();
    static {
        __options.put(Service_Option.__key,        new Service_Option());
        __options.put(Import_Option.__key,         new Import_Option());
        __options.put(Registry_Option.__key,       new Registry_Option());
        __options.put(ServiceManager_Option.__key, new ServiceManager_Option());
          __options.put(Args_Option.__key,           new Args_Option());
        __options.put(Component_Option.__key,      new Component_Option());
        __options.put(Help_Option.__key,           new Help_Option());
    };


    /**
     * The is the main method, which is called from java.
     */
    static public void main(String args[]) throws Exception {
        // We have to do this, cause the jdb under solaris does not allow to pass
        // arguments with included spaces.
        String arg = mergeString(args);

        UnoApp unoApp = new UnoApp(arg);

        if(unoApp._uno_url != null) // see, if we have to export the object
            export(unoApp._xMultiServiceFactory, unoApp._uno_url, unoApp.getObject());
        else
            System.err.println("result: " + unoApp.getObject());
    }


    Option _creator = null; // the creator gets set by option which provide objects
    Object _context = null; // the context for object creation
    Object _args[]  = null; // the args for object creation
    String _uno_url = null; // the url for object export
    XMultiServiceFactory _xMultiServiceFactory; // the service manager for object creation

    /**
     * Initializes <code>UnoApp</code>.
     * If string only is one word and the does not start with "-"
     * set the context with string.
     * <p>
     * @param  string   the arguments
     */
    UnoApp(String string) throws Exception {
        _xMultiServiceFactory = createSimpleServiceManager();

        String args[] = parseString(string);

        if(args.length == 1 && args[0].charAt(0) != '-')
            _context = string;
        else
            parseArgs(args);
    }

    /**
     * Interprets the args as a sequence of option names
     * followed by option parameters.
     * <p>
     * @param args   the arguments
     */
    protected void parseArgs(String args[]) throws Exception {
        int i[] = new int[1];
        while(i[0] < args.length) {
            Option option = (Option)__options.get(args[i[0]]);
            if(option == null) {
                System.err.println("unknown option:" + args[i[0]]);
                return;
            }

            ++ i[0];

            option.set(this, args, i);
        }
    }

    /**
     * Gets the object described by this <code>unoApp</code>.
     * If no creator is set, returns the context as object.
     * <p>
     * @return  the object
     */
    Object getObject () throws Exception {
        Object object = null;

        if(_creator == null)
            object = _context;

        else
            object = _creator.create(_context, _xMultiServiceFactory, _args);

        return object;
    }
}
