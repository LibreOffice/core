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
import org.openoffice.netbeans.modules.office.actions.ParcelFolderCookie;
import org.openoffice.netbeans.modules.office.actions.ParcelFolderSupport;

public class ParcelFolder extends DataFolder {

    public static final String LANGUAGE_ATTRIBUTE = "language";

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
        private static final String LANGUAGE = LANGUAGE_ATTRIBUTE;
        private static final String CLASSPATH = "classpath";

        private File location;
        private FileFilter filter;
        private String language;
        private String classpath;

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

            language = (String)pf.getPrimaryFile().getAttribute(LANGUAGE);

            ParcelFolderCookie cookie =
                (ParcelFolderCookie)pf.getCookie(ParcelFolderCookie.class);
            String s = cookie.getClasspath();
            if (s != null) {
                classpath = s;
            }
            else {
                classpath = ".";
                cookie.setClasspath(classpath);
            }
        }

        public File getTargetDir() {
            return location;
        }

        public FileFilter getFileFilter() {
            return filter;
        }

        public String getLanguage() {
            if (language == null)
                language = (String)getPrimaryFile().getAttribute(LANGUAGE);
            return language;
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

            prop = createFilterProperty();
            props.put(prop);

            // prop = createLanguageProperty();
            // props.put(prop);

            prop = createClasspathProperty();
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

        private String[] languages = {"Java", "BeanShell"};

        private Node.Property createLanguageProperty() {
            Node.Property prop =
               new PropertySupport.ReadWrite(LANGUAGE, String.class,
                   "Parcel Language", "Language of scripts in this Parcel") {
                    public void setValue(Object obj) {
                        if (obj instanceof String) {
                            language = (String)obj;

                            try {
                                getPrimaryFile().setAttribute(LANGUAGE, language);
                            }
                            catch (IOException ioe) {
                            }
                        }
                    }

                    public Object getValue() {
                        if (language == null)
                            language = (String)getPrimaryFile().getAttribute(LANGUAGE);
                        return language;
                    }

                    public PropertyEditor getPropertyEditor() {
                        return new PropertyEditorSupport() {
                            public String[] getTags() {
                                return languages;
                            }

                            public void setAsText(String text) {
                                for (int i = 0; i < languages.length; i++)
                                    if (text.equals(languages[i]))
                                        this.setValue(languages[i]);
                            }

                            public String getAsText() {
                                return (String)this.getValue();
                            }
                        };
                    }
                };
            return prop;
        }

        private FileFilter[] availableFilters = new FileFilter[] {
            BinaryOnlyFilter.getInstance(), AllFilesFilter.getInstance()};

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

        private Node.Property createClasspathProperty() {
           Node.Property prop =
               new PropertySupport.ReadWrite(CLASSPATH, String.class,
                   "Classpath", "Classpath property for scripts in this parcel") {
                    public void setValue(Object obj) {
                        if (obj instanceof String) {
                            classpath = (String)obj;

                            ParcelFolderCookie cookie = (ParcelFolderCookie)
                                getDataObject().getCookie(ParcelFolderCookie.class);
                            cookie.setClasspath(classpath);
                        }
                    }

                    public Object getValue() {
                        return classpath;
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
