/*************************************************************************
 *
 *  $RCSfile: Merger.java,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2004-06-10 15:55:27 $
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
package com.sun.star.filter.config.tools.merge;

//_______________________________________________

import java.lang.*;
import java.util.*;
import java.io.*;
import com.sun.star.filter.config.tools.utils.*;

//_______________________________________________

/** can merge different xml fragments together.
 *
 *
 */
public class Merger
{
    //___________________________________________
    // const

    private static final java.lang.String PROP_XMLVERSION      = "xmlversion"               ; // <= global cfg file
    private static final java.lang.String PROP_XMLENCODING     = "xmlencoding"              ; // <= global cfg file
    private static final java.lang.String PROP_XMLPATH         = "xmlpath"                  ; // <= global cfg file
    private static final java.lang.String PROP_XMLPACKAGE      = "xmlpackage"               ; // <= global cfg file

    private static final java.lang.String PROP_SETNAME_TYPES    = "setname_types"           ; // <= global cfg file
    private static final java.lang.String PROP_SETNAME_FILTERS  = "setname_filters"         ; // <= global cfg file
    private static final java.lang.String PROP_SETNAME_LOADERS  = "setname_frameloaders"    ; // <= global cfg file
    private static final java.lang.String PROP_SETNAME_HANDLERS = "setname_contenthandlers" ; // <= global cfg file

    private static final java.lang.String PROP_SUBDIR_TYPES    = "subdir_types"             ; // <= global cfg file
    private static final java.lang.String PROP_SUBDIR_FILTERS  = "subdir_filters"           ; // <= global cfg file
    private static final java.lang.String PROP_SUBDIR_LOADERS  = "subdir_frameloaders"      ; // <= global cfg file
    private static final java.lang.String PROP_SUBDIR_HANDLERS = "subdir_contenthandlers"   ; // <= global cfg file

    private static final java.lang.String PROP_EXTENSION_XCU   = "extension_xcu"            ; // <= global cfg file
    private static final java.lang.String PROP_EXTENSION_PKG   = "extension_pkg"            ; // <= global cfg file

    private static final java.lang.String PROP_DELIMITER       = "delimiter"                ; // <= global cfg file
    private static final java.lang.String PROP_TRIM            = "trim"                     ; // <= global cfg file
    private static final java.lang.String PROP_DECODE          = "decode"                   ; // <= global cfg file

    private static final java.lang.String PROP_FRAGMENTSDIR    = "fragmentsdir"             ; // <= cmdline
    private static final java.lang.String PROP_TEMPDIR         = "tempdir"                  ; // <= cmdline
    private static final java.lang.String PROP_OUTDIR          = "outdir"                   ; // <= cmdline
    private static final java.lang.String PROP_PKG             = "pkg"                      ; // <= cmdline

    private static final java.lang.String PROP_TCFG            = "tcfg"                     ; // <= cmdline
    private static final java.lang.String PROP_FCFG            = "fcfg"                     ; // <= cmdline
    private static final java.lang.String PROP_LCFG            = "lcfg"                     ; // <= cmdline
    private static final java.lang.String PROP_CCFG            = "ccfg"                     ; // <= cmdline

    private static final java.lang.String PROP_ITEMS           = "items"                    ; // <= pkg cfg files!

    //___________________________________________
    // member

    //-------------------------------------------
    /** TODO */
    private ConfigHelper m_aCfg;

    //-------------------------------------------
    /** TODO */
    private Logger m_aLog;

    //-------------------------------------------
    /** TODO */
    private java.io.File m_aFragmentsDir;

    //-------------------------------------------
    /** TODO */
    private java.io.File m_aTempDir;

    //-------------------------------------------
    /** TODO */
    private java.io.File m_aOutDir;

    //-------------------------------------------
    /** TODO */
    private java.util.Vector m_lTypes;
    private java.util.Vector m_lFilters;
    private java.util.Vector m_lLoaders;
    private java.util.Vector m_lHandlers;

    //___________________________________________
    // interface

    //-------------------------------------------
    /** initialize a new instance of this class and
     *  try to get all needed resources from the config module.
     *
     *  @param  aCfg
     *          provides access to all values of the global
     *          config file and to the command line.
     *
     *  @param  aLog
     *          can be used to print out log informations.
     */
    public Merger(ConfigHelper aCfg,
                  Logger       aLog)
        throws java.lang.Exception
    {
        m_aCfg = aCfg;
        m_aLog = aLog;

        m_aFragmentsDir = new java.io.File(m_aCfg.getString(PROP_FRAGMENTSDIR));
        m_aTempDir      = new java.io.File(m_aCfg.getString(PROP_TEMPDIR     ));
        m_aOutDir       = new java.io.File(m_aCfg.getString(PROP_OUTDIR      ));

        java.lang.String sDelimiter = m_aCfg.getString(PROP_DELIMITER);
        boolean          bTrim      = m_aCfg.getBoolean(PROP_TRIM);
        boolean          bDecode    = m_aCfg.getBoolean(PROP_DECODE);

        try
        {
            ConfigHelper aFcfg = new ConfigHelper(m_aCfg.getString(PROP_TCFG), null);
            m_lTypes = aFcfg.getStringList(PROP_ITEMS, sDelimiter, bTrim, bDecode);
        }
        catch(java.util.NoSuchElementException ex1) { m_lTypes = new java.util.Vector(); }

        try
        {
            ConfigHelper aFcfg = new ConfigHelper(m_aCfg.getString(PROP_FCFG), null);
            m_lFilters = aFcfg.getStringList(PROP_ITEMS, sDelimiter, bTrim, bDecode);
        }
        catch(java.util.NoSuchElementException ex1) { m_lFilters = new java.util.Vector(); }

        try
        {
            ConfigHelper aFcfg = new ConfigHelper(m_aCfg.getString(PROP_LCFG), null);
            m_lLoaders = aFcfg.getStringList(PROP_ITEMS, sDelimiter, bTrim, bDecode);
        }
        catch(java.util.NoSuchElementException ex1) { m_lLoaders = new java.util.Vector(); }

        try
        {
            ConfigHelper aFcfg = new ConfigHelper(m_aCfg.getString(PROP_CCFG), null);
            m_lHandlers = aFcfg.getStringList(PROP_ITEMS, sDelimiter, bTrim, bDecode);
        }
        catch(java.util.NoSuchElementException ex1) { m_lHandlers = new java.util.Vector(); }
    }

    //-------------------------------------------
    /** TODO */
    public synchronized void merge()
        throws java.lang.Exception
    {
        java.lang.StringBuffer sBuffer  = new java.lang.StringBuffer(1000000);
        java.lang.String       sPackage = m_aCfg.getString(PROP_PKG);

        m_aLog.setGlobalInfo("create package \""+sPackage+"\" ...");
        m_aLog.setDetailedInfo("generate package header ... ");

        sBuffer.append(
            XMLHelper.generateHeader(
                m_aCfg.getString(PROP_XMLVERSION ),
                m_aCfg.getString(PROP_XMLENCODING),
                m_aCfg.getString(PROP_XMLPATH    ),
                m_aCfg.getString(PROP_XMLPACKAGE )));

        // counts all transfered fragments
        // Can be used later to decide, if a generated package file
        // contains "nothing"!
        int nItemCount = 0;

        for (int i=0; i<4; ++i)
        {
            java.lang.String sSetName   = null;
            java.lang.String sSubDir    = null;
            java.util.Vector lFragments = null;

            try
            {
                switch(i)
                {
                    case 0: // types
                    {
                        m_aLog.setDetailedInfo("generate set for types ... ");
                        sSetName   = m_aCfg.getString(PROP_SETNAME_TYPES);
                        sSubDir    = m_aCfg.getString(PROP_SUBDIR_TYPES );
                        lFragments = m_lTypes;
                    }
                    break;

                    case 1: // filters
                    {
                        m_aLog.setDetailedInfo("generate set for filter ... ");
                        sSetName   = m_aCfg.getString(PROP_SETNAME_FILTERS);
                        sSubDir    = m_aCfg.getString(PROP_SUBDIR_FILTERS );
                        lFragments = m_lFilters;
                    }
                    break;

                    case 2: // loaders
                    {
                        m_aLog.setDetailedInfo("generate set for frame loader ... ");
                        sSetName   = m_aCfg.getString(PROP_SETNAME_LOADERS);
                        sSubDir    = m_aCfg.getString(PROP_SUBDIR_LOADERS );
                        lFragments = m_lLoaders;
                    }
                    break;

                    case 3: // handlers
                    {
                        m_aLog.setDetailedInfo("generate set for content handler ... ");
                        sSetName   = m_aCfg.getString(PROP_SETNAME_HANDLERS);
                        sSubDir    = m_aCfg.getString(PROP_SUBDIR_HANDLERS );
                        lFragments = m_lHandlers;
                    }
                    break;
                }

                nItemCount += lFragments.size();

                getFragments(
                    new java.io.File(m_aFragmentsDir, sSubDir),
                    sSetName,
                    lFragments,
                    1,
                    sBuffer);
            }
            catch(java.util.NoSuchElementException exIgnore)
                { continue; }
        }

        m_aLog.setDetailedInfo("generate package footer ... ");
        sBuffer.append(XMLHelper.generateFooter());

        // Attention!
        // If the package seem to be empty, it make no sense to generate a corresponding
        // xml file. We should suppress writing of this file on disk completly ...
        if (nItemCount < 1)
        {
            m_aLog.setWarning("Package is empty and will not result into a xml file on disk!? Please check configuration file.");
            return;
        }
        m_aLog.setGlobalInfo("package contains "+nItemCount+" items");

        java.io.File aPackage = new File(sPackage);
        m_aLog.setGlobalInfo("write temp package \""+aPackage.getPath()); // TODO encoding must be readed from the configuration
        FileHelper.writeEncodedBufferToFile(aPackage, "UTF-8", false, sBuffer); // check for success is done inside this method!
    }

    //-------------------------------------------
    /** TODO */
    private void getFragments(java.io.File           aDir       ,
                              java.lang.String       sSetName   ,
                              java.util.Vector       lFragments ,
                              int                    nPrettyTabs,
                              java.lang.StringBuffer sBuffer    )
        throws java.lang.Exception
    {
        java.util.Enumeration  pFragments = lFragments.elements();
        java.lang.String       sExtXcu    = m_aCfg.getString(PROP_EXTENSION_XCU);

        if (lFragments.size()<1)
            return;

        for (int tabs=0; tabs<nPrettyTabs; ++tabs)
            sBuffer.append("\t");
        sBuffer.append("<node oor:name=\""+sSetName+"\">\n");
        ++nPrettyTabs;

        while(pFragments.hasMoreElements())
        {
            java.lang.String sFragment = (java.lang.String)pFragments.nextElement();
            java.io.File     aFragment = new java.io.File(aDir, sFragment+"."+sExtXcu);

            // handle simple files only and check for existence!
            if (!aFragment.exists())
                throw new java.io.IOException("fragment \""+aFragment.getPath()+"\" does not exists.");

            if (!aFragment.isFile())
            {
                m_aLog.setWarning("fragment \""+aFragment.getPath()+"\" seem to be not a valid file.");
                continue;
            }

            // copy file content of original fragment
            // Note: A FileNotFoundException will be thrown automaticly by the
            // used reader objects. Let it break this method too. Our calli is interested
            // on such errors :-)
            m_aLog.setDetailedInfo("merge fragment \""+aFragment.getPath()+"\" ...");
            FileHelper.readEncodedBufferFromFile(aFragment, "UTF-8", sBuffer);

            sBuffer.append("\n");
        }

        --nPrettyTabs;
        for (int tabs=0; tabs<nPrettyTabs; ++tabs)
            sBuffer.append("\t");
        sBuffer.append("</node>\n");
    }
}
