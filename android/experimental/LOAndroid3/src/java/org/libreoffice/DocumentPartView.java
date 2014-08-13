package org.libreoffice;


public class DocumentPartView {
    private final int partIndex;
    private final String partName;

    public DocumentPartView(int partIndex, String partName) {
        this.partIndex = partIndex;
        this.partName = partName;
    }

    public String getPartName() {
        return partName;
    }

    public int getPartIndex() {
        return partIndex;
    }
}
