/*************************************************************************
 *
 *  $RCSfile: PropertyName.java,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change:$Date: 2008-01-14 13:22:08 $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 13:22:08 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
    final public static String OFFICE_WATCHER = "Watcher";
    final public static String LOG_WRITER = "LogWriter";
    final public static String TIME_OUT = "TimeOut";
    final public static String THREAD_TIME_OUT = "ThreadTimeOut";
    final public static String OFFICE_CLOSE_TIME_OUT = "OfficeCloseTimeOut";
    final public static String OPERATING_SYSTEM = "OperatingSystem";
    final public static String AUTO_RESTART = "AutoRestart";
    final public static String NEW_OFFICE_INSTANCE = "NewOfficeInstance";
    final public static String KEEP_DOCUMENT = "KeepDocument";
    /**
     * path to the configuration file to create an build environment
     */
    final public static String OOO_ENVSET = "OOO_EnvSet";
    /**
     * path to the source root of OpenOffice.org
     */
    final public static String SRC_ROOT = "SRC_ROOT";

    final public static String VERSION = "Version";
    /**
     * like unxsols4, unxsoli4, unxlngi5, wntmsci10
     */
    final public static String COMP_ENV = "COMP_ENV";

    /**
     * Path to a shel
     * This shell is used to run some commands outside of Java
     * example: /bin/tcsh c:\\myShell\\myShell.exe
     */
    final public static String SHELL = "Shell";

    final public static String WNTMSCI = "wntmsci";
    final public static String UNXLNGI = "unxlngi";
    final public static String UNXSOLS = "unxsols";
    final public static String UNXSOLI = "unxsoli";
}
