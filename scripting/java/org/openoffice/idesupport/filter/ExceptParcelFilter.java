package org.openoffice.idesupport.filter;

public class ExceptParcelFilter implements FileFilter {
    private static final String DESCRIPTION = "Remove specified Parcel";
    private static final ExceptParcelFilter filter = new ExceptParcelFilter();
    private static String parcelName = null;

    private ExceptParcelFilter() {
    }

    public void setParcelToRemove(String parcelName)
    {
        this.parcelName = parcelName;
    }

    public static ExceptParcelFilter getInstance() {
        return filter;
    }
    public boolean validate(String name) {
        if (name.startsWith(this.parcelName))
            return true;
        return false;
    }

    public String toString() {
        StringBuffer buf = new StringBuffer(DESCRIPTION + ": ");

        buf.append("<" + this.parcelName + ">");

        return buf.toString();
    }
}
