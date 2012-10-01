/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
package com.sun.star.wizards.query;

import com.sun.star.beans.PropertyAttribute;
import com.sun.star.beans.PropertyValue;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.Type;
import com.sun.star.wizards.common.Properties;
import com.sun.star.wizards.common.PropertyNames;

/** This class capsulates the class, that implements the minimal component, a
 * factory for creating the service (<CODE>__getServiceFactory</CODE>).
 */
public class CallQueryWizard
{

    /** Gives a factory for creating the service.
     * This method is called by the <code>JavaLoader</code>
     * <p>
     * @return Returns a <code>XSingleServiceFactory</code> for creating the component.
     * @see com.sun.star.comp.loader.JavaLoader
     * @param stringImplementationName The implementation name of the component.
     * @param xMSF The service manager, who gives access to every known service.
     * @param xregistrykey Makes structural information (except regarding tree structures) of a single
     *   registry key accessible.
     */
    public static com.sun.star.lang.XSingleServiceFactory __getServiceFactory(String stringImplementationName, com.sun.star.lang.XMultiServiceFactory xMSF, com.sun.star.registry.XRegistryKey xregistrykey)
    {
        com.sun.star.lang.XSingleServiceFactory xsingleservicefactory = null;
        if (stringImplementationName.equals(QueryWizardImplementation.class.getName()))
        {
            xsingleservicefactory = com.sun.star.comp.loader.FactoryHelper.getServiceFactory(QueryWizardImplementation.class, QueryWizardImplementation.__serviceName, xMSF, xregistrykey);
        }
        return xsingleservicefactory;
    }

    /** This class implements the component. At least the interfaces XServiceInfo,
     * XTypeProvider, and XInitialization should be provided by the service.
     */
    public static class QueryWizardImplementation extends com.sun.star.lib.uno.helper.PropertySet implements com.sun.star.lang.XInitialization, com.sun.star.lang.XServiceInfo, com.sun.star.task.XJobExecutor
    {
        private PropertyValue[] m_wizardContext;
        public String           Command;
        public final Integer    CommandType = com.sun.star.sdb.CommandType.QUERY;

        /** The constructor of the inner class has a XMultiServiceFactory parameter.
         * @param i_serviceFactory A special service factory could be introduced while initializing.
         */
        public QueryWizardImplementation( XMultiServiceFactory i_serviceFactory )
        {
            super();
            m_serviceFactory = i_serviceFactory;
            registerProperty( PropertyNames.COMMAND, (short)( PropertyAttribute.READONLY | PropertyAttribute.MAYBEVOID ) );
            registerProperty( PropertyNames.COMMAND_TYPE, PropertyAttribute.READONLY );
        }

        public void trigger(String sEvent)
        {
            try
            {
                if (sEvent.compareTo(PropertyNames.START) == 0)
                {
                    QueryWizard CurQueryWizard = new QueryWizard( m_serviceFactory, m_wizardContext );
                    Command = CurQueryWizard.start();
                }
            }
            catch (Exception exception)
            {
                System.err.println(exception);
            }
            System.gc();
        }
        /** The service name, that must be used to get an instance of this service.
         */
        private static final String __serviceName = "com.sun.star.wizards.query.CallQueryWizard";
        /** The service manager, that gives access to all registered services.
         */
        private com.sun.star.lang.XMultiServiceFactory m_serviceFactory;

        /** This method is a member of the interface for initializing an object
         * directly after its creation.
         * @param object This array of arbitrary objects will be passed to the
         * component after its creation.
         * @throws com.sun.star.uno.Exception Every exception will not be handled, but will be
         * passed to the caller.
         */
        public void initialize(Object[] object) throws com.sun.star.uno.Exception
        {
            m_wizardContext = Properties.convertToPropertyValueArray(object);
        }

        /** This method returns an array of all supported service names.
         * @return Array of supported service names.
         */
        public java.lang.String[] getSupportedServiceNames()
        {
            String[] stringSupportedServiceNames = new String[1];

            stringSupportedServiceNames[0] = __serviceName;

            return (stringSupportedServiceNames);
        }

        /** This method returns true, if the given service will be
         * supported by the component.
         * @param stringService Service name.
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
        @Override
        public byte[] getImplementationId()
        {
            byte[] byteReturn =
            {
            };

            try
            {
                byteReturn = (PropertyNames.EMPTY_STRING + this.hashCode()).getBytes();
            }
            catch (Exception exception)
            {
                System.err.println(exception);
            }

            return (byteReturn);
        }

        /** Return the class name of the component.
         * @return Class name of the component.
         */
        public java.lang.String getImplementationName()
        {
            return (QueryWizardImplementation.class.getName());
        }

        /** Provides a sequence of all types (usually interface types)
         * provided by the object.
         * @return Sequence of all types (usually interface types) provided by the
         * service.
         */
        @Override
        public Type[] getTypes()
        {
            Type[] typeReturn =
            {
            };

            try
            {
                typeReturn = new Type[]
                        {
                            new Type(com.sun.star.task.XJobExecutor.class), new Type(com.sun.star.lang.XTypeProvider.class), new Type(com.sun.star.lang.XServiceInfo.class), new Type(com.sun.star.lang.XInitialization.class)
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
