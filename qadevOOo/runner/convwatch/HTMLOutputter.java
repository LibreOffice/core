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

package convwatch;

import java.io.File;
import java.io.FileWriter;
import helper.OSHelper;

public class HTMLOutputter
{
    private FileWriter m_aOut;
    private String m_sFilename;
    private String m_sNamePrefix;              // the HTML files used a suffix to build it's right name

    /**
     * ls is the current line separator (carridge return)
     */
    private String ls;

    HTMLOutputter() {}
    public static HTMLOutputter create( String _sOutputPath, String _sHTMLFilename, String _sNamePrefix )
        {
            FileHelper.makeDirectories("", _sOutputPath);
            HTMLOutputter a = new HTMLOutputter();
            String fs = System.getProperty("file.separator");
            String sFilename = _sOutputPath + fs + _sHTMLFilename;

            try
            {
                File outputFile = new File(sFilename);
                a.m_aOut = new FileWriter(outputFile.toString());
                a.ls = System.getProperty("line.separator");
            }
            catch (java.io.IOException e)
            {
                e.printStackTrace();
                GlobalLogWriter.get().println("ERROR: Can't create HTML Outputter");
                return null;
            }
            a.m_sFilename = sFilename;
            a.m_sNamePrefix = _sNamePrefix;
            return a;
        }
    public String getFilename() {return m_sFilename;}

    public void header(String _sTitle)
        {
            try
            {
                m_aOut.write( "<html>" + ls);
                m_aOut.write( "<head>"  + ls);
                m_aOut.write( "<title>" + _sTitle + "</title>" + ls);
                m_aOut.write( "<link rel=\"stylesheet\" type=\"text/css\" href=\"/gfxcmp_ui/xmloff.css\" media=\"screen\" />" + ls);
                m_aOut.write( "<link rel=\"stylesheet\" type=\"text/css\" href=\"/gfxcmp_ui/style.css\" media=\"screen\" />" + ls);
                m_aOut.write( "</head>" + ls);
                m_aOut.write( "<body bgcolor=white>" + ls);
                m_aOut.flush();
            }
            catch (java.io.IOException e)
            {
            }
        }

    private final static String TEST_TABLETITLE = "Test";
    private final static String VISUAL_STATUS_TABLETITLE = "Visual status";
    private final static String VISUAL_STATUS_MESSAGE_TABLETITLE = "Message";

    public void indexSection(String _sOfficeInfo)
        {
            try
            {
                m_aOut.write( "<h2>Results for " + _sOfficeInfo + "</h2>" + ls);
                m_aOut.write( "<p>Legend:<br>");
                m_aOut.write( stronghtml(FIRSTGFX_TABLETITLE) + " contains the output printed via 'ghostscript' as a jpeg picture.<br>");

                m_aOut.write( "<table class=\"infotable\">" + ls);
                m_aOut.write( "<TR>");
                m_aOut.write( tableHeaderCell(TEST_TABLETITLE));
                m_aOut.write( tableHeaderCell(TEST_TABLETITLE));
                m_aOut.write( tableHeaderCell(VISUAL_STATUS_TABLETITLE));
                m_aOut.write( tableHeaderCell(VISUAL_STATUS_MESSAGE_TABLETITLE));
                m_aOut.write( "</TR>" + ls);
                m_aOut.flush();
            }
            catch (java.io.IOException e)
            {
            }
        }

    private String getHREF(String _sHREF, String _sPathInfo)
        {
            StringBuffer a = new StringBuffer();
            if (! OSHelper.isWindows())
            {
                a.append("<A HREF=\"");
                a.append(_sHREF);
                a.append("\">");
                a.append(_sPathInfo);
                a.append("</A>");
            }
            else
            {
                //! this should be replaced by a better method
                //! name(WIN|UNIX)
                a.append("<A HREF=\"");
                a.append(_sHREF);
                a.append("\">");
                a.append(_sPathInfo);
                a.append("</A>");

            }
            return a.toString();
        }

    private String tableDataCell(String _sValue)
        {
            StringBuffer a = new StringBuffer();
            a.append("<TD>");
            a.append(_sValue);
            a.append("</TD>");
            return a.toString();
        }

    private String tableHeaderCell(String _sValue)
        {
            StringBuffer a = new StringBuffer();
            a.append("<TH>");
            a.append(_sValue);
            a.append("</TH>");
            return a.toString();
        }

    public void indexLine(String _sHTMLFile, String _sHTMLName, String _sHTMLFile2, String _sHTMLName2, String _sStatusRunThrough, String _sStatusMessage)
        {
            try
            {
                m_aOut.write( "<TR>");
                m_aOut.write(tableDataCell( getHREF(_sHTMLFile, _sHTMLName) ) );
                if (_sHTMLFile2.length() > 0)
                {
                    m_aOut.write(tableDataCell( getHREF(_sHTMLFile2, _sHTMLName2) ) );
                }
                else
                {
                    m_aOut.write(tableDataCell( "" ) );
                }

                m_aOut.write( tableDataCell(_sStatusRunThrough) );
                m_aOut.write( tableDataCell(_sStatusMessage) );
                m_aOut.write( "</TR>" + ls);

                m_aOut.flush();
            }
            catch (java.io.IOException e)
            {
            }
        }

    public void close()
        {
            try
            {
                m_aOut.write( "</TABLE>" + ls);
                m_aOut.write( "</BODY></HTML>" + ls);
                m_aOut.flush();
                m_aOut.close();
            }
            catch (java.io.IOException e)
            {
            }
        }


    private String stronghtml(String _sValue)
        {
            StringBuffer a = new StringBuffer();
            a.append("<STRONG>");
            a.append(_sValue);
            a.append("</STRONG>");
            return a.toString();
        }

    private final static String FIRSTGFX_TABLETITLE = "Original print file as jpeg";
    private final static String SECONDGFX_TABLETITLE = "New print file as jpeg";
    private final static String DIFFER_TABLETITLE = "Difference file";
    private final static String STATUS_TABLETITLE = "Status";
    private final static String PIXELDIFF_TABLETITLE = "Pixel difference in %";

    private final static String PIXELDIFF_BM_TABLETITLE = "P.diff. in % after remove border";
    private final static String DIFFER_BM_TABLETITLE = "Diff file (RB)";

    private final static String OK_TABLETITLE = "OK?";









}
