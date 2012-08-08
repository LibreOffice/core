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

//_______________________________________________

import java.lang.*;
import java.util.*;
import java.io.*;
import com.sun.star.filter.config.tools.utils.*;

//_______________________________________________

/**
 *  Can split one xml file into its different xml fragments.
 *
 *
 */
public class Splitter
{
    //___________________________________________
    // member

    /** contains all real member of this instance.
     *  That make it easy to initialize an instance
     *  of this class inside a multi-threaded environment. */
    private SplitterData m_aDataSet;

    //___________________________________________
    // interface

    /** initialize a new instance of this class with all
     *  needed resources.
     *
     *  @param  aDataSet
     *          contains all needed parameters for this instance
     *          as a complete set, which can be filled outside.
     */
    public Splitter(SplitterData aDataSet)
    {
        m_aDataSet = aDataSet;
    }

    //___________________________________________
    // interface

    /** generate xml fragments for all cache items.
     *
     *  @throw  [java.lang.Exception]
     *          if anything will fail inside during
     *          this operation runs.
     */
    public synchronized void split()
        throws java.lang.Exception
    {
        createDirectoryStructures();

        // use some statistic values to check if all cache items
        // will be transformed realy.
        int nTypes           = m_aDataSet.m_aCache.getItemCount(Cache.E_TYPE          );
        int nFilters         = m_aDataSet.m_aCache.getItemCount(Cache.E_FILTER        );
        int nDetectServices  = m_aDataSet.m_aCache.getItemCount(Cache.E_DETECTSERVICE );
        int nFrameLoaders    = m_aDataSet.m_aCache.getItemCount(Cache.E_FRAMELOADER   );
        int nContentHandlers = m_aDataSet.m_aCache.getItemCount(Cache.E_CONTENTHANDLER);

        // generate all type fragments
        m_aDataSet.m_aDebug.setGlobalInfo("generate type fragments ...");
        java.util.Vector      lNames = m_aDataSet.m_aCache.getItemNames(Cache.E_TYPE);
        java.util.Enumeration it     = lNames.elements();
        while(it.hasMoreElements())
            generateXMLFragment(Cache.E_TYPE, (java.lang.String)it.nextElement(), m_aDataSet.m_aFragmentDirTypes);
        nTypes -= lNames.size();

        // generate filter fragments for the writer module
        m_aDataSet.m_aDebug.setGlobalInfo("generate filter fragments ...");
        m_aDataSet.m_aDebug.setGlobalInfo("\tfor module writer ...");
        java.util.HashMap rRequestedProps = new java.util.HashMap();
        rRequestedProps.put(Cache.PROPNAME_DOCUMENTSERVICE, "com.sun.star.text.TextDocument");
        lNames = m_aDataSet.m_aCache.getMatchedItemNames(Cache.E_FILTER, rRequestedProps);
        it     = lNames.elements();
        while(it.hasMoreElements())
            generateXMLFragment(Cache.E_FILTER, (java.lang.String)it.nextElement(), m_aDataSet.m_aFragmentDirModuleSWriter);
        nFilters -= lNames.size();

        // generate filter fragments for the writer/web module
        m_aDataSet.m_aDebug.setGlobalInfo("\tfor module writer/web ...");
        rRequestedProps.put(Cache.PROPNAME_DOCUMENTSERVICE, "com.sun.star.text.WebDocument");
        lNames = m_aDataSet.m_aCache.getMatchedItemNames(Cache.E_FILTER, rRequestedProps);
        it     = lNames.elements();
        while(it.hasMoreElements())
            generateXMLFragment(Cache.E_FILTER, (java.lang.String)it.nextElement(), m_aDataSet.m_aFragmentDirModuleSWeb);
        nFilters -= lNames.size();

        // generate filter fragments for the writer/global module
        m_aDataSet.m_aDebug.setGlobalInfo("\tfor module writer/global ...");
        rRequestedProps.put(Cache.PROPNAME_DOCUMENTSERVICE, "com.sun.star.text.GlobalDocument");
        lNames = m_aDataSet.m_aCache.getMatchedItemNames(Cache.E_FILTER, rRequestedProps);
        it     = lNames.elements();
        while(it.hasMoreElements())
            generateXMLFragment(Cache.E_FILTER, (java.lang.String)it.nextElement(), m_aDataSet.m_aFragmentDirModuleSGlobal);
        nFilters -= lNames.size();

        // generate filter fragments for the calc module
        m_aDataSet.m_aDebug.setGlobalInfo("\tfor module calc ...");
        rRequestedProps.put(Cache.PROPNAME_DOCUMENTSERVICE, "com.sun.star.sheet.SpreadsheetDocument");
        lNames = m_aDataSet.m_aCache.getMatchedItemNames(Cache.E_FILTER, rRequestedProps);
        it     = lNames.elements();
        while(it.hasMoreElements())
            generateXMLFragment(Cache.E_FILTER, (java.lang.String)it.nextElement(), m_aDataSet.m_aFragmentDirModuleSCalc);
        nFilters -= lNames.size();

        // generate filter fragments for the draw module
        m_aDataSet.m_aDebug.setGlobalInfo("\tfor module draw ...");
        rRequestedProps.put(Cache.PROPNAME_DOCUMENTSERVICE, "com.sun.star.drawing.DrawingDocument");
        lNames = m_aDataSet.m_aCache.getMatchedItemNames(Cache.E_FILTER, rRequestedProps);
        it     = lNames.elements();
        while(it.hasMoreElements())
            generateXMLFragment(Cache.E_FILTER, (java.lang.String)it.nextElement(), m_aDataSet.m_aFragmentDirModuleSDraw);
        nFilters -= lNames.size();

        // generate filter fragments for the impress module
        m_aDataSet.m_aDebug.setGlobalInfo("\tfor module impress ...");
        rRequestedProps.put(Cache.PROPNAME_DOCUMENTSERVICE, "com.sun.star.presentation.PresentationDocument");
        lNames = m_aDataSet.m_aCache.getMatchedItemNames(Cache.E_FILTER, rRequestedProps);
        it     = lNames.elements();
        while(it.hasMoreElements())
            generateXMLFragment(Cache.E_FILTER, (java.lang.String)it.nextElement(), m_aDataSet.m_aFragmentDirModuleSImpress);
        nFilters -= lNames.size();

        // generate filter fragments for the chart module
        m_aDataSet.m_aDebug.setGlobalInfo("\tfor module chart ...");
        rRequestedProps.put(Cache.PROPNAME_DOCUMENTSERVICE, "com.sun.star.chart2.ChartDocument");
        lNames = m_aDataSet.m_aCache.getMatchedItemNames(Cache.E_FILTER, rRequestedProps);
        it     = lNames.elements();
        while(it.hasMoreElements())
            generateXMLFragment(Cache.E_FILTER, (java.lang.String)it.nextElement(), m_aDataSet.m_aFragmentDirModuleSChart);
        nFilters -= lNames.size();

        // generate filter fragments for the math module
        m_aDataSet.m_aDebug.setGlobalInfo("\tfor module math ...");
        rRequestedProps.put(Cache.PROPNAME_DOCUMENTSERVICE, "com.sun.star.formula.FormulaProperties");
        lNames = m_aDataSet.m_aCache.getMatchedItemNames(Cache.E_FILTER, rRequestedProps);
        it     = lNames.elements();
        while(it.hasMoreElements())
            generateXMLFragment(Cache.E_FILTER, (java.lang.String)it.nextElement(), m_aDataSet.m_aFragmentDirModuleSMath);
        nFilters -= lNames.size();

        // generate fragments for 3rdParty or unspecified (may graphics) filters!
        m_aDataSet.m_aDebug.setGlobalInfo("\tfor unknown modules ...");
        rRequestedProps.put(Cache.PROPNAME_DOCUMENTSERVICE, "");
        lNames = m_aDataSet.m_aCache.getMatchedItemNames(Cache.E_FILTER, rRequestedProps);
        it     = lNames.elements();
        while(it.hasMoreElements())
            generateXMLFragment(Cache.E_FILTER, (java.lang.String)it.nextElement(), m_aDataSet.m_aFragmentDirModuleOthers);
        nFilters -= lNames.size();

        // generate all detect service fragments
        m_aDataSet.m_aDebug.setGlobalInfo("generate detect service fragments ...");
        lNames = m_aDataSet.m_aCache.getItemNames(Cache.E_DETECTSERVICE);
        it     = lNames.elements();
        while(it.hasMoreElements())
            generateXMLFragment(Cache.E_DETECTSERVICE, (java.lang.String)it.nextElement(), m_aDataSet.m_aFragmentDirDetectServices);
        nDetectServices -= lNames.size();

        // generate all frame loader fragments
        m_aDataSet.m_aDebug.setGlobalInfo("generate frame loader fragments ...");
        lNames = m_aDataSet.m_aCache.getItemNames(Cache.E_FRAMELOADER);
        it     = lNames.elements();
        while(it.hasMoreElements())
            generateXMLFragment(Cache.E_FRAMELOADER, (java.lang.String)it.nextElement(), m_aDataSet.m_aFragmentDirFrameLoaders);
        nFrameLoaders -= lNames.size();

        // generate all content handler fragments
        m_aDataSet.m_aDebug.setGlobalInfo("generate content handler fragments ...");
        lNames = m_aDataSet.m_aCache.getItemNames(Cache.E_CONTENTHANDLER);
        it     = lNames.elements();
        while(it.hasMoreElements())
            generateXMLFragment(Cache.E_CONTENTHANDLER, (java.lang.String)it.nextElement(), m_aDataSet.m_aFragmentDirContentHandlers);
        nContentHandlers -= lNames.size();

        // check if all cache items was handled
        if (
            (nTypes           != 0) ||
            (nFilters         != 0) ||
            (nDetectServices  != 0) ||
            (nFrameLoaders    != 0) ||
            (nContentHandlers != 0)
           )
        {
            java.lang.StringBuffer sStatistic = new java.lang.StringBuffer(256);
            sStatistic.append("some cache items seems to be not transformed:\n");
            sStatistic.append(nTypes          +" unhandled types\n"          );
            sStatistic.append(nFilters        +" unhandled filters\n"        );
            sStatistic.append(nDetectServices +" unhandled detect services\n");
            sStatistic.append(nFrameLoaders   +" unhandled frame loader\n"   );
            sStatistic.append(nContentHandlers+" unhandled content handler\n");
            throw new java.lang.Exception(sStatistic.toString());
        }
    }

    //___________________________________________

    /** generate a xml fragment file from the specified cache item.
     *
     *  @param  eItemType
     *          specify, which sub container of the cache must be used
     *          to locate the right item.
     *
     *  @param  sItemName
     *          the name of the cache item inside the specified sub container.
     *
     *  @param  aOutDir
     *          output directory.
     *
     *  @throw  [java.lang.Exception]
     *          if the fragment file already exists or could not be created
     *          successfully.
     */
    private void generateXMLFragment(int              eItemType,
                                     java.lang.String sItemName,
                                     java.io.File     aOutDir  )
        throws java.lang.Exception
    {
        java.lang.String sFileName = FileHelper.convertName2FileName(sItemName);
        java.lang.String sXML      = m_aDataSet.m_aCache.getItemAsXML(eItemType, sItemName, m_aDataSet.m_nFormat);
        java.io.File     aFile     = new java.io.File(aOutDir, sFileName+m_aDataSet.m_sFragmentExtension);

        if (aFile.exists())
            throw new java.lang.Exception("fragment["+eItemType+", \""+sItemName+"\"] file named \""+aFile.getPath()+"\" already exists.");

        java.io.FileOutputStream   aStream = new java.io.FileOutputStream(aFile);
        java.io.OutputStreamWriter aWriter = new java.io.OutputStreamWriter(aStream, m_aDataSet.m_sEncoding);
        aWriter.write(sXML, 0, sXML.length());
        aWriter.flush();
        aWriter.close();

        m_aDataSet.m_aDebug.setDetailedInfo("fragment["+eItemType+", \""+sItemName+"\"] => \""+aFile.getPath()+"\" ... OK");
    }

    //___________________________________________

    /** create all needed directory structures.
     *
     *  First it try to clear old structures and
     *  create new ones afterwards.
     *
     *  @throw  [java.lang.Exception]
     *          if some of the needed structures
     *          could not be created successfully.
     */
    private void createDirectoryStructures()
        throws java.lang.Exception
    {
        m_aDataSet.m_aDebug.setGlobalInfo("create needed directory structures ...");

        // delete simple files only; no directories!
        // Because this tool may run inside
        // a cvs environment its not a godd idea to do so.
        boolean bFilesOnly = false;
        FileHelper.makeDirectoryEmpty(m_aDataSet.m_aOutDir, bFilesOnly);

        if (
            (!m_aDataSet.m_aFragmentDirTypes.exists()           && !m_aDataSet.m_aFragmentDirTypes.mkdir()          ) ||
            (!m_aDataSet.m_aFragmentDirFilters.exists()         && !m_aDataSet.m_aFragmentDirFilters.mkdir()        ) ||
            (!m_aDataSet.m_aFragmentDirDetectServices.exists()  && !m_aDataSet.m_aFragmentDirDetectServices.mkdir() ) ||
            (!m_aDataSet.m_aFragmentDirFrameLoaders.exists()    && !m_aDataSet.m_aFragmentDirFrameLoaders.mkdir()   ) ||
            (!m_aDataSet.m_aFragmentDirContentHandlers.exists() && !m_aDataSet.m_aFragmentDirContentHandlers.mkdir()) ||
            (!m_aDataSet.m_aFragmentDirModuleSWriter.exists()   && !m_aDataSet.m_aFragmentDirModuleSWriter.mkdir()  ) ||
            (!m_aDataSet.m_aFragmentDirModuleSWeb.exists()      && !m_aDataSet.m_aFragmentDirModuleSWeb.mkdir()     ) ||
            (!m_aDataSet.m_aFragmentDirModuleSGlobal.exists()   && !m_aDataSet.m_aFragmentDirModuleSGlobal.mkdir()  ) ||
            (!m_aDataSet.m_aFragmentDirModuleSCalc.exists()     && !m_aDataSet.m_aFragmentDirModuleSCalc.mkdir()    ) ||
            (!m_aDataSet.m_aFragmentDirModuleSDraw.exists()     && !m_aDataSet.m_aFragmentDirModuleSDraw.mkdir()    ) ||
            (!m_aDataSet.m_aFragmentDirModuleSImpress.exists()  && !m_aDataSet.m_aFragmentDirModuleSImpress.mkdir() ) ||
            (!m_aDataSet.m_aFragmentDirModuleSMath.exists()     && !m_aDataSet.m_aFragmentDirModuleSMath.mkdir()    ) ||
            (!m_aDataSet.m_aFragmentDirModuleSChart.exists()    && !m_aDataSet.m_aFragmentDirModuleSChart.mkdir()   ) ||
            (!m_aDataSet.m_aFragmentDirModuleOthers.exists()    && !m_aDataSet.m_aFragmentDirModuleOthers.mkdir()   )
           )
        {
            throw new java.lang.Exception("some directory structures does not exists and could not be created successfully.");
        }
    }
}
