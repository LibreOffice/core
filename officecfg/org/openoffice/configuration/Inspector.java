/*************************************************************************
 *
 *  $RCSfile: Inspector.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: dg $ $Date: 2001-07-05 08:04:36 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
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
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
