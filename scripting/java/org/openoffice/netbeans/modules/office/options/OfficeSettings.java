/*************************************************************************
 *
 *  $RCSfile: OfficeSettings.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: toconnor $ $Date: 2002-11-26 12:46:47 $
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

package org.openoffice.netbeans.modules.office.options;

import java.util.Hashtable;
import java.util.Enumeration;
import java.io.File;
import java.io.IOException;

import org.openide.options.SystemOption;
import org.openide.util.HelpCtx;
import org.openide.util.NbBundle;

import org.openoffice.idesupport.SVersionRCFile;

/** Options for something or other.
 *
 * @author tomaso
 */
public class OfficeSettings extends SystemOption {

    // private static final long serialVersionUID = ...;

    public static final String OFFICE_DIRECTORY = "OfficeDirectory";
    public static final String WARN_BEFORE_DOC_DEPLOY = "WarnBeforeDocDeploy";
    public static final String WARN_AFTER_DIR_DEPLOY = "WarnAfterDirDeploy";
    public static final String WARN_BEFORE_MOUNT = "WarnBeforeMount";

    protected void initialize() {
        super.initialize();
        setWarnBeforeDocDeploy(true);
        setWarnAfterDirDeploy(true);
        setWarnBeforeMount(true);

        SVersionRCFile sversion = new SVersionRCFile();

        try {
            Hashtable versions = sversion.getVersions();
            Enumeration enum = versions.elements();
            String path;

            while (enum.hasMoreElements()) {
                path = (String)enum.nextElement();
                File f = new File(path);

                if (f.exists()) {
                    setOfficeDirectory(path);
                    return;
                }
            }
        }
        catch (IOException ioe) {
            /* do nothing a default will be used */
        }

        // if no office version is found try a default value
        setOfficeDirectory(System.getProperty("user.home") +
            System.getProperty("file.separator") + "OpenOffice643C");
    }

    public String displayName() {
        return "Office Settings";
    }

    public HelpCtx getHelpCtx() {
        return HelpCtx.DEFAULT_HELP;
    }

    public static OfficeSettings getDefault() {
        return (OfficeSettings)findObject(OfficeSettings.class, true);
    }

    public String getOfficeDirectory() {
        return (String)getProperty(OFFICE_DIRECTORY);
    }

    public void setOfficeDirectory(String path) {
        putProperty(OFFICE_DIRECTORY, path, true);
    }

    public boolean getWarnBeforeDocDeploy() {
        return ((Boolean)getProperty(WARN_BEFORE_DOC_DEPLOY)).booleanValue();
    }

    public void setWarnBeforeDocDeploy(boolean value) {
        putProperty(WARN_BEFORE_DOC_DEPLOY, new Boolean(value), true);
    }

    public boolean getWarnAfterDirDeploy() {
        return ((Boolean)getProperty(WARN_AFTER_DIR_DEPLOY)).booleanValue();
    }

    public void setWarnAfterDirDeploy(boolean value) {
        putProperty(WARN_AFTER_DIR_DEPLOY, new Boolean(value), true);
    }

    public boolean getWarnBeforeMount() {
        return ((Boolean)getProperty(WARN_BEFORE_MOUNT)).booleanValue();
    }

    public void setWarnBeforeMount(boolean value) {
        putProperty(WARN_BEFORE_MOUNT, new Boolean(value), true);
    }
}
