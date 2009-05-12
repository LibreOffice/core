/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ReportEngineMetaData.java,v $
 * $Revision: 1.4 $
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
package com.sun.star.report;

public interface ReportEngineMetaData
{

    /**
     * Checks, whether a certain output type is available.
     * Feed the mime-type of the output type in and you'll get
     * a true or false back.
     *
     * @param mimeType
     * @return true, if the output type is supported; false otherwise.
     */
    public boolean isOutputSupported(String mimeType);

    /**
     * Lists all supported output parameters for the given mime-type.
     * This listing can be used to build a generic user interface for
     * configuring a certain output.
     *
     * @param mimeType
     * @return
     */
    public String[] getOutputParameters(String mimeType);

    public Class getParameterType(String parameter);

    public boolean isMandatory(String parameter);

    public boolean isEnumeration(String parameter);

    public Object[] getEnumerationValues(String parameter);
}
