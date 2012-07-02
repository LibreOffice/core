/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package util;

import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.Iterator;

import com.sun.star.beans.PropertyValue;
import com.sun.star.document.XExporter;
import com.sun.star.document.XFilter;
import com.sun.star.document.XImporter;
import com.sun.star.io.XActiveDataSource;
import com.sun.star.io.XInputStream;
import com.sun.star.io.XOutputStream;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.uno.Any;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.xml.sax.InputSource;
import com.sun.star.xml.sax.XAttributeList;
import com.sun.star.xml.sax.XDocumentHandler;
import com.sun.star.xml.sax.XLocator;
import com.sun.star.xml.sax.XParser;


public class XMLTools {

    /**
     * The implementation of <code>com.sun.star.xml.sax.XAttributeList</code>
     * where attributes and their values can be added.
     */
    public static class AttributeList implements XAttributeList {
        private static class Attribute {
            public String Name ;
            public String Type ;
            public String Value ;
        }
        private Hashtable<String, Attribute> attrByName = new Hashtable<String, Attribute>() ;
        private ArrayList<Attribute> attributes = new ArrayList<Attribute>() ;
        private PrintWriter log = null ;

        /**
         * Creates a class instance.
         */
        public AttributeList() {}

        /**
         * Constructs a list which will report to <code>log</code>
         * specified about each <code>XDocumentHandler</code> method
         * call.
         */
        public AttributeList(PrintWriter log) {
            this.log = log ;
        }

        public AttributeList(XAttributeList list) {
            if (list == null) return ;
            for (short i = 0; i < list.getLength(); i++) {
                add(list.getNameByIndex(i), list.getTypeByIndex(i),
                    list.getValueByIndex(i)) ;
            }
        }

        /**
         * Adds an attribute with type and value specified.
         * @param name The attribute name.
         * @param type Value type (usually 'CDATA' used).
         * @param value Attribute value.
         */
        public void add(String name, String type, String value) {
            Attribute attr = new Attribute() ;
            attr.Name = name ;
            attr.Type = type ;
            attr.Value = value ;
            attributes.add(attr) ;
            attrByName.put(attr.Name, attr) ;
        }

        /**
         * Adds an attribute with value specified. As a type of
         * value 'CDATA' string specified.
         * @param name The attribute name.
         * @param value Attribute value.
         */
        public void add(String name, String value) {
            add(name, "CDATA", value) ;
        }

        /**
         * Clears all attributes added before.
         */
        public void clear() {
            attrByName.clear() ;
            attributes.clear() ;
        }

        /***************************************
        * XAttributeList methods
        ****************************************/

        public short getLength() {
            if (log != null)
                log.println("getLength() called -> " + attributes.size()) ;
            return (short) attributes.size() ;
        }

        public String getNameByIndex(short idx) {
            String name = attributes.get(idx).Name ;
            if (log != null)
                log.println("getNameByIndex(" + idx + ") called -> '" +
                name + "'") ;
            return name ;
        }

        public String getTypeByIndex(short idx) {
            String type = attributes.get(idx).Type  ;
            if (log != null)
                log.println("getTypeByIndex(" + idx + ") called -> '" +
                    type + "'") ;
            return type;
        }

        public String getTypeByName(String name) {
            String type = attrByName.get(name).Type ;
            if (log != null)
                log.println("getTypeByName('" + name + "') called -> '" +
                    type + "'") ;
            return type;
        }
        public String getValueByIndex(short idx) {
            String value = attributes.get(idx).Value ;
            if (log != null)
                log.println("getValueByIndex(" + idx + ") called -> '" +
                    value + "'") ;
            return  value;
        }

        public String getValueByName(String name) {
            String value = attrByName.get(name).Value ;
            if (log != null)
                log.println("getValueByName('" + name + "') called -> '" +
                    value + "'") ;
            return value;
        }
    }

    /**
    * This class writes all XML data handled into a stream specified
    * in the constructor.
    */
    public static class XMLWriter implements XDocumentHandler {
        private PrintWriter _log = null ;
        private String align = "" ;

        /**
        * Creates a SAX handler which writes all XML data
        * handled into a <code>log</code> stream specified.
        */
        public XMLWriter(PrintWriter log) {
            _log = log ;
        }

        /**
        * Creates a SAX handler which does nothing.
        */
        public XMLWriter() {
        }

        public void processingInstruction(String appl, String data) {
            if (_log == null) return ;
            _log.println(align + "<?" + appl + " " + data + "?>") ;
        }
        public void startDocument() {
            if (_log == null) return ;
            _log.println("START DOCUMENT:") ;
        }
        public void endDocument() {
            if (_log == null) return ;
            _log.println("END DOCUMENT:") ;
        }
        public void setDocumentLocator(XLocator loc) {
            if (_log == null) return ;
            _log.println("DOCUMENT LOCATOR: ('" + loc.getPublicId() +
                "','" + loc.getSystemId() + "')") ;
        }
        public void startElement(String name, XAttributeList attr) {
            if (_log == null) return ;
            _log.print(align + "<" + name + " ") ;
            if (attr != null) {
                short attrLen = attr.getLength() ;
                for (short i = 0; i < attrLen; i++) {
                    if (i != 0) _log.print(align + "       ") ;
                    _log.print(attr.getNameByIndex(i) + "[" +
                        attr.getTypeByIndex(i) + "]=\"" +
                        attr.getValueByIndex(i) + "\"") ;
                    if (i+1 != attrLen) {
                        _log.println() ;
                    }
                }
            }
            _log.println(">") ;

            align += "   " ;
        }

        public void endElement(String name) {
            if (_log == null) return ;
            align = align.substring(3) ;
            _log.println(align + "</" + name + ">") ;
        }

        public void characters(String chars) {
            if (_log == null) return ;
            _log.println(align + chars) ;
        }
        public void ignorableWhitespace(String sp) {
            if (_log == null) return ;
            _log.println(sp) ;
        }
    }

    /**
    * Checks if the XML structure is well formed (i.e. all tags opened must be
    * closed and all tags opened inside a tag must be closed
    * inside the same tag). It also checks parameters passed.
    * If any collisions found appropriate error message is
    * output into a stream specified. No XML data output, i.e.
    * no output will be performed if no errors occur.<p>
    * After document is completed there is a way to cehck
    * if the XML data and structure was valid.
    */
    public static class XMLWellFormChecker extends XMLWriter {
        protected boolean docStarted = false ;
        protected boolean docEnded = false ;
        protected ArrayList<String> tagStack = new ArrayList<String>() ;
        protected boolean wellFormed = true ;
        protected boolean noOtherErrors = true ;
        protected PrintWriter log = null ;
        protected boolean printXMLData = false ;

        public XMLWellFormChecker(PrintWriter log) {
            super() ;
            this.log = log ;
        }

        public XMLWellFormChecker(PrintWriter log_, boolean printXMLData) {
            super(printXMLData ? log_ : null) ;
            this.printXMLData = printXMLData ;
            this.log = log_ ;
        }

        /**
         * Reset all values. This is important e.g. for test of XFilter
         * interface, where 'filter()' method istbstarted twice.
         */
        public void reset() {
            docStarted = false ;
            docEnded = false ;
            tagStack = new ArrayList<String>() ;
            wellFormed = true ;
            noOtherErrors = true ;
            printXMLData = false ;
        }

        public void startDocument() {
            super.startDocument();

            if (docStarted) {
                printError("Document is started twice.") ;
                wellFormed = false ;
            }

            docStarted = true ;
        }
        public void endDocument() {
            super.endDocument();
            if (!docStarted) {
                wellFormed = false ;
                printError("Document ended but not started.") ;
            }
            docEnded = true ;
        }
        public void startElement(String name, XAttributeList attr) {
            super.startElement(name, attr);
            if (attr == null) {
                printError("attribute list passed as parameter to startElement()"+
                    " method has null value for tag <" + name + ">") ;
                noOtherErrors = false ;
            }
            tagStack.add(0, name) ;
        }
        public void endElement(String name) {
            super.endElement(name);
            if (wellFormed) {
                if (tagStack.size() == 0) {
                    wellFormed = false ;
                    printError("No tags to close (bad closing tag </" + name + ">)") ;
                } else {
                    String startTag = tagStack.get(0) ;
                    tagStack.remove(0) ;
                    if (!startTag.equals(name)) {
                        wellFormed = false ;
                        printError("Bad closing tag: </" + name +
                            ">; tag expected: </" + startTag + ">");
                    }
                }
            }
        }

        /**
        * Checks if there were no errors during document handling.
        * I.e. startDocument() and endDocument() must be called,
        * XML must be well formed, parameters must be valid.
        */
        public boolean isWellFormed() {
            if (!docEnded) {
                printError("Document was not ended.") ;
                wellFormed = false ;
            }

            return wellFormed && noOtherErrors ;
        }

        /**
        * Prints error message and all tags where error occurred inside.
        * Also prints "Tag trace" in case if the full XML data isn't
        * printed.
        */
        public void printError(String msg) {
            log.println("!!! Error: " + msg) ;
            if (printXMLData) return ;
            log.println("   Tag trace :") ;
            for (int i = 0; i < tagStack.size(); i++) {
                String tag = tagStack.get(i) ;
                log.println("      <" + tag + ">") ;
            }
        }
    }

    /**
    * Beside structure of XML this class also can check existence
    * of tags, inner tags, and character data. After document
    * completion there is a way to check if required tags and
    * character data was found. If there any error occurs an
    * appropriate message is output.
    */
    public static class XMLTagsChecker extends XMLWellFormChecker {
        protected HashMap<String,String> tags = new HashMap<String,String>() ;
        protected HashMap<String,String> chars = new HashMap<String,String>() ;
        protected boolean allOK = true ;

        public XMLTagsChecker(PrintWriter log) {
            super(log) ;
        }

        /**
        * Adds a tag name which must be contained in the XML data.
        */
        public void addTag(String tag) {
            tags.put(tag, "") ;
        }
        /**
        * Adds a tag name which must be contained in the XML data and
        * must be inside the tag with name <code>outerTag</code>.
        */
        public void addTagEnclosed(String tag, String outerTag) {
            tags.put(tag, outerTag) ;
        }
        /**
        * Adds a character data which must be contained in the XML data.
        */
        public void addCharacters(String ch) {
            chars.put(ch, "") ;
        }
        /**
        * Adds a character data which must be contained in the XML data and
        * must be inside the tag with name <code>outerTag</code>.
        */
        public void addCharactersEnclosed(String ch, String outerTag) {
            chars.put(ch, outerTag) ;
        }

        public void startElement(String name, XAttributeList attrs) {
            super.startElement(name, attrs) ;
            if (tags.containsKey(name)) {
                String outerTag = tags.get(name);
                if (!outerTag.equals("")) {
                    boolean isInTag = false ;
                    for (int i = 0; i < tagStack.size(); i++) {
                        if (outerTag.equals(tagStack.get(i))) {
                            isInTag = true ;
                            break ;
                        }
                    }
                    if (!isInTag) {
                        printError("Required tag <" + name + "> found, but is not enclosed in tag <" +
                            outerTag + ">") ;
                        allOK = false ;
                    }
                }
                tags.remove(name) ;
            }
        }

        public void characters(String ch) {
            super.characters(ch) ;

            if (chars.containsKey(ch)) {
                String outerTag = chars.get(ch);
                if (!outerTag.equals("")) {
                    boolean isInTag = false ;
                    for (int i = 0; i < tagStack.size(); i++) {
                        if (outerTag.equals(tagStack.get(i))) {
                            isInTag = true ;
                            break ;
                        }
                    }
                    if (!isInTag) {
                        printError("Required characters '" + ch + "' found, but are not enclosed in tag <" +
                            outerTag + ">") ;
                        allOK = false ;
                    }
                }
                chars.remove(ch) ;
            }
        }

        /**
        * Checks if the XML data was valid and well formed and if
        * all necessary tags and character data was found.
        */
        public boolean checkTags() {
            allOK &= isWellFormed() ;

            Iterator<String> badTags = tags.keySet().iterator() ;
            Iterator<String> badChars = chars.keySet().iterator() ;

            if (badTags.hasNext()) {
                allOK = false ;
                log.println("Required tags were not found in export :") ;
                while(badTags.hasNext()) {
                    log.println("   <" + badTags.next() + ">") ;
                }
            }
            if (badChars.hasNext()) {
                allOK = false ;
                log.println("Required characters were not found in export :") ;
                while(badChars.hasNext()) {
                    log.println("   <" + badChars.next() + ">") ;
                }
            }
            reset();
            return allOK ;
        }
    }

    /**
     * Represents an XML tag which must be found in XML data written.
     * This tag can contain only its name or tag name and attribute
     * name, or attribute value additionally.
     */
    public static class Tag {
        private String name = null;
        private String[][] attrList = new String[0][3] ;

        /**
         * Creates tag which has only a name. Attributes don't make sense.
         * @param tagName The name of the tag.
         */
        public Tag(String tagName) {
            name = tagName ;
        }

        /**
         * Creates a tag with the name specified, which must have an
         * attribute with name specified. The value of this attribute
         * doesn't make sense.
         * @param tagName The name of the tag.
         * @param attrName The name of attribute which must be contained
         * in the tag.
         */
        public Tag(String tagName, String attrName) {
            name = tagName ;
            attrList = new String[1][3] ;
            attrList[0][0] = attrName ;
        }

        /**
         * Creates a tag with the name specified, which must have an
         * attribute with the value specified. The type of value
         * assumed to be 'CDATA'.
         * @param tagName The name of the tag.
         * @param attrName The name of attribute which must be contained
         * in the tag.
         * @param attrValue Attribute value.
         */
        public Tag(String tagName, String attrName, String attrValue) {
            name = tagName ;
            attrList = new String[1][3] ;
            attrList[0][0] = attrName ;
            attrList[0][1] = "CDATA" ;
            attrList[0][2] = attrValue ;
        }

        /**
         * Creates a tag with the name specified, which must have
         * attributes specified. The value of thesee attributes
         * doesn't make sense.
         * @param tagName The name of the tag.
         * @param attrNames Array with names of attributes which must
         * be contained in the tag.
         */
        public Tag(String tagName, String[] attrNames) {
            name = tagName ;
            attrList = new String[attrNames.length][3] ;
            for (int i = 0; i < attrNames.length; i++) {
                attrList[i][0] = attrNames[i] ;
            }
        }

        /**
         * Creates a tag with the name specified, which must have an
         * attributes with their values specified. The type of all values
         * assumed to be 'CDATA'.
         * @param tagName The name of the tag.
         * @param attrValues An array with attribute names and their values.
         * <code>attrValues[N][0]</code> element contains the name of Nth
         * attribute, and <code>attrValues[N][1]</code> element contains
         * value of Nth attribute, if value is <code>null</code> then the
         * attribute value can be any.
         */
        public Tag(String tagName, String[][] attrValues) {
            name = tagName ;
            attrList = new String[attrValues.length][3] ;
            for (int i = 0; i < attrValues.length; i++) {
                attrList[i][0] = attrValues[i][0] ;
                attrList[i][1] = "CDATA" ;
                attrList[i][2] = attrValues[i][1] ;
            }
        }

        /**
         * Gets tag String description.
         */
        public String toString() {
            String ret = "<" + name ;
            for (int i = 0; i < attrList.length; i++) {
                ret += " " + attrList[i][0] + "=";
                if (attrList[i][2] == null) {
                    ret += "(not specified)";
                } else {
                    ret += "\"" + attrList[i][2] + "\"";
                }
            }
            ret += ">";

            return ret ;
        }

        protected boolean checkAttr(int attrListIdx, XAttributeList list) {
            short j  = 0 ;
            int listLen = list.getLength();
            while(j < listLen) {
                if (attrList[attrListIdx][0].equals(list.getNameByIndex(j))) {
                    if (attrList[attrListIdx][2] == null) return true ;
                    return attrList[attrListIdx][2].equals(list.getValueByIndex(j)) ;
                }
                j++ ;
            }
            return false ;
        }

        /**
         * Checks if this tag matches tag passed in parameters.
         * I.e. if tag specifies only it's name it mathes if names
         * are equal (attributes don't make sense). If there are
         * some attributes names specified in this tag method checks
         * if all names present in attribute list <code>list</code>
         * (attributes' values don't make sense). If attributes specified
         * with values method checks if these attributes exist and
         * have appropriate values.
         */
        public boolean isMatchTo(String tagName, XAttributeList list) {
            if (!name.equals(tagName)) return false;
            boolean result = true ;
            for (int i = 0; i < attrList.length; i++) {
                result &= checkAttr(i, list) ;
            }
            return result ;
        }
    }

    /**
     * Class realises extended XML data checking. It has possibilities
     * to check if a tag exists, if it has some attributes with
     * values, and if this tag is contained in another tag (which
     * also can specify any attributes). It can check if some
     * character data exists inside any tag specified.
     */
    public static class XMLChecker extends XMLWellFormChecker {
        protected HashSet<String> tagSet = new HashSet<String>() ;
        protected ArrayList<Tag[]> tags = new ArrayList<Tag[]>() ;
        protected ArrayList<Object[]> chars = new ArrayList<Object[]>() ;
        protected ArrayList<String> tagStack = new ArrayList<String>() ;
        protected ArrayList<AttributeList> attrStack = new ArrayList<AttributeList>() ;

        public XMLChecker(PrintWriter log, boolean writeXML) {
            super(log, writeXML) ;
        }

        public void addTag(Tag tag) {
            tags.add(new Tag[] {tag, null}) ;
            tagSet.add(tag.name) ;
        }

        public void addTagEnclosed(Tag tag, Tag outerTag) {
            tags.add(new Tag[] {tag, outerTag}) ;
            tagSet.add(tag.name) ;
        }

        public void addCharacters(String ch) {
            chars.add(new Object[] {ch.trim(), null}) ;
        }

        public void addCharactersEnclosed(String ch, Tag outerTag) {
            chars.add(new Object[] {ch.trim(), outerTag}) ;
        }

        public void startElement(String name, XAttributeList attr) {
            try {
            super.startElement(name, attr);

            if (tagSet.contains(name)) {
                for (int i = 0; i < tags.size(); i++) {
                    Tag[] tag = tags.get(i);
                    if (tag[0].isMatchTo(name, attr)) {
                        if (tag[1] == null) {
                            tags.remove(i--);
                        } else {
                            boolean isInStack = false ;
                            for (int j = 0; j < tagStack.size(); j++) {
                                if (tag[1].isMatchTo(tagStack.get(j),
                                    attrStack.get(j))) {

                                    isInStack = true ;
                                    break ;
                                }
                            }
                            if (isInStack) {
                                tags.remove(i--) ;
                            }
                        }
                    }
                }
            }

            tagStack.add(0, name) ;
            attrStack.add(0, new AttributeList(attr));
            } catch (Exception e) {
                e.printStackTrace(log);
            }
        }

        public void characters(String ch) {
            super.characters(ch) ;
            for (int i = 0; i < chars.size(); i++) {
                Object[] chr = chars.get(i);
                if (((String) chr[0]).equals(ch)) {
                    if (chr[1] == null) {
                        chars.remove(i--);
                    } else {
                        boolean isInStack = false ;
                        for (int j = 0; j < tagStack.size(); j++) {
                            if (((Tag) chr[1]).isMatchTo(tagStack.get(j),
                                attrStack.get(j))) {

                                isInStack = true ;
                                break ;
                            }
                        }
                        if (isInStack) {
                            chars.remove(i--) ;
                        }
                    }
                }
            }
        }

        public void endElement(String name) {
            try {
            super.endElement(name);

            if (tagStack.size() > 0) {
                tagStack.remove(0) ;
                attrStack.remove(0) ;
            }
            } catch(Exception e) {
                e.printStackTrace(log) ;
            }
        }

        public boolean check() {
            if (tags.size()> 0) {
                log.println("!!! Error: Some tags were not found :") ;
                for (int i = 0; i < tags.size(); i++) {
                    Tag[] tag = tags.get(i) ;
                    log.println("   Tag " + tag[0] + " was not found");
                    if (tag[1] != null)
                        log.println("      inside tag " + tag[1]) ;
                }
            }
            if (chars.size() > 0) {
                log.println("!!! Error: Some character data blocks were not found :") ;
                for (int i = 0; i < chars.size(); i++) {
                    Object[] ch = chars.get(i) ;
                    log.println("   Character data \"" + ch[0] + "\" was not found ") ;
                    if (ch[1] != null)
                        log.println("      inside tag " + ch[1]) ;
                }
            }

            if (!isWellFormed())
                log.println("!!! Some errors were found in XML structure") ;

            boolean result = tags.size() == 0 && chars.size() == 0 && isWellFormed();
            reset();
            return result;
        }
    }

    /**
    * Creates <code>XDocumentHandler</code> implementation in form
    * of <code>com.sun.star.xml.sax.Writer</code> service, which
    * writes XML data into a <code>com.sun.star.io.Pipe</code>
    * created.
    * @return Single element array which contains the handler
    * contained in <code>Any</code> structure.
    */
    public static Object[] getDocumentHandler(XMultiServiceFactory xMSF) {
        Object[] ret = new Object[1];
        try {
            XInterface Writer = (XInterface) xMSF.createInstance(
                                    "com.sun.star.xml.sax.Writer");
            XInterface oPipe = (XInterface) xMSF.createInstance
                ( "com.sun.star.io.Pipe" );
            XOutputStream xPipeOutput = UnoRuntime.
                queryInterface(XOutputStream.class, oPipe) ;

            XActiveDataSource xADS = UnoRuntime.queryInterface(XActiveDataSource.class,Writer);
            xADS.setOutputStream(xPipeOutput);
            XDocumentHandler handler = UnoRuntime.queryInterface(XDocumentHandler.class,Writer);

            Any arg = new Any(new Type(XDocumentHandler.class),handler);

            ret[0] = arg;
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace();
        }
        return ret;
    }

    public static PropertyValue[] createMediaDescriptor(String[] propNames, Object[] values) {
        PropertyValue[] props = new PropertyValue[propNames.length] ;

        for (int i = 0; i < props.length; i++) {
            props[i] = new PropertyValue() ;
            props[i].Name = propNames[i] ;
            if (values != null && i < values.length) {
                props[i].Value = values[i] ;
            }
        }

        return props ;
    }

    /**
     * Gets the hanlder, which writes all the XML data passed to the
     * file specified.
     * @param xMSF Soffice <code>ServiceManager</code> factory.
     * @param fileURL The file URL (in form file:///<path>) to which
     * XML data is written.
     * @return SAX handler to which XML data has to be written.
     */
    public static XDocumentHandler getFileXMLWriter(XMultiServiceFactory xMSF, String fileURL)
        throws com.sun.star.uno.Exception
    {
        XInterface oFacc = (XInterface)xMSF.createInstance(
            "com.sun.star.comp.ucb.SimpleFileAccess");
        XSimpleFileAccess xFacc = UnoRuntime.queryInterface
            (XSimpleFileAccess.class, oFacc) ;

        XInterface oWriter = (XInterface)xMSF.createInstance(
            "com.sun.star.xml.sax.Writer");
        XActiveDataSource xWriterDS = UnoRuntime.queryInterface(XActiveDataSource.class, oWriter);
        XDocumentHandler xDocHandWriter = UnoRuntime.queryInterface
            (XDocumentHandler.class, oWriter) ;

        if (xFacc.exists(fileURL))
            xFacc.kill(fileURL);
        XOutputStream fOut = xFacc.openFileWrite(fileURL) ;
        xWriterDS.setOutputStream(fOut);

        return xDocHandWriter ;
    }

    /**
     * Parses XML file and passes its data to the SAX handler specified.
     * @param xMSF Soffice <code>ServiceManager</code> factory.
     * @param fileURL XML file name (in form file:///<path>) to be parsed.
     * @param handler SAX handler to which XML data from file will
     * be transferred.
     */
    public static void parseXMLFile(XMultiServiceFactory xMSF,
        String fileURL, XDocumentHandler handler) throws com.sun.star.uno.Exception
    {
        XInterface oFacc = (XInterface)xMSF.createInstance(
            "com.sun.star.comp.ucb.SimpleFileAccess");
        XSimpleFileAccess xFacc = UnoRuntime.queryInterface
            (XSimpleFileAccess.class, oFacc) ;
        XInputStream oIn = xFacc.openFileRead(fileURL) ;

        XInterface oParser = (XInterface)xMSF.createInstance(
            "com.sun.star.xml.sax.Parser");
        XParser xParser = UnoRuntime.queryInterface(XParser.class, oParser);

        xParser.setDocumentHandler(handler) ;
        InputSource inSrc = new InputSource() ;
        inSrc.aInputStream = oIn ;
        xParser.parseStream(inSrc) ;

        oIn.closeInput();
    }

    /**
     * Exports document (the whole or a part) into the file specified
     * in XML format.
     * @param xMSF Soffice <code>ServiceManager</code> factory.
     * @param xDoc Document to be exported.
     * @param docType Type of document (for example 'Calc', 'Writer', 'Draw')
     * The type must start with <b>capital</b> letter.
     * @param exportType The type of export specifies if the whole
     * document will be exported or one of its parts (Meta info, Styles, etc.).
     * The following types supported (it also depends of document type) :
     *  "" (empty string) - for the whole document ;
     *  "Content" - only content exported ;
     *  "Meta" - meta document info exported ;
     *  "Settings" - view settings of document exported ;
     *  "Styles" - document styles exported ;
     * @param fileURL XML file name (in form file:///<path>) to be exported to.
     */
    public static void exportDocument(XMultiServiceFactory xMSF, XComponent xDoc,
        String docType, String exportType, String fileURL)
        throws com.sun.star.uno.Exception {

        XDocumentHandler xDocHandWriter = XMLTools.getFileXMLWriter(xMSF, fileURL) ;

        Any arg = new Any(new Type(XDocumentHandler.class), xDocHandWriter);
        XInterface oExp = (XInterface)xMSF.createInstanceWithArguments(
            "com.sun.star.comp." + docType + ".XML" + exportType + "Exporter",
            new Object[] {arg});

        XExporter xExp = UnoRuntime.queryInterface
            (XExporter.class, oExp) ;
        xExp.setSourceDocument(xDoc) ;

        XFilter filter = UnoRuntime.queryInterface(XFilter.class, oExp) ;
        filter.filter(XMLTools.createMediaDescriptor(
            new String[] {"FilterName"},
            new Object[] {"Custom filter"})) ;
    }

    /**
     * Imports document (the whole or a part) from the file specified
     * in XML format.
     * @param xMSF Soffice <code>ServiceManager</code> factory.
     * @param xDoc Target document to be imported.
     * @param docType Type of document (for example 'Calc', 'Writer', 'Draw')
     * The type must start with <b>capital</b> letter.
     * @param importType The type of export specifies if the whole
     * document will be exported or one of its parts (Meta info, Styles, etc.).
     * The following types supported (it hardly depends of XML data in file) :
     *  "" (empty string) - for the whole document ;
     *  "Content" - only content exported ;
     *  "Meta" - meta document info exported ;
     *  "Settings" - view settings of document exported ;
     *  "Styles" - document styles exported ;
     * @param fileURL XML file name (in form file:///<path>) to be imported from.
     */
    public static void importDocument(XMultiServiceFactory xMSF, XComponent xDoc,
        String docType, String importType, String fileURL)
        throws com.sun.star.uno.Exception {

        XInterface oImp = (XInterface)xMSF.createInstance(
            "com.sun.star.comp." + docType + ".XML" + importType + "Importer");
        XImporter xImp = UnoRuntime.queryInterface
            (XImporter.class, oImp) ;
        XDocumentHandler xDocHandImp = UnoRuntime.queryInterface
            (XDocumentHandler.class, oImp) ;

        xImp.setTargetDocument(xDoc) ;
        parseXMLFile(xMSF, fileURL, xDocHandImp) ;
    }
}
