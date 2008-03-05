/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PentahoReportEngineMetaData.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:29:43 $
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
package com.sun.star.report.pentaho;

import com.sun.star.report.ReportJobFactory;
import java.util.HashMap;
import java.util.HashSet;

import com.sun.star.report.DataSourceFactory;
import com.sun.star.report.InputRepository;
import com.sun.star.report.OutputRepository;
import com.sun.star.report.ReportEngineMetaData;
import com.sun.star.report.ReportEngineParameterNames;
import com.sun.star.report.ImageService;
import java.util.Vector;
import org.jfree.util.HashNMap;

public class PentahoReportEngineMetaData
        implements ReportEngineMetaData
{

    public static final String OPENDOCUMENT_TEXT = "application/vnd.oasis.opendocument.text";
    public static final String OPENDOCUMENT_SPREADSHEET = "application/vnd.oasis.opendocument.spreadsheet";
    public static final String OPENDOCUMENT_CHART = "application/vnd.oasis.opendocument.chart";
    public final static String CONTENT_TYPE = "content-type";
    public static final String DEBUG = "raw/text+xml";
    private HashSet mandatoryParameters;
    private HashMap parameterTypes;
    private HashNMap enumerationValues;

    public PentahoReportEngineMetaData()
    {
        mandatoryParameters = new HashSet();
        mandatoryParameters.add(ReportEngineParameterNames.CONTENT_TYPE);
        mandatoryParameters.add(ReportEngineParameterNames.INPUT_NAME);
        mandatoryParameters.add(ReportEngineParameterNames.INPUT_REPOSITORY);
        mandatoryParameters.add(ReportEngineParameterNames.OUTPUT_NAME);
        mandatoryParameters.add(ReportEngineParameterNames.OUTPUT_REPOSITORY);
        mandatoryParameters.add(ReportEngineParameterNames.INPUT_DATASOURCE_FACTORY);
        mandatoryParameters.add(ReportEngineParameterNames.IMAGE_SERVICE);
        mandatoryParameters.add(ReportEngineParameterNames.INPUT_REPORTJOB_FACTORY);
        mandatoryParameters.add(ReportEngineParameterNames.INPUT_MASTER_COLUMNS);
        mandatoryParameters.add(ReportEngineParameterNames.INPUT_MASTER_VALUES);
        mandatoryParameters.add(ReportEngineParameterNames.INPUT_DETAIL_COLUMNS);


        parameterTypes = new HashMap();
        parameterTypes.put(ReportEngineParameterNames.CONTENT_TYPE, String.class);
        parameterTypes.put(ReportEngineParameterNames.INPUT_NAME, String.class);
        parameterTypes.put(ReportEngineParameterNames.OUTPUT_NAME, String.class);
        parameterTypes.put(ReportEngineParameterNames.INPUT_REPOSITORY, InputRepository.class);
        parameterTypes.put(ReportEngineParameterNames.OUTPUT_REPOSITORY, OutputRepository.class);
        parameterTypes.put(ReportEngineParameterNames.INPUT_DATASOURCE_FACTORY, DataSourceFactory.class);
        parameterTypes.put(ReportEngineParameterNames.IMAGE_SERVICE, ImageService.class);
        parameterTypes.put(ReportEngineParameterNames.INPUT_REPORTJOB_FACTORY, ReportJobFactory.class);
        parameterTypes.put(ReportEngineParameterNames.INPUT_MASTER_COLUMNS, Vector.class);
        parameterTypes.put(ReportEngineParameterNames.INPUT_MASTER_VALUES, Vector.class);
        parameterTypes.put(ReportEngineParameterNames.INPUT_DETAIL_COLUMNS, Vector.class);

        enumerationValues = new HashNMap();
        enumerationValues.add(CONTENT_TYPE,
                PentahoReportEngineMetaData.OPENDOCUMENT_TEXT);
        enumerationValues.add(CONTENT_TYPE,
                PentahoReportEngineMetaData.OPENDOCUMENT_SPREADSHEET);
        enumerationValues.add(CONTENT_TYPE, PentahoReportEngineMetaData.DEBUG);
    }

    public Object[] getEnumerationValues(final String parameter)
    {
        return enumerationValues.toArray(parameter);
    }

    public Class getParameterType(final String parameter)
    {
        return (Class) parameterTypes.get(parameter);
    }

    public boolean isEnumeration(final String parameter)
    {
        return enumerationValues.containsKey(parameter);
    }

    /**
     * Checks, whether a certain output type is available. Feed the mime-type of the output
     * type in and you'll get a true or false back.
     *
     * @param mimeType
     * @return true, if the output type is supported; false otherwise.
     */
    public boolean isOutputSupported(final String mimeType)
    {
        return enumerationValues.containsValue(CONTENT_TYPE, mimeType.toLowerCase());
    }

    /**
     * Lists all supported output parameters for the given mime-type. This listing can be
     * used to build a generic user interface for configuring a certain output.
     *
     * @param mimeType
     * @return
     */
    public String[] getOutputParameters(final String mimeType)
    {
        return new String[0];
    }

    public boolean isMandatory(final String parameter)
    {
        return mandatoryParameters.contains(parameter);
    }
}
