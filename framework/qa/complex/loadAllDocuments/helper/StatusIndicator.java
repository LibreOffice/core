/*************************************************************************
 *
 *  $RCSfile: StatusIndicator.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Date: 2004-04-21 12:04:01 $
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
