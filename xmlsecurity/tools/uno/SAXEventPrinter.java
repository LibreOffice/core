/*************************************************************************
 *
 *  $RCSfile: SAXEventPrinter.java,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: mt $ $Date: 2004-07-12 13:15:24 $
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

package com.sun.star.xml.security.uno;

import org.w3c.dom.Node;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Attr;
import org.w3c.dom.NodeList;
import java.io.IOException;
import java.io.FileOutputStream;

/* uno classes */
import com.sun.star.xml.sax.XDocumentHandler;
import com.sun.star.xml.sax.XAttributeList;

/*
 * The SAXEventPrinter class is used to print out received
 * SAX event stream.
 */
class SAXEventPrinter implements XDocumentHandler
{
    /*
     * how many spaces as the indent of line
     */
    private int m_nIndent;

    /*
     * whether a NEW LINE character need to be appended to
     * each line
     */
    private boolean m_bIsFormatted;

    /*
     * the output stream to write
     */
    private FileOutputStream m_fileOutputStream;

    SAXEventPrinter(FileOutputStream fileOutputStream, boolean isFormatted)
    {
        m_nIndent = 0;
        m_fileOutputStream = fileOutputStream;
        m_bIsFormatted = isFormatted;
    }

    protected static void outputIndent(int m_nIndent, FileOutputStream fileOutputStream)
        throws IOException
    {
        for (int i=0; i<m_nIndent; ++i)
        {
            fileOutputStream.write(" ".getBytes());
        }
    }

    /*
     * displays the tree of a Node.
     */
    protected static void display(Node node, int indent, FileOutputStream fileOutputStream, boolean isFormatted)
        throws IOException
    {
        if (node != null)
        {
            int type = node.getNodeType();
            String message;
            NodeList children;
            int i, length;

            switch (type)
            {
            case Node.DOCUMENT_NODE:
                children = node.getChildNodes();
                length = children.getLength();
                for (i=0; i<length; ++i)
                {
                    display(children.item(i), indent+2, fileOutputStream, isFormatted);
                }

                break;

            case Node.ELEMENT_NODE:
                message = new String("<"+node.getNodeName());
                NamedNodeMap attrs = node.getAttributes();

                length = attrs.getLength();
                for (i=0; i<length; ++i)
                {
                    Attr attr = (Attr)attrs.item(i);
                    message += " "+attr.getNodeName()+"=\""+attr.getNodeValue()+"\"";
                }

                message += ">";

                if (isFormatted)
                {
                    outputIndent(indent, fileOutputStream);
                }

                fileOutputStream.write(message.getBytes("UTF-8"));

                if (isFormatted)
                {
                    fileOutputStream.write("\n".getBytes());
                }

                children = node.getChildNodes();
                length = children.getLength();
                for (i=0; i<length; ++i)
                {
                    display(children.item(i), indent+2, fileOutputStream, isFormatted);
                }

                if (isFormatted)
                {
                    outputIndent(indent, fileOutputStream);
                }

                fileOutputStream.write("</".getBytes());
                fileOutputStream.write(node.getNodeName().getBytes("UTF-8"));
                fileOutputStream.write(">".getBytes());

                if (isFormatted)
                {
                    fileOutputStream.write("\n".getBytes());
                }

                break;

            case Node.TEXT_NODE:
                message = node.getNodeValue();
                if (message != null )
                {
                    if (isFormatted)
                    {
                        outputIndent(indent, fileOutputStream);
                    }

                    fileOutputStream.write(node.getNodeValue().getBytes("UTF-8"));

                    if (isFormatted)
                    {
                        fileOutputStream.write("\n".getBytes());
                    }
                }
                break;

            case Node.PROCESSING_INSTRUCTION_NODE:
                if (isFormatted)
                {
                    outputIndent(indent, fileOutputStream);
                }

                fileOutputStream.write("<?".getBytes());
                fileOutputStream.write(node.getNodeName().getBytes("UTF-8"));
                fileOutputStream.write(node.getNodeValue().getBytes("UTF-8"));
                fileOutputStream.write("?>".getBytes());

                if (isFormatted)
                {
                    fileOutputStream.write("\n".getBytes());
                }

                break;
            default:
                break;
            }
        }
    }

    /*
     * XDocumentHandler
     */
    public void  startDocument ()
    {
    }

    public void endDocument()
    {
    }

    public void startElement (String str, com.sun.star.xml.sax.XAttributeList xattribs)
    {
        try
        {
            String message;

            message = new String("<"+str);
            if (xattribs !=null)
            {
                int length = xattribs.getLength();
                for (short i=0; i<length; ++i)
                {
                    message += " "+xattribs.getNameByIndex(i)+"=\""+xattribs.getValueByIndex(i)+"\"";
                }
            }
            message += ">";

            if (m_bIsFormatted)
            {
                outputIndent(m_nIndent, m_fileOutputStream);
            }

            m_fileOutputStream.write(message.getBytes("UTF-8"));

            if (m_bIsFormatted)
            {
                m_fileOutputStream.write("\n".getBytes());
            }

            m_nIndent += 2;
        }
        catch (IOException e)
        {
            e.printStackTrace();
        }
    }

    public void endElement(String str)
    {
        try
        {
            m_nIndent -= 2;
            if (m_bIsFormatted)
            {
                outputIndent(m_nIndent, m_fileOutputStream);
            }

            m_fileOutputStream.write("</".getBytes());
            m_fileOutputStream.write(str.getBytes("UTF-8"));
            m_fileOutputStream.write(">".getBytes());

            if (m_bIsFormatted)
            {
                m_fileOutputStream.write("\n".getBytes());
            }
        }
        catch (IOException e)
        {
            e.printStackTrace();
        }
    }

    public void characters(String str)
    {
        try
        {
            if (m_bIsFormatted)
            {
                outputIndent(m_nIndent, m_fileOutputStream);
            }

            m_fileOutputStream.write(str.getBytes("UTF-8"));

            if (m_bIsFormatted)
            {
                m_fileOutputStream.write("\n".getBytes());
            }
        }
        catch (IOException e)
        {
            e.printStackTrace();
        }
    }

    public void ignorableWhitespace(String str)
    {
    }

    public void processingInstruction(String aTarget, String aData)
    {
        try
        {
            if (m_bIsFormatted)
            {
                outputIndent(m_nIndent, m_fileOutputStream);
            }

            m_fileOutputStream.write("<?".getBytes());
            m_fileOutputStream.write(aTarget.getBytes("UTF-8"));
            m_fileOutputStream.write("?>".getBytes());

            if (m_bIsFormatted)
            {
                m_fileOutputStream.write("\n".getBytes());
            }
        }
        catch (IOException e)
        {
            e.printStackTrace();
        }
    }

    public void setDocumentLocator (com.sun.star.xml.sax.XLocator xLocator )
        throws com.sun.star.xml.sax.SAXException
    {
    }
}
