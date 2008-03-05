/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ReportEngineMetaData.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:24:59 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *    Copyright 2007 by Pentaho Corporation
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
