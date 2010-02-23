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
import java.util.*;
import java.io.*;
import com.sun.star.filter.config.tools.utils.*;

//_______________________________________________

/**
 *  Implements a simple command line tool, which can read a given xml
 *  configuration file of the filter configuration, analyze it
 *  and split it into different xml fragments.
 *  All parameters of this process can be given by a configuration file.
 *
 *
 */
public class FCFGSplit
{
    //___________________________________________
    // private const

    /** specify the command line parameter to set the debug level for this app. */
    private static final java.lang.String CMD_DEBUG = "debug" ;

    /** specify the command line parameter to set a configuration file for this app. */
    private static final java.lang.String CMD_CONFIG = "config";

    /** The following strings are used as property names of
     *  the configuration file we need here.
     *
     *  @seealso    readCfg()
     */

    private static final java.lang.String CFGKEY_XMLFILE                              = "xmlfile"                             ;
    private static final java.lang.String CFGKEY_INFORMAT                             = "informat"                            ;
    private static final java.lang.String CFGKEY_OUTFORMAT                            = "outformat"                           ;
    private static final java.lang.String CFGKEY_INENCODING                           = "inencoding"                          ;
    private static final java.lang.String CFGKEY_OUTENCODING                          = "outencoding"                         ;
    private static final java.lang.String CFGKEY_OUTDIR                               = "outdir"                              ;
    private static final java.lang.String CFGKEY_FRAGMENT_EXTENSION                   = "fragment_extension"                  ;
    private static final java.lang.String CFGKEY_CREATE_COMBINE_FILTER_FLAG           = "create_combine_filter_flag"          ;
    private static final java.lang.String CFGKEY_REMOVE_FILTER_FLAG_BROWSERPREFERRED  = "remove_filter_flag_browserpreferred" ;
    private static final java.lang.String CFGKEY_REMOVE_FILTER_FLAG_PREFERRED         = "remove_filter_flag_preferred"        ;
    private static final java.lang.String CFGKEY_REMOVE_FILTER_FLAG_3RDPARTY          = "remove_filter_flag_3rdparty"         ;
    private static final java.lang.String CFGKEY_REMOVE_FILTER_UINAMES                = "remove_filter_uinames"               ;
    private static final java.lang.String CFGKEY_REMOVE_GRAPHIC_FILTERS               = "remove_graphic_filters"              ;
    private static final java.lang.String CFGKEY_SET_DEFAULT_DETECTOR                 = "set_default_detector"                ;

    private static final java.lang.String CFGKEY_SUBDIR_TYPES                         = "subdir_types"                        ;
    private static final java.lang.String CFGKEY_SUBDIR_FILTERS                       = "subdir_filters"                      ;
    private static final java.lang.String CFGKEY_SUBDIR_DETECTSERVICES                = "subdir_detectservices"               ;
    private static final java.lang.String CFGKEY_SUBDIR_FRAMELOADERS                  = "subdir_frameloaders"                 ;
    private static final java.lang.String CFGKEY_SUBDIR_CONTENTHANDLERS               = "subdir_contenthandlers"              ;

    private static final java.lang.String CFGKEY_SEPERATE_FILTERS_BY_MODULE           = "seperate_filters_by_module"          ;

    private static final java.lang.String CFGKEY_SUBDIR_MODULE_SWRITER                = "subdir_module_swriter"               ;
    private static final java.lang.String CFGKEY_SUBDIR_MODULE_SWEB                   = "subdir_module_sweb"                  ;
    private static final java.lang.String CFGKEY_SUBDIR_MODULE_SGLOBAL                = "subdir_module_sglobal"               ;
    private static final java.lang.String CFGKEY_SUBDIR_MODULE_SCALC                  = "subdir_module_scalc"                 ;
    private static final java.lang.String CFGKEY_SUBDIR_MODULE_SDRAW                  = "subdir_module_sdraw"                 ;
    private static final java.lang.String CFGKEY_SUBDIR_MODULE_SIMPRESS               = "subdir_module_simpress"              ;
    private static final java.lang.String CFGKEY_SUBDIR_MODULE_SMATH                  = "subdir_module_smath"                 ;
    private static final java.lang.String CFGKEY_SUBDIR_MODULE_SCHART                 = "subdir_module_schart"                ;
    private static final java.lang.String CFGKEY_SUBDIR_MODULE_OTHERS                 = "subdir_module_others"                ;

    /** The following strings are used as property default
     *  values if a configuration key does not exist.
     *  It must be a string value, because the class java.util.Properties
     *  accept it as the only type. But of course the value must be
     *  convertable to the right target type.
     *
     *  @seealso    readCfg()
     */

    private static final java.lang.String DEFAULT_XMLFILE                             = ".//TypeDetection.xcu"                ;
    private static final java.lang.String DEFAULT_INFORMAT                            = "6.0"                                 ;
    private static final java.lang.String DEFAULT_OUTFORMAT                           = "6.Y"                                 ;
    private static final java.lang.String DEFAULT_INENCODING                          = "UTF-8"                               ;
    private static final java.lang.String DEFAULT_OUTENCODING                         = "UTF-8"                               ;
    private static final java.lang.String DEFAULT_OUTDIR                              = ".//temp"                             ;
    private static final java.lang.String DEFAULT_FRAGMENT_EXTENSION                  = ".xcu"                                ;
    private static final java.lang.String DEFAULT_CREATE_COMBINE_FILTER_FLAG          = "false"                               ;
    private static final java.lang.String DEFAULT_REMOVE_FILTER_FLAG_BROWSERPREFERRED = "false"                               ;
    private static final java.lang.String DEFAULT_REMOVE_FILTER_FLAG_PREFERRED        = "false"                               ;
    private static final java.lang.String DEFAULT_REMOVE_FILTER_FLAG_3RDPARTY         = "false"                               ;
    private static final java.lang.String DEFAULT_REMOVE_FILTER_UINAMES               = "false"                               ;
    private static final java.lang.String DEFAULT_REMOVE_GRAPHIC_FILTERS              = "false"                               ;
    private static final java.lang.String DEFAULT_SET_DEFAULT_DETECTOR                = "false"                               ;

    private static final java.lang.String DEFAULT_SUBDIR_TYPES                        = "Types"                               ;
    private static final java.lang.String DEFAULT_SUBDIR_FILTERS                      = "Filters"                             ;
    private static final java.lang.String DEFAULT_SUBDIR_DETECTSERVICES               = "DetectServices"                      ;
    private static final java.lang.String DEFAULT_SUBDIR_FRAMELOADERS                 = "FrameLoaders"                        ;
    private static final java.lang.String DEFAULT_SUBDIR_CONTENTHANDLERS              = "ContentHandlers"                     ;

    private static final java.lang.String DEFAULT_SEPERATE_FILTERS_BY_MODULE          = "false"                               ;

    private static final java.lang.String DEFAULT_SUBDIR_MODULE_SWRITER               = "SWriter"                             ;
    private static final java.lang.String DEFAULT_SUBDIR_MODULE_SWEB                  = "SWeb"                                ;
    private static final java.lang.String DEFAULT_SUBDIR_MODULE_SGLOBAL               = "SGlobal"                             ;
    private static final java.lang.String DEFAULT_SUBDIR_MODULE_SCALC                 = "SCalc"                               ;
    private static final java.lang.String DEFAULT_SUBDIR_MODULE_SDRAW                 = "SDraw"                               ;
    private static final java.lang.String DEFAULT_SUBDIR_MODULE_SIMPRESS              = "SImpress"                            ;
    private static final java.lang.String DEFAULT_SUBDIR_MODULE_SMATH                 = "SMath"                               ;
    private static final java.lang.String DEFAULT_SUBDIR_MODULE_SCHART                = "SChart"                              ;
    private static final java.lang.String DEFAULT_SUBDIR_MODULE_OTHERS                = "Others"                              ;

    //___________________________________________
    // private member

    /** contains the name of the reading xcu file. */
    private static java.lang.String m_sXMLFile;

    /** specify the xml file format, which must be interpreted at reading time. */
    private static int m_nInFormat;

    /** specify the xml file format, which must be used
     *  to generate all xcu fragments. */
    private static int m_nOutFormat;

    /** specify the file encoding for reading. */
    private static java.lang.String m_sInEncoding;

    /** specify the file encoding for writing fragments. */
    private static java.lang.String m_sOutEncoding;

    /** specify the target directory, where all results of this
     *  process can be generated.
     *  Note: May it will be cleared! */
    private static java.lang.String m_sOutDir;

    /** can be used to generate some output on the console. */
    private static Logger m_aDebug;

    /** contains the file extension for all generated xml fragments. */
    private static java.lang.String m_sFragmentExtension;

    /** specify the sub directory to generate type fragments.
     *  Its meaned relativ to m_sOutDir. */
    private static java.lang.String m_sSubDirTypes;

    /** specify the sub directory to generate filter fragments.
     *  Its meaned relativ to m_sOutDir. */
    private static java.lang.String m_sSubDirFilters;

    /** specify the sub directory to generate detect service fragments.
     *  Its meaned relativ to m_sOutDir. */
    private static java.lang.String m_sSubDirDetectServices;

    /** specify the sub directory to generate frame loader fragments.
     *  Its meaned relativ to m_sOutDir. */
    private static java.lang.String m_sSubDirFrameLoaders;

    /** specify the sub directory to generate content handler fragments.
     *  Its meaned relativ to m_sOutDir. */
    private static java.lang.String m_sSubDirContentHandlers;

    /** enable/disable generating of filter groups - seperated by
     *  application modules. */
    private static boolean m_bSeperateFiltersByModule;

    /** specify the sub directory to generate filter groups
     *  for the module writer. Will be used only,
     *  if m_bSeperateFiltersByModule is set to TRUE.*/
    private static java.lang.String m_sSubDirModuleSWriter;

    /** specify the sub directory to generate filter groups
     *  for the module writer/web. Will be used only,
     *  if m_bSeperateFiltersByModule is set to TRUE.*/
    private static java.lang.String m_sSubDirModuleSWeb;

    /** specify the sub directory to generate filter groups
     *  for the module writer/global. Will be used only,
     *  if m_bSeperateFiltersByModule is set to TRUE.*/
    private static java.lang.String m_sSubDirModuleSGlobal;

    /** specify the sub directory to generate filter groups
     *  for the module calc. Will be used only,
     *  if m_bSeperateFiltersByModule is set to TRUE.*/
    private static java.lang.String m_sSubDirModuleSCalc;

    /** specify the sub directory to generate filter groups
     *  for the module draw. Will be used only,
     *  if m_bSeperateFiltersByModule is set to TRUE.*/
    private static java.lang.String m_sSubDirModuleSDraw;

    /** specify the sub directory to generate filter groups
     *  for the module impress. Will be used only,
     *  if m_bSeperateFiltersByModule is set to TRUE.*/
    private static java.lang.String m_sSubDirModuleSImpress;

    /** specify the sub directory to generate filter groups
     *  for the module math. Will be used only,
     *  if m_bSeperateFiltersByModule is set to TRUE.*/
    private static java.lang.String m_sSubDirModuleSMath;

    /** specify the sub directory to generate filter groups
     *  for the module chart. Will be used only,
     *  if m_bSeperateFiltersByModule is set to TRUE.*/
    private static java.lang.String m_sSubDirModuleSChart;

    /** specify the sub directory to generate filter groups
     *  for unknown modules - e.g. the graphic filters.
     *  Will be used only, if m_bSeperateFiltersByModule
     *  is set to TRUE.*/
    private static java.lang.String m_sSubDirModuleOthers;

    private static boolean m_bCreateCombineFilterFlag;
    private static boolean m_bRemoveFilterFlagBrowserPreferred;
    private static boolean m_bRemoveFilterFlagPreferred;
    private static boolean m_bRemoveFilterFlag3rdparty;
    private static boolean m_bRemoveFilterUINames;
    private static boolean m_bRemoveGraphicFilters;
    private static boolean m_bSetDefaultDetector;

    //___________________________________________
    // main

    /** main.
     *
     *  Analyze the command line arguments, load the configuration file,
     *  fill a cache from the specified xml file and generate all
     *  needed xml fragments inside the specified output directory.
     *
     *  @param  lArgs
     *          contains the command line arguments.
     */
    public static void main(java.lang.String[] lArgs)
    {
        long t_start = System.currentTimeMillis();

        // must be :-)
        FCFGSplit.printCopyright();
        // can be used as exit code
        int nErr = 0;

        // --------------------------------------------------------------------
        // analyze command line parameter
        ConfigHelper aCmdLine = null;
        try
        {
            aCmdLine = new ConfigHelper("com/sun/star/filter/config/tools/split/FCFGSplit.cfg", lArgs);
        }
        catch(java.lang.Throwable exCmdLine)
        {
            exCmdLine.printStackTrace();
            FCFGSplit.printHelp();
            System.exit(--nErr);
        }

        // --------------------------------------------------------------------
        // help requested?
        if (aCmdLine.isHelp())
        {
            FCFGSplit.printHelp();
            System.exit(--nErr);
        }

        // --------------------------------------------------------------------
        // initialize an output channel for errors/warnings etc.
        int nLevel = aCmdLine.getInt(CMD_DEBUG, Logger.LEVEL_DETAILEDINFOS);
        m_aDebug = new Logger(nLevel);
        try
        {
            FCFGSplit.readCfg(aCmdLine);
        }
        catch(java.lang.Exception exCfgLoad)
        {
            m_aDebug.setException(exCfgLoad);
            System.exit(--nErr);
        }

        // --------------------------------------------------------------------
        // check if the required resources exists
        java.io.File aXMLFile = new java.io.File(m_sXMLFile);
        if (!aXMLFile.exists() || !aXMLFile.isFile())
        {
            m_aDebug.setError("The specified xml file \""+aXMLFile.getPath()+"\" does not exist or seems not to be a simple file.");
            System.exit(--nErr);
        }

        java.io.File aOutDir = new java.io.File(m_sOutDir);
        if (!aOutDir.exists() || !aOutDir.isDirectory())
        {
            m_aDebug.setError("The specified directory \""+aOutDir.getPath()+"\" does not exist or seems not to be a directory.");
            System.exit(--nErr);
        }

        if (m_nInFormat == Cache.FORMAT_UNSUPPORTED)
        {
            m_aDebug.setError("The specified xml format for input is not supported.");
            System.exit(--nErr);
        }

        if (m_nOutFormat == Cache.FORMAT_UNSUPPORTED)
        {
            m_aDebug.setError("The specified xml format for output is not supported.");
            System.exit(--nErr);
        }

        // --------------------------------------------------------------------
        // load the xml file
        m_aDebug.setGlobalInfo("loading xml file \""+aXMLFile.getPath()+"\" ...");
        long t_load_start = System.currentTimeMillis();
        Cache aCache = new Cache(m_aDebug);
        try
        {
            aCache.fromXML(aXMLFile, m_nInFormat);
        }
        catch(java.lang.Throwable exLoad)
        {
            m_aDebug.setException(exLoad);
            System.exit(--nErr);
        }
        long t_load_end = System.currentTimeMillis();

        // --------------------------------------------------------------------
        // validate the content, fix some problems and convert it to the output format
        m_aDebug.setGlobalInfo("validate and transform to output format ...");
        long t_transform_start = System.currentTimeMillis();
        try
        {
            aCache.validate(m_nInFormat);
            if (
                (m_nInFormat  == Cache.FORMAT_60) &&
                (m_nOutFormat == Cache.FORMAT_6Y)
               )
            {
                aCache.transform60to6Y(m_bCreateCombineFilterFlag         ,
                                       m_bRemoveFilterFlagBrowserPreferred,
                                       m_bRemoveFilterFlagPreferred       ,
                                       m_bRemoveFilterFlag3rdparty        ,
                                       m_bRemoveFilterUINames             ,
                                       m_bRemoveGraphicFilters            ,
                                       m_bSetDefaultDetector              );
            }
            aCache.validate(m_nOutFormat);
        }
        catch(java.lang.Throwable exTransform)
        {
            m_aDebug.setException(exTransform);
            System.exit(--nErr);
        }
        long t_transform_end = System.currentTimeMillis();

        // --------------------------------------------------------------------
        // generate all xml fragments
        m_aDebug.setGlobalInfo("generate xml fragments into directory \""+aOutDir.getPath()+"\" ...");
        long t_split_start = System.currentTimeMillis();
        try
        {
            SplitterData aDataSet = new SplitterData();
            aDataSet.m_aDebug                       = m_aDebug                  ;
            aDataSet.m_aCache                       = aCache                    ;
            aDataSet.m_nFormat                      = m_nOutFormat              ;
            aDataSet.m_sEncoding                    = m_sOutEncoding            ;
            aDataSet.m_bSeperateFiltersByModule     = m_bSeperateFiltersByModule;
            aDataSet.m_sFragmentExtension           = m_sFragmentExtension      ;
            aDataSet.m_aOutDir                      = aOutDir                   ;

            aDataSet.m_aFragmentDirTypes            = new java.io.File(aOutDir, m_sSubDirTypes          );
            aDataSet.m_aFragmentDirFilters          = new java.io.File(aOutDir, m_sSubDirFilters        );
            aDataSet.m_aFragmentDirDetectServices   = new java.io.File(aOutDir, m_sSubDirDetectServices );
            aDataSet.m_aFragmentDirFrameLoaders     = new java.io.File(aOutDir, m_sSubDirFrameLoaders   );
            aDataSet.m_aFragmentDirContentHandlers  = new java.io.File(aOutDir, m_sSubDirContentHandlers);

            if (m_bSeperateFiltersByModule)
            {
                aDataSet.m_aFragmentDirModuleSWriter  = new java.io.File(aDataSet.m_aFragmentDirFilters, m_sSubDirModuleSWriter );
                aDataSet.m_aFragmentDirModuleSWeb     = new java.io.File(aDataSet.m_aFragmentDirFilters, m_sSubDirModuleSWeb    );
                aDataSet.m_aFragmentDirModuleSGlobal  = new java.io.File(aDataSet.m_aFragmentDirFilters, m_sSubDirModuleSGlobal );
                aDataSet.m_aFragmentDirModuleSCalc    = new java.io.File(aDataSet.m_aFragmentDirFilters, m_sSubDirModuleSCalc   );
                aDataSet.m_aFragmentDirModuleSDraw    = new java.io.File(aDataSet.m_aFragmentDirFilters, m_sSubDirModuleSDraw   );
                aDataSet.m_aFragmentDirModuleSImpress = new java.io.File(aDataSet.m_aFragmentDirFilters, m_sSubDirModuleSImpress);
                aDataSet.m_aFragmentDirModuleSMath    = new java.io.File(aDataSet.m_aFragmentDirFilters, m_sSubDirModuleSMath   );
                aDataSet.m_aFragmentDirModuleSChart   = new java.io.File(aDataSet.m_aFragmentDirFilters, m_sSubDirModuleSChart  );
                aDataSet.m_aFragmentDirModuleOthers   = new java.io.File(aDataSet.m_aFragmentDirFilters, m_sSubDirModuleOthers  );
            }
            else
            {
                aDataSet.m_aFragmentDirModuleSWriter  = aDataSet.m_aFragmentDirFilters;
                aDataSet.m_aFragmentDirModuleSWeb     = aDataSet.m_aFragmentDirFilters;
                aDataSet.m_aFragmentDirModuleSGlobal  = aDataSet.m_aFragmentDirFilters;
                aDataSet.m_aFragmentDirModuleSCalc    = aDataSet.m_aFragmentDirFilters;
                aDataSet.m_aFragmentDirModuleSDraw    = aDataSet.m_aFragmentDirFilters;
                aDataSet.m_aFragmentDirModuleSImpress = aDataSet.m_aFragmentDirFilters;
                aDataSet.m_aFragmentDirModuleSMath    = aDataSet.m_aFragmentDirFilters;
                aDataSet.m_aFragmentDirModuleSChart   = aDataSet.m_aFragmentDirFilters;
                aDataSet.m_aFragmentDirModuleOthers   = aDataSet.m_aFragmentDirFilters;
            }

            Splitter aSplitter = new Splitter(aDataSet);
            aSplitter.split();
        }
        catch(java.lang.Throwable exSplit)
        {
            m_aDebug.setException(exSplit);
            System.exit(--nErr);
        }
        long t_split_end = System.currentTimeMillis();

        // --------------------------------------------------------------------
        // generate some special views
        m_aDebug.setGlobalInfo("generate views and statistics ...");
        long t_statistics_start = System.currentTimeMillis();
        try
        {
            aCache.analyze();
            aCache.toHTML(aOutDir, m_nOutFormat, m_sOutEncoding);
            m_aDebug.setDetailedInfo(aCache.getStatistics());
        }
        catch(java.lang.Throwable exStatistics)
        {
            m_aDebug.setException(exStatistics);
            System.exit(--nErr);
        }
        long t_statistics_end = System.currentTimeMillis();

        // --------------------------------------------------------------------
        // analyze some time stamps
        long t_end = System.currentTimeMillis();

        java.lang.StringBuffer sTimes = new java.lang.StringBuffer(100);
        sTimes.append("Needed times:\n"                  );
        sTimes.append("t [all]\t\t=\t"                   );
        sTimes.append(t_end-t_start                      );
        sTimes.append(" ms\n"                            );
        sTimes.append("t [load]\t=\t"                    );
        sTimes.append(t_load_end-t_load_start            );
        sTimes.append(" ms\n"                            );
        sTimes.append("t [transform]\t=\t"               );
        sTimes.append(t_transform_end-t_transform_start  );
        sTimes.append(" ms\n"                            );
        sTimes.append("t [split]\t=\t"                   );
        sTimes.append(t_split_end-t_split_start          );
        sTimes.append(" ms\n"                            );
        sTimes.append("t [statistics]\t=\t"              );
        sTimes.append(t_statistics_end-t_statistics_start);
        sTimes.append(" ms\n"                            );
        m_aDebug.setDetailedInfo(sTimes.toString());

        // everyting seems to be ok.
        // Return "OK" to calli.
        m_aDebug.setGlobalInfo("Finish.");
        System.exit(0);
    }

    //___________________________________________

    /** read the configuration file.
     *
     *  @param  aCfg
     *          contains the content of the
     *          loaded configuration file.
     */
    private static void readCfg(java.util.Properties aCfg)
    {
        m_sXMLFile                  = aCfg.getProperty(CFGKEY_XMLFILE                   , DEFAULT_XMLFILE                   );

        m_sInEncoding               = aCfg.getProperty(CFGKEY_INENCODING                , DEFAULT_INENCODING                );
        m_sOutEncoding              = aCfg.getProperty(CFGKEY_OUTENCODING               , DEFAULT_OUTENCODING               );
        m_sOutDir                   = aCfg.getProperty(CFGKEY_OUTDIR                    , DEFAULT_OUTDIR                    );
        m_sFragmentExtension        = aCfg.getProperty(CFGKEY_FRAGMENT_EXTENSION        , DEFAULT_FRAGMENT_EXTENSION        );

        m_sSubDirTypes              = aCfg.getProperty(CFGKEY_SUBDIR_TYPES              , DEFAULT_SUBDIR_TYPES              );
        m_sSubDirFilters            = aCfg.getProperty(CFGKEY_SUBDIR_FILTERS            , DEFAULT_SUBDIR_FILTERS            );
        m_sSubDirDetectServices     = aCfg.getProperty(CFGKEY_SUBDIR_DETECTSERVICES     , DEFAULT_SUBDIR_DETECTSERVICES     );
        m_sSubDirFrameLoaders       = aCfg.getProperty(CFGKEY_SUBDIR_FRAMELOADERS       , DEFAULT_SUBDIR_FRAMELOADERS       );
        m_sSubDirContentHandlers    = aCfg.getProperty(CFGKEY_SUBDIR_CONTENTHANDLERS    , DEFAULT_SUBDIR_CONTENTHANDLERS    );

        m_sSubDirModuleSWriter      = aCfg.getProperty(CFGKEY_SUBDIR_MODULE_SWRITER     , DEFAULT_SUBDIR_MODULE_SWRITER     );
        m_sSubDirModuleSWeb         = aCfg.getProperty(CFGKEY_SUBDIR_MODULE_SWEB        , DEFAULT_SUBDIR_MODULE_SWEB        );
        m_sSubDirModuleSGlobal      = aCfg.getProperty(CFGKEY_SUBDIR_MODULE_SGLOBAL     , DEFAULT_SUBDIR_MODULE_SGLOBAL     );
        m_sSubDirModuleSCalc        = aCfg.getProperty(CFGKEY_SUBDIR_MODULE_SCALC       , DEFAULT_SUBDIR_MODULE_SCALC       );
        m_sSubDirModuleSDraw        = aCfg.getProperty(CFGKEY_SUBDIR_MODULE_SDRAW       , DEFAULT_SUBDIR_MODULE_SDRAW       );
        m_sSubDirModuleSImpress     = aCfg.getProperty(CFGKEY_SUBDIR_MODULE_SIMPRESS    , DEFAULT_SUBDIR_MODULE_SIMPRESS    );
        m_sSubDirModuleSMath        = aCfg.getProperty(CFGKEY_SUBDIR_MODULE_SMATH       , DEFAULT_SUBDIR_MODULE_SMATH       );
        m_sSubDirModuleSChart       = aCfg.getProperty(CFGKEY_SUBDIR_MODULE_SCHART      , DEFAULT_SUBDIR_MODULE_SCHART      );
        m_sSubDirModuleOthers       = aCfg.getProperty(CFGKEY_SUBDIR_MODULE_OTHERS      , DEFAULT_SUBDIR_MODULE_OTHERS      );

        m_bSeperateFiltersByModule          = new java.lang.Boolean(aCfg.getProperty(CFGKEY_SEPERATE_FILTERS_BY_MODULE         , DEFAULT_SEPERATE_FILTERS_BY_MODULE         )).booleanValue();
        m_bCreateCombineFilterFlag          = new java.lang.Boolean(aCfg.getProperty(CFGKEY_CREATE_COMBINE_FILTER_FLAG         , DEFAULT_CREATE_COMBINE_FILTER_FLAG         )).booleanValue();
        m_bRemoveFilterFlagBrowserPreferred = new java.lang.Boolean(aCfg.getProperty(CFGKEY_REMOVE_FILTER_FLAG_BROWSERPREFERRED, DEFAULT_REMOVE_FILTER_FLAG_BROWSERPREFERRED)).booleanValue();
        m_bRemoveFilterFlagPreferred        = new java.lang.Boolean(aCfg.getProperty(CFGKEY_REMOVE_FILTER_FLAG_PREFERRED       , DEFAULT_REMOVE_FILTER_FLAG_PREFERRED       )).booleanValue();
        m_bRemoveFilterFlag3rdparty         = new java.lang.Boolean(aCfg.getProperty(CFGKEY_REMOVE_FILTER_FLAG_3RDPARTY        , DEFAULT_REMOVE_FILTER_FLAG_3RDPARTY        )).booleanValue();
        m_bRemoveFilterUINames              = new java.lang.Boolean(aCfg.getProperty(CFGKEY_REMOVE_FILTER_UINAMES              , DEFAULT_REMOVE_FILTER_UINAMES              )).booleanValue();
        m_bRemoveGraphicFilters             = new java.lang.Boolean(aCfg.getProperty(CFGKEY_REMOVE_GRAPHIC_FILTERS             , DEFAULT_REMOVE_GRAPHIC_FILTERS             )).booleanValue();
        m_bSetDefaultDetector               = new java.lang.Boolean(aCfg.getProperty(CFGKEY_SET_DEFAULT_DETECTOR               , DEFAULT_SET_DEFAULT_DETECTOR               )).booleanValue();

        java.lang.String sFormat = aCfg.getProperty(CFGKEY_INFORMAT, DEFAULT_INFORMAT);
        m_nInFormat = Cache.mapFormatString2Format(sFormat);

        sFormat = aCfg.getProperty(CFGKEY_OUTFORMAT, DEFAULT_OUTFORMAT);
        m_nOutFormat = Cache.mapFormatString2Format(sFormat);
    }

    //___________________________________________

    /** prints out a copyright message on stdout.
     */
    private static void printCopyright()
    {
        java.lang.StringBuffer sOut = new java.lang.StringBuffer(256);
        sOut.append("FCFGSplit\n");
        sOut.append("Copyright: 2000 by Sun Microsystems, Inc.\n");
        sOut.append("All Rights Reserved.\n");
        System.out.println(sOut.toString());
    }

    //___________________________________________

    /** prints out a help message on stdout.
     */
    private static void printHelp()
    {
        java.lang.StringBuffer sOut = new java.lang.StringBuffer(1000);
        sOut.append("_______________________________________________________________________________\n\n"   );
        sOut.append("usage: FCFGSplit "+CMD_CONFIG+"=<file name> "+CMD_DEBUG+"=<level>\n"                       );
        sOut.append("parameters:\n"                                                                         );
        sOut.append("\t-help\n"                                                                             );
        sOut.append("\t\tshow this little help.\n\n"                                                        );
        sOut.append("\t"+CMD_CONFIG+"=<file name>\n"                                                        );
        sOut.append("\t\tspecify the configuration file.\n\n"                                               );
        sOut.append("\t"+CMD_DEBUG+"=<level>\n"                                                             );
        sOut.append("\t\tprints out some debug messages.\n"                                                 );
        sOut.append("\t\tlevel=[0..4]\n"                                                                    );
        sOut.append("\t\t0 => no debug messages\n"                                                          );
        sOut.append("\t\t1 => print out errors only\n"                                                      );
        sOut.append("\t\t2 => print out errors & warnings\n"                                                );
        sOut.append("\t\t3 => print out some global actions   (e.g. load file ...)\n"                       );
        sOut.append("\t\t4 => print out more detailed actions (e.g. load item nr. xxx of file.)\n\n"        );
        System.out.println(sOut.toString());
    }
}
