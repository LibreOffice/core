package org.openoffice.idesupport.filter;

public class AllFilesFilter implements FileFilter {
    private static final AllFilesFilter filter = new AllFilesFilter();

    private AllFilesFilter() {
    }

    public static AllFilesFilter getInstance() {
        return filter;
    }

    public boolean validate(String name) {
        return true;
    }

    public String toString() {
        return "<all files>";
    }
}
