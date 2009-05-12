 /*
 ************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: IReportDefinitionReadAccess.java,v $
 *
 * $Revision: 1.2.36.1 $
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
 ***********************************************************************
 */
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
