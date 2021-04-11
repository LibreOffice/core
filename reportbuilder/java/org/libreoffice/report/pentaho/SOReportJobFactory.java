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
package org.libreoffice.report.pentaho;

import com.sun.star.beans.NamedValue;
import com.sun.star.beans.PropertyVetoException;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.beans.XPropertyChangeListener;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XVetoableChangeListener;
import com.sun.star.container.XChild;
import com.sun.star.embed.XStorage;
import com.sun.star.frame.XModel;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XSingleComponentFactory;
import com.sun.star.lib.uno.helper.Factory;
import com.sun.star.lib.uno.helper.PropertySetMixin;
import com.sun.star.lib.uno.helper.WeakBase;
import com.sun.star.registry.InvalidRegistryException;
import com.sun.star.registry.InvalidValueException;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.registry.XSimpleRegistry;
import com.sun.star.sdbc.SQLException;
import org.jfree.report.ReportDataFactoryException;
import org.libreoffice.report.DataSourceException;
import org.libreoffice.report.DataSourceFactory;
import org.libreoffice.report.JobProperties;
import org.libreoffice.report.ReportEngineParameterNames;
import org.libreoffice.report.ReportExecutionException;
import org.libreoffice.report.ReportJob;
import org.libreoffice.report.ReportJobDefinition;
import org.libreoffice.report.ReportJobFactory;
import org.libreoffice.report.SDBCReportDataFactory;
import org.libreoffice.report.SOImageService;
import org.libreoffice.report.StorageRepository;
import com.sun.star.report.XReportDefinition;
import com.sun.star.sdb.XDocumentDataSource;
import com.sun.star.sdbc.XConnection;
import com.sun.star.sdbc.XRowSet;
import com.sun.star.task.XJob;
import com.sun.star.uno.Exception;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;

import java.util.logging.Level;
import java.util.logging.Logger;

import java.io.Writer;
import java.io.PrintWriter;
import java.io.StringWriter;

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

        private static final Logger LOGGER = Logger.getLogger(_SOReportJobFactory.class.getName());
        /**
         * The service name, that must be used to get an instance of this service.
         */
        private static final String __serviceName =
                "org.libreoffice.report.pentaho.SOReportJobFactory";
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
            /* The component describes what arguments are expected and in which
             * order! At this point you can read the objects and initialize
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
        private static String[] getServiceNames()
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
                final XRegistryKey xRegistryRootKey = simpleReg.getRootKey();
                // read locale
                final XRegistryKey locale = xRegistryRootKey.openKey(value);
                if (locale != null)
                {
                    final String newLocale = locale.getStringValue();
                    if (newLocale != null)
                    {
                        currentLocale = newLocale.replace('-', '_');
                    }
                }
            }
            catch (InvalidValueException ex)
            {
                Logger.getLogger(SOReportJobFactory.class.getName()).log(Level.SEVERE, null, ex);
            }
            catch (InvalidRegistryException ex)
            {
                Logger.getLogger(SOReportJobFactory.class.getName()).log(Level.SEVERE, null, ex);
            }

            return currentLocale;
        }

        // tdf#94446 if this is a SQLException in disguise, throw that
        // original exception instead of the wrapper exception, so that
        // dbaccess can apply its special handling for
        // SQLException::ErrorCode of dbtools::ParameterInteractionCancelled
        // in OLinkedDocumentsAccess::open if ParameterInteractionCancelled
        // was the root cause
        public void rethrow_sql_exception(Throwable exception)
                throws com.sun.star.sdbc.SQLException
        {
            if (exception instanceof ReportDataFactoryException == false)
                return;
            exception = ((ReportDataFactoryException)exception).getParent();
            if (exception instanceof DataSourceException == false)
                return;
            exception = ((DataSourceException)exception).getCause();
            if (exception instanceof SQLException == false)
                return;
            throw (SQLException)exception;
        }

        public Object execute(final NamedValue[] namedValue)
                throws com.sun.star.lang.IllegalArgumentException, com.sun.star.uno.Exception
        {
            final ClassLoader cl = java.lang.Thread.currentThread().getContextClassLoader();
            Thread.currentThread().setContextClassLoader(this.getClass().getClassLoader());
            try
            {
                final XSimpleRegistry simpleReg = UnoRuntime.queryInterface(XSimpleRegistry.class,
                        m_cmpCtx.getServiceManager().createInstanceWithContext("com.sun.star.configuration.ConfigurationRegistry", m_cmpCtx));

                String currentLocale = getLocaleFromRegistry(simpleReg, "org.openoffice.Setup", "L10N/ooSetupSystemLocale");
                if (currentLocale == null || "".equals(currentLocale))
                {
                    currentLocale = getLocaleFromRegistry(simpleReg, "org.openoffice.Office.Linguistic", "General/DefaultLocale");
                }
                if (currentLocale != null && !"".equals(currentLocale))
                {
                    System.setProperty("org.pentaho.reporting.libraries.formula.locale", currentLocale);
                }
                final ReportJob job = createReportJob(namedValue);
                job.execute();

            }
            catch (java.lang.Exception e)
            {
                LOGGER.severe("ReportProcessing failed: " + e);
                Writer result = new StringWriter();
                PrintWriter printWriter = new PrintWriter(result);
                e.printStackTrace(printWriter);

                // if this is a wrapped SQLException, rethrow that instead
                rethrow_sql_exception(e.getCause());

                throw new com.sun.star.lang.WrappedTargetException(e, e.toString() + '\n' + result.toString(), this, null);
            }
            catch (java.lang.IncompatibleClassChangeError e)
            {
                LOGGER.severe("Detected an IncompatibleClassChangeError");
                Writer result = new StringWriter();
                PrintWriter printWriter = new PrintWriter(result);
                e.printStackTrace(printWriter);
                throw new com.sun.star.lang.WrappedTargetException(e, e.toString() + '\n' + result.toString(), this, null);
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
            Integer maxRows = null;

            for (int i = 0; i < namedValue.length; ++i)
            {
                final NamedValue aProps = namedValue[i];
                if ("ActiveConnection".equalsIgnoreCase(aProps.Name))
                {
                    activeConnection = UnoRuntime.queryInterface(XConnection.class, aProps.Value);
                }
                else if ("ReportDefinition".equalsIgnoreCase(aProps.Name))
                {
                    report = UnoRuntime.queryInterface(XReportDefinition.class, aProps.Value);
                }
                else if ("InputStorage".equalsIgnoreCase(aProps.Name))
                {
                    input = UnoRuntime.queryInterface(XStorage.class, aProps.Value);
                }
                else if ("OutputStorage".equalsIgnoreCase(aProps.Name))
                {
                    output = UnoRuntime.queryInterface(XStorage.class, aProps.Value);
                }
                else if ("RowSet".equalsIgnoreCase(aProps.Name))
                {
                    rowSet = UnoRuntime.queryInterface(XRowSet.class, aProps.Value);
                }
                else if ("mimetype".equalsIgnoreCase(aProps.Name))
                {
                    mimetype = (String) aProps.Value;
                }
                else if ("MaxRows".equalsIgnoreCase(aProps.Name))
                {
                    maxRows = (Integer) aProps.Value;
                }
                else if (ReportEngineParameterNames.AUTHOR.equalsIgnoreCase(aProps.Name))
                {
                    author = (String) aProps.Value;
                }
                else if (ReportEngineParameterNames.TITLE.equalsIgnoreCase(aProps.Name))
                {
                    title = (String) aProps.Value;
                }
            }

            if (input == null || output == null)
            {
                throw new com.sun.star.lang.IllegalArgumentException();
            }

            if (rowSet == null)
            {
                if (report == null || activeConnection == null)
                {
                    throw new com.sun.star.lang.IllegalArgumentException();
                }
                mimetype = report.getMimeType();
            }
            else
            {
                final XPropertySet set = UnoRuntime.queryInterface(XPropertySet.class, rowSet);
                if (set == null)
                {
                    throw new com.sun.star.lang.IllegalArgumentException();
                }
                activeConnection = UnoRuntime.queryInterface(XConnection.class, set.getPropertyValue("ActiveConnection"));
            }
            if (mimetype == null)
            {
                mimetype = PentahoReportEngineMetaData.OPENDOCUMENT_TEXT;
            }

            final XChild child = UnoRuntime.queryInterface(XChild.class, activeConnection);
            final XDocumentDataSource docSource = UnoRuntime.queryInterface(XDocumentDataSource.class, child.getParent());
            final XModel model = UnoRuntime.queryInterface(XModel.class, docSource.getDatabaseDocument());
            final DataSourceFactory dataFactory = new SDBCReportDataFactory(m_cmpCtx, activeConnection);
            final StorageRepository storageRepository = new StorageRepository(input, output, model.getURL());

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
            procParms.setProperty(ReportEngineParameterNames.MAXROWS, maxRows);
            if (author != null)
            {
                procParms.setProperty(ReportEngineParameterNames.AUTHOR, author);
            }
            if (title != null)
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
     * Gives a factory for creating the service. This method is called by the <code>JavaLoader</code>.
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
            if (sImplName.equals(_SOReportJobFactory.class.getName()))
            {
                xFactory = Factory.createComponentFactory(_SOReportJobFactory.class, _SOReportJobFactory.getServiceNames());
            }
            else if (sImplName.equals(SOFunctionManager.class.getName()))
            {
                xFactory = Factory.createComponentFactory(SOFunctionManager.class, SOFunctionManager.getServiceNames());
            }
            else if (sImplName.equals(SOFormulaParser.class.getName()))
            {
                xFactory = Factory.createComponentFactory(SOFormulaParser.class, SOFormulaParser.getServiceNames());
            }
        }
        catch (java.lang.IncompatibleClassChangeError e2)
        {
        }

        return xFactory;
    }

}
