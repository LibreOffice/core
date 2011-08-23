/************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

import java.io.FileWriter;
import java.io.InputStream;
import java.io.FileInputStream;
import java.io.BufferedInputStream;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.FileNotFoundException;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.Writer;
import java.io.PrintWriter;
import java.util.Vector;
import java.util.Properties;

import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.ParserConfigurationException;

import org.w3c.dom.Node;
import org.w3c.dom.Document;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;
import org.xml.sax.SAXParseException;
/**
 *  This class will diff 2 Xml files.
 *
 *  @author   Stephen Mak
 */

public final class XmlDiff {

    private static final String PROPSFILE = "XmlDiff.properties";
    private static final String FILE1 = "XmlDiff.file1";
    private static final String FILE2 = "XmlDiff.file2";
    private static final String OUTPUT= "XmlDiff.output";
    private static final String IGNORE_TAGS= "XmlDiff.tags";

    private Properties props_ = null;
    private static PrintWriter writer_ = null;
    private String[] tags_ = null;
    private String file1_ = null;
    private String file2_ = null;

    /**
     *  Constructor.  Load the properties file.
     */

    public XmlDiff() throws IOException {

        Class c = this.getClass();
        InputStream is = c.getResourceAsStream(PROPSFILE);
        BufferedInputStream bis = new BufferedInputStream(is);
        props_ = new Properties();
        props_.load(bis);
        bis.close();

        String file1 = props_.getProperty(FILE1, "");
        String file2 = props_.getProperty(FILE2, "");
        String tagsString = props_.getProperty(IGNORE_TAGS, "");
        String output = props_.getProperty("debug.output", "System.out");
        setOutput(output);
        tags_ = parseTags(tagsString);
    }

    /**
     * diff 2 xml, but overwrite the property file's file1/2 setting with
     * the input argument
     */
    public boolean diff(String file1, String file2)  throws IOException {
        file1_ = file1;
        file2_ = file2;
        return diff();
    }

    public boolean diff() throws IOException {

        boolean result = false;

        writer_.println("parsing "+ file1_ + "...");
        // parse the Xml file
        Document doc1 = parseXml(file1_);

        writer_.println("parsing "+ file1_ + "...");
        Document doc2 = parseXml(file2_);

        if (doc1 != null && doc2 != null) {
          writer_.println("diffing "+ file1_ + " & " + file2_ + "...");
          result = compareNode(doc1, doc2);
        }
        return result;
    }

    private void diffLog(String errMsg, Node node1, Node node2) {

        String node1Str = "";
        String node2Str = "";

        if (node1 != null) {
            node1Str = "[Type]:" + nodeInfo(node1) + 
                       " [Name]:" + node1.getNodeName(); 
            if (node1.getNodeValue() != null)
                node1Str += " [Value]:" + node1.getNodeValue();
        }

        if (node2 != null) {
            node2Str = "[Type]:" + nodeInfo(node2) +
                       " [Name]:" + node2.getNodeName(); 
            if (node2.getNodeValue() != null)
                node2Str += " [Value]:" + node2.getNodeValue();
        }

        writer_.println(errMsg);
        writer_.println("  Node1 - " + node1Str);
        writer_.println("  Node2 - " + node2Str);
    }

    private String nodeInfo(Node node) {
    
        String str = null;
        switch (node.getNodeType()) {
    
            case Node.ELEMENT_NODE:
                str = "ELEMENT";
                break;
            case Node.ATTRIBUTE_NODE:
                str = "ATTRIBUTE";
                break;
            case Node.TEXT_NODE:
                str = "TEXT";
                break;
            case Node.CDATA_SECTION_NODE:
                str = "CDATA_SECTION";
                break;
            case Node.ENTITY_REFERENCE_NODE:
                str = "ENTITY_REFERENCE";
                break;
            case Node.ENTITY_NODE:
                str = "ENTITY";
                break;
            case Node.PROCESSING_INSTRUCTION_NODE:
                str = "PROCESSING_INSTRUCTION";
                break;
            case Node.COMMENT_NODE:
                str = "COMMENT";
                break;
            case Node.DOCUMENT_NODE:
                str = "DOCUMENT";
                break;
            case Node.DOCUMENT_TYPE_NODE:
                str = "DOCUMENT_TYPE";
                break;
            case Node.DOCUMENT_FRAGMENT_NODE:
                str = "DOCUMENT_FRAGMENT";
                break;
            case Node.NOTATION_NODE:
                str = "NOTATION";
                break;
        }
        return str;
    }

    private boolean ignoreTag(String nodeName) {


        if (tags_ != null) {
            for (int i = 0; i < tags_.length; i++) {
                if (tags_[i].equals(nodeName))
                    return true;
            }
        }
        return false;
    }

    // for future use if we want to compare attributes
    private boolean attributesEqual(Node node1, Node node2) {
        return true;
    }

    private boolean compareNode(Node node1, Node node2) {
        boolean equal = false;

        while (true) {

            if (node1 == null && node2 == null) {
                equal = true;
                break;
            } else if (node1 == null || node2 == null) {
                diffLog("DIFF: one of the node is null", node1, node2); 
                break;
            }

            if (node1.getNodeType() != node2.getNodeType()) {
                diffLog("DIFF: nodetype is different", node1, node2); 
                break;
            }

            if (node1.getNodeName() == null && node2.getNodeName() == null) {
                // empty
            } else if (node1.getNodeName() == null ||
                       node2.getNodeName() == null) {
                diffLog("DIFF: one of the nodeName is null", node1, node2); 
                break;
            } else if (!node1.getNodeName().equals(node2.getNodeName())) {
                diffLog("DIFF: nodeName is different", node1, node2); 
                break;
            }

            if (ignoreTag(node1.getNodeName())) {
                diffLog("DIFF: Some tag(s) is ignored", node1, node2); 
                equal = true;
                break;
            }

            if (node1.getNodeValue() == null && node2.getNodeValue() == null) {
                // empty
            } else if (node1.getNodeValue() == null ||
                       node2.getNodeValue() == null) {
                diffLog("DIFF: one of the nodevalue is null", node1, node2); 
                break;
            } else if (!node1.getNodeValue().equals(node2.getNodeValue())) {
                diffLog("DIFF: nodeValue is different", node1, node2); 
                break;
            }

            // try to compare attributes if necessary
            if (!attributesEqual(node1, node2))
                break;

            NodeList node1Children = node1.getChildNodes();
            NodeList node2Children = node2.getChildNodes();

            // number of children have to be the same
            if (node1Children == null && node2Children == null) {
                equal = true;
                break;
            }

            if (node1Children == null || node2Children == null) {
                diffLog("DIFF: one node's children is null", node1, node2); 
                break;
            }

            if (node1Children.getLength() != node2Children.getLength())  {
                diffLog("DIFF: num of children is different", node1, node2); 
                break;
            }

            // compare all the childrens
            equal = true;

            for (int i = 0; i < node1Children.getLength(); i++) {
                if (!compareNode(node1Children.item(i),
                                 node2Children.item(i))) {
                    equal = false;
                    break;
                }
            }
            break;
        }

        return equal;
    }

    private Document parseXml (String filename) throws IOException {

        Document w3cDocument = null;

        FileInputStream fis;

        try {
            fis = new FileInputStream(filename);
        } catch (FileNotFoundException ex) {
            ex.printStackTrace(writer_);
            writer_.println(ex.getMessage());
            return w3cDocument;
        }

        /** factory for DocumentBuilder objects */
        DocumentBuilderFactory factory = null;
        factory = DocumentBuilderFactory.newInstance();
        factory.setNamespaceAware(true);
        factory.setValidating(false);

        /** DocumentBuilder object */
        DocumentBuilder builder = null;

        try {
            builder = factory.newDocumentBuilder();
        } catch (ParserConfigurationException ex) {
            ex.printStackTrace(writer_);
            writer_.println(ex.getMessage());
            return null;
        }


        builder.setErrorHandler(
            new org.xml.sax.ErrorHandler() {
                // ignore fatal errors (an exception is guaranteed)
                public void fatalError(SAXParseException e)
                    throws SAXException {
                    throw e;
                }

                public void error(SAXParseException e)
                    throws SAXParseException {
                    // make sure validation error is thrown.
                    throw e;
                }

                public void warning(SAXParseException e)
                    throws SAXParseException {
                }
            }
        );

        try {
            w3cDocument = builder.parse(fis);
            w3cDocument.getDocumentElement().normalize();
        } catch (SAXException ex) {
            ex.printStackTrace(writer_);
            writer_.println(ex.getMessage());
            return w3cDocument;
        }

        return w3cDocument;
    }

    private String [] parseTags(String tagsString) {
        Vector tagsVector = new Vector();
        if (tagsString.length() == 0) 
            return null;

        int start = 0;
        int end = 0;
        // break the tag string into a vector of strings by words
        for (end = tagsString.indexOf(" ", start);
             end != -1 ;
             start = end + 1, end = tagsString.indexOf(" ", start)) {
            tagsVector.add(tagsString.substring(start,end));
        }

        tagsVector.add(tagsString.substring(start,tagsString.length()));

        // convert the vector to array
        String[] tags= new String[tagsVector.size()];
        tagsVector.copyInto(tags);

        return tags;
    }


    /**
     *  Set the output to the specified argument.
     *  This method is only used internally to prevent
     *  invalid string parameter.
     *
     *  @param   str   output specifier
     */
    private static void setOutput(String str) {

        if (writer_ == null) {

            if (str.equals("System.out")) {

                setOutput(System.out);

            } else if (str.equals("System.err")) {

                setOutput(System.err);

            } else {

                try {

                    setOutput(new FileWriter(str));

                } catch (IOException e) {

                    e.printStackTrace(System.err);
                }
            }
        }
    }

    /**
     *  Set the output to an OutputStream object.
     *
     *  @param   stream   OutputStream object
     */

    private static void setOutput(OutputStream stream) {

        setOutput(new OutputStreamWriter(stream));
    }

    /**
     *  Set the Writer object to manage the output.
     *
     *  @param   w   Writer object to write out
     */

    private static void setOutput(Writer w) {

        if (writer_ != null) {

            writer_.close();
        }

        writer_ = new PrintWriter(new BufferedWriter(w), true);
    }

    public static void main(String args[]) throws IOException {

        if (args.length != 0 && args.length != 2) {
            System.out.println("Usage: XmlDiff [<file1> <file2>].");
            return;
        }

        XmlDiff xmldiff = new XmlDiff();

        boolean same = false;
        if (args.length == 2) {
            same = xmldiff.diff(args[0], args[1]);
        } else {
            same = xmldiff.diff();
        }

        System.out.println("Diff result: " + same); 
    }
}

