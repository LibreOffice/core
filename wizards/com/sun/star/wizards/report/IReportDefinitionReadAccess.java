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
package com.sun.star.wizards.report;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.report.XReportDefinition;

/**
 * This interface contains only one function to give access to the ReportDefinition.
 * The ReportDefinition will be initialized very late, late after the ReportLayouters.
 * So we need this interface for the late access.
 */
public interface IReportDefinitionReadAccess
{

    /**
     * Gives access to a ReportDefinition, if initialized.
     * @return a ReportDefinition or null.
     */
    public XReportDefinition getReportDefinition(); /* should throw NullPointerException but does not. */


    /**
     * This ServiceFactory is the 'global' Service Factory, which knows all and every thing in the program.
     * @return the global service factory of the program
     */
    public XMultiServiceFactory getGlobalMSF();

    /**
     * Returns the file path to the default report definition, we need this name for early initialisation
     * @return
     */
    public String getDefaultHeaderLayout();
}
