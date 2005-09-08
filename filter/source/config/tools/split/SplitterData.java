/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SplitterData.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:36:47 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
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
