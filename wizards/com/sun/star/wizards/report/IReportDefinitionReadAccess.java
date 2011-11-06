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


package com.sun.star.wizards.report;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.report.XReportDefinition;

/**
 * This interface contains only one function to give access to the ReportDefinition.
 * The ReportDefinition will be initialized very late, late after the ReportLayouters.
 * So we need this interface for the late access.
 * @author ll93751
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
