package org.openoffice.netbeans.modules.office.loader;

import java.io.File;
import java.io.IOException;
import java.beans.PropertyEditor;
import java.beans.PropertyEditorSupport;

import org.openide.loaders.DataFolder;
import org.openide.loaders.DataObject;
import org.openide.loaders.DataFilter;
import org.openide.loaders.DataObjectExistsException;

import org.openide.filesystems.FileObject;
import org.openide.filesystems.FileUtil;

import org.openide.nodes.CookieSet;
import org.openide.nodes.Node;
import org.openide.nodes.PropertySupport;
import org.openide.nodes.Sheet;
import org.openide.util.HelpCtx;

import org.openoffice.idesupport.filter.*;
import org.openoffice.idesupport.zip.ParcelZipper;
import org.openoffice.netbeans.modules.office.actions.ParcelFolderSupport;

public class ParcelFolder extends DataFolder {

    public ParcelFolder(FileObject pf, ParcelFolderDataLoader loader)
        throws DataObjectExistsException {
        super(pf, loader);
        CookieSet cookies = getCookieSet();
        cookies.add(new ParcelFolderSupport(this));
    }

    public Node createNodeDelegate() {
        return new ParcelFolderNode(this, new ParcelFolderFilter());
    }

    public class ParcelFolderNode extends DataFolder.FolderNode {
        private static final String LOCATION = "location";
        private static final String FILTER = "filter";

        private File location;
        private FileFilter filter;

        private final FileFilter DEFAULT_FILTER = BinaryOnlyFilter.getInstance();

        public ParcelFolderNode(ParcelFolder pf, DataFilter dataFilter) {
            super(pf.createNodeChildren(dataFilter));

            location = (File)pf.getPrimaryFile().getAttribute(LOCATION);
            if (location == null)
                location = FileUtil.toFile(pf.getPrimaryFile());

            String name = (String)pf.getPrimaryFile().getAttribute(FILTER);
            if (name == null)
                filter = DEFAULT_FILTER;
            else {
                for (int i = 0; i < availableFilters.length; i++)
                    if (name.equals(availableFilters[i].toString()))
                        filter = availableFilters[i];
            }
        }

        public File getTargetDir() {
            return location;
        }

        public FileFilter getFileFilter() {
            return filter;
        }

        public Sheet createSheet() {
            Sheet sheet;
            Sheet.Set props;
            Node.Property prop;

            sheet = super.createSheet();
            props = sheet.get(Sheet.PROPERTIES);
            if (props == null) {
                props = Sheet.createPropertiesSet();
                sheet.put(props);
            }

            // prop = createLocationProperty();
            // props.put(prop);

            prop = createFilterProperty();
            props.put(prop);

            return sheet;
        }

        private Node.Property createLocationProperty() {
           Node.Property prop =
               new PropertySupport.ReadWrite(LOCATION, File.class,
                   "Location", "Output location of Parcel Zip File") {
                    public void setValue(Object obj) {
                        if (obj instanceof File) {
                            location = (File)obj;
                            try {
                                getPrimaryFile().setAttribute(LOCATION, location);
                            }
                            catch (IOException ioe) {
                                System.out.println("Error setting location attribute");
                            }
                        }
                    }

                    public Object getValue() {
                        return location;
                    }
                };
            prop.setValue("files", Boolean.FALSE);
            return prop;
        }

        private FileFilter[] availableFilters =
            new FileFilter[] {BinaryOnlyFilter.getInstance(), AllFilesFilter.getInstance()};

        private Node.Property createFilterProperty() {
            Node.Property prop =
               new PropertySupport.ReadWrite(FILTER, String.class,
                   "File Filter", "Files to be included in Parcel") {
                    public void setValue(Object obj) {
                        if (obj instanceof FileFilter) {
                            filter = (FileFilter)obj;

                            try {
                                getPrimaryFile().setAttribute(FILTER, filter.toString());
                            }
                            catch (IOException ioe) {
                                System.out.println("Error setting filter attribute");
                            }
                        }
                    }

                    public Object getValue() {
                        return filter;
                    }

                    public PropertyEditor getPropertyEditor() {
                        return new PropertyEditorSupport() {
                            public String[] getTags() {
                                String[] tags = new String[availableFilters.length];

                                for (int i = 0; i < availableFilters.length; i++)
                                    tags[i] = availableFilters[i].toString();

                                return tags;
                            }

                            public void setAsText(String text) {
                                for (int i = 0; i < availableFilters.length; i++)
                                    if (text.equals(availableFilters[i].toString()))
                                        this.setValue(availableFilters[i]);
                            }

                            public String getAsText() {
                                return this.getValue().toString();
                            }
                        };
                    }
                };
            return prop;
        }
    }

    private class ParcelFolderFilter implements DataFilter {
        public boolean acceptDataObject(DataObject dobj) {
            String name = dobj.getPrimaryFile().getNameExt();
            if (name.equals(ParcelZipper.PARCEL_DESCRIPTOR_XML))
                return false;
            return true;
        }
    }
}
