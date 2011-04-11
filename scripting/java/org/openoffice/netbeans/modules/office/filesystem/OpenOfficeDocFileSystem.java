/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

package org.openoffice.netbeans.modules.office.filesystem;

import java.beans.*;
import java.io.*;
import java.util.*;
import java.util.zip.*;

import org.openide.ErrorManager;
import org.openide.filesystems.*;
import org.openide.filesystems.FileSystem; // override java.io.FileSystem
import org.openide.util.NbBundle;

// ISSUES:
// - This FS saves (updates) the file on 'setDocument' or 'removeNotify'.
//   It has to let the user to decide to update or not.
//
// TODOS:
// - 'Update' action on the mounted document which saves all recent modifications.
// - To introduce 'scope' editable property to control editable portion of
//   the mounted document.
// - Acceptable document type identification before mount.

/**
 * OpenOffice.org Document filesystem.
 */
public class OpenOfficeDocFileSystem
    extends AbstractFileSystem
{
    public static final  String SCRIPTS_ROOT  = "Scripts";   // must be a folder
    public static final  String SEPARATOR     = "/";         // zip file separator

    private static final int    OS_UNKNOWN   = 0;
    private static final int    OS_UNIX      = 1;
    private static final int    OS_MACOS     = 2;
    private static final int    OS_WINDOWS   = 3;

    private static final int    REFRESH_OFF   = -1;          // -1 is desabled
    private static final int    REFRESH_TIME  = REFRESH_OFF; // (mS)
    private static final String TMP_FILE_PREF = "sx_";
    private static final String TMP_FILE_SUFX = ".sxx";

    private transient Map       cache;      // filesystem cache
    private transient File      docFile;    // OpenOffice document
    private transient ZipFile   zipFile;

    private static transient int osType;    // type of OS

    private transient ChildrenStrategy childrenStrategy;
    private transient EditableStrategy editableStrategy;

    private transient boolean isModified;   // true if an entry has been removed

    /**
     * Static constructor.
     */
    static {
        // Identify the type of OS
        String osname = System.getProperty("os.name");
        if (osname.startsWith("Mac OS"))
            osType = OS_MACOS;
        else if (osname.startsWith("Windows"))
            osType = OS_WINDOWS;
        else
            osType = OS_UNIX;
    }

    /**
     * Default constructor. Initializes new OpenOffice filesystem.
     */
    public OpenOfficeDocFileSystem()
    {
        // Create the filesystem cache
        cache            = new HashMap();

        // Initialize strategies
        editableStrategy = new EditableStrategy(SCRIPTS_ROOT);
        childrenStrategy = new ChildrenStrategy();

        // Create and use implementations of filesystem functionality:
        info    = new InfoImpl();
        change  = new ChangeImpl();

        // Handle filesystem.attributes files normally:
        DefaultAttributes defattr = new DefaultAttributes(
            info, change, new ListImpl());

        // Handle filesystem.attributes files normally + adds virtual attribute
        // "java.io.File" that is used in conversion routines FileUtil.toFile and
        // FileUtil.fromFile
        //defattr = new InnerAttrs(this, info, change, new ListImpl());
        // (Otherwise set attr to a special implementation, and use ListImpl for list.)
        attr = defattr;
        list = defattr;

        // transfer = new TransferImpl();
        setRefreshTime(REFRESH_OFF);
    }

    /**
     * Constructor. Initializes new OpenOffice filesystem with FS capability.
     */
    public OpenOfficeDocFileSystem(FileSystemCapability cap)
    {
        this();
        setCapability(cap);
    }

    /**
     * Provides unique signature of an instance of the filesystem.
     * NOTE: The scope is not a part of the signature so it is impossible
     *       to mount the same archive more then once.
     */
    public static String computeSystemName(File file)
    {
        return OpenOfficeDocFileSystem.class.getName() + "[" + file + "]";
    }

    // ----------- PROPERTIES --------------

    /**
     * Provides the 'human readable' name of the instance of the filesystem.
     */
    public String getDisplayName()
    {
        if (!isValid())
            return NbBundle.getMessage(OpenOfficeDocFileSystem.class,
                "LAB_invalid_file_system", ((docFile != null)? docFile.toString(): ""));
        else
            return NbBundle.getMessage(OpenOfficeDocFileSystem.class,
                "LAB_valid_file_system", docFile.toString());
    }

    /**
     * Retrives the 'document' property.
     */
    public File getDocument()
    {
        return docFile;
    }

    /**
     * Sets the 'document' property.
     */
    // Bean setter. Changing the OpenOffice document (or in general, the identity
    // of the root file object) should cause everything using this filesystem
    // to refresh. The system name must change and refreshRoot should be used
    // to ensure that everything is correctly updated.
    public synchronized void setDocument(File file)
        throws java.beans.PropertyVetoException, java.io.IOException
    {
System.out.println("OpenOfficeDocFileSystem.setDocument: file=\"" + file.toString() + "\"");
        if((file.exists() == false) || (file.isFile() == false)) {
            IOException ioe = new IOException(
                file.toString() + " does not exist");
            ErrorManager.getDefault().annotate(ioe, NbBundle.getMessage(
                OpenOfficeDocFileSystem.class, "EXC_root_dir_does_not_exist",
                file.toString()));
            throw ioe;
        }
        // update the document
        try {
          updateDocument();
        } catch(IOException ioe) {
            // cannot save all!!!
System.out.println("*** OpenOfficeDocFileSystem.setDocument:");
System.out.println("    file: " + ((docFile != null)? docFile.toString(): ""));
System.out.println("    exception: " + ioe.getMessage());
        }
        // new document type verification!!!
        closeDocument();
        // open a new document
        try {
            openDocument(file);
            firePropertyChange(PROP_ROOT, null, refreshRoot());
            setRefreshTime(REFRESH_TIME);
        } catch(IOException ioe) {
            // cannot open a new document!!!
System.out.println("*** OpenOfficeDocFileSystem.setDocument:");
System.out.println("    file: " + ((file != null)? file.toString(): ""));
System.out.println("    exception: " + ioe.getMessage());
        }
    }

    /**
     * Retrives 'readonly' property.
     * NOTE: The portion of the mounted document available to the user is
     *       always editable.
     */
    public boolean isReadOnly()
    {
        return false;
    }

    /**
     * Sets 'readonly' property.
     * NOTE: The portion of the mounted document available to the user is
     *       always editable.
     */
    public void setReadOnly(boolean flag)
    {
        // sorry! it is not supported.
    }

    // ----------- SPECIAL CAPABILITIES --------------

    /**
     * Participates in the environment configuration.
     * This is how you can affect the classpath for execution, compilation, etc.
     */
    public void prepareEnvironment(FileSystem.Environment environment)
    {
        // BUG: the compiller cannot access files withing the OpenOffice document.
        //environment.addClassPath(docFile.toString());
    }

    /* -----------------------------------------------------------
     * Affect the name and icon of files on this filesystem according to their
     * "status", e.g. version-control modification-commit state:
     /*
    private class StatusImpl implements Status {
        public Image annotateIcon(Image icon, int iconType, Set files) {
            // You may first modify it, e.g. by adding a check mark to the icon
            // if that makes sense for this file or group of files.
            return icon;
        }
        public String annotateName(String name, Set files) {
            // E.g. add some sort of suffix to the name if some of the
            // files are modified but not backed up or committed somehow:
            if (theseFilesAreModified(files))
                return NbBundle.getMessage(OpenOfficeDocFileSystem.class, "LBL_modified_files", name);
            else
                return name;
        }
    }

    private transient Status status;

    public Status getStatus() {
        if (status == null) {
           status = new StatusImpl();
        }
        return status;
    }
    // And use fireFileStatusChanged whenever you know something has changed.
     */

    /*
    // Filesystem-specific actions, such as version-control operations.
    // The actions should typically be CookieActions looking for DataObject
    // cookies, where the object's primary file is on this type of filesystem.
    public SystemAction[] getActions() {
// ------>>>>  UPDATE OPENOFFICE DOCUMENT  <<<<------
    return new SystemAction[] {
        SystemAction.get(SomeAction.class),
        null, // separator
        SystemAction.get(SomeOtherAction.class)
    };
    }
     */

    /**
     * Notifies this filesystem that it has been removed from the repository.
     * Concrete filesystem implementations could perform clean-up here.
     * The default implementation does nothing.
     * <p>Note that this method is <em>advisory</em> and serves as an optimization
     * to avoid retaining resources for too long etc. Filesystems should maintain correct
     * semantics regardless of whether and when this method is called.
     */
    public void removeNotify()
    {
        setRefreshTime(REFRESH_OFF);    // disable refresh
        // update the document
        try {
          updateDocument();
        } catch(IOException ioe) {
            // cannot save all!!!
System.out.println("*** OpenOfficeDocFileSystem.removeNotify:");
System.out.println("    exception: " + ioe.getMessage());
        }
        closeDocument();
        super.removeNotify();
    }

    /*
     * Opens (mounts) an OpenOffice document.
     */
    private void openDocument(File file)
        throws IOException, PropertyVetoException
    {
        synchronized(cache) {
            setSystemName(computeSystemName(file));
            docFile   = file;
            zipFile   = new ZipFile(docFile);
            cacheDocument(zipFile.entries(), editableStrategy);
            isModified = false;
        } // synchronized
    }

    /*
     * Closes the document and cleans up the cache.
     */
    private void closeDocument()
    {
        synchronized(cache) {
            // if a document mounted - close it
            if(docFile != null) {
                // close the document archive
                if(zipFile != null) {
                    try {
                        zipFile.close();
                    } catch(IOException ioe) {
                        // sorry! we can do nothing about it.
                    }
                }
                zipFile = null;
                // clean up cache
                scanDocument(new CleanStrategy());
                docFile = null;
                isModified = false;
            }
        } // synchronized
    }

    /*
     * Creates a document cache.
     */
    private void cacheDocument(Enumeration entries, Strategy editables)
    {
        Entry    cacheEntry;
        ZipEntry archEntry;
        synchronized(cache) {
            cache.clear();
            // root folder
            cacheEntry = new ReadWriteEntry(null);
            cache.put(cacheEntry.getName(), cacheEntry);
            // the rest of items
            while(entries.hasMoreElements()) {
                archEntry   = (ZipEntry)entries.nextElement();
                cacheEntry  = new Entry(archEntry);
                if(editables.evaluate(cacheEntry))
                    cacheEntry  = new ReadWriteEntry(archEntry);
                cache.put(cacheEntry.getName(), cacheEntry);
            }
        } // synchronized
    }

    /*
     * Updates the document.
     */
    private void updateDocument()
        throws IOException
    {
        if(docFile == null)
            return;
        synchronized(cache) {
            ModifiedStrategy modifiedStrategy = new ModifiedStrategy();
            scanDocument(modifiedStrategy);
            if((isModified == true) ||
                (modifiedStrategy.isModified() == true))
            {
                File tmpFile = null;
                try {
                    // create updated document
                    tmpFile = File.createTempFile(
                        TMP_FILE_PREF, TMP_FILE_SUFX, docFile.getParentFile());
                    saveDocument(tmpFile);
                } catch(IOException ioe) {
                    if(tmpFile != null)
                        tmpFile.delete();
                    throw ioe;
                }
                // close the document archive
                if(zipFile != null) {
                    try {
                        zipFile.close();
                    } catch(IOException ioe) {
                    }
                }
                zipFile = null;
                // create the document and backup
                File newFile = new File(docFile.getParentFile() + File.separator +
                    "~" + docFile.getName());
                if(newFile.exists())
                    newFile.delete();   // delete old backup
                docFile.renameTo(newFile);
                tmpFile.renameTo(docFile);
                // open the document archive
                zipFile   = new ZipFile(docFile);
            }
            isModified = false;
        } // synchronized
    }

    /*
     * Saves the document in a new archive.
     */
    private void saveDocument(File file)
        throws IOException
    {
        synchronized(cache) {
            SaveStrategy saver = new SaveStrategy(file);
            scanDocument(saver);
            saver.close();
        } // synchronized
    }

    /*
     * Provides each individual entry in the cached document to an apraiser.
     */
    private void scanDocument(Strategy strategy)
    {
        synchronized(cache) {
            Iterator  itr = cache.values().iterator();
            while(itr.hasNext()) {
                strategy.evaluate((Entry)itr.next());
            }
        } // synchronized
    }

    /*
     * Retrives or creates a file.
     */
    private Entry getFileEntry(String name)
        throws IOException
    {
        Entry cEntry = null;
        synchronized(cache) {
            cEntry = (Entry)cache.get(name);
            if(cEntry == null) {
                // create a new file
                ZipEntry    zEntry = new ZipEntry(name);
                zEntry.setTime(new Date().getTime());
                cEntry = new Entry(zEntry);
                if(editableStrategy.evaluate(cEntry) == false) {
                    throw new IOException(
                        "cannot create/edit readonly file");    // I18N
                }
                cEntry  = new ReadWriteEntry(zEntry);
                cache.put(cEntry.getName(), cEntry);
                isModified = true;
            }
        } // synchronized
        return cEntry;
    }

    /*
     * Retrives or creates a folder.
     */
    private Entry getFolderEntry(String name)
        throws IOException
    {
        Entry cEntry = null;
        synchronized(cache) {
            cEntry = (Entry)cache.get(name);
            if(cEntry == null) {
                // create a new folder
                ZipEntry    zEntry = new ZipEntry(name + SEPARATOR);
                zEntry.setMethod(ZipEntry.STORED);
                zEntry.setSize(0);
                CRC32 crc = new CRC32();
                zEntry.setCrc(crc.getValue());
                zEntry.setTime(new Date().getTime());
                cEntry  = new Entry(zEntry);
                if(editableStrategy.evaluate(cEntry) == false) {
                    throw new IOException(
                        "cannot create folder");    // I18N
                }
                cEntry  = new ReadWriteEntry(zEntry);
                cEntry.getOutputStream();           // sets up modified flag
                cache.put(cEntry.getName(), cEntry);
                isModified = true;
            } else {
                if(cEntry.isFolder() == false)
                    cEntry = null;
            }
        } // synchronized
        return cEntry;
    }

    /*
     * Converts the name to ZIP file name.
     * Removes the leading file separator if there is one.
     * This is WORKAROUND of the BUG in AbstractFileObject:
     * While AbstractFileObject reprecents the root of the filesystem it uses
     * the absolute path (the path starts with '/'). It is inconsistent with
     * the rest of the code.
     * WORKAROUND: we have to strip leading '/' if it is in the name.
     */
    private static String zipName(String name)
    {
        String zname = ((name.startsWith(File.separator))?
            name.substring(File.separator.length()): name);
        switch(osType) {
            case OS_MACOS:
                zname = zname.replace(':', '/');        // ':' by '/'
                break;
            case OS_WINDOWS:
                zname = zname.replace((char)0x5c, '/'); // '\' by '/'
                break;
            default:
                break;
        }
        return zname;
    }

    // ----------- IMPLEMENTATIONS OF ABSTRACT FUNCTIONALITY ----------

    /* -----------------------------------------------------------
     * Information about files and operations on the contents which do
     * not affect the file's presence or name.
     */
    private class InfoImpl
        implements Info
    {
        public boolean folder(String name) {
            synchronized(cache) {
                String zname = zipName(name);
                Entry entry = (Entry)cache.get(zname);
                if(entry != null)
                    return entry.isFolder();
                // logical zip file entry
                childrenStrategy.setParent(zname);
                scanDocument(childrenStrategy);
                return (childrenStrategy.countChildren() > 0);
            }
        }

        public Date lastModified(String name) {
            synchronized(cache) {
                Entry entry = (Entry)cache.get(zipName(name));
                return new Date((entry != null)? entry.getTime(): 0L);
            }
        }

        public boolean readOnly(String name) {
            synchronized(cache) {
                Entry entry = (Entry)cache.get(zipName(name));
                return (entry != null)? entry.isReadOnly(): false;
            }
        }

        public String mimeType(String name) {
            // Unless you have some special means of determining MIME type
            // (e.g. HTTP headers), ask IDE to use its normal heuristics:
            // the MIME resolver pool and then file extensions, or if nothing
            // matches, just content/unknown.
            return null;
        }

        public long size(String name) {
            synchronized(cache) {
                Entry entry = (Entry)cache.get(zipName(name));
                return (entry != null)? entry.getSize(): 0;
            } // synchronized
        }

        public InputStream inputStream(String name)
            throws FileNotFoundException
        {
            synchronized(cache) {
                Entry entry = (Entry)cache.get(zipName(name));
                return (entry != null)? entry.getInputStream(): null;
            } // synchronized
        }

        public OutputStream outputStream(String name)
            throws IOException
        {
            return getFileEntry(zipName(name)).getOutputStream();
        }

        // AbstractFileSystem handles locking the file to the rest of the IDE.
        // This only means that you should define how the file should be locked
        // to the outside world--perhaps it does not need to be.
        public void lock(String name)
            throws IOException
        {
/*
            File file = getFile(name);
            if (file.exists() == true && file.canWrite() == false) {
                IOException ioe = new IOException("file " + file +
                    " could not be locked");
                ErrorManager.getDefault().annotate(ioe, NbBundle.getMessage(
                    OpenOfficeDocFileSystem.class, "EXC_file_could_not_be_locked",
                    file.getName(), getDisplayName(), file.getPath()));
                throw ioe;
            }
*/
        }

        public void unlock(String name) {
            // Nothing special needed to unlock a file to the outside world.
        }

        public void markUnimportant(String name) {
            // Do nothing special. Version-control systems may use this to mark
            // certain files (e.g. *.class) as not needing to be stored in the VCS
            // while others (source files) are by default important.
        }

    }

    /* -----------------------------------------------------------
     * Operations that change the available files.
     */
    private class ChangeImpl
        implements Change
    {
        public void createFolder(String name)
            throws IOException
        {
            synchronized(cache) {
                String zname = zipName(name);
                if(cache.get(zname) != null) {
                    throw new IOException(
                        "cannot create new folder: " + name);           // I18N
                }
                getFolderEntry(zname);
            } // synchronized
        }

        public void createData(String name)
            throws IOException
        {
            synchronized(cache) {
                String zname = zipName(name);
                if(cache.get(zname) != null) {
                    throw new IOException(
                        "cannot create new data: " + name);             // I18N
                }
                OutputStream os = getFileEntry(zname).getOutputStream();
                os.close();
            } // synchronized
        }

        public void rename(String oldName, String newName)
            throws IOException
        {
            String oname = zipName(oldName);
            String nname = zipName(newName);
            if((oname.length() == 0) || (nname.length() == 0)) {
                throw new IOException(
                    "cannot move or rename the root folder");           // I18N
            }
            synchronized(cache) {
                if(cache.get(nname) != null) {
                    throw new IOException(
                        "target file/folder " + newName + " exists");   // I18N
                }
                Entry entry = (Entry)cache.get(oname);
                if(entry == null) {
                    throw new IOException(
                        "there is no such a file/folder " + oldName);   // I18N
                }
                if(entry.isReadOnly() == true) {
                    throw new IOException(
                        "file/folder " + oldName + " is readonly");     // I18N
                }
                entry.rename(nname);
                if(editableStrategy.evaluate(entry) == false) {
                    entry.rename(oname);
                    throw new IOException(
                        "cannot create file/folder");                   // I18N
                }
                cache.remove(oname);
                cache.put(entry.getName(), entry);
            } // synchronized
        }

        public void delete(String name)
            throws IOException
        {
            String zname = zipName(name);
            if(zname.length() == 0) {
                throw new IOException(
                    "cannot delete the root folder");                   // I18N
            }
            synchronized(cache) {
                Entry entry = (Entry)cache.remove(zname);
                if(entry != null) {
                    // BUG: this is the design bug. Cache has to
                    //      remember that the entry was removed.
                    isModified = true;
                    entry.clean();
                }
            } // synchronized
        }
    }

    /* -----------------------------------------------------------
     * Operation which provides the directory structure.
     */
    private class ListImpl
        implements List
    {
        public String[] children(String name)
        {
            String[] children = null;
            synchronized(cache) {
                String zname = zipName(name);
                Entry entry = (Entry)cache.get(zname);
                if(entry != null) {
                    // real zip file entry
                    if(entry.isFolder()) {
                        childrenStrategy.setParent(entry.getName());
                    }
                } else {
                    // logical zip file entry
                    // (portion of the path of a real zip file entry)
                    childrenStrategy.setParent(zname);
                }
                scanDocument(childrenStrategy);
                children = childrenStrategy.getChildren();
            } // synchronize
            return children;
        }

    }

    /** -----------------------------------------------------------
     * This class adds new virtual attribute "java.io.File".
     * Because of the fact that FileObjects of __Sample__FileSystem are convertible
     * to java.io.File by means of attributes. */
    /*private static class InnerAttrs extends DefaultAttributes {
        //static final long serialVersionUID = 1257351369229921993L;
        __Sample__FileSystem sfs;
        public InnerAttrs(__Sample__FileSystem sfs, AbstractFileSystem.Info info,
        AbstractFileSystem.Change change,AbstractFileSystem.List list ) {
            super(info, change, list);
            this.sfs = sfs;
        }
        public Object readAttribute(String name, String attrName) {
            if (attrName.equals("java.io.File"))  // NOI18N
                return sfs.getFile(name);

            return super.readAttribute(name, attrName);
        }
    }*/

    /* -----------------------------------------------------------
    // Optional special implementations of copy and (cross-directory) move.
    private class TransferImpl implements Transfer {

    public boolean copy(String name, Transfer target, String targetName) throws IOException {
        // Only permit special implementation within single FS
        // (or you could implement it across filesystems if you wished):
        if (target != this) return false;
        // Specially copy the file in an efficient way, e.g. implement
        // a copy-on-write algorithm.
        return true;
    }

    public boolean move(String name, Transfer target, String targetName) throws IOException {
        // Only permit special implementation within single FS
        // (or you could implement it across filesystems if you wished):
        if (target != this) return false;
        // Specially move the file, e.g. retain rename information even
        // across directories in a version-control system.
        return true;
    }

    }
     */

    /* -----------------------------------------------------------
     * This interface hides an action will be performed on an entry.
     */
    private interface Strategy
    {
        public boolean evaluate(Entry entry);
    }

    /* -----------------------------------------------------------
     * Recognizes editable (read-write) entires
     */
    private class EditableStrategy
        implements Strategy
    {
        private String scope;

        public EditableStrategy(String scope)
        {
            this.scope = scope;
        }

        public boolean evaluate(Entry entry)
        {
            // recognizes all entries in a subtree of the
            // 'scope' as editable entries
            return (entry != null)?
                entry.getName().startsWith(scope): false;
        }
    }

    /* -----------------------------------------------------------
     * Recognizes and accumulates immediate children of the parent.
     */
    private class ChildrenStrategy
        implements Strategy
    {
        private String     parent;
        private Collection children = new HashSet();

        public ChildrenStrategy()
        {
        }

        public void setParent(String name)
        {
            parent = (name.length() > 0)? (name + SEPARATOR): "";
            if(children == null)
                children = (java.util.List)new LinkedList();
            children.clear();
        }

        public boolean evaluate(Entry entry)
        {
            // do not accept "children" of a file
            // ignore "read only" part of the filesystem
            if(entry.isReadOnly() == false) {
                // identify a child
                if( (entry.getName().length() > 0) &&
                    (entry.getName().startsWith(parent)))
                {
                    // identify an immediate child
                    String child = entry.getName();
                    if(parent.length() > 0) {
                        child = entry.getName().substring(parent.length());
                    }
                    int    idx   = child.indexOf(SEPARATOR);
                    if(idx > 0)     // more path elements ahead
                        child = child.substring(0, idx);
                    return children.add(child);
                }
            }
            return false;
        }

        public int countChildren()
        {
            return children.size();
        }

        public String[] getChildren()
        {
            String[]  chn = new String[children.size()];
            Iterator  itr = children.iterator();
            int       idx = 0;
            while(itr.hasNext()) {
                chn[idx++] = (String)itr.next();
            }
            return chn;
        }
    }

    /* -----------------------------------------------------------
     * Recognizes cache entries which have to be save into new archive.
     */
    private class ModifiedStrategy
        implements Strategy
    {
        private boolean modified;

        public boolean evaluate(Entry entry)
        {
            modified |= entry.isModified();
            return entry.isModified();
        }

        public boolean isModified()
        {
            return modified;
        }
    }

    /* -----------------------------------------------------------
     * Saves each entry in the filesystem cache.
     */
    private class SaveStrategy
        implements Strategy
    {
        ZipOutputStream docos;
        IOException     ioexp;

        public SaveStrategy(File newdoc)
            throws IOException
        {
            docos = new ZipOutputStream(new FileOutputStream(newdoc));
            ioexp = null; // success by default
        }

        public boolean evaluate(Entry entry)
        {
            if(entry.getName().length() == 0)
                return false;
            try {
                entry.save(docos);
            } catch(IOException ioe) {
                if(ioexp == null)
                    ioexp = ioe;
            }
            return true;
        }

        public void close()
            throws IOException
        {
            if(docos != null) {
                try {
                    docos.close();
                } catch (IOException ioe) {
                    ioexp = ioe;
                } finally {
                    docos = null;
                }
                if(ioexp != null) {
                    throw ioexp;
                }
            }
        }
    }

    /* -----------------------------------------------------------
     * Cleans each entiry in the filesystem cache.
     */
    private class CleanStrategy
        implements Strategy
    {
        public boolean evaluate(Entry entry)
        {
            try {
                entry.clean();
            } catch(java.lang.Exception exp) {
                // sorry! can do nothing about it.
            }
            return true;
        }
    }

    /* -----------------------------------------------------------
     * ReadOnly cache entry
     */
    private class Entry
    {
        private String  name;
        private boolean folder;
        private long    size;
        private long    time;
        private File    node;       // data files only

        public Entry(ZipEntry entry)
        {
            if(entry != null) {
                name   = entry.getName();
                folder = entry.isDirectory();
                size   = entry.getSize();
                time   = entry.getTime();
                // removes tail file separator from a folder name
                if((folder == true) && (name.endsWith(SEPARATOR))) {
                    name = name.substring(
                        0, name.length() - SEPARATOR.length());
                }
            } else {
                // 'null' is special cace of root folder
                name   = "";
                folder = true;
                size   = 0;
                time   = -1;
            }
        }

        public boolean isReadOnly()
        {
            return true;
        }

        public boolean isFolder()
        {
            return folder;
        }

        public boolean isModified()
        {
            return false;
        }

        public String getName()
        {
            return name;
        }

        public long getSize()
        {
            return size;
        }

        public long getTime()
        {
            // ajust last modified time to the java.io.File
            return (time >= 0)? time: 0;
        }

        public InputStream getInputStream()
            throws FileNotFoundException
        {
            return (isFolder() == false)? new FileInputStream(getFile()): null;
        }

        public OutputStream getOutputStream()
            throws IOException
        {
            return null;
        }

        public void rename(String name)
            throws IOException
        {
//            throw new IOException(
//                "cannot rename readonly file: " + getName());   // I18N
            // BUG: this is the design bug. Cache has to mamage such kind
            //      of operation in order to keep the data integrity.
            this.name = name;
        }

        public void save(ZipOutputStream arch)
            throws IOException
        {
            InputStream is    = null;
            ZipEntry    entry = new ZipEntry(
                getName() + ((isFolder())? SEPARATOR: ""));
            try {
                if(isFolder()) {
                    // folder
                    entry.setMethod(ZipEntry.STORED);
                    entry.setSize(0);
                    CRC32 crc = new CRC32();
                    entry.setCrc(crc.getValue());
                    entry.setTime(getTime());
                    arch.putNextEntry(entry);
                } else {
                    // file
                    if(isModified() == false)
                        entry.setTime(getTime());
                    else
                        entry.setTime(node.lastModified());
                    arch.putNextEntry(entry);
                    is = getInputStream();
                    FileUtil.copy(is, arch);
                }
            } finally {
                // close streams
                if(is != null) {
                    try {
                        is.close();
                    } catch(java.io.IOException ioe) {
                        // sorry! can do nothing about it.
                    }
                }
                if(arch != null)
                    arch.closeEntry();
            }
        }

        public void clean()
            throws IOException
        {
            if(node != null)
                node.delete();
        }

        public String toString()
        {
            return (
                ((isReadOnly())? "RO ": "RW ") +
                ((isFolder())? "D": "F") +
                " \"" + getName() + "\"");
        }

        /* package */ File getFile()
            throws FileNotFoundException
        {
            if(node == null) {
                try {
                    node = File.createTempFile(TMP_FILE_PREF, TMP_FILE_SUFX);
                   // copy the file from archive to the cache
                   OutputStream nos = null;
                   InputStream  zis = null;
                    try {
                        ZipEntry entry = zipFile.getEntry(getName());
                        if(entry != null) {
                            // copy existing file to the cache
                            zis = zipFile.getInputStream(entry);
                            nos = new FileOutputStream(node);
                            FileUtil.copy(zis, nos);
                        }
                    } finally {
                        // close streams
                        if(nos != null) {
                            try {
                                nos.close();
                            } catch(java.io.IOException ioe) {
                            }
                        }
                        if(zis != null) {
                            try {
                                zis.close();
                            } catch(java.io.IOException ioe) {
                            }
                        }
                    }
                } catch(java.lang.Exception exp) {
                    // delete cache file
                    if(node != null)
                        node.delete();
                    node = null;
                    throw new FileNotFoundException(
                        "cannot access file: " + getName());    // I18N
                }
            }
            return node;
        }

    }

    /* -----------------------------------------------------------
     * ReadWrite cache entry
     */
    private class ReadWriteEntry
        extends Entry
    {
        private boolean modified;

        // 'null' is special cace of root folder
        public ReadWriteEntry(ZipEntry entry)
        {
            super(entry);
        }

        public boolean isReadOnly()
        {
            return false;
        }

        public boolean isModified()
        {
            return modified;
        }

        public void rename(String name)
            throws IOException
        {
            modified = true;
            super.rename(name);
        }

        public OutputStream getOutputStream()
            throws IOException
        {
            modified = true;
            return (isFolder() == false)? new FileOutputStream(getFile()): null;
        }
    }
}
