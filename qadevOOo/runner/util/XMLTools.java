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
import java.util.Iterator;

import com.sun.star.beans.PropertyValue;
import com.sun.star.xml.sax.XAttributeList;
import com.sun.star.xml.sax.XDocumentHandler;
import com.sun.star.xml.sax.XLocator;


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
        private final HashMap<String, Attribute> attrByName = new HashMap<String, Attribute>() ;
        private final ArrayList<Attribute> attributes = new ArrayList<Attribute>() ;

        /**
         * Creates a class instance.
         */
        public AttributeList() {}

        private AttributeList(XAttributeList list) {
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





        /***************************************
        * XAttributeList methods
        ****************************************/

        public short getLength() {
            return (short) attributes.size() ;
        }

        public String getNameByIndex(short idx) {
            String name = attributes.get(idx).Name ;
            return name ;
        }

        public String getTypeByIndex(short idx) {
            String type = attributes.get(idx).Type  ;
            return type;
        }

        public String getTypeByName(String name) {
            String type = attrByName.get(name).Type ;
            return type;
        }
        public String getValueByIndex(short idx) {
            String value = attributes.get(idx).Value ;
            return value;
        }

        public String getValueByName(String name) {
            String value = attrByName.get(name).Value ;
            return value;
        }
    }

    /**
    * This class writes all XML data handled into a stream specified
    * in the constructor.
    */
    private static class XMLWriter implements XDocumentHandler {
        private PrintWriter _log = null ;
        private String align = "" ;

        /**
        * Creates a SAX handler which writes all XML data
        * handled into a <code>log</code> stream specified.
        */
        private XMLWriter(PrintWriter log) {
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
    * After document is completed there is a way to check
    * if the XML data and structure was valid.
    */
    private static class XMLWellFormChecker extends XMLWriter {
        private boolean docStarted = false ;
        private boolean docEnded = false ;
        ArrayList<String> tagStack = new ArrayList<String>() ;
        private boolean wellFormed = true ;
        private boolean noOtherErrors = true ;
        PrintWriter log = null ;
        private boolean printXMLData = false ;

        private XMLWellFormChecker(PrintWriter log) {
            super() ;
            this.log = log ;
        }

        private XMLWellFormChecker(PrintWriter log_, boolean printXMLData) {
            super(printXMLData ? log_ : null) ;
            this.printXMLData = printXMLData ;
            this.log = log_ ;
        }

        /**
         * Reset all values. This is important e.g. for test of XFilter
         * interface, where 'filter()' method istbstarted twice.
         */
        void reset() {
            docStarted = false ;
            docEnded = false ;
            tagStack = new ArrayList<String>() ;
            wellFormed = true ;
            noOtherErrors = true ;
            printXMLData = false ;
        }

        @Override
        public void startDocument() {
            super.startDocument();

            if (docStarted) {
                printError("Document is started twice.") ;
                wellFormed = false ;
            }

            docStarted = true ;
        }
        @Override
        public void endDocument() {
            super.endDocument();
            if (!docStarted) {
                wellFormed = false ;
                printError("Document ended but not started.") ;
            }
            docEnded = true ;
        }
        @Override
        public void startElement(String name, XAttributeList attr) {
            super.startElement(name, attr);
            if (attr == null) {
                printError("attribute list passed as parameter to startElement()"+
                    " method has null value for tag <" + name + ">") ;
                noOtherErrors = false ;
            }
            tagStack.add(0, name) ;
        }
        @Override
        public void endElement(String name) {
            super.endElement(name);
            if (wellFormed) {
                if (tagStack.isEmpty()) {
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
        void printError(String msg) {
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
        private final HashMap<String,String> tags = new HashMap<String,String>() ;
        private final HashMap<String,String> chars = new HashMap<String,String>() ;
        private boolean allOK = true ;

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
        * Adds a character data which must be contained in the XML data and
        * must be inside the tag with name <code>outerTag</code>.
        */
        public void addCharactersEnclosed(String ch, String outerTag) {
            chars.put(ch, outerTag) ;
        }

        @Override
        public void startElement(String name, XAttributeList attrs) {
            super.startElement(name, attrs) ;
            if (tags.containsKey(name)) {
                String outerTag = tags.get(name);
                if (outerTag.length() != 0) {
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

        @Override
        public void characters(String ch) {
            super.characters(ch) ;

            if (chars.containsKey(ch)) {
                String outerTag = chars.get(ch);
                if (outerTag.length() != 0) {
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
            if (!isWellFormed())
                allOK = false ;

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
        private final String name;
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
         * Gets tag String description.
         */
        @Override
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

        private boolean checkAttr(int attrListIdx, XAttributeList list) {
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
         * I.e. if tag specifies only its name it matches if names
         * are equal (attributes don't make sense). If there are
         * some attributes names specified in this tag method checks
         * if all names present in attribute list <code>list</code>
         * (attributes' values don't make sense). If attributes specified
         * with values method checks if these attributes exist and
         * have appropriate values.
         */
        private boolean isMatchTo(String tagName, XAttributeList list) {
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
        private final HashSet<String> tagSet = new HashSet<String>() ;
        private final ArrayList<Tag[]> tags = new ArrayList<Tag[]>() ;
        private final ArrayList<Object[]> chars = new ArrayList<Object[]>() ;
        private final ArrayList<String> tagStack = new ArrayList<String>() ;
        private final ArrayList<AttributeList> attrStack = new ArrayList<AttributeList>() ;

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



        public void addCharactersEnclosed(String ch, Tag outerTag) {
            chars.add(new Object[] {ch.trim(), outerTag}) ;
        }

        @Override
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

        @Override
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

        @Override
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

            boolean result = tags.isEmpty() && chars.isEmpty() && isWellFormed();
            reset();
            return result;
        }
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




}
