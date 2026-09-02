package org.libreoffice.androidlib.lok;

import java.io.Serializable;

import com.android.tools.r8.keepanno.annotations.KeepItemKind;
import com.android.tools.r8.keepanno.annotations.UsedByNative;

@UsedByNative(kind = KeepItemKind.CLASS_AND_MEMBERS)
public class LokClipboardEntry implements Serializable {
    public String mime;
    public byte[] data;
}
