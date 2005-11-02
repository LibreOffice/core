/*************************************************************************
 *
 *  $RCSfile: PropertyName.java,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change:$Date: 2005-11-02 17:44:35 $
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
package util;

public interface PropertyName {
    final public static String APP_EXECUTION_COMMAND = "AppExecutionCommand";
    final public static String CONNECTION_STRING = "ConnectionString";
    final public static String TEST_BASE = "TestBase";
    final public static String TEST_DOCUMENT_PATH = "TestDocumentPath";
    final public static String LOGGING_IS_ACTIVE = "LoggingIsActive";
    final public static String DEBUG_IS_ACTIVE = "DebugIsActive";
    final public static String OUT_PRODUCER = "OutProducer";
    final public static String SHORT_WAIT= "ShortWait";
    final public static String OFFICE_PROVIDER = "OfficeProvider";
    final public static String LOG_WRITER = "LogWriter";
    final public static String TIME_OUT = "TimeOut";
    final public static String THREAD_TIME_OUT = "ThreadTimeOut";
    final public static String OFFICE_CLOSE_TIME_OUT = "OfficeCloseTimeOut";
    final public static String OPERATING_SYSTEM = "OperatingSystem";
    final public static String AUTO_RESTART = "AutoRestart";
    final public static String NEW_OFFICE_INSTANCE = "NewOfficeInstance";
    final public static String KEEP_DOCUMENT = "KeepDocument";


    final public static String WNTMSCI = "wntmsci";
    final public static String UNXLNGI = "unxlngi";
    final public static String UNXSOLS = "unxsols";
    final public static String UNXSOLI = "unxsoli";
}
