/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ManifestParser.java,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:19:49 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

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

import com.sun.star.script.framework.container.XMLParser;
import org.openoffice.netbeans.modules.office.options.OfficeSettings;
import org.openoffice.idesupport.OfficeInstallation;

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
            OfficeInstallation oi = OfficeSettings.getDefault().getOfficeDirectory();
            String id = oi.getURL("share/dtd/officedocument/1_0/");

            is = new InputSource(inputStream);
            is.setSystemId(id);

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
