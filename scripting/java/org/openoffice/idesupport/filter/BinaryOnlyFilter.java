package org.openoffice.idesupport.filter;

public class BinaryOnlyFilter implements FileFilter {
    private static final String[] EXTENSIONS = {".class", ".jar"};
    private static final String DESCRIPTION = "Binaries Only";
    private static final BinaryOnlyFilter filter = new BinaryOnlyFilter();

    private BinaryOnlyFilter() {
    }

    public static BinaryOnlyFilter getInstance() {
        return filter;
    }
    public boolean validate(String name) {
        for (int i = 0; i < EXTENSIONS.length; i++)
            if (name.endsWith(EXTENSIONS[i]))
                return true;
        return false;
    }

    public String toString() {
        StringBuffer buf = new StringBuffer(DESCRIPTION + ": ");

        for (int i = 0; i < EXTENSIONS.length - 1; i++)
            buf.append("<" + EXTENSIONS[i] + "> ");
        buf.append("<" + EXTENSIONS[EXTENSIONS.length - 1] + ">");

        return buf.toString();
    }
}
