package org.openoffice.netbeans.modules.office.utils;

import java.io.InputStream;
import java.io.OutputStream;
import java.io.IOException;
import java.io.File;

import org.w3c.dom.Document;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;
import org.xml.sax.SAXParseException;

import org.openide.xml.XMLUtil;

import org.openoffice.netbeans.modules.office.options.OfficeSettings;
import org.openoffice.idesupport.xml.XMLParser;

public class ManifestParser implements XMLParser {

    private static ManifestParser parser = null;

    private ManifestParser() {
    }

    public static ManifestParser getManifestParser() {
        if (parser == null) {
            synchronized(ManifestParser.class) {
                if (parser == null)
                    parser = new ManifestParser();
            }
        }
        return parser;
    }

    public Document parse(InputStream inputStream) {
        InputSource is;
        Document result = null;

        try {
            is = new InputSource(inputStream);
            is.setSystemId("file://" +
                OfficeSettings.getDefault().getOfficeDirectory() +
                File.separator + "share" +
                File.separator + "dtd" +
                File.separator + "officedocument" +
                File.separator + "1_0" + File.separator);
            result = XMLUtil.parse(is, false, false, null, null);
        }
        catch (IOException ioe) {
            System.out.println("IO Error parsing stream.");
            return null;
        }
        catch (SAXParseException spe) {
            System.out.println("Sax Error parsing stream: " + spe.getMessage());
            System.out.println("\tPublicId: " + spe.getPublicId());
            System.out.println("\tSystemId: " + spe.getSystemId());
            return null;
        }
        catch (SAXException se) {
            System.out.println("Sax Error parsing stream: " + se.getMessage());
            return null;
        }

        return result;
    }

    public void write(Document doc, OutputStream out) throws IOException {
        XMLUtil.write(doc, out, "");
    }
}
