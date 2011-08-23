/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

package com.sun.star.filter.config.tools.split;

//_______________________________________________

import java.lang.*;
import java.io.*;
import com.sun.star.filter.config.tools.utils.*;

//_______________________________________________

/**
 *  Data container for class Splitter.
 *
 *
 */
public class SplitterData
{
    /** can be used to generate some debug output. */
    public Logger m_aDebug;

    /** contains all configuration structures, for which the xml
        fragments should be generated. */
    public Cache m_aCache;

    /** specify the output xml format. */
    public int m_nFormat;

    /** specify the encoding for the output xml files. */
    public java.lang.String m_sEncoding;

    /** directory to generate some generic views. */
    public java.io.File m_aOutDir;

    /** directories to generate all xml fragments there.
     *  Must be relative to "m_aOutDir"! */
    public java.io.File m_aFragmentDirTypes;
    public java.io.File m_aFragmentDirFilters;
    public java.io.File m_aFragmentDirDetectServices;
    public java.io.File m_aFragmentDirFrameLoaders;
    public java.io.File m_aFragmentDirContentHandlers;

    /** enable/disable grouping of filters by its application modules. */
    public boolean m_bSeperateFiltersByModule;

    /** directories to group all filter fragments ... if requested.
     *  Must be relative to "m_aOutDir/m_aFragmentDirFilters" and
     *  will be used only, if "m_bSeperateFiltersByModule" is set to true. */
    public java.io.File m_aFragmentDirModuleSWriter;
    public java.io.File m_aFragmentDirModuleSWeb;
    public java.io.File m_aFragmentDirModuleSGlobal;
    public java.io.File m_aFragmentDirModuleSCalc;
    public java.io.File m_aFragmentDirModuleSDraw;
    public java.io.File m_aFragmentDirModuleSImpress;
    public java.io.File m_aFragmentDirModuleSMath;
    public java.io.File m_aFragmentDirModuleSChart;
    public java.io.File m_aFragmentDirModuleOthers;

    /** file extension for generated xml fragments. */
    public java.lang.String m_sFragmentExtension;
}
