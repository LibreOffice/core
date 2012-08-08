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
package com.sun.star.filter.config.tools.merge;

import java.lang.*;
import java.util.*;
import java.io.*;
import com.sun.star.filter.config.tools.utils.*;

/** can merge different xml fragments together.
 *
 *
 */
public class Merger
{
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
    private static final java.lang.String PROP_DEBUG           = "debug"                    ; // <= cmdline

    private static final java.lang.String PROP_TCFG            = "tcfg"                     ; // <= cmdline
    private static final java.lang.String PROP_FCFG            = "fcfg"                     ; // <= cmdline
    private static final java.lang.String PROP_LCFG            = "lcfg"                     ; // <= cmdline
    private static final java.lang.String PROP_CCFG            = "ccfg"                     ; // <= cmdline
    private static final java.lang.String PROP_LANGUAGEPACK    = "languagepack"             ; // <= cmdline

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
//        m_aOutDir       = new java.io.File(m_aCfg.getString(PROP_OUTDIR      ));

        java.lang.String sDelimiter = m_aCfg.getString(PROP_DELIMITER);
        boolean          bTrim      = m_aCfg.getBoolean(PROP_TRIM);
        boolean          bDecode    = m_aCfg.getBoolean(PROP_DECODE);

        try
        {
            ConfigHelper aFcfg = new ConfigHelper(m_aCfg.getString(PROP_TCFG), null);
            m_lTypes = aFcfg.getStringList(PROP_ITEMS, sDelimiter, bTrim, bDecode);
        }
        catch(java.util.NoSuchElementException ex1)
        {
            m_lTypes = new java.util.Vector();
            //m_aLog.setWarning("Fragment list of types is missing. Parameter \"items\" seems to be invalid.");
        }

        try
        {
            ConfigHelper aFcfg = new ConfigHelper(m_aCfg.getString(PROP_FCFG), null);
            m_lFilters = aFcfg.getStringList(PROP_ITEMS, sDelimiter, bTrim, bDecode);
        }
        catch(java.util.NoSuchElementException ex1)
        {
            m_lFilters = new java.util.Vector();
            //m_aLog.setWarning("Fragment list of filters is missing. Parameter \"items\" seems to be invalid.");
        }

        try
        {
            ConfigHelper aFcfg = new ConfigHelper(m_aCfg.getString(PROP_LCFG), null);
            m_lLoaders = aFcfg.getStringList(PROP_ITEMS, sDelimiter, bTrim, bDecode);
        }
        catch(java.util.NoSuchElementException ex1)
        {
            m_lLoaders = new java.util.Vector();
            //m_aLog.setWarning("Fragment list of frame loader objects is missing. Parameter \"items\" seems to be invalid.");
        }

        try
        {
            ConfigHelper aFcfg = new ConfigHelper(m_aCfg.getString(PROP_CCFG), null);
            m_lHandlers = aFcfg.getStringList(PROP_ITEMS, sDelimiter, bTrim, bDecode);
        }
        catch(java.util.NoSuchElementException ex1)
        {
            m_lHandlers = new java.util.Vector();
            //m_aLog.setWarning("Fragment list of content handler objects is missing. Parameter \"items\" seems to be invalid.");
        }
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
                m_aCfg.getString (PROP_XMLVERSION         ),
                m_aCfg.getString (PROP_XMLENCODING        ),
                m_aCfg.getString (PROP_XMLPATH            ),
                m_aCfg.getString (PROP_XMLPACKAGE         ),
                m_aCfg.getBoolean(PROP_LANGUAGEPACK, false)));

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
        if (lFragments.size()<1)
        {
//            m_aLog.setWarning("List of fragments is empty!? Will be ignored ...");
            return;
        }

        java.util.Enumeration  pFragments = lFragments.elements();
        java.lang.String       sExtXcu    = m_aCfg.getString(PROP_EXTENSION_XCU);

        for (int tabs=0; tabs<nPrettyTabs; ++tabs)
            sBuffer.append("\t");
        sBuffer.append("<node oor:name=\""+sSetName+"\">\n");
        ++nPrettyTabs;

        // special mode for generating language packs.
        // In such case we must live with some missing fragment files.
        // Reason behind; Not all filters are realy localized.
        // But we dont use a different fragment list. We try to locate
        // any fragment file in its language-pack version ...
        boolean bHandleLanguagePacks = m_aCfg.getBoolean(PROP_LANGUAGEPACK, false);
        boolean bDebug               = m_aCfg.getBoolean(PROP_DEBUG       , false);
        java.lang.String sEncoding   = "UTF-8";
        if (bDebug)
            sEncoding = "UTF-8Special";

        while(pFragments.hasMoreElements())
        {
            java.lang.String sFragment = (java.lang.String)pFragments.nextElement();
            java.io.File     aFragment = new java.io.File(aDir, sFragment+"."+sExtXcu);

            // handle simple files only and check for existence!
            if (!aFragment.exists())
            {
                if (bHandleLanguagePacks)
                {
                    m_aLog.setWarning("language fragment \""+aFragment.getPath()+"\" does not exist. Will be ignored.");
                    continue;
                }
                else
                    throw new java.io.IOException("fragment \""+aFragment.getPath()+"\" does not exists.");
            }

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
            FileHelper.readEncodedBufferFromFile(aFragment, sEncoding, sBuffer);

            sBuffer.append("\n");
        }

        --nPrettyTabs;
        for (int tabs=0; tabs<nPrettyTabs; ++tabs)
            sBuffer.append("\t");
        sBuffer.append("</node>\n");
    }
}
