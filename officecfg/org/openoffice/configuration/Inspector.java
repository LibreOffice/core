/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Inspector.java,v $
 * $Revision: 1.4 $
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
package org.openoffice.configuration;

import java.io.*;
import org.xml.sax.*;
import javax.xml.parsers.SAXParserFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.parsers.SAXParser;

/**
 * Title:        Inspector
 * Description:  Validates an xml document against a dtd and retrieves the necessary
                 package and component informations<p>
 */
public class Inspector extends HandlerBase
{
    public java.lang.String     componentName;
    public java.lang.String     packageName;
    public java.lang.String     categoryName;
    public java.lang.String     transformationFile;

    public Inspector()
    {
        componentName       = new String();
        packageName         = new String();
        transformationFile  = new String("instance.xsl");
    }

    //===========================================================
    // SAX DocumentHandler methods
    //===========================================================
    public InputSource resolveEntity(java.lang.String publicId,
                                     java.lang.String systemId)
                          throws SAXException
    {
        // take the transformation file 'instance2.xsl' if schema.description2.dtd is used
        if (new File(systemId).getName().equalsIgnoreCase("schema.description2.dtd"))
            transformationFile  = "instance2.xsl";

        return new InputSource(systemId);
    }


    //===========================================================
    // SAX DocumentHandler methods
    //===========================================================

    public void setDocumentLocator (Locator l)
    {
        // Save this to resolve relative URIs or to give diagnostics.
        System.out.println ("** Start validating: " + l.getSystemId());
    }

    public void startElement(java.lang.String name,
                         AttributeList attributes) throws SAXException
    {
        if (componentName.length() == 0 && name == "schema:component")
        {
            componentName = attributes.getValue("cfg:name");
            packageName = attributes.getValue("cfg:package");
            categoryName = attributes.getValue("schema:category");

            if (categoryName == null)
                categoryName = new String("PrivateApplProfile");
        }
    }

    public void startDocument ()
    throws SAXException
    {
    }

    public void endDocument ()
    throws SAXException
    {
        System.out.println ("** Document is valid!");
    }

    //===========================================================
    // SAX ErrorHandler methods
    //===========================================================

    // treat validation errors as fatal
    public void error (SAXParseException e)
    throws SAXParseException
    {
        throw e;
    }

    // dump warnings too
    public void warning (SAXParseException err)
    throws SAXParseException
    {
        System.out.println ("** Warning"
            + ", line " + err.getLineNumber ()
            + ", uri " + err.getSystemId ());
        System.out.println("   " + err.getMessage ());
    }

    //===========================================================
    // Helpers ...
    //===========================================================

    public static void main (String argv [])
    {
        if (argv.length != 1) {
            System.err.println ("Usage: cmd filename");
            System.exit (1);
        }
        // Use the validating parser
        SAXParserFactory factory = SAXParserFactory.newInstance();
        factory.setValidating(true);
        try
        {
            // Parse the input
            SAXParser saxParser = factory.newSAXParser();
            saxParser.parse( new File(argv [0]), new Inspector() );
        }
        catch (SAXParseException spe) {
           // Error generated by the parser
           System.out.println ("\n** Parsing error"
              + ", line " + spe.getLineNumber ()
              + ", uri " + spe.getSystemId ());
           System.out.println("   " + spe.getMessage() );
           System.exit (1);
        }
        catch (SAXException sxe) {
           // Error generated by this application
           // (or a parser-initialization error)
           Exception  x = sxe;
           if (sxe.getException() != null)
               x = sxe.getException();
           x.printStackTrace();
           System.exit (1);
        }
        catch (ParserConfigurationException pce) {
            // Parser with specified options can't be built
            pce.printStackTrace();
            System.exit (1);

        }
        catch (IOException ioe) {
           // I/O error
           ioe.printStackTrace();
           System.exit (1);
        }

        System.exit (0);
    }

}
