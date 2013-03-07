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
