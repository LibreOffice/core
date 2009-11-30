/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SOReportJobFactory.java,v $
 * $Revision: 1.7 $
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
package com.sun.star.report.pentaho;

import com.sun.star.beans.NamedValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.beans.PropertyVetoException;
import com.sun.star.beans.XPropertyChangeListener;
import com.sun.star.beans.XVetoableChangeListener;
import com.sun.star.embed.XStorage;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XSingleComponentFactory;
import com.sun.star.lib.uno.helper.Factory;
import com.sun.star.lib.uno.helper.PropertySetMixin;
import com.sun.star.lib.uno.helper.WeakBase;
import com.sun.star.registry.InvalidRegistryException;
import com.sun.star.registry.InvalidValueException;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.registry.XSimpleRegistry;
import com.sun.star.report.DataSourceFactory;
import com.sun.star.report.JobProperties;
import com.sun.star.report.ReportEngineParameterNames;
import com.sun.star.report.ReportExecutionException;
import com.sun.star.report.ReportJob;
import com.sun.star.report.ReportJobDefinition;
import com.sun.star.report.ReportJobFactory;
import com.sun.star.report.SDBCReportDataFactory;
import com.sun.star.report.SOImageService;
import com.sun.star.report.StorageRepository;
import com.sun.star.report.XReportDefinition;
import com.sun.star.report.pentaho.SOReportJobFactory._SOReportJobFactory;
import com.sun.star.sdbc.XConnection;
import com.sun.star.sdbc.XRowSet;
import com.sun.star.task.XJob;
import com.sun.star.uno.Exception;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import java.util.logging.Level;
import java.util.logging.Logger;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

/**
 * This class capsulates the class, that implements the minimal component, a factory for creating the service
 * (<CODE>__getComponentFactory</CODE>) and a method, that writes the information into the given registry key
 * (<CODE>__writeRegistryServiceInfo</CODE>).
 */
public class SOReportJobFactory
{

    private SOReportJobFactory()
    {
    }

    public static class _SOReportJobFactory extends WeakBase implements XInitialization, XServiceInfo, XJob, XPropertySet, ReportJobFactory
    {

        private static final Log LOGGER = LogFactory.getLog(_SOReportJobFactory.class);
        /**
         * The service name, that must be used to get an instance of this service.
         */
        private static final String __serviceName =
                                    "com.sun.star.report.pentaho.SOReportJobFactory";
        private final PropertySetMixin m_prophlp;
        /**
         * The initial component contextr, that gives access to the service manager, supported singletons, ... It's
         * often later used
         */
        private final XComponentContext m_cmpCtx;
        private XConnection activeConnection;
        private XReportDefinition report;

        public _SOReportJobFactory(final XComponentContext xCompContext)
        {
            m_cmpCtx = xCompContext;
            m_prophlp = new PropertySetMixin(m_cmpCtx, this,
                new Type(XJob.class),
                null); // no optionals
        }

        /**
         * This method is a member of the interface for initializing an object directly after its creation.
         *
         * @param object This array of arbitrary objects will be passed to the component after its creation.
         * @throws Exception Every exception will not be handled, but will be passed to the caller.
         */
        public void initialize(final Object[] object)
            throws com.sun.star.uno.Exception
        {
            /* The component describes what arguments its expected and in which
             * order!At this point you can read the objects and can intialize
             * your component using these objects.
             */
        }

        /**
         * This method returns an array of all supported service names.
         *
         * @return Array of supported service names.
         */
        public String[] getSupportedServiceNames()
        {
            return getServiceNames();
        }

        /**
         * This method is a simple helper function to used in the static component initialisation functions as well as
         * in getSupportedServiceNames.
         */
        public static String[] getServiceNames()
        {
            return new String[]
                {
                    __serviceName
                };
        }

        /**
         * This method returns true, if the given service will be supported by the component.
         *
         * @param sServiceName Service name.
         * @return True, if the given service name will be supported.
         */
        public boolean supportsService(final String sServiceName)
        {
            return sServiceName.equals(__serviceName);
        }

        /**
         * Return the class name of the component.
         *
         * @return Class name of the component.
         */
        public String getImplementationName()
        {
            return SOReportJobFactory.class.getName();
        }

        private String getLocaleFromRegistry(final XSimpleRegistry simpleReg, final String path, final String value)
        {
            String currentLocale = null;
            try
            {
                simpleReg.open(path, true, false);
                XRegistryKey xRegistryRootKey = simpleReg.getRootKey();
                // read locale
                XRegistryKey locale = xRegistryRootKey.openKey(value);
                if ( locale != null )
                {
                    final String newLocale = locale.getStringValue();
                    if ( newLocale != null )
                    {
                        currentLocale = newLocale.replace('-', '_');
                    }
                }
            }
            catch ( InvalidValueException ex )
            {
                Logger.getLogger(SOReportJobFactory.class.getName()).log(Level.SEVERE, null, ex);
            }
            catch ( InvalidRegistryException ex )
            {
                Logger.getLogger(SOReportJobFactory.class.getName()).log(Level.SEVERE, null, ex);
            }

            return currentLocale;
        }

        public Object execute(final NamedValue[] namedValue)
            throws com.sun.star.lang.IllegalArgumentException, com.sun.star.uno.Exception
        {
            final ClassLoader cl = java.lang.Thread.currentThread().getContextClassLoader();
            Thread.currentThread().setContextClassLoader(this.getClass().getClassLoader());
            try
            {
                XSimpleRegistry simpleReg = (XSimpleRegistry) UnoRuntime.queryInterface(XSimpleRegistry.class,
                    m_cmpCtx.getServiceManager().createInstanceWithContext("com.sun.star.configuration.ConfigurationRegistry", m_cmpCtx));

                String currentLocale = getLocaleFromRegistry(simpleReg,"org.openoffice.Setup","L10N/ooSetupSystemLocale");
                if ( currentLocale == null || "".equals(currentLocale) )
                {
                    currentLocale = getLocaleFromRegistry(simpleReg,"org.openoffice.Office.Linguistic","General/DefaultLocale");
                }
                if ( currentLocale != null && !"".equals(currentLocale) )
                    System.setProperty("org.pentaho.reporting.libraries.formula.locale", currentLocale);
                final ReportJob job = createReportJob(namedValue);
                job.execute();

            }
            catch ( java.lang.Exception e )
            {
                LOGGER.error("ReportProcessing failed", e);
                throw new com.sun.star.lang.WrappedTargetException(e.getMessage(), this, null);
            }
            catch ( java.lang.IncompatibleClassChangeError e2 )
            {
                LOGGER.error("Detected an IncompatibleClassChangeError");
                throw new com.sun.star.lang.WrappedTargetException("caught a " + e2.getClass().getName(), this, new com.sun.star.uno.Exception(e2.getLocalizedMessage()));
            }
            Thread.currentThread().setContextClassLoader(cl);

            return null;
        }

        public ReportJob createReportJob(final NamedValue[] namedValue) throws IllegalArgumentException, ReportExecutionException, Exception
        {
            XStorage input = null;
            XStorage output = null;
            XRowSet rowSet = null;
            String mimetype = null;
            String author = null;
            String title = null;

            for ( int i = 0; i < namedValue.length; ++i )
            {
                final NamedValue aProps = namedValue[i];
                if ( "ActiveConnection".equalsIgnoreCase(aProps.Name) )
                {
                    activeConnection = (XConnection) UnoRuntime.queryInterface(XConnection.class, aProps.Value);
                }
                else if ( "ReportDefinition".equalsIgnoreCase(aProps.Name) )
                {
                    report = (XReportDefinition) UnoRuntime.queryInterface(XReportDefinition.class, aProps.Value);
                }
                else if ( "InputStorage".equalsIgnoreCase(aProps.Name) )
                {
                    input = (XStorage) UnoRuntime.queryInterface(XStorage.class, aProps.Value);
                }
                else if ( "OutputStorage".equalsIgnoreCase(aProps.Name) )
                {
                    output = (XStorage) UnoRuntime.queryInterface(XStorage.class, aProps.Value);
                }
                else if ( "RowSet".equalsIgnoreCase(aProps.Name) )
                {
                    rowSet = (XRowSet) UnoRuntime.queryInterface(XRowSet.class, aProps.Value);
                }
                else if ( "mimetype".equalsIgnoreCase(aProps.Name) )
                {
                    mimetype = (String) aProps.Value;
                }
                else if ( ReportEngineParameterNames.AUTHOR.equalsIgnoreCase(aProps.Name) )
                {
                    author = (String) aProps.Value;
                }
                else if ( ReportEngineParameterNames.TITLE.equalsIgnoreCase(aProps.Name) )
                {
                    title = (String) aProps.Value;
                }
            }

            if ( input == null || output == null )
            {
                throw new com.sun.star.lang.IllegalArgumentException();
            }

            if ( rowSet == null )
            {
                if ( report == null || activeConnection == null )
                {
                    throw new com.sun.star.lang.IllegalArgumentException();
                }
                mimetype = report.getMimeType();
            }
            else
            {
                final XPropertySet set = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, rowSet);
                if ( set == null )
                {
                    throw new com.sun.star.lang.IllegalArgumentException();
                }
                activeConnection = (XConnection) UnoRuntime.queryInterface(XConnection.class, set.getPropertyValue("ActiveConnection"));
            }
            if ( mimetype == null )
            {
                mimetype = PentahoReportEngineMetaData.OPENDOCUMENT_TEXT;
            }

            final DataSourceFactory dataFactory = new SDBCReportDataFactory(m_cmpCtx, activeConnection);
            final StorageRepository storageRepository = new StorageRepository(input, output);

            final String inputName = "content.xml";
            final String outputName = "content.xml";

            final PentahoReportEngine engine = new PentahoReportEngine();
            final ReportJobDefinition definition = engine.createJobDefinition();
            final JobProperties procParms = definition.getProcessingParameters();
            procParms.setProperty(ReportEngineParameterNames.INPUT_REPOSITORY, storageRepository);
            procParms.setProperty(ReportEngineParameterNames.OUTPUT_REPOSITORY, storageRepository);
            procParms.setProperty(ReportEngineParameterNames.INPUT_NAME, inputName);
            procParms.setProperty(ReportEngineParameterNames.OUTPUT_NAME, outputName);
            procParms.setProperty(ReportEngineParameterNames.CONTENT_TYPE, mimetype);
            procParms.setProperty(ReportEngineParameterNames.INPUT_DATASOURCE_FACTORY, dataFactory);
            procParms.setProperty(ReportEngineParameterNames.IMAGE_SERVICE, new SOImageService(m_cmpCtx));
            procParms.setProperty(ReportEngineParameterNames.INPUT_REPORTJOB_FACTORY, this);
            if ( author != null )
            {
                procParms.setProperty(ReportEngineParameterNames.AUTHOR, author);
            }
            if ( title != null )
            {
                procParms.setProperty(ReportEngineParameterNames.TITLE, title);
            }

            return engine.createJob(definition);
        }

        // com.sun.star.beans.XPropertySet:
        public com.sun.star.beans.XPropertySetInfo getPropertySetInfo()
        {
            return m_prophlp.getPropertySetInfo();
        }

        public void setPropertyValue(final String aPropertyName, final Object aValue)
            throws UnknownPropertyException, PropertyVetoException, com.sun.star.lang.IllegalArgumentException,
                   WrappedTargetException
        {
            m_prophlp.setPropertyValue(aPropertyName, aValue);
        }

        public Object getPropertyValue(final String aPropertyName)
            throws UnknownPropertyException, WrappedTargetException
        {
            return m_prophlp.getPropertyValue(aPropertyName);
        }

        public void addPropertyChangeListener(final String aPropertyName, final XPropertyChangeListener xListener)
            throws UnknownPropertyException, WrappedTargetException
        {
            m_prophlp.addPropertyChangeListener(aPropertyName, xListener);
        }

        public void removePropertyChangeListener(final String aPropertyName, final XPropertyChangeListener xListener)
            throws UnknownPropertyException, WrappedTargetException
        {
            m_prophlp.removePropertyChangeListener(aPropertyName, xListener);
        }

        public void addVetoableChangeListener(final String aPropertyName, final XVetoableChangeListener xListener)
            throws UnknownPropertyException, WrappedTargetException
        {
            m_prophlp.addVetoableChangeListener(aPropertyName, xListener);
        }

        public void removeVetoableChangeListener(final String aPropertyName, final XVetoableChangeListener xListener)
            throws UnknownPropertyException, WrappedTargetException
        {
            m_prophlp.removeVetoableChangeListener(aPropertyName, xListener);
        }
    }

    /**
     * Gives a factory for creating the service. This method is called by the <code>JavaLoader</code>
     * <p/>
     *
     * @param sImplName the name of the implementation for which a service is desired
     * @return returns a <code>XSingleComponentFactory</code> for creating the component
     * @see com.sun.star.comp.loader.JavaLoader
     */
    public static XSingleComponentFactory __getComponentFactory(final String sImplName)
    {
        XSingleComponentFactory xFactory = null;

        try
        {
            if ( sImplName.equals(_SOReportJobFactory.class.getName()) )
            {
                xFactory = Factory.createComponentFactory(_SOReportJobFactory.class, _SOReportJobFactory.getServiceNames());
            }
            else if ( sImplName.equals(SOFunctionManager.class.getName()) )
            {
                xFactory = Factory.createComponentFactory(SOFunctionManager.class, SOFunctionManager.getServiceNames());
            }
            else if ( sImplName.equals(SOFormulaParser.class.getName()) )
            {
                xFactory = Factory.createComponentFactory(SOFormulaParser.class, SOFormulaParser.getServiceNames());
            }
        }
        catch ( java.lang.IncompatibleClassChangeError e2 )
        {
        }

        return xFactory;
    }

    /**
     * Writes the service information into the given registry key. This method is called by the <code>JavaLoader</code>
     * <p/>
     *
     * @param regKey the registryKey
     * @return returns true if the operation succeeded
     * @see com.sun.star.comp.loader.JavaLoader
     */
    public static boolean __writeRegistryServiceInfo(final XRegistryKey regKey)
    {
        return Factory.writeRegistryServiceInfo(SOFunctionManager.class.getName(),
            SOFunctionManager.getServiceNames(),
            regKey) &&
            Factory.writeRegistryServiceInfo(_SOReportJobFactory.class.getName(),
            _SOReportJobFactory.getServiceNames(),
            regKey) &&
            Factory.writeRegistryServiceInfo(SOFormulaParser.class.getName(),
            SOFormulaParser.getServiceNames(),
            regKey);
    }
}
