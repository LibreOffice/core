/*
 ************************************************************************
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

package graphical;

import java.io.File;
import java.io.FileWriter;
// import util.utils;
// import helper.OSHelper;

public class HTMLResult
{
    private FileWriter m_aOut;
    // private String m_sFilename;
    // private String m_sNamePrefix;              // the HTML files used a suffix to build it's right name

    /**
     * ls is the current line separator (carridge return)
     */
    private String ls;

    public HTMLResult( String _sOutputPath, String _sHTMLFilename )
        {
            FileHelper.makeDirectories("", _sOutputPath);
            // HTMLResult a = new HTMLResult();
            String sFilename = FileHelper.appendPath(_sOutputPath, _sHTMLFilename);

            try
            {
                File outputFile = new File(sFilename);
                m_aOut = new FileWriter(outputFile.toString());
                ls = System.getProperty("line.separator");
            }
            catch (java.io.IOException e)
            {
                e.printStackTrace();
                GlobalLogWriter.println("ERROR: Can't create HTML Outputter");
                // return null;
            }
            // m_sFilename = sFilename;
            // a.m_sNamePrefix = _sNamePrefix;
            // return a;
        }

    // public String getFilename() {return m_sFilename;}

    private void writeln(String _sStr)
    {
            try
            {
                m_aOut.write( _sStr );
                m_aOut.write ( ls );
            }
            catch (java.io.IOException e)
            {
            }
    }
    private void flush()
    {
        try
        {
            m_aOut.flush();
        }
        catch (java.io.IOException e)
        {
        }
    }


    /**
     * create the HTML header
     * @param _sTitle
     */
    public void header(String _sTitle)
        {
                writeln( "<HTML>");
                writeln( "<HEAD>" );
                writeln( "<TITLE>" + _sTitle + "</TITLE>");
                writeln( "<LINK rel=\"stylesheet\" type=\"text/css\" href=\"/gfxcmp_ui/xmloff.css\" media=\"screen\" />");
                writeln( "<LINK rel=\"stylesheet\" type=\"text/css\" href=\"/gfxcmp_ui/style.css\" media=\"screen\" />");
                writeln( "</HEAD>");
                writeln( "<BODY bgcolor=white>");
                flush();
        }

    final static String TEST_TABLETITLE = "Document";
    final static String VISUAL_STATUS_TABLETITLE = "Visual status";
    final static String VISUAL_STATUS_MESSAGE_TABLETITLE = "Message";
    final static String FIRSTGFX_TABLETITLE = "Original print file as jpeg";

    public void indexSection(String _sOfficeInfo)
        {
                writeln( "<H2>Results for " + _sOfficeInfo + "</H2>");
                writeln( "<P>This result was created at: " + DateHelper.getDateTimeForHumanreadableLog());
                writeln( "<P>Legend:<BR>");
                writeln( stronghtml(FIRSTGFX_TABLETITLE) + " contains the output printed via 'ghostscript' as a jpeg picture.<BR>");

                writeln( "<TABLE class=\"infotable\">");
                writeln( "<TR>");
                writeln( tableHeaderCell(TEST_TABLETITLE));
                writeln( tableHeaderCell(""));
                writeln( tableHeaderCell(VISUAL_STATUS_TABLETITLE));
                writeln( tableHeaderCell(VISUAL_STATUS_MESSAGE_TABLETITLE));
                writeln( "</TR>");
                flush();
        }
/**
 * Returns the given _sHREF & _sPathInfo as a HTML String
 * <A HREF="_sHREF">_sPathInfo</A>
 * @param _sHREF
 * @param _sPathInfo
 * @return
 */
    private String getHREF(String _sHREF, String _sPathInfo)
        {
            StringBuffer a = new StringBuffer();
            a.append("<A HREF=\"");
            a.append(_sHREF);
            a.append("\">");
            a.append(_sPathInfo);
            a.append("</A>");
            return a.toString();
        }

    /**
     * Returns the given _sValue as a HTML Table cell with _sValue as content
     * @param _sValue
     * @return
     */
    private String tableDataCell(String _sValue)
        {
            StringBuffer a = new StringBuffer();
            a.append("<TD>");
            a.append(_sValue);
            a.append("</TD>");
            return a.toString();
        }

    /**
     * Returns the given _sValue as a HTML Table header cell with _sValue as content
     * @param _sValue
     * @return
     */
    private String tableHeaderCell(String _sValue)
        {
            StringBuffer a = new StringBuffer();
            a.append("<TH>");
            a.append(_sValue);
            a.append("</TH>");
            return a.toString();
        }

    public void indexLine(String _sHTMLFile, String _sHTMLName, String _sStatusRunThrough, String _sStatusMessage)
        {
                writeln( "<TR>");
                writeln(tableDataCell( getHREF(_sHTMLFile, _sHTMLName) ) );
                writeln(tableDataCell( "" ) );
                writeln( tableDataCell(_sStatusRunThrough) );
                writeln( tableDataCell(_sStatusMessage) );
                writeln( "</TR>");
                flush();
        }

    public void close()
        {
            writeln( "</TABLE>");
            writeln( "</BODY></HTML>");
            try
            {
                m_aOut.close();
            }
            catch (java.io.IOException e)
            {
            }
        }

// -----------------------------------------------------------------------------
    private String stronghtml(String _sValue)
        {
            StringBuffer a = new StringBuffer();
            a.append("<STRONG>");
            a.append(_sValue);
            a.append("</STRONG>");
            return a.toString();
        }

}
