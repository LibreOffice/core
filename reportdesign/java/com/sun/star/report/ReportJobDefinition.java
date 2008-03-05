/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ReportJobDefinition.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:26:33 $
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

/**
 * The report job is created by the report job factory and holds all properties
 * required to complete the reporting task.
 *
 * @author Thomas Morgner
 */
public interface ReportJobDefinition
{

    /**
     * The parameters of the root report definition. The report parameters are using
     * by the query factory to parametrize the query statement.
     *
     * The query parameters for the subreports are defined using mappings, it would not
     * make sense to define them here.
     *
     * @return a map containing the report parameters
     */
    public ParameterMap getQueryParameters();

    /**
     * The report processing parameters control the behaviour of the report. There are
     * several mandatory parameters, some optional and possibly some not-yet-understood
     * parameters. Use the engine meta data to find out, which parameters are supported.
     *
     * @return the processing parameters
     */
    public JobProperties getProcessingParameters();
}
