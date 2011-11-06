/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
