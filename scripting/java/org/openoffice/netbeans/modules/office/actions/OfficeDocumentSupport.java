/*************************************************************************
 *
 *  $RCSfile: OfficeDocumentSupport.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: toconnor $ $Date: 2003-02-20 12:00:21 $
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

package org.openoffice.netbeans.modules.office.actions;

import java.io.IOException;
import java.io.File;
import java.beans.PropertyVetoException;
import java.util.Enumeration;
import java.util.Set;
import java.util.HashSet;
import java.util.Iterator;
import javax.swing.event.ChangeListener;
import javax.swing.event.ChangeEvent;

import org.openide.ErrorManager;
import org.openide.filesystems.FileObject;
import org.openide.filesystems.FileUtil;
import org.openide.filesystems.FileChangeListener;
import org.openide.filesystems.FileEvent;
import org.openide.filesystems.FileAttributeEvent;
import org.openide.filesystems.FileRenameEvent;
import org.openide.cookies.OpenCookie;

import org.openoffice.idesupport.OfficeDocument;

import org.openoffice.netbeans.modules.office.options.OfficeSettings;
import org.openoffice.netbeans.modules.office.loader.OfficeDocumentDataObject;
import org.openoffice.netbeans.modules.office.utils.ZipMounter;
import org.openoffice.netbeans.modules.office.utils.ManifestParser;

public class OfficeDocumentSupport implements OfficeDocumentCookie, OpenCookie, FileChangeListener
{
    protected OfficeDocumentDataObject dataObj;
    private boolean isMounted = false;
    private OfficeDocument document;
    private Set listeners;

    public OfficeDocumentSupport(OfficeDocumentDataObject dataObj) {
        this.dataObj = dataObj;
        FileObject fo = dataObj.getPrimaryFile();
        try {
            this.document = new OfficeDocument(FileUtil.toFile(fo));
        }
        catch (Exception e) {
            e.printStackTrace();
        }
        fo.addFileChangeListener(this);
    }

    public void mount() {
        File file = FileUtil.toFile(dataObj.getPrimaryFile());

        try {
            ZipMounter.getZipMounter().mountZipFile(file);
            isMounted = true;
        }
        catch (IOException ioe) {
            ErrorManager.getDefault().notify(ioe);
        }
        catch (PropertyVetoException pve) {
            ErrorManager.getDefault().notify(pve);
        }
    }

    public void open () {
        File file = FileUtil.toFile(dataObj.getPrimaryFile());

        OfficeSettings settings = OfficeSettings.getDefault();
        File soffice = new File(settings.getOfficeDirectory().getPath(
            File.separator + "soffice"));

        try {
            Process p = Runtime.getRuntime ().exec (new String[] {
                soffice.getAbsolutePath(), file.getAbsolutePath ()
            });
        } catch (IOException ioe) {
            ErrorManager.getDefault().notify(ioe);
        }
    }

    public Enumeration getParcels() {
        return document.getParcels();
    }

    public void removeParcel(String name) {
        document.removeParcel(name);
        dataObj.getPrimaryFile().refresh(true);
    }

    public void addChangeListener(ChangeListener cl) {
        if (listeners == null)
            listeners = new HashSet();

        listeners.add(cl);
    }

    public void removeChangeListener(ChangeListener cl) {
        if (listeners == null)
            return;

        listeners.remove(cl);
    }

    public void fileChanged(FileEvent fe) {
        if (listeners != null) {
            Iterator iter = listeners.iterator();

            while (iter.hasNext())
                ((ChangeListener)iter.next()).stateChanged(new ChangeEvent(this));
        }
    }

    public void fileAttributeChanged(FileAttributeEvent fe) {}
    public void fileDataCreated(FileEvent fe) {}
    public void fileDeleted(FileEvent fe) {}
    public void fileFolderCreated(FileEvent fe) {}
    public void fileRenamed(FileRenameEvent fe) {}
}
