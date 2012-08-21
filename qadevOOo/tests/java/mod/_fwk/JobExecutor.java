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

package mod._fwk;

import java.io.PrintWriter;

import lib.Status;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import com.sun.star.beans.NamedValue;
import com.sun.star.beans.PropertyValue;
import com.sun.star.container.XHierarchicalNameAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNameContainer;
import com.sun.star.container.XNameReplace;
import com.sun.star.container.XNamed;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.task.XJob;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.XChangesBatch;
import com.sun.star.uno.AnyConverter;

/**
 * Test for object that implements the following interfaces :
 * <ul>
 *  <li><code>com::sun::star::task::XJobExecutor</code></li>
 * </ul><p>
 * The following files are needed for testcase creation :
 * <ul>
 *  <li> <code>qadevlibs/JobExecutor.jar</code> :
 *      this is java component <code>test.Job</code> which
 *      should be registered before or during this testcase
 *      creation. This component must implement
 *      <code>com.sun.star.task.XJob</code> interface and count
 *      <code>execute</code> method calls. It also should
 *      implement <code>container.XNamed</code> interface and
 *      <code>getName</code> method must return string with number
 *      of calls.</li>
 * <ul> <p>
 *
 * Also <b>important prerequicity</b>: if Job and Event is not yet
 * added to configuration or <code>test.Job</code> component is
 * not yet registered, the SOffice <b>must be destroyed</b> during
 * testcase initialization. <p>
 *
 * @see com.sun.star.task.XJobExecutor
 * @see ifc.task._XJobExecutor
 */
public class JobExecutor extends TestCase {

    static Job job = new Job();
    XNamed xNamed = null;
    Object oRootCfg = null;

    /**
     * For testcase initializing :
     * <ol>
     *  <li> Implementation <code>test.Job</code> must be registered in
     *      SOffice registry. </li>
     *  <li> Configuration in package <code>org.OpenOffice.Office.Jobs</code>
     *      must be updated. <code>TestJob</code> must be registered for
     *      service <code>test.Job</code> and event for this job named
     *      <code>TextEvent</code> must be registered. </li>
     * </ol>. <p>
     *
     * First these two conditions are checked. If job and event are not
     * registered they are inserted into configuration and commited.
     * After what SOffice must be destroyed for proper initialization
     * of <code>JobExecutor</code> after startup. <p>
     *
     * Then if the implementation was not registered before it is
     * registered in soffice <code>applicat.rbd</code> file (registering
     * the component in currently running Java environment has no effect
     * for <code>JobExecutor</code> in some reasons). <p>
     *
     * Note: SOffice is started again while the next
     * <code>(XMultiServiceFactory)SOLink.getMSF()</code>  call.
     */
    protected void initialize(TestParameters Param, PrintWriter log) {
        boolean serviceRegistered = false;
        boolean configured = false;

        try {
            Object obj = ((XMultiServiceFactory)Param.getMSF()).createInstance("test.Job");
            serviceRegistered = obj != null;
        } catch(com.sun.star.uno.Exception e) {}

        log.println("Service test.Job is "
            + (serviceRegistered ? "already" : "not yet")  + " registered.");
        if (! serviceRegistered){
            String message = "You have to register 'test.Job' before office is stared.\n";
            message += "Please run '$OFFICEPATH/program/pkgchk $DOCPTH/qadevlibs/JobExecutor.jar'";
            throw new StatusException(message, new Exception());
        }


        XNameAccess jobs = null;
        XNameAccess events = null;
        try {
            Object obj = ((XMultiServiceFactory)Param.getMSF()).createInstance
                ("com.sun.star.configuration.ConfigurationProvider");
            XMultiServiceFactory xConfigMSF = UnoRuntime.queryInterface(XMultiServiceFactory.class, obj);
            PropertyValue[] args = new PropertyValue[1];
            args[0] = new PropertyValue();
            args[0].Name = "nodepath";
            args[0].Value = "org.openoffice.Office.Jobs";
            oRootCfg = xConfigMSF.createInstanceWithArguments(
                "com.sun.star.configuration.ConfigurationUpdateAccess", args);
            XHierarchicalNameAccess xHNA = UnoRuntime.queryInterface(XHierarchicalNameAccess.class, oRootCfg);
            obj = xHNA.getByHierarchicalName("Jobs");
            jobs = UnoRuntime.queryInterface
                (XNameAccess.class, obj);
            obj = xHNA.getByHierarchicalName("Events");
            events = UnoRuntime.queryInterface
                (XNameAccess.class, obj);
        } catch (Exception e) {
            throw new StatusException("Couldn't get configuration", e);
        }

        configured = jobs.hasByName("TestJob") && events.hasByName("TestEvent");

        log.println("Test job and event is "
            + (configured ? "already" : "not yet")  + " configured.");

        if (!configured) {
            try {
                log.println("Adding configuration to Jobs  ...");
                XSingleServiceFactory jobsFac = UnoRuntime.queryInterface(XSingleServiceFactory.class, jobs);
                Object oNewJob = jobsFac.createInstance();
                XNameReplace xNewJobNR = UnoRuntime.queryInterface(XNameReplace.class, oNewJob);
                xNewJobNR.replaceByName("Service", "test.Job");
                XNameContainer xJobsNC = UnoRuntime.queryInterface(XNameContainer.class, jobs);
                xJobsNC.insertByName("TestJob", oNewJob);

                log.println("Adding configuration to Events  ...");
                XSingleServiceFactory eventsFac = UnoRuntime.queryInterface(XSingleServiceFactory.class, events);
                Object oNewEvent = eventsFac.createInstance();

                XNameAccess xNewEventNA = UnoRuntime.queryInterface(XNameAccess.class, oNewEvent);
                Object oJobList = xNewEventNA.getByName("JobList");
                XSingleServiceFactory jobListFac = (XSingleServiceFactory)
                    AnyConverter.toObject(new Type(XSingleServiceFactory.class),
                    oJobList);
                XNameContainer jobListNC = (XNameContainer)
                    AnyConverter.toObject(new Type(XNameContainer.class),
                    oJobList);
                log.println("\tAdding TimeStamps to Events ...");
                Object oNewJobTimeStamps = jobListFac.createInstance();

                jobListNC.insertByName("TestJob",  oNewJobTimeStamps);


                XNameContainer xEventsNC = UnoRuntime.queryInterface(XNameContainer.class, events);
                xEventsNC.insertByName("TestEvent", oNewEvent);

                XChangesBatch xCB = UnoRuntime.queryInterface(XChangesBatch.class, oRootCfg);
                xCB.commitChanges();

                try {
                    Thread.sleep(1000);
                } catch (InterruptedException ex) {}

            } catch (com.sun.star.uno.Exception e) {
                e.printStackTrace(log);
                throw new StatusException("Couldn't change config", e);
            }
        }

    }

    /**
     * Creating a Testenvironment for the interfaces to be tested.
     *
     * Service <code>com.sun.star.comp.framework.JobExecutor</code>
     * is created.
     */
    protected TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        XInterface oObj = null;

        Object job = null;
        try {
            oObj = (XInterface)((XMultiServiceFactory)Param.getMSF()).createInstance(
                "com.sun.star.comp.framework.JobExecutor");
            job = ((XMultiServiceFactory)Param.getMSF()).createInstance("test.Job");
        } catch(com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException(
                Status.failed("Couldn't create instance"));
        }

        xNamed = UnoRuntime.queryInterface(XNamed.class, job);
        log.println("Count = " + xNamed.getName());

        TestEnvironment tEnv = new TestEnvironment( oObj );

        tEnv.addObjRelation("CallCounter", xNamed);

        return tEnv;
    } // finish method getTestEnvironment

    protected void cleanup( TestParameters Param, PrintWriter log) {
    }
}

/**
 * Currently not used.
 */
class Job implements
        XServiceInfo, XSingleServiceFactory {

    private static class Impl implements XServiceInfo, XTypeProvider, XJob, XNamed {
        int callCount = 0;

        public byte[] getImplementationId() {
            return toString().getBytes();
        }

        public Type[] getTypes() {
            Class<?> interfaces[] = getClass().getInterfaces();
            Type types[] = new Type[interfaces.length];
            for(int i = 0; i < interfaces.length; ++ i)
                types[i] = new Type(interfaces[i]);
            return types;
        }

        public Object execute(NamedValue[] param) {
            callCount++;

            return null;
        }

        public String getName() {
            return String.valueOf(callCount);
        }

        public void setName(String n) {}

        public boolean supportsService(String name) {
            return __serviceName.equals(name);
        }

        public String[] getSupportedServiceNames() {
            return new String[] {__serviceName};
        }

        public String getImplementationName() {
            return getClass().getName();
        }
    }

    public static final String __serviceName = "test.Job";
    static Impl impl = new Impl();

    public Object createInstanceWithArguments(Object[] args) {
    return impl;
    }

    public Object createInstance() {
        return createInstanceWithArguments(null);
    }

    public boolean supportsService(String name) {
        return __serviceName.equals(name);
    }

    public String[] getSupportedServiceNames() {
        return new String[] {__serviceName};
    }

    public String getImplementationName() {
        return getClass().getName();
    }
}
