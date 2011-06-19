/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
package com.sun.star.registration;

import com.sun.star.beans.NamedValue;
import com.sun.star.comp.loader.FactoryHelper;
import com.sun.star.frame.DispatchResultEvent;
import com.sun.star.frame.DispatchResultState;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.registry.*;
import com.sun.star.servicetag.*;
import com.sun.star.system.*;
import com.sun.star.task.*;
import com.sun.star.uno.*;
import com.sun.star.uri.XExternalUriReferenceTranslator;
import com.sun.star.util.XStringSubstitution;

import java.io.*;
import java.net.URI;
import java.net.URL;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.Locale;
import java.util.Set;
import java.net.HttpURLConnection;

public class Registration {

    public static XSingleServiceFactory __getServiceFactory(String implName,
        XMultiServiceFactory multiFactory, XRegistryKey regKey) {
        XSingleServiceFactory xSingleServiceFactory = null;

        if (implName.equals(Registration.class.getName())) {
            xSingleServiceFactory = FactoryHelper.getServiceFactory(_Registration.class, _serviceName, multiFactory, regKey);
        }

        return xSingleServiceFactory;
    }

    static final String _serviceName = "com.sun.star.comp.framework.DoRegistrationJob";

    static public class _Registration implements XJob {
        XComponentContext xComponentContext;

        XStringSubstitution xPathSubstService = null;
        XExternalUriReferenceTranslator xUriTranslator = null;

        RegistrationData theRegistrationData = null;

        public _Registration(XComponentContext xComponentContext) {
            this.xComponentContext = xComponentContext;
        }

        private String resolvePath(String path) {
            try {
                if( xPathSubstService == null || xUriTranslator == null ) {
                    XMultiComponentFactory theServiceManager = xComponentContext.getServiceManager();
                    if( xPathSubstService == null ) {
                        Object o = theServiceManager.createInstanceWithContext(
                            "com.sun.star.util.PathSubstitution",
                            xComponentContext );
                        xPathSubstService = (XStringSubstitution)
                            UnoRuntime.queryInterface(XStringSubstitution.class, o);
                    }

                    if( xUriTranslator == null ) {
                        Object o = theServiceManager.createInstanceWithContext(
                            "com.sun.star.uri.ExternalUriReferenceTranslator",
                            xComponentContext );
                        xUriTranslator =  (XExternalUriReferenceTranslator)
                            UnoRuntime.queryInterface(XExternalUriReferenceTranslator.class, o);
                    }
                }

                String s = xPathSubstService.substituteVariables(path, true);
                return xUriTranslator.translateToExternal(s);
            } catch (java.lang.Exception e) {
                return path;
            }
        }

        private void openBrowser(String url) {
            try {
                XMultiComponentFactory theServiceManager = xComponentContext.getServiceManager();

                Object o = theServiceManager.createInstanceWithContext(
                    "com.sun.star.system.SystemShellExecute",
                    xComponentContext );

                XSystemShellExecute xShellExecuteService = (XSystemShellExecute)
                    UnoRuntime.queryInterface(XSystemShellExecute.class, o);

                xShellExecuteService.execute( url, "", SystemShellExecuteFlags.DEFAULTS );
            } catch (java.lang.Exception e) {
            }
       }

        private ServiceTag getServiceTagFromRegistrationData(File xmlFile, String productURN) {
            try {
                RegistrationData storedRegData = RegistrationData.loadFromXML(new FileInputStream(xmlFile));
                Set<ServiceTag> storedServiceTags = storedRegData.getServiceTags();

                Iterator<ServiceTag> tagIterator = storedServiceTags.iterator();
                while( tagIterator.hasNext() ) {
                    ServiceTag tag = tagIterator.next();
                    if( tag.getProductURN().equals(productURN) ) {
                        theRegistrationData = storedRegData;
                        return tag;
                    }
                }

                // product URN has changed, remove registration data file
                xmlFile.delete();
            } catch (IOException e) {
                // fall through intentionally
            } catch (IllegalArgumentException e) {
                // file is damaged (or a name clash appeared)
                xmlFile.delete();
            }
            return null;
        }

        /*
         * XJob
         *
         * NOTE: as this Job hets triggered by the the JobExecutor service from first start
         * wizard and registration reminder code (because their frames do not implement
         * XDispatchProvider), making this an XAsyncJob doesn't make sense as the
         * JobExecutor waits for the jobFinished call on the listener passed.
         */
        public Object execute(NamedValue[] args)
            throws com.sun.star.lang.IllegalArgumentException, com.sun.star.uno.Exception {

            final NamedValue[] f_args = args;

            new Thread(
                new Runnable () {
                    public void run() {
                        try {
                            executeImpl(f_args);
                        } catch(com.sun.star.uno.Exception e) {
                        }
                    }
                }
            ).start();

            NamedValue ret[] = new NamedValue[1];
            ret[0] = new NamedValue( "Deactivate", new Boolean(false) );
            return ret;
        }

        public synchronized void executeImpl(NamedValue[] args)
            throws com.sun.star.lang.IllegalArgumentException, com.sun.star.uno.Exception {

            // extract the interesting part of the argument list
            NamedValue[] theJobConfig = null;
            NamedValue[] theEnvironment = null;

            int c = args.length;
            for (int i=0; i<c; ++i) {
                if (args[i].Name.equals("JobConfig"))
                    theJobConfig = (NamedValue[]) AnyConverter.toArray(args[i].Value);
                else if (args[i].Name.equals("Environment"))
                    theEnvironment = (NamedValue[]) AnyConverter.toArray(args[i].Value);
            }

            if (theEnvironment==null)
                throw new com.sun.star.lang.IllegalArgumentException("no environment");

            boolean saveConfig = false;

            String productName              = "";
            String productVersion           = "";
            String productURN               = "";
            String productParent            = "";
            String productParentURN         = "";
            String productDefinedInstanceID = "";
            String productSource            = "";
            String vendor                   = "";

            String urlRegData = null;
            String registrationURL = null;

            c = theJobConfig.length;
            for (int i=0; i<c; ++i) {
                if( theJobConfig[i].Name.equals("ProductName") ) {
                    productName = AnyConverter.toString(theJobConfig[i].Value);
                } else if( theJobConfig[i].Name.equals("ProductVersion") ) {
                    productVersion = AnyConverter.toString(theJobConfig[i].Value);
                } else if( theJobConfig[i].Name.equals("ProductURN") ) {
                    productURN = AnyConverter.toString(theJobConfig[i].Value);
                } else if( theJobConfig[i].Name.equals("ProductParent") ) {
                    productParent = AnyConverter.toString(theJobConfig[i].Value);
                } else if( theJobConfig[i].Name.equals("ProductParentURN") ) {
                    productParentURN = AnyConverter.toString(theJobConfig[i].Value);
                } else if( theJobConfig[i].Name.equals("ProductSource") ) {
                    productSource = AnyConverter.toString(theJobConfig[i].Value);
                } else if( theJobConfig[i].Name.equals("Vendor") ) {
                    vendor = AnyConverter.toString(theJobConfig[i].Value);
                } else if( theJobConfig[i].Name.equals("RegistrationData") ) {
                    urlRegData = resolvePath(AnyConverter.toString(theJobConfig[i].Value));
                } else if( theJobConfig[i].Name.equals("RegistrationURL") ) {
                    registrationURL = AnyConverter.toString(theJobConfig[i].Value);
                } else {
                    System.err.println( theJobConfig[i].Name + " = " + AnyConverter.toString(theJobConfig[i].Value) );
                }
            }

            if (registrationURL==null)
                throw new com.sun.star.lang.IllegalArgumentException("no registration url");

            boolean local_only = false;

            c = theEnvironment.length;
            for (int i=0; i<c; ++i) {
                if( theEnvironment[i].Name.equals("EventName") ) {
                    if( ! AnyConverter.toString(theEnvironment[i].Value).equals("onRegisterNow") ) {
                        local_only = true;
                    }
                }
            }

            try {

                /* ensure only one thread accesses/writes registration.xml at a time
                 * regardless how many instances of this Job exist.
                 */
                synchronized( _serviceName ) {

                    File xmlRegData = new File( new URI( urlRegData ) );

                    ServiceTag tag = getServiceTagFromRegistrationData(xmlRegData, productURN);
                    if( tag == null ) {
                        tag = ServiceTag.newInstance(
                            ServiceTag.generateInstanceURN(),
                            productName,
                            productVersion,
                            productURN,
                            productParent,
                            productParentURN,
                            productDefinedInstanceID,
                            vendor,
                            System.getProperty("os.arch"),
                            Installer.getZoneName(),
                            productSource);

                        theRegistrationData = new RegistrationData();
                        theRegistrationData.addServiceTag(tag);
                        theRegistrationData.storeToXML( new FileOutputStream( xmlRegData ) );
                    }

                    // Store the service tag in local registry, which might have been installed later
                    if( Registry.isSupported() ) {
                        // ignore communication failures with local service tag client
                        try {
                            if( Registry.getSystemRegistry().getServiceTag(tag.getInstanceURN()) == null ) {
                                Registry.getSystemRegistry().addServiceTag(tag);
                            }
                        } catch( java.io.IOException e) {
                            e.printStackTrace();
                        } catch (java.lang.RuntimeException e) {
                            e.printStackTrace();
                        }
                    }
                }

                if( ! local_only ) {
                    registrationURL = registrationURL.replaceAll("\\$\\{registry_urn\\}", theRegistrationData.getRegistrationURN());
                    registrationURL = registrationURL.replaceAll("\\$\\{locale\\}", Locale.getDefault().getLanguage());

                    HttpURLConnection con = (HttpURLConnection) new URL(registrationURL).openConnection();
                    con.setDoInput(true);
                    con.setDoOutput(true);
                    con.setUseCaches(false);
                    con.setAllowUserInteraction(false);
                    con.setRequestMethod("POST");

                    con.setRequestProperty("Content-Type", "text/xml;charset=\"utf-8\"");
                    try {
                        con.connect();

                        OutputStream out = con.getOutputStream();
                        theRegistrationData.storeToXML(out);
                        out.flush();
                        out.close();

                        int returnCode = con.getResponseCode();
                    } catch(java.lang.Exception e) {
                        // IOException and UnknownHostException
                    }
                    openBrowser(registrationURL);
                }
            } catch (java.net.MalformedURLException e) {
                e.printStackTrace();
                throw new com.sun.star.lang.IllegalArgumentException( e.toString() );
            } catch (java.net.URISyntaxException e) {
                e.printStackTrace();
                throw new com.sun.star.lang.IllegalArgumentException( e.toString() );
            } catch (java.io.IOException e) {
                e.printStackTrace();
                throw new com.sun.star.uno.RuntimeException( e.toString() );
            } catch (java.lang.RuntimeException e) {
                e.printStackTrace();
                throw new com.sun.star.uno.RuntimeException( e.toString() );
            }
        }
    }
}
