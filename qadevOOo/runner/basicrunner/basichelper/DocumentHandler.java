/*************************************************************************
 *
 *  $RCSfile: DocumentHandler.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-11-18 16:13:22 $
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
package basicrunner.basichelper;

import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.uno.Type;
import com.sun.star.uno.Any;
import com.sun.star.xml.sax.XDocumentHandler;
import com.sun.star.xml.sax.XLocator;
import com.sun.star.xml.sax.XAttributeList;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.NoSuchElementException;
import java.util.Vector;
import util.XMLTools.Tag;
import util.XMLTools;
import java.io.StringWriter;
import java.io.PrintWriter;

/**
 * This class provides a handler of the BASIC test document.
 * @see com.sun.star.lang.XSingleServiceFactory
 * @see com.sun.star.lang.XServiceInfo
 */
public class DocumentHandler implements XServiceInfo, XSingleServiceFactory {
    /** The service name of this class **/
    static final String __serviceName = "basichelper.DocumentHandler";
    /** The actual handler of the document **/
    static DocumentHandlerImpl oDocumentHandler = null;
    /** A string writer **/
    private StringWriter writer;
    /** The log writer (just a wrapper around <code>writer</code>) **/
    private PrintWriter log;

    /**
     * Create an instance of the document handler.
     * @param args A boolean value as <codde>args[0]</code> determines,
     *             if checked XML data is printed to the log.
     *             Default is false.
     * @return The document handler
     */
    public Object createInstanceWithArguments(Object[] args) {
        boolean printXML = false;
        if (args != null && args.length!=0 && args[0] instanceof Boolean)
            printXML = ((Boolean)args[0]).booleanValue();
        writer = new StringWriter();
        log = new PrintWriter(writer);
        oDocumentHandler = new DocumentHandlerImpl(log, printXML, writer);
        return oDocumentHandler;
    }

    /**
     * Create an instance of the document handler.
     * @return The document handler
     */
    public Object createInstance() {
        return createInstanceWithArguments(null);
    }

    /** Get the unique id for this implementation
     * @return The id.
     */
    public byte[] getImplementationId() {
        return toString().getBytes();
    }

    /** Get all implemented types.
     * @return The implemented UNO types.
     */
    public Type[] getTypes() {
        Class interfaces[] = getClass().getInterfaces();
        Type types[] = new Type[interfaces.length];
        for(int i = 0; i < interfaces.length; ++ i)
            types[i] = new Type(interfaces[i]);
        return types;
    }

    /** Is this servioce supported?
     * @param name The service name.
     * @return True, if the service is supported.
     */
    public boolean supportsService(String name) {
        return __serviceName.equals(name);
    }

    /**
     * Get all supported service names.
     * @return All supported servcices.
     */
    public String[] getSupportedServiceNames() {
        return new String[] {__serviceName};
    }

    /**
     * Get the implementation name of this class.
     * @return The implementation name.
     */
    public String getImplementationName() {
        return getClass().getName();
    }
}

/**
 * The actual implementation of the document handler
 * @see util.XMLTools.XMLChecker
 * @see com.sun.star.lang.XInitialization
 * @see com.sun.star.xml.sax.XDocumentHandler
 * @see com.sun.star.container.XNameAccess
 * @see com.sun.star.lang.XTypeProvider
 */
class DocumentHandlerImpl extends XMLTools.XMLChecker
                        implements XInitialization, XDocumentHandler,
                                                XNameAccess, XTypeProvider {
    /** A string writer **/
    private StringWriter writer;

    /**
     * Constructor
     * @param log_ A log writer.
     * @param printXML Should XML data be printed to the log?
     * @param logWriter A wrapper around <code>log_</code> for convenience.
     */
    public DocumentHandlerImpl(PrintWriter log_,
                                    boolean printXML, StringWriter logWriter) {
        super(log_, printXML);
        writer = logWriter;
    }

    /**
     * Initialize this class with rules.
     * @param parm1 An array of filter rules:
     *              <code>processAction()</code> is called for every rule.
     * @throws com.sun.star.uno.Exception for an incorrect rule.
     */
    public void initialize(Object[] parm1) throws com.sun.star.uno.Exception {
        if (!(parm1[0] instanceof Object[])) return;
        for (int i=0; i<parm1.length; i++) {
            processActionForXMLChecker((Object[])parm1[i]);
        }
    }

    /**
    * Method processes all filters received from basic tests.
    * Called by initialize().
    * @param filterRule An array building one filter rule.
    * @throws com.sun.star.uno.Exception for an incorrect rule.
    */
    private void processActionForXMLChecker(Object[] filterRule)
                                        throws com.sun.star.uno.Exception {
        int arrLen = filterRule.length;
        String oTagName;
        Object[] oTags;
        Object[] oTag;
        int tagsNum = arrLen-1;
        Vector allTags = new Vector();
        String CDATA = "";
        String action = "";

        // First element of rule is RuleName and should be String
        if (!(filterRule[0] instanceof String)) {
            throw new com.sun.star.uno.Exception("Error: incorrect filter rule "+
            "received from basic test! Rule name must be a String.");
        } else {
            action = (String) filterRule[0];
        }

        // Searching for character data and defining amount of tags received.
        for (int j=1; j<arrLen; j++) {
            if ( (filterRule[j] instanceof String) && (j != 1) ) {
                CDATA = (String) filterRule[j];
                tagsNum--;
            }
        }

        // Adding received tags to internal array.
        oTags = new Object[tagsNum];
        for (int j=1; j<=tagsNum; j++) {
            if (filterRule[j] instanceof Object[]) {
                oTags[j-1] = (Object[]) filterRule[j];
            }
        }

        // Process all received tags for a given filter rule
        for (int i=0; i<oTags.length; i++) {
            if (oTags[i] instanceof Object[]) {
                oTag = (Object[]) oTags[i];
                oTagName = (String) oTag[0];
            } else if (oTags[i] instanceof Object) {
                oTag = new Object[1];
                oTag[0] = (Object) oTags[i];
                oTagName = (String) oTag[0];
            } else {
                throw new com.sun.star.uno.Exception("Error: invalid tag "+
                                    "received from basic test! Check tag "
                                    +i+" in rule '"+action+"'.");
            }

            // Action for constructor Tag(TagName, attrName, attrValue)
            if (oTag.length == 3) {
                if ((oTag[1] instanceof String)&&(oTag[2] instanceof String)) {
                    allTags.add(new Tag(oTagName,
                                    (String) oTag[1], (String) oTag[2]));
                } else {
                    throw new com.sun.star.uno.Exception("Error: invalid tag '"+
                                    oTagName+"' received from basic test!");
                }

            // Action for constructors:
            // Tag(TagName, String[][] attrValues )
            // Tag(TagName, String[] attrNames)
            // Tag(TagName, String attrName)
            //
            } else if (oTag.length == 2) {
                if (oTag[1] instanceof String[][]) {
                    allTags.add(new Tag(oTagName, (String[][]) oTag[1]));
                } else if (oTag[1] instanceof String[]) {
                    allTags.add(new Tag(oTagName, (String[]) oTag[1]));
                } else if (oTag[1] instanceof String) {
                    allTags.add(new Tag(oTagName, (String) oTag[1]));
                } else {
                    throw new com.sun.star.uno.Exception("Error: invalid tag '"+
                    oTagName+"' received from basic test!");
                }

            // Action for constructor Tag(TagName)
            } else if (oTag.length == 1) {
                if (oTag[0] instanceof String) {
                    allTags.add(new Tag(oTagName));
                } else {
                    throw new com.sun.star.uno.Exception("Error: invalid tag '"+
                    oTagName+"' received from basic test!");
                }
            } else {
                throw new com.sun.star.uno.Exception("Error: invalid tag '"+
                oTagName+"' received from basic test!");
            }
        }

        // Adding tags to XMLChecker
        if ( action.equals((String)"TagExists") ) {
            for (int i=0; i<allTags.size(); i++) {
                addTag((Tag)allTags.get(i));
            }
        } else if (action.equals((String)"TagEnclosed")) {
            addTagEnclosed((Tag) allTags.get(0), (Tag) allTags.get(1));
        } else if (action.equals((String)"CharsEnclosed")) {
            addCharactersEnclosed(CDATA, (Tag) allTags.get(0));
        } else {
            throw new com.sun.star.uno.Exception("Error: incorrect rule name '"+
            action+"' received from basic test!");
        }
    }

    /**
     * Get the names of the elements.
     * @return element names.
     */
    public String[] getElementNames() {
        return new String[]{"XMLCode", "XMLIsCorrect"};
    }

    /**
     * Is this an element?
     * @param name Element name.
     * @return true, if <code>name>/code> is the name of an element.
     */
    public boolean hasByName(String name) {
        return (name.equals("XMLCode") || name.equals("XMLIsCorrect"));
    }

    /**
     * Get an element by its name.
     * @param name The element name.
     * @return The element with the specified <code>name</code>.
     * @throws NoSuchElementException Is thrown, if name does not exist.
     */
    public Object getByName(String name) throws NoSuchElementException{
        if (name.equals("XMLIsCorrect"))
            return Boolean.valueOf(this.check());
        else if (name.equals("XMLCode")) {
            return writer.getBuffer().toString();
        } else
            throw new NoSuchElementException();
    }

    /**
     * Are there any elements?
     * @return Always true.
     */
    public boolean hasElements() {
        return true;
    }

    /**
     * Get the element type.
     * @return The type.
     */
    public Type getElementType() {
        return new Type(Object.class);
    }

    /**
     * Get a unique id for this implementation.
     * @return The id.
     */
    public byte[] getImplementationId() {
        return toString().getBytes();
    }

    /**
     * Return all implemented types of this class.
     * @return The implemented UNO types.
     */
    public Type[] getTypes() {
        Class interfaces[] = getClass().getInterfaces();
        Type types[] = new Type[interfaces.length];
        for(int i = 0; i < interfaces.length; ++ i)
            types[i] = new Type(interfaces[i]);
        return types;
    }
}
