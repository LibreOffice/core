/*************************************************************************
 *
 *  $RCSfile: Logger.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 17:32:37 $
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
 **********************************************************************_*/

package com.sun.star.filter.config.tools.utils;

//_______________________________________________
// imports

import java.lang.*;
import java.util.*;

//_______________________________________________
// definition

/** can be used to print out some debug messages
 *  and group it into warnings/errors or info statements.
 *
 *
 */
public class Logger
{
    //___________________________________________
    // const

    /** only error message will be shown. */
    public static final int LEVEL_ERRORS = 1;

    /** only errors and warnings will be shown. */
    public static final int LEVEL_WARNINGS = 2;

    /** enable errors/warnings and some global info
     *  message. */
    public static final int LEVEL_GLOBALINFOS = 3;

    /** enable anything! */
    public static final int LEVEL_DETAILEDINFOS = 4;

    //___________________________________________
    // member

    /** enable/disable different output level.
     *  e.g. warnings/errors/infos */
    private int m_nLevel;

    //___________________________________________
    // ctor

    /** initialize new debug object with the specified
     *  debug level.
     *
     *  @param  nLevel
     *          the new debug level.
     *          See const values LEVEL_xxx too.
     */
    public Logger(int nLevel)
    {
        m_nLevel = nLevel;
    }

    //___________________________________________
    // interface

    /** initialize new debug object with a default
     *  debug level.
     */
    public Logger()
    {
        m_nLevel = LEVEL_DETAILEDINFOS;
    }

    //___________________________________________
    // interface

    /** prints out an exception ... if the right level is set.
     *
     *  @param  ex
     *          the exception object
     */
    public synchronized void setException(java.lang.Throwable ex)
    {
        if (m_nLevel >= LEVEL_ERRORS)
        {
            System.err.println("Exception:\n");
            ex.printStackTrace();
        }
    }

    //___________________________________________
    // interface

    /** prints out an error ... if the right level is set.
     *
     *  @param  sError
     *          the error message.
     */
    public synchronized void setError(java.lang.String sError)
    {
        if (m_nLevel >= LEVEL_ERRORS)
            System.err.println("Error    :\t\""+sError+"\"");
    }

    //___________________________________________
    // interface

    /** prints out a warning ... if the right level is set.
     *
     *  @param  sWarning
     *          the warning message.
     */
    public synchronized void setWarning(java.lang.String sWarning)
    {
        if (m_nLevel >= LEVEL_WARNINGS)
            System.err.println("Warning  :\t\""+sWarning+"\"");
    }

    //___________________________________________
    // interface

    /** prints out a global info message ... if the right level is set.
     *
     *  Global infos should be used to describe a complex operation.
     *  E.g.: loading of a document.
     *  But not for every sub operation like e.g. analyzing lines
     *  during loading the document!
     *
     *  @param  sInfo
     *          the info message.
     */
    public synchronized void setGlobalInfo(java.lang.String sInfo)
    {
        if (m_nLevel >= LEVEL_GLOBALINFOS)
            System.out.println("Info     :\t\""+sInfo+"\"");
    }

    //___________________________________________
    // interface

    /** prints out a mode detailed info message ... if the right level is set.
     *
     *  Such detailed message are e.g. "analyze line [n] of file ...".
     *
     *  @param  sInfo
     *          the info message.
     */
    public synchronized void setDetailedInfo(java.lang.String sInfo)
    {
        if (m_nLevel >= LEVEL_DETAILEDINFOS)
            System.out.println("Detail   :\t\""+sInfo+"\"");
    }
}
