/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SAXEventPrinter.java,v $
 * $Revision: 1.3 $
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
