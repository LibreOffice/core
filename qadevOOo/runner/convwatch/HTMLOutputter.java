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
    FileWriter m_aOut;
    String m_sFilename;
    String m_sNamePrefix;              // the HTML files used a suffix to build it's right name

    /**
     * ls is the current line separator (carridge return)
     */
    String ls;

    HTMLOutputter() {}
    public static HTMLOutputter create( String _sOutputPath, String _sHTMLFilename, String _sNamePrefix, String _sTitle )
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

    final static String TEST_TABLETITLE = "Test";
    final static String VISUAL_STATUS_TABLETITLE = "Visual status";
    final static String VISUAL_STATUS_MESSAGE_TABLETITLE = "Message";

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

    String getHREF(String _sHREF, String _sPathInfo)
        {
            StringBuffer a = new StringBuffer();
            if (! OSHelper.isWindows())
            {
                // System.out.println("Tu'nix system.");
                a.append("<A HREF=\"");
                a.append(_sHREF);
                a.append("\">");
                a.append(_sPathInfo);
                a.append("</A>");
            }
            else
            {
                // System.out.println("Windows system.");
                //! this should be replaced by a better method
                //! name(WIN|UNIX)
                a.append("<A HREF=\"");
                a.append(_sHREF);
                a.append("\">");
                a.append(_sPathInfo);
                // a.append("(first)");
                a.append("</A>");
                // if (_sHREF.charAt(1) == ':' && (_sHREF.charAt(0) == 'x' || _sHREF.charAt(0) == 'X'))
                // int index = 0;
                // index = _sHREF.indexOf("X:");
                // if (index == -1)
                // {
                //     index = _sHREF.indexOf("x:");
                // }
                // if (index >= 0)
                // {
                //     // int index = 0;
                //     // remove "X:" and insert "/tausch"
                //     StringBuffer sbUNIXPath = new StringBuffer( _sHREF.substring(0, index) );
                //     sbUNIXPath.append("/tausch");
                //     sbUNIXPath.append(_sHREF.substring(index + 2));
                //     String sUNIXPath = sbUNIXPath.toString();
                //     sUNIXPath = utils.replaceAll13(sUNIXPath, "\\", "/");

                //     a.append("<A HREF=\"");
                //     a.append(sUNIXPath);
                //     a.append("\">");
                //     a.append("(second)");
                //     a.append("</A>");
                // }
                // else
                // {
                //     System.out.println("Path is '" + _sHREF + "'");
                // }

            }
            return a.toString();
        }

    String tableDataCell(String _sValue)
        {
            StringBuffer a = new StringBuffer();
            a.append("<TD>");
            a.append(_sValue);
            a.append("</TD>");
            return a.toString();
        }

    String tableHeaderCell(String _sValue)
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


    String stronghtml(String _sValue)
        {
            StringBuffer a = new StringBuffer();
            a.append("<STRONG>");
            a.append(_sValue);
            a.append("</STRONG>");
            return a.toString();
        }

    final static String FIRSTGFX_TABLETITLE = "Original print file as jpeg";
    final static String SECONDGFX_TABLETITLE = "New print file as jpeg";
    final static String DIFFER_TABLETITLE = "Difference file";
    final static String STATUS_TABLETITLE = "Status";
    final static String PIXELDIFF_TABLETITLE = "Pixel difference in %";

    final static String PIXELDIFF_BM_TABLETITLE = "P.diff. in % after remove border";
    final static String DIFFER_BM_TABLETITLE = "Diff file (RB)";

    final static String OK_TABLETITLE = "OK?";
    public void checkSection(String _sDocumentName)
        {
            try
            {
                m_aOut.write( "<H2>Results for the document " + _sDocumentName + "</H2>" + ls);

                m_aOut.write( "<p>Legend:<br>");
                m_aOut.write( stronghtml(FIRSTGFX_TABLETITLE) + " contains the output printed via 'ghostscript' as a jpeg picture.<br>");
                m_aOut.write( stronghtml(SECONDGFX_TABLETITLE) + " contains the same document opened within OpenOffice.org also printed via ghostscript as jpeg.<br>");
                m_aOut.write( stronghtml(DIFFER_TABLETITLE)+" is build via composite from original and new picture. The result should be a whole black picture, if there are no differences.<br>At the moment "+stronghtml(STATUS_TABLETITLE)+" is only ok, if the difference file contains only one color (black).</p>" );
                m_aOut.write( stronghtml(DIFFER_BM_TABLETITLE) + " is build via composite from original and new picture after the border of both pictures are removed, so differences based on center problems may solved here");
                m_aOut.write( "</p>");
                m_aOut.write( "<p>Some words about the percentage value<br>");
                m_aOut.write( "If a character is on the original page (a) and on the new page this character is moved to an other position only (b) , this means the difference is 100%.<br>");
                m_aOut.write( "If character (b) is also bigger than character (a) the percentage is grow over the 100% mark.<br>");
                m_aOut.write( "This tool count only the pixels which are differ to it's background color. It makes no sense to count all pixels, or the difference percentage will most the time in a very low percentage range.");
                m_aOut.write( "</p>");

                m_aOut.write( "<table class=\"infotable\">" + ls);

                m_aOut.write( "<TR>" + ls);
                m_aOut.write( tableHeaderCell( FIRSTGFX_TABLETITLE) );
                m_aOut.write( tableHeaderCell( SECONDGFX_TABLETITLE ) );
                m_aOut.write( tableHeaderCell(DIFFER_TABLETITLE ) );
                m_aOut.write( tableHeaderCell(PIXELDIFF_TABLETITLE ) );

                m_aOut.write( tableHeaderCell(DIFFER_BM_TABLETITLE) );
                m_aOut.write( tableHeaderCell(PIXELDIFF_BM_TABLETITLE ) );

                m_aOut.write( tableHeaderCell( OK_TABLETITLE) );

                m_aOut.write( "</TR>" + ls);
                m_aOut.flush();
            }
            catch (java.io.IOException e)
            {
            }
        }

    public void checkLine(StatusHelper _aStatus, boolean _bCurrentResult)
        {
            try
            {
                m_aOut.write( "<TR>" + ls);
                String sLink = getHREF(FileHelper.getBasename(_aStatus.m_sOldGfx), FileHelper.getBasename(_aStatus.m_sOldGfx));
                m_aOut.write( tableDataCell(sLink) );

                sLink = getHREF(FileHelper.getBasename(_aStatus.m_sNewGfx), FileHelper.getBasename(_aStatus.m_sNewGfx));
                m_aOut.write( tableDataCell(sLink) );

                sLink = getHREF(FileHelper.getBasename(_aStatus.m_sDiffGfx), FileHelper.getBasename(_aStatus.m_sDiffGfx));
                m_aOut.write( tableDataCell(sLink) );

                String sPercent = String.valueOf(_aStatus.nPercent) + "%";
                if (_aStatus.nPercent > 0 && _aStatus.nPercent < 5)
                {
                    sPercent += " (less 5% is ok)";
                }
                m_aOut.write(tableDataCell( sPercent ) );

                if (_aStatus.m_sDiff_BM_Gfx == null)
                {
                    sLink = "No diffs, therefore no moves";
                    m_aOut.write( tableDataCell(sLink) );
                    m_aOut.write(tableDataCell( "" ) );
                }
                else
                {
                    sLink = getHREF(FileHelper.getBasename(_aStatus.m_sDiff_BM_Gfx), FileHelper.getBasename(_aStatus.m_sDiff_BM_Gfx));
                    m_aOut.write( tableDataCell(sLink) );

                    String sPercent2 = String.valueOf(_aStatus.nPercent2) + "%";
                    if (_aStatus.nPercent2 > 0 && _aStatus.nPercent2 < 5)
                    {
                        sPercent2 += " (less 5% is ok)";
                    }
                    m_aOut.write(tableDataCell( sPercent2 ) );
                }

                // is the check positiv, in a defined range
                if (_bCurrentResult)
                {
                    m_aOut.write(tableDataCell( "YES" ) );
                }
                else
                {
                    m_aOut.write(tableDataCell( "NO" ) );
                }

                m_aOut.write( "</TR>" + ls);
            }
            catch (java.io.IOException e)
            {
            }
        }


    public void checkDiffDiffSection(String _sDocumentName)
        {
            try
            {
                m_aOut.write( "<H2>Results for the document " + _sDocumentName + "</H2>" + ls);

                m_aOut.write( "<p>Legend:<br>");
                m_aOut.write( "</p>");

                m_aOut.write( "<table class=\"infotable\">" + ls);

                m_aOut.write( "<TR>" + ls);
                m_aOut.write( tableHeaderCell( "Source to actual difference" ) );
                m_aOut.write( tableHeaderCell( "Actual difference" ) );
                m_aOut.write( tableHeaderCell(DIFFER_TABLETITLE ) );
                m_aOut.write( tableHeaderCell(PIXELDIFF_TABLETITLE ) );

                m_aOut.write( tableHeaderCell( OK_TABLETITLE) );

                m_aOut.write( "</TR>" + ls);
                m_aOut.flush();
            }
            catch (java.io.IOException e)
            {
            }
        }

    public void checkDiffDiffLine(StatusHelper _aStatus, boolean _bCurrentResult)
        {
            try
            {
                m_aOut.write( "<TR>" + ls);
                // the link to the old difference can't offer here
                //  String sLink = getHREF(FileHelper.getBasename(_aStatus.m_sOldGfx), FileHelper.getBasename(_aStatus.m_sOldGfx));
                //  m_aOut.write( tableDataCell(sLink) );

                String sBasename = FileHelper.getBasename(m_sFilename);
                String sNew = sBasename.substring(m_sNamePrefix.length());

                String sLink;
                sLink = getHREF(sNew, sNew);
                m_aOut.write( tableDataCell(sLink) );

                sLink = getHREF(FileHelper.getBasename(_aStatus.m_sNewGfx), FileHelper.getBasename(_aStatus.m_sNewGfx));
                m_aOut.write( tableDataCell(sLink) );

                sLink = getHREF(FileHelper.getBasename(_aStatus.m_sDiffGfx), FileHelper.getBasename(_aStatus.m_sDiffGfx));
                m_aOut.write( tableDataCell(sLink) );

                String sPercent = String.valueOf(_aStatus.nPercent) + "%";
                // if (_aStatus.nPercent > 0 && _aStatus.nPercent < 5)
                // {
                //     sPercent += " (less 5% is ok)";
                // }
                m_aOut.write(tableDataCell( sPercent ) );

                // is the check positiv, in a defined range
                if (_bCurrentResult)
                {
                    m_aOut.write(tableDataCell( "YES" ) );
                }
                else
                {
                    m_aOut.write(tableDataCell( "NO" ) );
                }

                m_aOut.write( "</TR>" + ls);
            }
            catch (java.io.IOException e)
            {
            }
        }

}
