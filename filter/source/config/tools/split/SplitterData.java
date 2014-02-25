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

package com.sun.star.filter.config.tools.split;



import java.lang.*;
import java.io.*;
import com.sun.star.filter.config.tools.utils.*;



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
    public boolean m_bSeparateFiltersByModule;

    /** directories to group all filter fragments ... if requested.
     *  Must be relative to "m_aOutDir/m_aFragmentDirFilters" and
     *  will be used only, if "m_bSeparateFiltersByModule" is set to true. */
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
