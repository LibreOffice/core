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
import org.openoffice.idesupport.zip.ParcelZipper;

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

    // ensure that ParcelZipper's XMLParser is set
    static {
        ParcelZipper.setXMLParser(ManifestParser.getManifestParser());
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
        File soffice = new File(settings.getOfficeDirectory() +
            File.separator + "soffice");

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
