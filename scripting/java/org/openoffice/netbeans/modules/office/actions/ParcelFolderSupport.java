package org.openoffice.netbeans.modules.office.actions;

import java.util.Vector;
import java.util.StringTokenizer;

import java.io.*;
import java.beans.PropertyVetoException;
import java.awt.Dialog;

import org.w3c.dom.Document;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

import org.openide.TopManager;
import org.openide.DialogDescriptor;
import org.openide.ErrorManager;
import org.openide.xml.XMLUtil;
import org.openide.execution.NbClassPath;

import org.openide.cookies.OpenCookie;
import org.openide.loaders.DataObject;
import org.openide.loaders.DataNode;

import org.openide.filesystems.FileObject;
import org.openide.filesystems.FileSystem;
import org.openide.filesystems.JarFileSystem;
import org.openide.filesystems.FileUtil;
import org.openide.filesystems.Repository;

import org.openide.nodes.*;
import org.openide.windows.OutputWriter;

import org.openoffice.netbeans.modules.office.loader.ParcelFolder;
import org.openoffice.netbeans.modules.office.options.OfficeSettings;
import org.openoffice.netbeans.modules.office.utils.ManifestParser;

import org.openoffice.idesupport.zip.ParcelZipper;
import org.openoffice.idesupport.filter.FileFilter;
import org.openoffice.idesupport.ui.ConfigurePanel;

public class ParcelFolderSupport implements ParcelFolderCookie
{
    protected ParcelFolder pf;

    public ParcelFolderSupport(ParcelFolder pf) {
        this.pf = pf;
    }

    // ensure that ParcelZipper's XMLParser is set
    static {
        ParcelZipper.setXMLParser(ManifestParser.getManifestParser());
    }

    public void generate() {
        ParcelFolder.ParcelFolderNode node =
            (ParcelFolder.ParcelFolderNode)pf.getNodeDelegate();

        FileObject parcelBase = pf.getPrimaryFile();
        FileObject contentsBase =
            parcelBase.getFileObject(ParcelZipper.CONTENTS_DIRNAME);

        File parcelDir = FileUtil.toFile(parcelBase);
        File contentsDir = FileUtil.toFile(contentsBase);

        File targetfile = new File(node.getTargetDir() + File.separator +
            parcelBase.getName() + "." + ParcelZipper.PARCEL_EXTENSION);

        configure();

        final OutputWriter out =
            ParcelSupport.getOutputWindowWriter(parcelDir.getName() + " (generating)");
        try {
            out.println("Generating: " + parcelDir.getName(), null);
            ParcelZipper.getParcelZipper().zipParcel(contentsDir, targetfile, node.getFileFilter());
            out.println("\nGENERATION SUCCESSFUL.");
            out.println("\nRight click on the generated parcel to deploy it");

            if (node.getTargetDir().equals(parcelDir))
                parcelBase.refresh(true);
        }
        catch (IOException ioe) {
            out.println("GENERATION FAILED: reason: " + ioe.getClass().getName() + ": "+ ioe.getMessage());
        }
        finally
        {
            if( out != null)
            {
                out.close();
            }
        }
    }

    public void configure() {
        FileObject primary = pf.getPrimaryFile();
        File contents = FileUtil.toFile(primary.getFileObject(ParcelZipper.CONTENTS_DIRNAME));
        File parcelDescriptor = new File(contents, ParcelZipper.PARCEL_DESCRIPTOR_XML);
        InputSource is;
        Document previous = null;

        try {
            is = new InputSource(new FileInputStream(parcelDescriptor));
            previous = XMLUtil.parse(is, false, false, null, null);
        }
        catch (FileNotFoundException fnfe) {
            System.out.println("Couldn't find file: " + parcelDescriptor.getName());
        }
        catch (IOException ioe) {
            System.out.println("IO Error parsing file: " + parcelDescriptor.getName());
        }
        catch (SAXException se) {
            System.out.println("Sax Error parsing file: " + parcelDescriptor.getName());
        }

        Vector classpath = getClasspath();
        classpath.addElement(contents.getAbsolutePath());

        ConfigurePanel configuror = new ConfigurePanel(contents.getAbsolutePath(), classpath, previous);
        DialogDescriptor descriptor = new DialogDescriptor(configuror, "Choose Methods to Export as Scripts");

        Dialog dialog = TopManager.getDefault().createDialog(descriptor);
        dialog.show();

        if (descriptor.getValue() == DialogDescriptor.OK_OPTION) {
            try {
                Document doc = configuror.getConfiguration();
                FileOutputStream fos = new FileOutputStream(parcelDescriptor);
                XMLUtil.write(doc, fos, "");
            }
            catch (Exception e) {
                ErrorManager.getDefault().notify(e);
            }
        }
    }

    private Vector getClasspath() {
        Vector result = new Vector();

        String classpath = NbClassPath.createRepositoryPath().getClassPath();
        StringTokenizer tokens = new StringTokenizer(classpath, File.pathSeparator);

        while(tokens.hasMoreTokens())
            result.addElement(tokens.nextToken());

        OfficeSettings settings = OfficeSettings.getDefault();
        File classesDir = new File(settings.getOfficeDirectory() +
            File.separator + "program" + File.separator + "classes");
        File[] jarfiles = classesDir.listFiles();

        for (int i = 0; i < jarfiles.length; i++)
            result.addElement(jarfiles[i].getAbsolutePath());

        return result;
    }
}
