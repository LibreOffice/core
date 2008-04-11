/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: StatusIndicator.java,v $
 * $Revision: 1.4 $
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
package complex.loadAllDocuments.helper;

// __________ Imports __________

// structs, const, ...
import com.sun.star.beans.PropertyValue;

// exceptions
import com.sun.star.uno.Exception;
import com.sun.star.uno.RuntimeException;

// interfaces
import com.sun.star.task.XStatusIndicator;

// helper
import com.sun.star.uno.UnoRuntime;

// others
//import java.lang.*;

// __________ Implementation __________

/**
 * Implemets a simple status indicator, which
 * provide informations about state of a load request.
 * It can be used as an argument e.g. for loadComponentFromURL().
 */
public class StatusIndicator implements com.sun.star.task.XStatusIndicator
{
    // ____________________

    /**
     * @const SHOWSTATUS_NO         don't show the status - but save information about using of this indicator object
     * @const SHOWSTATUS_LOG        the possible set protocol object will be used (it covers STDOUT, STDERR automaticly too)
     * @const SHOWSTATUS_DIALOG     the status will be shown inside a java dialog
     * @const SHOWSTATUS_LINK       the status will be notified to interested listener (one listener only!)
     */
    public static final int SHOWSTATUS_NO       =   0;
    public static final int SHOWSTATUS_LOG      =   1;
    public static final int SHOWSTATUS_DIALOG   =   4;
    public static final int SHOWSTATUS_LINK     =   8;

    // ____________________

    /**
     * @member m_sText      text, which describe the current status
     * @member m_nRange     max value for any progress
     * @member m_nValue     the progress value
     * @member m_nOut       regulate, how the status will be shown
     * @member m_aProtocol  used for logging and transport information about used interface of this object
     */
    private String          m_sText     ;
    private int             m_nRange    ;
    private int             m_nValue    ;
    private int             m_nOut      ;
//    private Protocol        m_aProtocol ;
    private boolean         m_bWasUsed  ;

    // ____________________

    /**
     * ctor
     * It's initialize an object of this class with default values.
     */
    public StatusIndicator( int nOut)
    {
        m_sText     = new String()  ;
        m_nRange    = 100           ;
        m_nValue    = 0             ;
        m_nOut      = nOut          ;
        //m_aProtocol = aProtocol     ;
        m_bWasUsed  = false;
//        aProtocol.resetUsingState();
    }

    // ____________________

    /**
     * It starts the progress and set the initial text and range.
     *
     * @param sText
     *          the initial text for showing
     *
     * @param nRange
     *          the new range for following progress
     */
    public void start( /*IN*/String sText, /*IN*/int nRange )
    {
        synchronized(this)
        {
            //m_aProtocol.log("start("+sText+","+nRange+")\n");
            m_bWasUsed = true;
//            m_aProtocol.itWasUsed();

            m_sText  = sText ;
            m_nRange = nRange;
            m_nValue = 0     ;
        }
        impl_show();
    }

    // ____________________

    /**
     * Finish the progress and reset internal members.
     */
    public void end()
    {
        synchronized(this)
        {
            //m_aProtocol.log("end()\n");
            m_bWasUsed = true;
//            m_aProtocol.itWasUsed();

            m_sText  = new String();
            m_nRange = 100;
            m_nValue = 0;
        }
        impl_show();
    }

    // ____________________

    /**
     * Set the new description text.
     *
     * @param sText
     *          the new text for showing
     */
    public void setText( /*IN*/String sText )
    {
        synchronized(this)
        {
            //m_aProtocol.log("setText("+sText+")\n");
            m_bWasUsed = true;
//            m_aProtocol.itWasUsed();

            m_sText = sText;
        }
        impl_show();
    }

    // ____________________

    /**
     * Set the new progress value.
     *
     * @param nValue
     *          the new progress value
     *          Must fit the range!
     */
    public void setValue( /*IN*/int nValue )
    {
        synchronized(this)
        {
            //m_aProtocol.log("setValue("+nValue+")\n");
            m_bWasUsed = true;
//            m_aProtocol.itWasUsed();

            if (nValue<=m_nRange)
                m_nValue = nValue;
        }
        impl_show();
    }

    // ____________________

    /**
     * Reset text and progress value to her defaults.
     */
    public void reset()
    {
        synchronized(this)
        {
            //m_aProtocol.log("reset()\n");
            m_bWasUsed = true;
//            m_aProtocol.itWasUsed();

            m_sText  = new String();
            m_nValue = 0;
        }
        impl_show();
    }

    // ____________________

    /**
     * Internal helper to show the status.
     * Currently it's implement as normal text out on stdout.
     * But of course other thimngs are possible here too.
     * e.g. a dialog
     */
    private void impl_show()
    {
/*        synchronized(this)
        {
            if ((m_nOut & SHOWSTATUS_LOG) == SHOWSTATUS_LOG)
                //m_aProtocol.log("\t["+m_nValue+"/"+m_nRange+"] "+m_sText+"\n");

            //if ((m_nOut & SHOWSTATUS_DIALOG) == SHOWSTATUS_DIALOG)
                // not supported yet!

            //if ((m_nOut & SHOWSTATUS_LINK) == SHOWSTATUS_LINK)
                // not supported yet!
        } */
    }

    public boolean wasUsed() {
        return m_bWasUsed;
    }
}
