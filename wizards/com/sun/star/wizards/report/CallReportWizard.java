 /*************************************************************************
 *
 *  $RCSfile: CallReportWizard.java,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: bc $ $Date: 2002-06-27 12:08:40 $
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
 */

package com.sun.star.wizards.report;

import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;

import com.sun.star.uno.Type;

import com.sun.star.task.XJobExecutor;

import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.XNamingService;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Any;

import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XStorable;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.comp.loader.FactoryHelper;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.document.*;
import com.sun.star.comp.helper.*;
import com.sun.star.text.*;
import com.sun.star.text.XTextRange;
import com.sun.star.table.XCellRange;
//import com.sun.star.sheet.XCellRangeAddressable;
import com.sun.star.sheet.*;
import com.sun.star.sdb.*;
import com.sun.star.container.*;
import com.sun.star.container.XIndexAccess;
import java.io.*;
import java.util.*;
// import com.sun.star.lang.*;

import com.sun.star.sheet.*;
import com.sun.star.document.*;
import com.sun.star.table.*;
import com.sun.star.text.XTextRange;
//import com.sun.star.beans.*;
import com.sun.star.util.XNumberFormats;
import com.sun.star.util.XNumberFormatsSupplier;
import com.sun.star.util.NumberFormat;
import com.sun.star.lang.Locale;

import java.io.*;
import java.util.*;





/** This class capsulates the class, that implements the minimal component, a
 * factory for creating the service (<CODE>__getServiceFactory</CODE>) and a
 * method, that writes the information into the given registry key
 * (<CODE>__writeRegistryServiceInfo</CODE>).
 * @author Bertram Nolte
 */
public class CallReportWizard {

    /** Gives a factory for creating the service.
     * This method is called by the <code>JavaLoader</code>
     * <p>
     * @return Returns a <code>XSingleServiceFactory</code> for creating the
     * component.
     * @see com.sun.star.comp.loader.JavaLoader#
     * @param stringImplementationName The implementation name of the component.
     * @param xmultiservicefactory The service manager, who gives access to every
     * known service.
     * @param xregistrykey Makes structural information (except regarding tree
     * structures) of a single
     * registry key accessible.
     */
    public static XSingleServiceFactory __getServiceFactory(String stringImplementationName, XMultiServiceFactory xMSF, XRegistryKey xregistrykey)
    {
                XSingleServiceFactory xsingleservicefactory = null;
        if ( stringImplementationName.equals(
            ReportWizardImplementation.class.getName() ) )
            xsingleservicefactory = FactoryHelper.getServiceFactory(
            ReportWizardImplementation.class,
            ReportWizardImplementation.__serviceName,
            xMSF,
            xregistrykey );
            return xsingleservicefactory;
    }

    /** Writes the service information into the given registry key.
     * This method is called by the <code>JavaLoader</code>.
     * @return returns true if the operation succeeded
     * @see com.sun.star.comp.loader.JavaLoader#
     * @param xregistrykey Makes structural information (except regarding tree
     * structures) of a single
     * registry key accessible.
     */
    public static boolean __writeRegistryServiceInfo(XRegistryKey xregistrykey)
    {
        return FactoryHelper.writeRegistryServiceInfo(
                ReportWizardImplementation.class.getName(),
                ReportWizardImplementation.__serviceName,
                xregistrykey );
    }

    /** This class implements the component. At least the interfaces XServiceInfo,
     * XTypeProvider, and XInitialization should be provided by the service.
     */
    public static class ReportWizardImplementation implements XInitialization, XTypeProvider, XServiceInfo, XJobExecutor
    {

        /** The constructor of the inner class has a XMultiServiceFactory parameter.
         * @param xmultiservicefactoryInitialization A special service factory
         * could be introduced while initializing.
         */
        public ReportWizardImplementation(XMultiServiceFactory xmultiservicefactoryInitialization)
        {
            xmultiservicefactory = xmultiservicefactoryInitialization;
        }



            public void trigger(String sEvent)
                {
                        if ( sEvent.compareTo("start") == 0) {
                              try
                            {
                               XComponentLoader xcomponentloader = ( XComponentLoader ) UnoRuntime.queryInterface(XComponentLoader.class, xmultiservicefactory.createInstance("com.sun.star.frame.Desktop" ));
                                  ReportWizard.startReportWizard(xmultiservicefactory);
                            }
                         catch( Exception exception )
                            {
                                System.err.println( exception );
                            }
                        }
                        else if ( sEvent.compareTo("fill") == 0) {
                                try
                            {
                               XComponentLoader xcomponentloader = ( XComponentLoader ) UnoRuntime.queryInterface(XComponentLoader.class, xmultiservicefactory.createInstance("com.sun.star.frame.Desktop" ));
                                 Dataimport.createReport(xmultiservicefactory);
                            }
                            catch( Exception exception )
                            {
                                System.err.println( exception );
                            }
                        }
                }

        /** The service name, that must be used to get an instance of this service.
         */
        private static final String __serviceName = "com.sun.star.wizards.report.CallReportWizard";

        /** The service manager, that gives access to all registered services.
         */
        private XMultiServiceFactory xmultiservicefactory;

        /** This method is a member of the interface for initializing an object
         * directly after its creation.
         * @param object This array of arbitrary objects will be passed to the
         * component after its creation.
         * @throws Exception Every exception will not be handled, but will be
         * passed to the caller.
         */
        public void initialize(Object[] object) throws com.sun.star.uno.Exception
        {
            xmultiservicefactory = ( XMultiServiceFactory ) UnoRuntime.queryInterface(XMultiServiceFactory.class, object[ 0 ] );
        }

        /** This method returns an array of all supported service names.
         * @return Array of supported service names.
         */
        public java.lang.String[] getSupportedServiceNames()
        {
            String []stringSupportedServiceNames = new String[ 1 ];

            stringSupportedServiceNames[ 0 ] = __serviceName;

            return( stringSupportedServiceNames );
        }

        /** This method returns true, if the given service will be
         * supported by the component.
         * @param stringService Service name.
         * @return True, if the given service name will be supported.
         */
        public boolean supportsService(String stringService)
        {
            boolean booleanSupportsService = false;

            if ( stringService.equals( __serviceName ) )
            {
                booleanSupportsService = true;
            }
            return( booleanSupportsService );
        }

        /** This method returns an array of bytes, that can be used to
         * unambiguously distinguish between two sets of types, e.g.
         * to realise hashing functionality when the object is introspected.
         * Two objects that return the same ID also have to return
         * the same set of types in getTypes(). If an unique
         * implementation Id cannot be provided this method has to
         * return an empty sequence. Important: If the object
         * aggregates other objects the ID has to be unique for the
         * whole combination of objects.
         * @return Array of bytes, in order to distinguish between two sets.
         */
        public byte[] getImplementationId()
        {
            byte[] byteReturn = {};

            try
            {
                byteReturn = new String( "" + this.hashCode() ).getBytes();
            }
            catch( Exception exception ) {
                System.err.println( exception );
            }

            return( byteReturn );
        }

        /** Return the class name of the component.
         * @return Class name of the component.
         */
        public java.lang.String getImplementationName()
        {
            return( ReportWizardImplementation.class.getName() );
        }

        /** Provides a sequence of all types (usually interface types)
         * provided by the object.
         * @return Sequence of all types (usually interface types) provided by the
         * service.
         */
        public com.sun.star.uno.Type[] getTypes()
        {
            Type[] typeReturn = {};

        try
        {
            typeReturn = new Type[] {
                new Type( XJobExecutor.class ) ,
                new Type( XTypeProvider.class ),
                new Type( XServiceInfo.class ),
                new Type( XInitialization.class )
            };
        } catch( Exception exception ) {
            System.err.println( exception );
        }

        return( typeReturn );
        }
    }
}

