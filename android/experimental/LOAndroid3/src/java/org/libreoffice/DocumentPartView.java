package org.libreoffice;


import android.graphics.Bitmap;

public class DocumentPartView {
    private final int partIndex;
    private final String partName;
    private final Bitmap thumbnail;

    public DocumentPartView(int partIndex, String partName, Bitmap thumbnail) {
        this.partIndex = partIndex;
        this.partName = partName;
        this.thumbnail = thumbnail;
    }

    public String getPartName() {
        return partName;
    }

    public int getPartIndex() {
        return partIndex;
    }
    public Bitmap getThumbnail() {
        return thumbnail;
    }
}
