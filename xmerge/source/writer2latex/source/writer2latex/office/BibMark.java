/************************************************************************
 *
 *  BibMark.java
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
 *  Copyright: 2002-2003 by Henrik Just
 *
 *  All Rights Reserved.
 *
 *  Version 0.3 (2003-07-30)
 *
 */

package writer2latex.office;

import org.w3c.dom.Node;

import writer2latex.util.*;
import writer2latex.office.*;

/**
 *  <p>This class represents a single bibliography-mark.</p>
 */
public final class BibMark {
    // Available fields
    public static final int ADDRESS = 0;
    public static final int ANNOTE = 1;
    public static final int AUTHOR = 2;
    public static final int BOOKTITLE = 3;
    public static final int CHAPTER = 4;
    // public static final int CROSSREF = 5; // BibTeX, missing in OOo
    public static final int EDITION = 6;
    public static final int EDITOR = 7;
    public static final int HOWPUBLISHED = 8;
    public static final int INSTITUTION = 9;
    public static final int JOURNAL = 10;
    // public static final int KEY = 11; // BibTeX, missing in OOo
    public static final int MONTH = 12;
    public static final int NOTE = 13;
    public static final int NUMBER = 14;
    public static final int ORGANIZATIONS = 15; // BibTeX: organization
    public static final int PAGES = 16;
    public static final int PUBLISHER = 17;
    public static final int SCHOOL = 18;
    public static final int SERIES = 19;
    public static final int TITLE = 20 ;
    public static final int REPORT_TYPE = 21;  // BibTeX: report
    public static final int VOLUME = 22;
    public static final int YEAR = 23;
    // remaining fields are not standard in BibTeX
    public static final int URL = 24;
    public static final int CUSTOM1 = 25;
    public static final int CUSTOM2 = 26;
    public static final int CUSTOM3 = 27;
    public static final int CUSTOM4 = 28;
    public static final int CUSTOM5 = 29;
    public static final int ISBN = 30;
    public static final int FIELD_COUNT = 31;


    // Private data
    private String sIdentifier;
    private String sEntryType;
    private String[] fields = new String[FIELD_COUNT];

    /**
     *  <p>Create a new BibMark from scratch.</p>
     */
    public BibMark(String sIdentifier, String sEntryType) {
        this.sIdentifier = sIdentifier;
        this.sEntryType = sEntryType;
    }

    /**
     *  <p>Create a new BibMark from a text:bibliography-mark node.</p>
     */
    public BibMark(Node node) {
        sIdentifier = Misc.getAttribute(node,XMLString.TEXT_IDENTIFIER);
        sEntryType = Misc.getAttribute(node,XMLString.TEXT_BIBLIOGRAPHY_TYPE);
        if (sEntryType==null) { // bug in OOo 1.0!
            sEntryType = Misc.getAttribute(node,XMLString.TEXT_BIBILIOGRAPHIC_TYPE);
        }
        fields[ADDRESS] = Misc.getAttribute(node,XMLString.TEXT_ADDRESS);
        fields[ANNOTE] = Misc.getAttribute(node,XMLString.TEXT_ANNOTE);
        fields[AUTHOR] = Misc.getAttribute(node,XMLString.TEXT_AUTHOR);
        fields[BOOKTITLE] = Misc.getAttribute(node,XMLString.TEXT_BOOKTITLE);
        fields[CHAPTER] = Misc.getAttribute(node,XMLString.TEXT_CHAPTER);
        fields[EDITION] = Misc.getAttribute(node,XMLString.TEXT_EDITION);
        fields[EDITOR] = Misc.getAttribute(node,XMLString.TEXT_EDITOR);
        fields[HOWPUBLISHED] = Misc.getAttribute(node,XMLString.TEXT_HOWPUBLISHED);
        fields[INSTITUTION] = Misc.getAttribute(node,XMLString.TEXT_INSTITUTION);
        fields[JOURNAL] = Misc.getAttribute(node,XMLString.TEXT_JOURNAL);
        fields[MONTH] = Misc.getAttribute(node,XMLString.TEXT_MONTH);
        fields[NOTE] = Misc.getAttribute(node,XMLString.TEXT_NOTE);
        fields[NUMBER] = Misc.getAttribute(node,XMLString.TEXT_NUMBER);
        fields[ORGANIZATIONS] = Misc.getAttribute(node,XMLString.TEXT_ORGANIZATIONS);
        fields[PAGES] = Misc.getAttribute(node,XMLString.TEXT_PAGES);
        fields[PUBLISHER] = Misc.getAttribute(node,XMLString.TEXT_PUBLISHER);
        fields[SCHOOL] = Misc.getAttribute(node,XMLString.TEXT_SCHOOL);
        fields[SERIES] = Misc.getAttribute(node,XMLString.TEXT_SERIES);
        fields[TITLE] = Misc.getAttribute(node,XMLString.TEXT_TITLE);
        fields[REPORT_TYPE] = Misc.getAttribute(node,XMLString.TEXT_REPORT_TYPE);
        fields[VOLUME] = Misc.getAttribute(node,XMLString.TEXT_VOLUME);
        fields[YEAR] = Misc.getAttribute(node,XMLString.TEXT_YEAR);
        fields[URL] = Misc.getAttribute(node,XMLString.TEXT_URL);
        fields[CUSTOM1] = Misc.getAttribute(node,XMLString.TEXT_CUSTOM1);
        fields[CUSTOM2] = Misc.getAttribute(node,XMLString.TEXT_CUSTOM2);
        fields[CUSTOM3] = Misc.getAttribute(node,XMLString.TEXT_CUSTOM3);
        fields[CUSTOM4] = Misc.getAttribute(node,XMLString.TEXT_CUSTOM4);
        fields[CUSTOM5] = Misc.getAttribute(node,XMLString.TEXT_CUSTOM5);
        fields[ISBN] = Misc.getAttribute(node,XMLString.TEXT_ISBN);
    }

    /**
     *  <p>Get the identifier.</p>
     */
    public String getIdentifier() { return sIdentifier; }

    /**
     *  <p>Get the entry type.</p>
     */
    public String getEntryType() { return sEntryType; }

    /**
     *  <p>Set a specific field.</p>
     */
    public void setField(int nField,String sValue) { fields[nField] = sValue; }

    /**
     *  <p>Return a specific field.</p>
     */
    public String getField(int nField) { return fields[nField]; }
}