package org.openoffice.idesupport;

import java.io.File;
import java.util.ArrayList;
import org.openoffice.idesupport.zip.ParcelZipper;

import com.sun.star.script.framework.browse.ScriptEntry;

public class ExtensionFinder implements MethodFinder {

    private String[] extensions;
    private String language;

    public ExtensionFinder(String language, String[] extensions) {
        this.language = language;
        this.extensions = extensions;
    }

    public ScriptEntry[] findMethods(File basedir) {
        String parcelName;
        ArrayList files = new ArrayList(10);
        ScriptEntry[] empty = new ScriptEntry[0];

        if (basedir == null || basedir.exists() == false ||
            basedir.isDirectory() == false)
            return empty;

        parcelName = basedir.getName();
        if (parcelName.equals(ParcelZipper.CONTENTS_DIRNAME))
            parcelName = basedir.getParentFile().getName();

        findFiles(files, basedir, parcelName);

        if (files.size() != 0)
            return (ScriptEntry[])files.toArray(empty);
        return empty;
    }

    private void findFiles(ArrayList list, File basedir, String parcelName) {
        File[] children = basedir.listFiles();
        File f;

        for (int i = 0; i < children.length; i++) {
            f = children[i];

            if (f.isDirectory())
                findFiles(list, f, parcelName);
            else {
                for (int j = 0; j < extensions.length; j++) {
                    if (f.getName().endsWith(extensions[j])) {
                        ScriptEntry entry = new ScriptEntry(language,
                            f.getName(), f.getName(), parcelName);
                        list.add(entry);
                        break;
                    }
                }
            }
        }
    }
}
