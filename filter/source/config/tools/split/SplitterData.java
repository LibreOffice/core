/*************************************************************************
 *
 *  $RCSfile: SplitterData.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2004-06-10 15:56:11 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
