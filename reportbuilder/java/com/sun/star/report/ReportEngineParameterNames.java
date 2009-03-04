/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ReportEngineParameterNames.java,v $
 * $Revision: 1.5 $
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

/**
 * Making this enumeration typesafe and fully defined here would
 * be nice, wouldn't it?
 */
public class ReportEngineParameterNames
{
  public static final String CONTENT_TYPE = "content-type";
  public static final String INPUT_NAME= "input.name";
  public static final String INPUT_REPOSITORY = "input.repository";
  public static final String OUTPUT_NAME= "output.name";
  public static final String OUTPUT_REPOSITORY = "output.repository";
  public static final String INPUT_DATASOURCE_FACTORY = "input.datasource-factory";
  public static final String IMAGE_SERVICE = "ImageService";
  public static final String INPUT_REPORTJOB_FACTORY = "input.reportjob-factory";
  public static final String INPUT_MASTER_COLUMNS = "input.master-columns";
  public static final String INPUT_MASTER_VALUES = "input.master-values";
  public static final String INPUT_DETAIL_COLUMNS = "input.detail-columns";
  public static final String MIMETYPE = "output.mimetype";
  public static final String AUTHOR = "Author";
  public static final String TITLE = "Title";

  private ReportEngineParameterNames ()
  {
  }
}
