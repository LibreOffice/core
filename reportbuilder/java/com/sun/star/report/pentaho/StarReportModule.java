/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


package com.sun.star.report.pentaho;

import org.pentaho.reporting.libraries.base.boot.AbstractModule;
import org.pentaho.reporting.libraries.base.boot.ModuleInitializeException;
import org.pentaho.reporting.libraries.base.boot.SubSystem;

public class StarReportModule extends AbstractModule
{

    public StarReportModule()
            throws ModuleInitializeException
    {
        loadModuleInfo();
    }

    /**
     * Initializes the module. Use this method to perform all initial setup operations. This
     * method is called only once in a modules lifetime. If the initializing cannot be
     * completed, throw a ModuleInitializeException to indicate the error,. The module will
     * not be available to the system.
     *
     * @param subSystem the subSystem.
     * @throws org.jfree.base.modules.ModuleInitializeException
     *          if an error ocurred while initializing the module.
     */
    public void initialize(final SubSystem subSystem)
            throws ModuleInitializeException
    {
    }
}
