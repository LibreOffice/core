/************************************************************************
 *
 *  MIMETypes.java
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *  Copyright: 2002-2004 by Henrik Just
 *
 *  All Rights Reserved.
 *
 *  Version 0.3.3b (2004-02-26)
 *
 */

package writer2latex.office;

/* Some helpers to handle the MIME types used by OOo
 */

public final class MIMETypes{
    // OOo MIME types, taken from
    // http://framework.openoffice.org/documentation/mimetypes/mimetypes.html
    public static final String WRITER="application/vnd.sun.xml.writer";
    public static final String CALC="application/vnd.sun.xml.calc";
    public static final String IMPRESS="application/vnd.sun.xml.impress";
    public static final String DRAW="application/vnd.sun.xml.draw";
    public static final String CHART="application/vnd.sun.xml.chart";
    public static final String MATH="application/vnd.sun.xml.math";
    // Various graphics formats
    public static final String PNG="image/png";
    public static final String JPEG="image/jpeg";
    public static final String EPS="image/x-eps"; // or application/postscript?
    public static final String SVM="(starview metafile)"; // no MIME type defined!
    // Desitination formats
    public static final String XHTML="text/html";
    public static final String XHTML_MATHML="application/xhtml+xml";
    public static final String XHTML_MATHML_XSL="application/xml";
    public static final String LATEX="application/x-latex";
    public static final String BIBTEX="application/x-bibtex";
    public static final String TEXT="text";

    // Magic signatures for some binary graphics files
    public static final byte[] PNG_SIG = { (byte) 0x89, 0x50, 0x4e, 0x47 }; // .PNG
    public static final byte[] JPEG_SIG = { (byte) 0xff, (byte) 0xd8, (byte) 0xff, (byte) 0xe0 };
    public static final byte[] EPS_SIG = { 0x25, 0x21 }; // %!
    public static final byte[] SVM_SIG = { 0x56, 0x43, 0x4c, 0x4d, 0x54, 0x46 }; // VCLMTF

    // Preferred file extensions for some graphics files
    public static final String PNG_EXT = ".png";
    public static final String JPEG_EXT = ".jpg"; // this is the default in graphicx.sty
    public static final String EPS_EXT = ".eps";
    public static final String SVM_EXT = ".svm";

    private static final boolean isType(byte[] blob, byte[] sig) {
        int n = sig.length;
        for (int i=0; i<n; i++) {
            if (blob[i]!=sig[i]) { return false; }
        }
        return true;
    }

    public static final String getMagicMIMEType(byte[] blob) {
        if (isType(blob,PNG_SIG)) { return PNG; }
        if (isType(blob,JPEG_SIG)) { return JPEG; }
        if (isType(blob,EPS_SIG)) { return EPS; }
        if (isType(blob,SVM_SIG)) { return SVM; }
        return "";
    }

    public static final String getFileExtension(String sMIME) {
        if (PNG.equals(sMIME)) { return PNG_EXT; }
        if (JPEG.equals(sMIME)) { return JPEG_EXT; }
        if (EPS.equals(sMIME)) { return EPS_EXT; }
        if (SVM.equals(sMIME)) { return SVM_EXT; }
        return "";
    }


}