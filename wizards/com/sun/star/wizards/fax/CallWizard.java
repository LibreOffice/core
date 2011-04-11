/*
 ************************************************************************
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
package com.sun.star.wizards.fax;

import com.sun.star.beans.XPropertyAccess;
import com.sun.star.comp.loader.FactoryHelper;
import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.task.XJob;
import com.sun.star.task.XJobExecutor;
import com.sun.star.uno.Type;

/**
 * This class capsulates the class, that implements the minimal component, a factory for
 * creating the service (<CODE>__getServiceFactory</CODE>).
 *
 * @author $author$
 * @version $Revision: 1.3.192.1 $
 */
public class CallWizard
{

    /**
     * Gives a factory for creating the service. This method is called by the
     * <code>JavaLoader</code>
     *
     * <p></p>
     *
     * @param stringImplementationName The implementation name of the component.
     * @param xMSF The service manager, who gives access to every known service.
     * @param xregistrykey Makes structural information (except regarding tree
     *        structures) of a single registry key accessible.
     *
     * @return Returns a <code>XSingleServiceFactory</code> for creating the component.
     *
     * @see com.sun.star.comp.loader.JavaLoader#
     */
    public static XSingleServiceFactory __getServiceFactory(String stringImplementationName, XMultiServiceFactory xMSF, XRegistryKey xregistrykey)
    {
        XSingleServiceFactory xsingleservicefactory = null;

        if (stringImplementationName.equals(WizardImplementation.class.getName()))
        {
            xsingleservicefactory = FactoryHelper.getServiceFactory(WizardImplementation.class, WizardImplementation.__serviceName, xMSF, xregistrykey);
        }

        return xsingleservicefactory;
    }

    /**
     * This class implements the component. At least the interfaces XServiceInfo,
     * XTypeProvider, and XInitialization should be provided by the service.
     */
    public static class WizardImplementation implements XInitialization, XTypeProvider, XServiceInfo, XJobExecutor
    {

        /**
         * The constructor of the inner class has a XMultiServiceFactory parameter.
         *
         * @param xmultiservicefactoryInitialization A special service factory could be
         *        introduced while initializing.
         */
        public WizardImplementation(XMultiServiceFactory xmultiservicefactoryInitialization)
        {
            xmultiservicefactory = xmultiservicefactoryInitialization;

            if (xmultiservicefactory != null)
            {
            }
        }

        /**
         * Execute Wizard
         *
         * @param str only valid parameter is 'start' at the moment.
         */
        public void trigger(String str)
        {
            if (str.equalsIgnoreCase("start"))
            {
                FaxWizardDialogImpl lw = new FaxWizardDialogImpl(xmultiservicefactory);
                if (!FaxWizardDialogImpl.running)
                {
                    lw.startWizard(xmultiservicefactory, null);
                }
            }
        }        //*******************************************
        /**
         * The service name, that must be used to get an instance of this service.
         */
        private static final String __serviceName = "com.sun.star.wizards.fax.CallWizard";
        /**
         * The service manager, that gives access to all registered services.
         */
        private XMultiServiceFactory xmultiservicefactory;

        /**
         * This method is a member of the interface for initializing an object directly
         * after its creation.
         *
         * @param object This array of arbitrary objects will be passed to the component
         *        after its creation.
         *
         * @throws com.sun.star.uno.Exception Every exception will not be handled, but
         *         will be passed to the caller.
         */
        public void initialize(Object[] object) throws com.sun.star.uno.Exception
        {
        }

        /**
         * This method returns an array of all supported service names.
         *
         * @return Array of supported service names.
         */
        public java.lang.String[] getSupportedServiceNames()
        {
            String[] stringSupportedServiceNames = new String[1];
            stringSupportedServiceNames[0] = __serviceName;

            return (stringSupportedServiceNames);
        }

        /**
         * This method returns true, if the given service will be supported by the
         * component.
         *
         * @param stringService Service name.
         *
         * @return True, if the given service name will be supported.
         */
        public boolean supportsService(String stringService)
        {
            boolean booleanSupportsService = false;

            if (stringService.equals(__serviceName))
            {
                booleanSupportsService = true;
            }

            return (booleanSupportsService);
        }

        /**
         * This method returns an array of bytes, that can be used to unambiguously
         * distinguish between two sets of types, e.g. to realise hashing functionality
         * when the object is introspected. Two objects that return the same ID also
         * have to return the same set of types in getTypes(). If an unique
         * implementation Id cannot be provided this method has to return an empty
         * sequence. Important: If the object aggregates other objects the ID has to be
         * unique for the whole combination of objects.
         *
         * @return Array of bytes, in order to distinguish between two sets.
         */
        public byte[] getImplementationId()
        {
            byte[] byteReturn =
            {
            };

            try
            {
                byteReturn = ("" + this.hashCode()).getBytes();
            }
            catch (Exception exception)
            {
                System.err.println(exception);
            }

            return (byteReturn);
        }

        /**
         * Return the class name of the component.
         *
         * @return Class name of the component.
         */
        public java.lang.String getImplementationName()
        {
            return (WizardImplementation.class.getName());
        }

        /**
         * Provides a sequence of all types (usually interface types) provided by the
         * object.
         *
         * @return Sequence of all types (usually interface types) provided by the
         *         service.
         */
        public com.sun.star.uno.Type[] getTypes()
        {
            Type[] typeReturn =
            {
            };

            try
            {
                typeReturn = new Type[]
                        {
                            new Type(XPropertyAccess.class), new Type(XJob.class), new Type(XJobExecutor.class), new Type(XTypeProvider.class), new Type(XServiceInfo.class), new Type(XInitialization.class)
                        };
            }
            catch (Exception exception)
            {
                System.err.println(exception);
            }

            return (typeReturn);
        }
    }
}
