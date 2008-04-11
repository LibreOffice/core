/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ManifestParser.java,v $
 * $Revision: 1.8 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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
