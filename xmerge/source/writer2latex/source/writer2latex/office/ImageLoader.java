/************************************************************************
 *
 *  ImageLoader.java
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
 *  Version 0.3.3g (2004-10-22)
 *
 */

package writer2latex.office;

import java.io.IOException;

import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import org.openoffice.xmerge.converter.xml.EmbeddedBinaryObject;
import org.openoffice.xmerge.converter.xml.OfficeDocument;

import writer2latex.util.*;
import writer2latex.xmerge.*;

/**
 *  <p>This class extracts images from an OOo file.
 *  The images are returned as BinaryGraphicsDocument.</p>
 */
public final class ImageLoader {
    private OfficeDocument oooDoc;
    private String sOutFileName;
    private int nImageCount = 0; // used to generate filenames
    private boolean bExtractEPS; // should EPS be extracted from SVM?

    public ImageLoader(OfficeDocument oooDoc, String sOutFileName, boolean bExtractEPS) {
        this.oooDoc = oooDoc;
        this.sOutFileName = sOutFileName;
        this.bExtractEPS = bExtractEPS;
    }

    public BinaryGraphicsDocument getImage(Node node) {
        // node must be a draw:image element.
        // variables to hold data about the image:
        String sMIME = null;
        String sExt = null;
        byte[] blob = null;

        String sHref = Misc.getAttribute(node,XMLString.XLINK_HREF);
        if (sHref==null) {
            // Image must be contained in an office:binary-element as base64:
            Node obd = Misc.getChildByTagName(node,XMLString.OFFICE_BINARY_DATA);
            if (obd!=null) {
                StringBuffer buf = new StringBuffer();
                NodeList nl = obd.getChildNodes();
                int nLen = nl.getLength();
                for (int i=0; i<nLen; i++) {
                    if (nl.item(i).getNodeType()==Node.TEXT_NODE) {
                        buf.append(nl.item(i).getNodeValue());
                    }
                }
                // TODO: Use another base64 decoder. This one is "nonstandard"
                /*try {
                    blob = new sun.misc.BASE64Decoder().decodeBuffer(buf.toString());
                } catch (IOException e) {
                    System.out.println("Oops - error converting base64");
                }
                sMIME = MIMETypes.getMagicMIMEType(blob);
                sExt = MIMETypes.getFileExtension(sMIME);*/
                // Temp: Because of issues with xmerge, we kill the result
                blob = null;
                // TODO: Report to Issuezilla
            }
        }
        else if (sHref.startsWith("#")) {
            // Image is embedded in package:
            sHref = sHref.substring(1);
            if (sHref.startsWith("./")) { sHref = sHref.substring(2); }
            if (oooDoc.getEmbeddedObject(sHref) instanceof EmbeddedBinaryObject) {
                EmbeddedBinaryObject object
                    = (EmbeddedBinaryObject) oooDoc.getEmbeddedObject(sHref);
                blob = object.getBinaryData();
                sMIME = object.getType();
                sExt = MIMETypes.getFileExtension(sMIME);
            }
        }
        else {
            // This is a linked image
            // TODO: Download the image from the url in sHref
        }

        if (blob==null) { return null; }

        // Assign a name (without extension)
        String sName = sOutFileName+"-img"+(++nImageCount);

        BinaryGraphicsDocument bgd = null;

        if (bExtractEPS && MIMETypes.SVM.equals(MIMETypes.getMagicMIMEType(blob))) {
            // Look for postscript:
            int[] offlen = new int[2];
            if (SVMReader.readSVM(blob,offlen)) {
                bgd = new BinaryGraphicsDocument(sName,
                             MIMETypes.EPS_EXT,MIMETypes.EPS);
                bgd.read(blob,offlen[0],offlen[1]);
             }
        }

        if (bgd==null) {
            bgd = new BinaryGraphicsDocument(sName,sExt,sMIME);
            bgd.read(blob);
        }

        return bgd;
    }
}
