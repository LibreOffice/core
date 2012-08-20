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

package complex.framework.autosave;

// __________ Imports __________

// structs, const, ...
import com.sun.star.beans.PropertyValue;
import com.sun.star.bridge.XUnoUrlResolver;

// exceptions
import com.sun.star.container.NoSuchElementException;
import com.sun.star.uno.Exception;
import com.sun.star.uno.RuntimeException;
import java.io.IOException;
import java.lang.InterruptedException;
import java.net.ConnectException;

// interfaces
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.Any;

// helper
import com.sun.star.uno.IBridge;
import com.sun.star.uno.UnoRuntime;

// others
import javax.swing.*;
import javax.swing.border.*;
import java.awt.*;
import java.lang.*;
import java.io.*;
import java.util.*;
import java.sql.*;

// __________ Implementation __________

/**
 * Implements a log mechanism to create a protocol of all steps of e.g. an api test
 * It provides the possibility to write the logged meesages to a file and/or
 * to stdout/stderr (if neccessary at the same time!).
 *
 *  TODO
 *      - implement filter, which e.g. supress showing of INFO data
 */
public class Protocol extends JComponent
{
    // ____________________
    /**
     * Note: Following values can be combined - they are interpreted as flags.
     *
     * @const   MODE_STDOUT             messages are logged to stdout
     * @const   MODE_STDERR             messages are logged to stderr
     * @const   MODE_ASCII              messages are logged to an ascii file
     * @const   MODE_HTML               messages are logged to a html file
     *
     * @const   TYPE_SCOPE_OPEN         open, mark or count a new scope for following log statements
     * @const   TYPE_SCOPE_CLOSE        close, mark or count the current scope
     * @const   TYPE_TESTMARK           it marks the beginning of a (sub)test, can be used for statistic purposes
     * @const   TYPE_OK                 this protocol line is marked as a OK message
     * @const   TYPE_ERROR              this protocol line is marked as an error
     * @const   TYPE_WARNING            this protocol line is marked as a warning
     * @const   TYPE_INFO               this protocol line represent some debug data for analyzing
     */
    public  static final int    MODE_STDOUT             = 1;
    public  static final int    MODE_STDERR             = 2;
    public  static final int    MODE_ASCII              = 4;
    public  static final int    MODE_HTML               = 8;

    public  static final int    TYPE_OK                 =    1;
    public  static final int    TYPE_ERROR              =    2;
    public  static final int    TYPE_WARNING            =    4;
    public  static final int    TYPE_INFO               =    8;
    public  static final int    TYPE_SCOPE_OPEN         =   16;
    public  static final int    TYPE_SCOPE_CLOSE        =   32;
    public  static final int    TYPE_TESTMARK           =   64;
    public  static final int    TYPE_ERROR_INFO         =  128;
    public  static final int    TYPE_WARNING_INFO       =  256;
    public  static final int    TYPE_STATISTIC          =  512;
    public  static final int    TYPE_LINK               = 1024;

    public  static final int    FILTER_NONE             = 0;
    public  static final int    FILTER_OK               = TYPE_OK;
    public  static final int    FILTER_ERROR            = TYPE_ERROR;
    public  static final int    FILTER_WARNING          = TYPE_WARNING;
    public  static final int    FILTER_INFO             = TYPE_INFO;
    public  static final int    FILTER_SCOPES           = TYPE_SCOPE_OPEN | TYPE_SCOPE_CLOSE;
    public  static final int    FILTER_TESTMARK         = TYPE_TESTMARK;
    public  static final int    FILTER_ERROR_INFO       = TYPE_ERROR_INFO;
    public  static final int    FILTER_WARNING_INFO     = TYPE_WARNING_INFO;
    public  static final int    FILTER_STATISTIC        = TYPE_STATISTIC;
    public  static final int    FILTER_LINK             = TYPE_LINK;

    // ____________________
    /**
     */
    private static final int    MARK_DIFF               = 5;

    private static final String BGCOLOR_LINECOL         = "#95CC77";
    private static final String FGCOLOR_LINECOL_NORMAL  = "#ffffbd";
    private static final String FGCOLOR_LINECOL_MARKED  = "#000088";

    private static final String BGCOLOR_STANDARD        = "#ffffff";
    private static final String FGCOLOR_STANDARD        = "#000000";

    private static final String BGCOLOR_SCOPE           = "#eeeeee";
    private static final String FGCOLOR_SCOPE           = "#000000";

    private static final String BGCOLOR_TIMESTAMP       = "#e0e0e0";
    private static final String FGCOLOR_TIMESTAMP       = "#000000";

    private static final String BGCOLOR_TESTMARK        = "#0000ff";
    private static final String FGCOLOR_TESTMARK        = "#ffffff";

    private static final String BGCOLOR_OK              = "#88dd88";
    private static final String FGCOLOR_OK              = "#ffffff";

    private static final String BGCOLOR_WARNING         = "#ffff00";
    private static final String FGCOLOR_WARNING         = "#000000";

    private static final String BGCOLOR_WARNING_INFO    = "#ffffcc";
    private static final String FGCOLOR_WARNING_INFO    = "#000000";

    private static final String BGCOLOR_ERROR           = "#ff0000";
    private static final String FGCOLOR_ERROR           = "#ffff00";

    private static final String BGCOLOR_ERROR_INFO      = "#ffbbbb";
    private static final String FGCOLOR_ERROR_INFO      = "#000000";

    private static final String BGCOLOR_INFO            = "#eeeeee";
    private static final String FGCOLOR_INFO            = "#000000";

    private static final String BGCOLOR_STATISTIC       = "#0000ff";
    private static final String FGCOLOR_STATISTIC       = "#ffffff";

    private static final String BGCOLOR_LINK            = BGCOLOR_INFO;
    private static final String FGCOLOR_LINK            = FGCOLOR_INFO;

    // ____________________
    /**
     * @member  m_nMode         the mode, in which this protocol object runs
     * @member  m_nFilter       can be used to filter log messages by type
     * @member  m_sFileName     we need it to open the log file on demand (if nMode require such log file)
     * @member  m_nLine         used as line number for the protocol
     * @member  m_nScope        used to format scopes
     * @member  m_nErrors       count errors in protocol
     * @member  m_nWarnings     count warnings in protocol
     * @member  m_nTestMarks    count test marker in protocol
     */
    private int                 m_nMode     ;
    private int                 m_nFilter   ;
    private String              m_sFileName ;
    private long                m_nLine     ;
    private long                m_nScope    ;
    private long                m_nErrors   ;
    private long                m_nWarnings ;
    private long                m_nTestMarks;
    private Timestamp           m_aStartTime;
    private Timestamp           m_aEndTime  ;

    // ____________________
    /**
     * special helper class to represent one line of a protocol.
     * Such line can be specified as a special one (ERROR, WARNING ...).
     * That makes it possible to analyze the whole protocol using tools.
     */
    class ProtocolLine
    {
        /// the line number of this protocol line (size of the vector of all protocol lines cn be used to count such lines!)
        private long m_nLine;
        /// deepness of the current scope
        private long m_nScope;
        /// mark line as an error, warning, data entry ... (see const definitions before)
        private int m_nType;
        /// of course, we have to know the logged message too :-)
        private String m_sMessage;
        /// and it can be usefull to know the current time, when this line was created
        private Timestamp m_aStamp;

        /** ctor for fast initializing of such line */
        public ProtocolLine( long   nLine    ,
                             long   nScope   ,
                             int    nType    ,
                             String sMessage )
        {
            m_aStamp   = new Timestamp(System.currentTimeMillis());
            m_nLine    = nLine   ;
            m_nScope   = nScope  ;
            m_nType    = nType   ;
            m_sMessage = sMessage;
        }

        /** format this line as an ascii string for writing log files */
        public synchronized String toString()
        {
            StringBuffer sLine = new StringBuffer(1000);

            // insert line number
            // Use right bound notation and format 6 digits!
            sLine.append("["    );
            if (m_nLine<10)
                sLine.append("     ");
            else
            if (m_nLine<100)
                sLine.append("    ");
            else
            if (m_nLine<1000)
                sLine.append("   ");
            else
            if (m_nLine<10000)
                sLine.append("  ");
            else
            if (m_nLine<100000)
                sLine.append(" ");
            sLine.append(m_nLine);
            sLine.append("] "   );

            // add time stamp
            // close with a "TAB" ... because some time stamps are not normalized to
            // a well defined string length .-)
            sLine.append(m_aStamp.toString()+" \t");

            // add special line type
            if ((m_nType & TYPE_OK) == TYPE_OK)
                sLine.append(" OK           ");
            else
            if ((m_nType & TYPE_ERROR) == TYPE_ERROR)
                sLine.append(" ERROR        ");
            else
            if ((m_nType & TYPE_ERROR_INFO) == TYPE_ERROR_INFO)
                sLine.append(" ERROR INFO   ");
            else
            if ((m_nType & TYPE_WARNING) == TYPE_WARNING)
                sLine.append(" WARNING      ");
            else
            if ((m_nType & TYPE_WARNING_INFO) == TYPE_WARNING_INFO)
                sLine.append(" WARNING INFO ");
            else
            if ((m_nType & TYPE_INFO) == TYPE_INFO)
                sLine.append(" INFO         ");
            else
            if ((m_nType & TYPE_TESTMARK) == TYPE_TESTMARK)
                sLine.append(" TEST         ");
            else
            if ((m_nType & TYPE_LINK) == TYPE_LINK)
                sLine.append(" LINK         ");
            else
            if ((m_nType & TYPE_STATISTIC) == TYPE_STATISTIC)
                sLine.append(" STATISTIC    ");
            else
            if (
                ((m_nType & TYPE_SCOPE_OPEN ) == TYPE_SCOPE_OPEN ) ||
                ((m_nType & TYPE_SCOPE_CLOSE) == TYPE_SCOPE_CLOSE)
               )
                sLine.append(" SCOPE        ");
            else
                sLine.append("              ");

            // add scope information
            for (int s=0; s<m_nScope; ++s)
                sLine.append(" ");

            if ((m_nType & TYPE_SCOPE_OPEN) == TYPE_SCOPE_OPEN)
                sLine.append(" { ");
            else
            if ((m_nType & TYPE_SCOPE_CLOSE) == TYPE_SCOPE_CLOSE)
                sLine.append(" } ");
            else
                sLine.append("   ");

            // add message
            sLine.append(m_sMessage);
            sLine.append("\n"      );

            return sLine.toString();
        }

        /**
         * format this line as a string for writing log files
         * using the html format
         */
        public synchronized String toHTML()
        {
            StringBuffer sLine = new StringBuffer(1000);
            sLine.append("<tr>");

            // insert line number
            if (m_nLine % MARK_DIFF == 0)
                impl_generateColoredHTMLCell(sLine, new Long(m_nLine).toString(), BGCOLOR_LINECOL, FGCOLOR_LINECOL_MARKED, true);
            else
                impl_generateColoredHTMLCell(sLine, new Long(m_nLine).toString(), BGCOLOR_LINECOL, FGCOLOR_LINECOL_NORMAL, false);

            // add time stamp
            impl_generateColoredHTMLCell(sLine, m_aStamp.toString()+" ", BGCOLOR_TIMESTAMP, FGCOLOR_TIMESTAMP, false);

            // add log type info
            boolean bTypeCellFilled = false;
            if ((m_nType & TYPE_ERROR_INFO) == TYPE_ERROR_INFO)
            {
                impl_generateColoredHTMLCell(sLine, "ERROR INFO", BGCOLOR_ERROR_INFO, FGCOLOR_ERROR_INFO, false);
                bTypeCellFilled = true;
            }
            else
            if ((m_nType & TYPE_ERROR) == TYPE_ERROR)
            {
                impl_generateColoredHTMLCell(sLine, "ERROR", BGCOLOR_ERROR, FGCOLOR_ERROR, true);
                bTypeCellFilled = true;
            }
            else
            if ((m_nType & TYPE_WARNING_INFO) == TYPE_WARNING_INFO)
            {
                impl_generateColoredHTMLCell(sLine, "WARNING INFO", BGCOLOR_WARNING_INFO, FGCOLOR_WARNING_INFO, false);
                bTypeCellFilled = true;
            }
            else
            if ((m_nType & TYPE_WARNING) == TYPE_WARNING)
            {
                impl_generateColoredHTMLCell(sLine, "WARNING", BGCOLOR_WARNING, FGCOLOR_WARNING, true);
                bTypeCellFilled = true;
            }
            else
            if ((m_nType & TYPE_OK) == TYPE_OK)
            {
                impl_generateColoredHTMLCell(sLine, "OK", BGCOLOR_OK, FGCOLOR_OK, true);
                bTypeCellFilled = true;
            }
            else
            if ((m_nType & TYPE_INFO) == TYPE_INFO)
            {
                impl_generateColoredHTMLCell(sLine, "INFO", BGCOLOR_INFO, FGCOLOR_INFO, false);
                bTypeCellFilled = true;
            }
            else
            if ((m_nType & TYPE_TESTMARK) == TYPE_TESTMARK)
            {
                impl_generateColoredHTMLCell(sLine, "TEST", BGCOLOR_TESTMARK, FGCOLOR_TESTMARK, true);
                bTypeCellFilled = true;
            }
            else
            if ((m_nType & TYPE_STATISTIC) == TYPE_STATISTIC)
            {
                impl_generateColoredHTMLCell(sLine, "STATISTIC", BGCOLOR_STATISTIC, FGCOLOR_STATISTIC, false);
                bTypeCellFilled = true;
            }
            else
            if ((m_nType & TYPE_LINK) == TYPE_LINK)
            {
                impl_generateColoredHTMLCell(sLine, "LINK", BGCOLOR_LINK, FGCOLOR_LINK, false);
                bTypeCellFilled = true;
            }
            else
            if (
                ((m_nType & TYPE_SCOPE_OPEN ) == TYPE_SCOPE_OPEN ) ||
                ((m_nType & TYPE_SCOPE_CLOSE) == TYPE_SCOPE_CLOSE)
               )
            {
                impl_generateColoredHTMLCell(sLine, "SCOPE", BGCOLOR_SCOPE, FGCOLOR_SCOPE, false);
                bTypeCellFilled = true;
            }

            // if no tyope information was added to the current coloum, we must
            // write any content into this cell. Otherwise some browser
            // shows a strange layout!
            if (! bTypeCellFilled)
                impl_generateColoredHTMLCell(sLine, " ", BGCOLOR_STANDARD, FGCOLOR_STANDARD, false);

            // add scope information
            sLine.append("<td>");
            for (int s=0; s<m_nScope; ++s)
                sLine.append("&nbsp;&nbsp;&nbsp;");
            String sColor = "#000000";
            if ((m_nScope % 2) == 0)
                sColor = "#808080";
            if ((m_nType & TYPE_SCOPE_OPEN) == TYPE_SCOPE_OPEN)
                sLine.append("<font color=\""+sColor+"\">{ "+m_nScope+"</font>");
            else
            if ((m_nType & TYPE_SCOPE_CLOSE) == TYPE_SCOPE_CLOSE)
                sLine.append("<font color=\""+sColor+"\">"+m_nScope+" }</font>");
            sLine.append("</td>\n");

            // add message
            sLine.append("<td>"    );
            sLine.append(m_sMessage);
            sLine.append("</td>\n" );

            sLine.append("</tr>\n" );

            return sLine.toString();
        }

        /** detect, if this line object represent an error */
        public synchronized boolean isError()
        {
            return (
                    ((m_nType & TYPE_ERROR) == TYPE_ERROR) &&
                    ((m_nType & TYPE_INFO ) != TYPE_INFO )
                   );
        }

        /** detect, if this line object represent a warning */
        public synchronized boolean isWarning()
        {
            return (
                    ((m_nType & TYPE_WARNING) == TYPE_WARNING) &&
                    ((m_nType & TYPE_INFO   ) != TYPE_INFO   )
                   );
        }

        /** detect, if this line object represent a marked position */
        public synchronized boolean isTestMark()
        {
            return ((m_nType & TYPE_TESTMARK) == TYPE_TESTMARK);
        }

        /**
         * create a colored table cell formated as HTML.
         *
         * @param   sCell
         *          an outside string buffer, which can be
         *          used to generate the
         *          needed HTML code there.
         *
         * @param   sContent
         *          the text content of this cell.
         *
         * @param   sBGColor
         *          a string, which represent the background color
         *          coded in HTML.
         *
         * @param   sFGColor
         *          a string, which represent the foreground color
         *          coded in HTML.
         *
         * @param   bBold
         *          enable/disable bold state for the text content.
         */
        private void impl_generateColoredHTMLCell(StringBuffer sCell   ,
                                                  String       sContent,
                                                  String       sBGColor,
                                                  String       sFGColor,
                                                  boolean      bBold   )
        {
            sCell.append("<td bgcolor=\""+sBGColor+"\">");
            sCell.append("<font color=\""+sFGColor+"\">");
            if (bBold)
                sCell.append("<b>");
            sCell.append(sContent);
            if (bBold)
                sCell.append("</b>");
            sCell.append("</font></td>\n");
        }
    }

    // ____________________
    /**
     * ctor
     * It creates a new instance of this class and innitialize it in the right mode.
     *
     * @param nMode
     *          specify how the log should be generated.
     *
     * @param nFilter
     *          can be used to filter log messages by it's type.
     *
     * @param sFileName
     *          the name of the log file (if nMode requires a log file)
     */
    public Protocol(int    nMode    ,
                    int    nFilter  ,
                    String sFileName)
    {
        m_nMode      = nMode;
        m_nFilter    = nFilter;
        m_sFileName  = sFileName;
        m_nLine      = 0;
        m_nScope     = 1;
        m_nWarnings  = 0;
        m_nErrors    = 0;
        m_aStartTime = new Timestamp(System.currentTimeMillis());
    }

    // ____________________
    /**
     * For some modes it's neccessary, that we write some additional
     * informations to the log. E.g. for html we must generate close targets.
     */
    public synchronized void finish()
    {
        // Preferr HTML ... because we can't write ASCII and HTML contents to the same log file!
        String sContent;
        if ((m_nMode & MODE_HTML) == MODE_HTML)
            sContent = impl_generateHTMLFooter();
        else
        if ((m_nMode & MODE_ASCII) == MODE_ASCII)
            sContent = impl_generateAsciiFooter();
        else
            return;

        impl_writeToLogFile(m_sFileName, true, sContent);
    }

    // ____________________
    /**
     * log an unspecified message.
     *
     * Sometimes it's not neccessary to set a special type for an message.
     * The pure message seams to be enough. The type of such "pure messages"
     * will be set to INFO.
     *
     * @param   sMessage
     *              the pure message
     *
     * @see     #log(int, String)
     */
    public synchronized void log( /*IN*/ String sMessage )
    {
        log(TYPE_INFO, sMessage);
    }

    // ____________________
    /**
     * log an exception.
     *
     * It uses all informations available by this exception object
     * to generate the log. So exceptions are printed out using a
     * standard format.
     *
     * @param   exThrowable
     *              the exception
     */
    public synchronized void log( /*IN*/ Throwable exThrowable )
    {
        log(TYPE_SCOPE_OPEN | TYPE_ERROR, "exception \""+exThrowable.getMessage()+"\"");

        StackTraceElement[] lStack = exThrowable.getStackTrace();
        for (int i=0; i<lStack.length; ++i)
           log(TYPE_ERROR_INFO, lStack[i].toString());

        log(TYPE_SCOPE_CLOSE | TYPE_ERROR_INFO, "");
    }

    // ____________________
    /**
     * log different property arrays.
     *
     * @param   lProps
     *              the array of properties
     */
    public synchronized void log( /*IN*/ com.sun.star.beans.NamedValue[] lProps )
    {
        StringBuffer sValues = new StringBuffer(1000);
        impl_logPropertyArray(sValues, lProps);

        log(TYPE_SCOPE_OPEN  | TYPE_INFO, "property array ["+lProps.length+"]:");
        log(TYPE_SCOPE_CLOSE | TYPE_INFO, sValues.toString()                   );
    }

    public synchronized void log( /*IN*/ com.sun.star.beans.PropertyValue[] lProps )
    {
        StringBuffer sValues = new StringBuffer(1000);
        impl_logPropertyArray(sValues, lProps);

        log(TYPE_SCOPE_OPEN  | TYPE_INFO, "property array ["+lProps.length+"]:");
        log(TYPE_SCOPE_CLOSE | TYPE_INFO, sValues.toString()                   );
    }

    public synchronized void log( /*IN*/ com.sun.star.beans.NamedValue aProp )
    {
        StringBuffer sValue = new StringBuffer(1000);
        impl_logProperty(sValue, aProp);

        log(TYPE_SCOPE_OPEN  | TYPE_INFO, "property:"      );
        log(TYPE_SCOPE_CLOSE | TYPE_INFO, sValue.toString());
    }

    public synchronized void log( /*IN*/ com.sun.star.beans.PropertyValue aProp )
    {
        StringBuffer sValue = new StringBuffer(1000);
        impl_logProperty(sValue, aProp);

        log(TYPE_SCOPE_OPEN  | TYPE_INFO, "property:"      );
        log(TYPE_SCOPE_CLOSE | TYPE_INFO, sValue.toString());
    }

    public synchronized void log( /*IN*/ Object aAny )
    {
        StringBuffer sValue = new StringBuffer(1000);
        impl_logAny(sValue, aAny);

        log(TYPE_SCOPE_OPEN  | TYPE_INFO, "any:"           );
        log(TYPE_SCOPE_CLOSE | TYPE_INFO, sValue.toString());
    }

    // ____________________
    /**
     * log a message.
     *
     * It looks for the internal set mode and decide, how this message
     * will be handled. Then it generates a special object which represent
     * one protocol line, format it and print it out.
     *
     * @param nType
     *          mark a line as a special one or open/close scopes
     *
     * @param sMessage
     *          the message, which the outside code wish to be written into the log
     */
    public synchronized void log( /*IN*/ int    nType    ,
                                  /*IN*/ String sMessage )
    {
        nType = (nType & ~m_nFilter);
        if (nType == 0)
            return;

        ++m_nLine;

        // it's neccessary to open scopes before creatig the protocol line
        // to guarantee right tab handling for new scope value!
        if ((nType & TYPE_SCOPE_OPEN) == TYPE_SCOPE_OPEN)
            ++m_nScope;

        // create the protocol line
        ProtocolLine aLine     = new ProtocolLine(m_nLine, m_nScope, nType, sMessage);
        String       sAsciiLog = aLine.toString();
        String       sHTMLLog  = aLine.toHTML();

        // it's neccessary to close scope after creatig the protocol line
        // to guarantee right tab handling for old scope value!
        if (
            ( m_nScope                  >  0               ) &&
            ((nType & TYPE_SCOPE_CLOSE) == TYPE_SCOPE_CLOSE)
           )
        {
            --m_nScope;
        }

        // update statistic values
        if (aLine.isTestMark())
            ++m_nTestMarks;
        if (aLine.isWarning())
            ++m_nWarnings;
        if (aLine.isError())
            ++m_nErrors;

        // no else - it's a bit field of possible reactions!
        if ((m_nMode & MODE_STDOUT) == MODE_STDOUT)
            System.out.print(sAsciiLog);
        // no else - it's a bit field of possible reactions!
        if ((m_nMode & MODE_STDERR) == MODE_STDERR)
            System.err.print(sAsciiLog);
        // no else - it's a bit field of possible reactions!
        // But these both conditions must be handled together.
        // Because we cant generate different types of log contents to the same log file.
        // We preferr HTML if both types are set.
        if (
            ((m_nMode & MODE_HTML ) == MODE_HTML ) ||
            ((m_nMode & MODE_ASCII) == MODE_ASCII)
           )
        {
            boolean bAppend = (m_nLine>1);
            String  sContent;
            if ((m_nMode & MODE_HTML) == MODE_HTML)
            {
                if (! bAppend)
                    sContent = impl_generateHTMLHeader()+sHTMLLog;
                else
                    sContent = sHTMLLog;
            }
            else
            {
                if (! bAppend)
                    sContent = impl_generateAsciiHeader()+sAsciiLog;
                else
                    sContent = sAsciiLog;
            }

            impl_writeToLogFile(m_sFileName, bAppend, sContent);
        }
    }

    // ____________________
    public synchronized void defineHyperlink( /*IN*/ String sTarget     ,
                                              /*IN*/ String sDescription)
    {
        if ((m_nMode & MODE_HTML) != MODE_HTML)
            return;

        StringBuffer sLog = new StringBuffer(1000);
        sLog.append("<a href=\"");
        sLog.append(sTarget     );
        sLog.append("\">"       );
        sLog.append(sDescription);
        sLog.append("</a>"      );

        log(TYPE_LINK, sLog.toString());
    }

    // ____________________
    /**
     * log the current statistic values
     * We write it into our protocol buffer and
     * reset it.
     */
    public synchronized void logStatistics()
    {
                  m_aEndTime = new Timestamp(System.currentTimeMillis());
        Timestamp aDiff      = new Timestamp(m_aEndTime.getTime()-m_aStartTime.getTime());

        int nLogType = TYPE_STATISTIC;
        if (m_nErrors > 0)
            nLogType = TYPE_ERROR_INFO;
        else
        if (m_nWarnings > 0)
            nLogType = TYPE_WARNING_INFO;

        log(nLogType | TYPE_SCOPE_OPEN , "statistic:"                      );
        log(nLogType                   , "tests        = "+m_nTestMarks    );
        log(nLogType                   , "errors       = "+m_nErrors       );
        log(nLogType                   , "warnings     = "+m_nWarnings     );
        log(nLogType                   , "elapsed time = "+aDiff.toString());
        log(nLogType | TYPE_SCOPE_CLOSE, ""                                );

        resetStatistics();
    }

    public synchronized void resetStatistics()
    {
        m_nTestMarks = 0;
        m_nWarnings  = 0;
        m_nErrors    = 0;
        m_aStartTime = new Timestamp(System.currentTimeMillis());
    }

    // ____________________
    /**
     * returns a generic html header for generating html log files
     *
     * It's used from the method finish() to generate a valid html formated file.
     * For that its neccessary to open some special html targets like e.g. <html>.
     *
     * @return  A string, which includes the whole header.
     *
     * @see     #finish()
     * @see     #impl_generateHTMLFooter()
     */
    private String impl_generateHTMLHeader()
    {
        return "<html>\n<head>\n<title>"+m_sFileName+"</title>\n</head>\n<body>\n<table>\n";
    }

    private String impl_generateAsciiHeader()
    {
        return "********************************************************************************\n";
    }

    private String impl_generateHTMLFooter()
    {
        return "\n</table>\n</body>\n</html>\n";
    }

    private String impl_generateAsciiFooter()
    {
        return "\n\n";
    }

    // ____________________
    /**
     * helper to log different representations of a property(array)
     *
     * @param   sOut
     *              used to generate the log output there.
     *
     * @param   lProps
     *              represent the property(array) to be logged.
     */
    private void impl_logPropertyArray( /*OUT*/ StringBuffer                       sOut   ,
                                        /*IN */ com.sun.star.beans.PropertyValue[] lProps )
    {
        int i = 0;
        int c = lProps.length;

        for (i=0; i<c; ++i)
            impl_logProperty(sOut, lProps[i]);
    }

    private void impl_logPropertyArray( /*OUT*/ StringBuffer                    sOut   ,
                                        /*IN */ com.sun.star.beans.NamedValue[] lProps )
    {
        int i = 0;
        int c = lProps.length;

        for (i=0; i<c; ++i)
            impl_logProperty(sOut, lProps[i]);
    }

    private void impl_logProperty( /*OUT*/ StringBuffer                  sOut  ,
                                   /*IN*/  com.sun.star.beans.NamedValue aProp )
    {
        sOut.append("\""+aProp.Name+"\" = ");
        impl_logAny(sOut, aProp.Value);
    }

    private void impl_logProperty( /*OUT*/ StringBuffer                     sOut  ,
                                   /*IN*/  com.sun.star.beans.PropertyValue aProp )
    {
        sOut.append("\""+aProp.Name+"\" = ");
        impl_logAny(sOut, aProp.Value);
    }

    // ____________________
    /**
     * it trys to convert the given any into a suitable string notation .-)
    */
    private synchronized void impl_logAny( /*OUT*/ StringBuffer sOut ,
                                           /*IN */ Object       aAny )
    {
        try
        {
            if (com.sun.star.uno.AnyConverter.isVoid(aAny))
            {
                sOut.append("[void] {");
            }
            else
            if (com.sun.star.uno.AnyConverter.isChar(aAny))
            {
                sOut.append("[char] {");
                sOut.append(com.sun.star.uno.AnyConverter.toChar(aAny));
                sOut.append("}");
            }
            else
            if (com.sun.star.uno.AnyConverter.isBoolean(aAny))
            {
                sOut.append("[boolean] {");
                if (com.sun.star.uno.AnyConverter.toBoolean(aAny))
                    sOut.append("TRUE");
                else
                    sOut.append("FALSE");
                sOut.append("}");
            }
            else
            if (com.sun.star.uno.AnyConverter.isByte(aAny))
            {
                sOut.append("[byte] {");
                sOut.append(com.sun.star.uno.AnyConverter.toByte(aAny));
                sOut.append("}");
            }
            else
            if (com.sun.star.uno.AnyConverter.isShort(aAny))
            {
                sOut.append("[short] {");
                sOut.append(com.sun.star.uno.AnyConverter.toShort(aAny));
                sOut.append("}");
            }
            else
            if (com.sun.star.uno.AnyConverter.isInt(aAny))
            {
                sOut.append("[int] {");
                sOut.append(com.sun.star.uno.AnyConverter.toInt(aAny));
                sOut.append("}");
            }
            else
            if (com.sun.star.uno.AnyConverter.isLong(aAny))
            {
                sOut.append("[long] {");
                sOut.append(com.sun.star.uno.AnyConverter.toLong(aAny));
                sOut.append("}");
            }
            else
            if (com.sun.star.uno.AnyConverter.isFloat(aAny))
            {
                sOut.append("[float] {");
                sOut.append(com.sun.star.uno.AnyConverter.toFloat(aAny));
                sOut.append("}");
            }
            else
            if (com.sun.star.uno.AnyConverter.isDouble(aAny))
            {
                sOut.append("[double] {");
                sOut.append(com.sun.star.uno.AnyConverter.toDouble(aAny));
                sOut.append("}");
            }
            else
            if (com.sun.star.uno.AnyConverter.isString(aAny))
            {
                sOut.append("[string] {");
                sOut.append(com.sun.star.uno.AnyConverter.toString(aAny));
                sOut.append("}");
            }
            else
            if (com.sun.star.uno.AnyConverter.isEnum(aAny))
            {
                sOut.append("[enum] {");
                sOut.append("}");
            }
            else
            if (com.sun.star.uno.AnyConverter.isType(aAny))
            {
                sOut.append("[type] {");
                sOut.append("}");
            }
            else
            if (com.sun.star.uno.AnyConverter.isArray(aAny))
            {
                if (aAny instanceof java.lang.String[])
                {
                    sOut.append("[sequence< string >] {");
                    sOut.append("}");
                }
                else
                if (aAny instanceof com.sun.star.beans.PropertyValue[])
                {
                    sOut.append("[sequence< PropertyValue >] {");
                    com.sun.star.beans.PropertyValue[] lSubProps = (com.sun.star.beans.PropertyValue[])com.sun.star.uno.AnyConverter.toArray(aAny);
                    impl_logPropertyArray(sOut, lSubProps);
                    sOut.append("}");
                }
                else
                if (aAny instanceof com.sun.star.beans.NamedValue[])
                {
                    sOut.append("[sequence< NamedValue >] {");
                    com.sun.star.beans.NamedValue[] lSubProps = (com.sun.star.beans.NamedValue[])com.sun.star.uno.AnyConverter.toArray(aAny);
                    impl_logPropertyArray(sOut, lSubProps);
                    sOut.append("}");
                }
                else
                {
                    sOut.append("[unknown array] {-}");
                }
            }
            else
            if (com.sun.star.uno.AnyConverter.isObject(aAny))
            {
                sOut.append("[object] {");
                // TODO
                sOut.append("}");
            }

            if ((m_nMode & MODE_HTML) == MODE_HTML)
                sOut.append("<br>");
            else
                sOut.append("\n");
        }
        catch(com.sun.star.lang.IllegalArgumentException exIll)
        {
            sOut.append("Got exception during property conversion.\n");
            sOut.append(exIll.getMessage());
            sOut.append("\n");
        }
    }

    // ____________________
    /**
     * Writes the given content to the specified log file.
     */
    private void impl_writeToLogFile(String  sFileName,
                                     boolean bAppend  ,
                                     String  sContent )
    {
        try
        {
            FileWriter aLogFile = new FileWriter(sFileName, bAppend);
            aLogFile.write(sContent);
            aLogFile.flush();
            aLogFile.close();
            aLogFile = null;
        }
        catch (java.io.IOException exIO)
        {
            System.err.println("Can't dump protocol into log file.");
            System.err.println(exIO);
            exIO.printStackTrace();
        }
    }
}
