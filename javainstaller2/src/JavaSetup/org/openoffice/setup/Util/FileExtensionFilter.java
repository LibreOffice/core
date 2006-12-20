package org.openoffice.setup.Util;
import java.io.File;
import java.io.FilenameFilter;

public class FileExtensionFilter implements FilenameFilter {

    private String fileExtension;

    public FileExtensionFilter(String extension) {
        fileExtension = extension;
        if ( ! fileExtension.startsWith(".") ) {
            fileExtension = "." + fileExtension;
        }
    }

    public boolean accept(File dir, String name) {
        if (name.endsWith(fileExtension)) {
            File candidate = new File(dir, name);
            if (candidate.isFile() && candidate.canRead()) {
                return true;
            }
        }
        return false;
    }
}