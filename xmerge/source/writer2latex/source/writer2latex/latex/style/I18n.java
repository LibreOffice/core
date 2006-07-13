/************************************************************************
 *
 *  I18n.java
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
 *  Copyright: 2002-2005 by Henrik Just
 *
 *  All Rights Reserved.
 *
 *  Version 0.3.3j (2005-03-06)
 *
 */

package writer2latex.latex.style;

/* These classes takes care of i18n in Writer2LaTeX.
   In LaTeX, i18n is a mixture of inputencodings, fontencodings and babel languages.
   I18n thus manages these, and in particular implements a Unicode->LaTeX
   translation that can handle different inputencodings and fontencodings.
   The translation is table driven, using symbols.xml (embedded in the jar)
   Various sections of symbols.xml handles different cases:
     * common symbols in various font encodings such as T1, T2A, LGR etc.
     * input encodings such as ISO-8859-1 (latin-1), ISO-8859-7 (latin/greek) etc.
     * additional symbol fonts such as wasysym, dingbats etc.
     * font-specific symbols, eg. for 8-bit fonts/private use area

   TODO: Move UnicodeCharacter, UnicodeRow, UnicodeTable and UnicodeTableHandler
   as well as UnicodeStringParser to separate files!!
   (could be in a new package writer2latex.latex.unicode)
*/

import java.io.*;
import java.util.Hashtable;
import java.util.Stack;
import java.util.Vector;
import java.util.Enumeration;
import org.xml.sax.*;
import org.xml.sax.helpers.*;
import javax.xml.parsers.*;

import writer2latex.util.CSVList;
import writer2latex.util.Config;
import writer2latex.office.*;
import writer2latex.latex.LaTeXDocumentPortion;
import writer2latex.latex.ConverterPalette;

// Helper class: Parse a unicode string.
// Note: Some 8-bit fonts have additional "spacer" characters that are used
// for manual placement of accents. These are ignored between the base character
// and the combining character, thus we are parsing according to the rule
// <base char> <spacer char>* <combining char>?
class UnicodeStringParser {
    private UnicodeTable table; // the table to use
    private String s; // the string
    private int i; // the current index
    private char c; // the current character
    private char cc; // the current combining character

    protected void reset(UnicodeTable table, String s) {
        this.table=table;
        this.s=s;
        i=0;
    }

    protected boolean next() {
        if (i>=s.length()) { return false; }
        // Pick up base character
        c = s.charAt(i++);
        if (table.getCharType(c)==UnicodeCharacter.COMBINING) {
            // Lonely combining character - combine with space
            cc = c;
            c = ' ';
            return true;
        }

        // Skip characters that should be ignored
        while (i<s.length() && table.getCharType(s.charAt(i))==UnicodeCharacter.IGNORE) { i++; }
        // Pick up combining character, if any
        if (i<s.length() && table.getCharType(s.charAt(i))==UnicodeCharacter.COMBINING) {
            cc = s.charAt(i++);
        }
        else {
            cc = '\u0000';
        }

        // Fix french spacing
        if (c=='\u00A0') { // non-breaking space
            if (i>1) {
                // replace with space if we are right after an opening guillemet
                if (s.charAt(i-2)=='\u00AB') { c=' '; }
            }
            if (i<s.length()) {
                // replace with space if we are right before a closing guillemet,
                // an em-dash or !, ?, : or ;
                char cr = s.charAt(i);
                if (cr=='\u00BB' || cr=='\u2014' || cr=='!' || cr=='?'  || cr==':' || cr==';') {
                    c=' ';
                }
            }
        }

        return true;
    }

    protected char getChar() { return c; }

    protected boolean hasCombiningChar() { return cc!='\u0000'; }

    protected char getCombiningChar() { return cc; }

}

// Helper class: A struct to hold the LaTeX representations of a unicode character
class UnicodeCharacter implements Cloneable {
    final static int NORMAL = 0;     // this is a normal character
    final static int COMBINING = 1;  // this character should be ignored
    final static int IGNORE = 2;     // this is a combining character
    final static int UNKNOWN = 3;     // this character is unknown

    int nType;     // The type of character
    String sMath;  // LaTeX representation in math mode
    String sText;  // LaTeX representation in text mode
    int nFontencs; // Valid font encoding(s) for the text mode representation

    protected Object clone() {
        UnicodeCharacter uc = new UnicodeCharacter();
        uc.nType = this.nType;
        uc.sMath = this.sMath;
        uc.sText = this.sText;
        uc.nFontencs = this.nFontencs;
        return uc;
    }
}

// Helper class: A row of 256 unicode characters
class UnicodeRow implements Cloneable {
    UnicodeCharacter[] entries;
    UnicodeRow(){ entries=new UnicodeCharacter[256]; }

    protected Object clone() {
        UnicodeRow ur = new UnicodeRow();
        for (int i=0; i<256; i++) {
            if (this.entries[i]!=null) {
                ur.entries[i] = (UnicodeCharacter) this.entries[i].clone();
            }
        }
        return ur;
    }
}

// Helper class: Table of up to 65536 unicode characters
class UnicodeTable {
    protected UnicodeRow[] table=new UnicodeRow[256];
    private UnicodeTable parent;

    // Constructor; creates a new table, possibly based on a parent
    // Note: The parent must be fully loaded before the child is created.
    public UnicodeTable(UnicodeTable parent){
        this.parent = parent;
        if (parent!=null) {
            // *Copy* the rows from the parent
            for (int i=0; i<256; i++) {
                table[i] = parent.table[i];
            }
        }
    }

    // Make sure the required entry exists
    private void createEntry(int nRow, int nCol) {
        if (table[nRow]==null) {
            table[nRow]=new UnicodeRow();
        }
        else if (parent!=null && table[nRow]==parent.table[nRow]) {
            // Before changing a row it must be *cloned*
            table[nRow] = (UnicodeRow) parent.table[nRow].clone();
        }
        if (table[nRow].entries[nCol]==null) {
            table[nRow].entries[nCol]=new UnicodeCharacter();
        }
    }

    // Addd a single character (type only), by number
    protected void addCharType(char c, int nType) {
        int nRow=c/256; int nCol=c%256;
        createEntry(nRow,nCol);
        table[nRow].entries[nCol].nType = nType;
    }

    // Addd a single character (type only), by name
    protected void addCharType(char c, String sType) {
        int nRow=c/256; int nCol=c%256;
        createEntry(nRow,nCol);
        if ("combining".equals(sType)) {
            table[nRow].entries[nCol].nType = UnicodeCharacter.COMBINING;
        }
        else if ("ignore".equals(sType)) {
            table[nRow].entries[nCol].nType = UnicodeCharacter.IGNORE;
        }
        else {
            table[nRow].entries[nCol].nType = UnicodeCharacter.NORMAL;
        }
    }

    // Add a single math character to the table
    protected void addMathChar(char c, String sLaTeX){
        int nRow=c/256; int nCol=c%256;
        createEntry(nRow,nCol);
        table[nRow].entries[nCol].sMath=sLaTeX;
    }

    // Add a single text character to the table
    protected void addTextChar(char c, String sLaTeX, int nFontencs){
        int nRow=c/256; int nCol=c%256;
        createEntry(nRow,nCol);
        table[nRow].entries[nCol].sText=sLaTeX;
        table[nRow].entries[nCol].nFontencs=nFontencs;
    }

    // Retrieve entry for a character (or null)
    private UnicodeCharacter getEntry(char c) {
        int nRow=c/256; int nCol=c%256;
        if (table[nRow]==null) return null;
        return table[nRow].entries[nCol];
    }

    // Get character type
    public int getCharType(char c) {
        UnicodeCharacter entry = getEntry(c);
        if (entry==null) return UnicodeCharacter.UNKNOWN;
        return entry.nType;
    }

    // Check to see if this math character exists?
    public boolean hasMathChar(char c) {
        UnicodeCharacter entry = getEntry(c);
        if (entry==null) return false;
        return entry.sMath!=null;
    }

    // Get math character (or null)
    public String getMathChar(char c) {
        UnicodeCharacter entry = getEntry(c);
        if (entry==null) return null;
        return entry.sMath;
    }

    // Check to see if this text character exists?
    public boolean hasTextChar(char c) {
        UnicodeCharacter entry = getEntry(c);
        if (entry==null) return false;
        return entry.sText!=null;
    }

    // Get text character (or null)
    public String getTextChar(char c) {
        UnicodeCharacter entry = getEntry(c);
        if (entry==null) return null;
        return entry.sText;
    }

    // Get font encoding(s) for text character (or null)
    public int getFontencs(char c) {
        UnicodeCharacter entry = getEntry(c);
        if (entry==null) return 0;
        return entry.nFontencs;
    }

    // Get number of defined characters
    public int getCharCount() {
        int nCount = 0;
        for (int nRow=0; nRow<256; nRow++) {
            if (table[nRow]!=null) {
                for (int nCol=0; nCol<256; nCol++) {
                    UnicodeCharacter entry = table[nRow].entries[nCol];
                    if (entry!=null) nCount++;
                }
            }
        }
        return nCount;
    }

}

// Helper classs: SAX handler to parse symbols.xml from jar
class UnicodeTableHandler extends DefaultHandler{
    private Hashtable tableSet; // collection of all tables
    private UnicodeTable table; // the current table
    private String sSymbolSets;
    private boolean bGlobalReadThisSet;
    private boolean bReadThisSet;
    private int nGlobalFontencs = 0; // The global fontencodings for current symbol set
    private int nFontencs = 0; // The currently active fontencodings
    private boolean b8bit = false;

    UnicodeTableHandler(Hashtable tableSet, String sSymbolSets){
        this.sSymbolSets = sSymbolSets;
        this.tableSet = tableSet;
    }

    public void startElement(String nameSpace, String localName, String qName, Attributes attributes){
        if (qName.equals("symbols")) {
            //root element - create root table!
            table = new UnicodeTable(null);
            tableSet.put("root",table);
        }
        else if (qName.equals("symbol-set")) {
            // start a new symbol set; maybe we want to include it?
            bGlobalReadThisSet = sSymbolSets.indexOf(attributes.getValue("name")) >= 0;
            bReadThisSet = bGlobalReadThisSet;
            // Change global and current fontencodings
            nGlobalFontencs = I18n.readFontencs(attributes.getValue("fontenc"));
            nFontencs = nGlobalFontencs;
        }
        else if (qName.equals("special-symbol-set")) {
            // start a new special symbol set; this requires a new table
            table = new UnicodeTable((UnicodeTable) tableSet.get("root"));
            tableSet.put(attributes.getValue("name"),table);

            // Read it if it requires nothing, or something we read
            bGlobalReadThisSet = attributes.getValue("requires")==null ||
                                 sSymbolSets.indexOf(attributes.getValue("requires")) >= 0;
            bReadThisSet = bGlobalReadThisSet;
            b8bit = "true".equals(attributes.getValue("eight-bit"));
            // Change global and current fontencodings
            nGlobalFontencs = I18n.readFontencs(attributes.getValue("fontenc"));
            nFontencs = nGlobalFontencs;
        }
        else if (qName.equals("symbol-subset")) {
            // Do we requires something here?
            if (attributes.getValue("requires")!=null) {
                bReadThisSet = sSymbolSets.indexOf(attributes.getValue("requires")) >= 0;
            }
            // Change current fontencodings
            nFontencs = I18n.readFontencs(attributes.getValue("fontenc"));
        }
        else if (qName.equals("symbol")) {
            if (bReadThisSet) {
                char c=(char)Integer.parseInt(attributes.getValue("char"),16);
                String sEqChar=attributes.getValue("eq-char");
                if (sEqChar!=null) { // copy existing definitions, if any
                    char eqc = (char)Integer.parseInt(sEqChar,16);
                    if (table.getCharType(eqc)!=UnicodeCharacter.UNKNOWN) {
                        table.addCharType(c,table.getCharType(eqc));
                    }
                    if (table.hasMathChar(eqc)) {
                        table.addMathChar(c,table.getMathChar(eqc));
                    }
                    if (table.hasTextChar(eqc)) {
                        table.addTextChar(c,table.getTextChar(eqc),table.getFontencs(eqc));
                    }
                }
                else {
                    String sType=attributes.getValue("char-type");
                    String sMath=attributes.getValue("math");
                    String sText=attributes.getValue("text");
                    if (sType!=null) table.addCharType(c,sType);
                    if (sMath!=null) table.addMathChar(c,sMath);
                    if (sText!=null) table.addTextChar(c,sText,nFontencs);
                }
            }
        }
        else if (qName.equals("preserve-symbol")) {
            if (bReadThisSet) {
                String sMode=attributes.getValue("mode");
                char c=(char)Integer.parseInt(attributes.getValue("char"),16);
                table.addCharType(c,attributes.getValue("char-type"));
                if ("math".equals(sMode) || "both".equals(sMode)) {
                    table.addMathChar(c,Character.toString(c));
                }
                if ("text".equals(sMode) || "both".equals(sMode)) {
                    table.addTextChar(c,Character.toString(c),nFontencs);
                }
            }
        }
        else if (qName.equals("preserve-symbols")) {
            if (bReadThisSet) {
                String sMode=attributes.getValue("mode");
                String sType=attributes.getValue("char-type");
                char c1=(char)Integer.parseInt(attributes.getValue("first-char"),16);
                char c2=(char)Integer.parseInt(attributes.getValue("last-char"),16);
                boolean bMath = "math".equals(sMode) || "both".equals(sMode);
                boolean bText = "text".equals(sMode) || "both".equals(sMode);
                for (char c=c1; c<=c2; c++) {
                    table.addCharType(c,sType);
                    if (bMath) {
                        table.addMathChar(c,Character.toString(c));
                    }
                    if (bText) {
                        table.addTextChar(c,Character.toString(c),nFontencs);
                    }
                }
            }
        }
    }

    public void endElement(String nameSpace, String localName, String qName){
        if (qName.equals("symbol-subset")) {
            // Revert to global setting of reading status
            bReadThisSet = bGlobalReadThisSet;
            // Revert to global fontencoding
            nFontencs = nGlobalFontencs;
        }
        else if (qName.equals("special-symbol-set")) {
            if (b8bit) {
               // Row 0 = Row 240 (F0)
               // Note: 8-bit fonts are supposed to be relocated to F000..F0FF
               // This may fail on import from msword, hence this hack
               table.table[0] = table.table[240];
            }
            b8bit = false;
        }
    }

}

// Finally the actual conversion class!
public class I18n {
    // **** Inputencodings ****
    public static final int ASCII = 0;
    public static final int LATIN1 = 1; // ISO Latin 1 (ISO-8859-1)
    public static final int LATIN2 = 2; // ISO Latin 1 (ISO-8859-1)
    public static final int ISO_8859_7 = 3; // ISO latin/greek
    public static final int CP1250 = 4; // Microsoft Windows Eastern European
    public static final int CP1251 = 5; // Microsoft Windows Cyrillic
    public static final int KOI8_R = 6; // Latin/russian
    public static final int UTF8 = 7; // UTF-8

    // Read an inputencoding from a string
    public static final int readInputenc(String sInputenc) {
        if ("ascii".equals(sInputenc)) return ASCII;
        else if ("latin1".equals(sInputenc)) return LATIN1;
        else if ("latin2".equals(sInputenc)) return LATIN2;
        else if ("iso-8859-7".equals(sInputenc)) return ISO_8859_7;
        else if ("cp1250".equals(sInputenc)) return CP1250;
        else if ("cp1251".equals(sInputenc)) return CP1251;
        else if ("koi8-r".equals(sInputenc)) return KOI8_R;
        else if ("utf8".equals(sInputenc)) return UTF8;
        else return ASCII; // unknown = ascii
    }

    // Return the LaTeX name of an inputencoding
    public static final String writeInputenc(int nInputenc) {
        switch (nInputenc) {
            case ASCII : return "ascii";
            case LATIN1 : return "latin1";
            case LATIN2 : return "latin2";
            case ISO_8859_7 : return "iso-8859-7";
            case CP1250 : return "cp1250";
            case CP1251 : return "cp1251";
            case KOI8_R : return "koi8-r";
            case UTF8 : return "utf8";
            default : return "???";
        }
    }

    // Return the java i18n name of an inputencoding
    public static final String writeJavaEncoding(int nInputenc) {
        switch (nInputenc) {
            case ASCII : return "ASCII";
            case LATIN1 : return "ISO8859_1";
            case LATIN2 : return "ISO8859_2";
            case ISO_8859_7 : return "ISO8859_7";
            case CP1250 : return "Cp1250";
            case CP1251 : return "Cp1251";
            case KOI8_R : return "KOI8_R";
            case UTF8 : return "UTF-8";
            default : return "???";
        }
    }

    // **** Fontencodings ****
    public static final int OT1_ENC = 1;
    public static final int T1_ENC = 2;
    public static final int T2A_ENC = 4;
    public static final int T3_ENC = 8;
    public static final int LGR_ENC = 16;
    public static final int ANY_ENC = 31;

    // read set of font encodings from a string
    public static final int readFontencs(String sFontencs) {
        sFontencs = sFontencs.toUpperCase();
        if ("ANY".equals(sFontencs)) return ANY_ENC;
        int nFontencs = 0;
        int nOT1 = sFontencs.indexOf("OT1");
        int nT1 = sFontencs.indexOf("T1");
        int nT2A = sFontencs.indexOf("T2A");
        int nT3 = sFontencs.indexOf("T3");
        int nLGR = sFontencs.indexOf("LGR");
        if (nOT1>=0) nFontencs+=OT1_ENC;
        if (nT1==0 || (nT1>0 && nT1!=nOT1+1)) nFontencs+=T1_ENC;
        if (nT2A>=0) nFontencs+=T2A_ENC;
        if (nT3>=0) nFontencs+=T3_ENC;
        if (nLGR>=0) nFontencs+=LGR_ENC;
        return nFontencs;
    }

    // return string representation of a single font encoding
    public static final String writeFontenc(int nFontenc) {
        switch (nFontenc) {
            case OT1_ENC: return "OT1";
            case T1_ENC: return "T1";
            case T2A_ENC: return "T2A";
            case T3_ENC: return "T3";
            case LGR_ENC: return "LGR";
        }
        return null;
    }

    // check that a given set of font encodings contains a specific font encoding
    public static final boolean supportsFontenc(int nFontencs, int nFontenc) {
        return (nFontencs & nFontenc) != 0;
    }

    // get one fontencoding from a set of fontencodings
    public static final int getFontenc(int nFontencs) {
        if (supportsFontenc(nFontencs,T1_ENC)) return T1_ENC;
        if (supportsFontenc(nFontencs,T2A_ENC)) return T2A_ENC;
        if (supportsFontenc(nFontencs,T3_ENC)) return T3_ENC;
        if (supportsFontenc(nFontencs,LGR_ENC)) return LGR_ENC;
        return 0;
    }

    // get the font encoding for a specific iso language
    public static final int getFontenc(String sLang) {
        // Greek uses "local greek" encoding
        if ("el".equals(sLang)) return LGR_ENC;
        // Russian, ukrainian, bulgarian and serbian uses T2A encoding
        else if ("ru".equals(sLang)) return T2A_ENC;
        else if ("uk".equals(sLang)) return T2A_ENC;
        else if ("bg".equals(sLang)) return T2A_ENC;
        else if ("sr".equals(sLang)) return T2A_ENC;
        // Other languages uses T1 encoding
        else return T1_ENC;
    }

    // return cs for a fontencoding
    public static final String getFontencCs(int nFontenc) {
        switch (nFontenc) {
            case T1_ENC: return "\\textlatin"; // requires babel
            case T2A_ENC: return "\\textcyrillic"; // requires babel with russian, bulgarian or ukrainian option
            case T3_ENC: return "\\textipa"; // requires tipa.sty
            case LGR_ENC: return "\\textgreek"; // requires babel with greek option
            default: return null;
        }
    }

    // **** Languages ****

    // Convert iso language to babel language
    // todo: include iso country
    // todo: support automatic choice of inputenc (see comments)
    public static final String getBabelLanguage(String sLanguage) {
        if ("en".equals(sLanguage)) return "english"; // latin1
        else if ("bg".equals(sLanguage)) return "bulgarian"; // cp1251?
        else if ("cs".equals(sLanguage)) return "czech"; // latin2
        else if ("da".equals(sLanguage)) return "danish"; // latin1
        else if ("de".equals(sLanguage)) return "ngerman"; // latin1
        else if ("el".equals(sLanguage)) return "greek"; // iso-8859-7
        else if ("es".equals(sLanguage)) return "spanish"; // latin1
        else if ("fi".equals(sLanguage)) return "finnish"; // latin1 (latin9?)
        else if ("fr".equals(sLanguage)) return "french"; // latin1 (latin9?)
        else if ("ga".equals(sLanguage)) return "irish" ; // latin1
        else if ("hr".equals(sLanguage)) return "croatian"; // latin2
        else if ("hu".equals(sLanguage)) return "magyar"; // latin2
        else if ("is".equals(sLanguage)) return "icelandic"; // latin1
        else if ("it".equals(sLanguage)) return "italian"; // latin1
        else if ("nl".equals(sLanguage)) return "dutch"; // latin1
        else if ("no".equals(sLanguage)) return "norsk"; // latin1
        else if ("pl".equals(sLanguage)) return "polish"; // latin2
        else if ("pt".equals(sLanguage)) return "portuges"; // latin1
        else if ("ro".equals(sLanguage)) return "romanian"; // latin2
        else if ("ru".equals(sLanguage)) return "russian"; // cp1251?
        else if ("sk".equals(sLanguage)) return "slovak"; // latin2
        else if ("sl".equals(sLanguage)) return "slovene"; // latin2
        else if ("sr".equals(sLanguage)) return "serbian"; // cp1251?
        else if ("sv".equals(sLanguage)) return "swedish"; // latin1
        else if ("uk".equals(sLanguage)) return "ukrainian"; // cp1251?
        else return null; // unknown or unsupported language
    }

    // End of static part of I18n!

    // Global variables:
    private Hashtable tableSet; // all tables
    private UnicodeTable table; // currently active table (top of stack)
    private Stack tableStack; // stack of active tables
    private UnicodeStringParser ucparser; // Unicode string parser
    private String sDefaultLang = null; // Default language to use
    private int nDefaultFontenc = 0; // Fontenc for the default language
    private boolean bAlwaysUseDefaultLang = false; // Ignore sLang parameter to convert()
    private boolean bT2A = false; // do we use cyrillic letters?
    private boolean bGreek = false; // do we use greek letters?
    private boolean bPolytonicGreek = false; // do we use polytonic greek letters?
    private boolean bGreekMath;
    private String sMajorityLanguage = null; // cache result from method
    private Config config;
    private WriterStyleCollection wsc;

    // Constructor; loads symbol table based on the configuration
    public I18n(WriterStyleCollection wsc, Config config, ConverterPalette palette) {
        this.wsc = wsc;
        this.config = config;
        String sSymbols="ascii"; // always load common symbols
        if (config.getInputencoding()!=ASCII) {
            sSymbols+="|"+writeInputenc(config.getInputencoding());
        }

        if (config.useWasysym()) sSymbols+="|wasysym";
        if (config.useBbding()) sSymbols+="|bbding";
        if (config.useIfsym()) sSymbols+="|ifsym";
        if (config.usePifont()) sSymbols+="|dingbats";
        if (config.useEurosym()) sSymbols+="|eurosym";
        if (config.useTipa()) sSymbols+="tipa";

        readSymbols(sSymbols);

        bGreekMath = config.greekMath();

        bAlwaysUseDefaultLang = !config.multilingual();


        ucparser = new UnicodeStringParser();
    }

    // Constructor; loads specific symbols
    public I18n(String sSymbols) {
        readSymbols(sSymbols);
    }

    public void appendDeclarations(LaTeXDocumentPortion pack, LaTeXDocumentPortion decl) {
        // inputenc TODO: Remove dirty, autogenerated
        if (config.getInputencoding()==I18n.UTF8){
            pack.append("\\usepackage[dirty,autogenerated]{ucs}").nl();
        }
        pack.append("\\usepackage[")
                .append(writeInputenc(config.getInputencoding()))
                .append("]{inputenc}").nl();

        // fontenc
        CSVList fontencs = new CSVList(',');
        if (bT2A) { fontencs.addValue("T2A"); }
        if (bGreek) { fontencs.addValue("LGR"); }
        if (config.useTipa()) { fontencs.addValue("T3"); }
        fontencs.addValue("T1");
        pack.append("\\usepackage[").append(fontencs.toString())
            .append("]{fontenc}").nl();

        // babel
        convertLanguageUsage(pack);

        if (config.useTipa()) {
            pack.append("\\usepackage[noenc]{tipa}").nl()
                .append("\\usepackage{tipx}").nl();
        }

        // Bbding: (Has to avoid some nameclashes.)
        if (config.useBbding()) {
            pack.append("\\usepackage{bbding}").nl()
                .append("\\let\\bbCross\\Cross\\let\\Cross\\undefined").nl()
                .append("\\let\\bbSquare\\Square\\let\\Square\\undefined").nl()
                .append("\\let\\bbTrianbleUp\\TriangleUp\\let\\TriangleUp\\undefined").nl()
                .append("\\let\\bbTrianlgeDown\\TriangleDown\\let\\TriangleDown\\undefined").nl();
        }

        if (config.useIfsym()) { pack.append("\\usepackage[geometry,weather,misc,clock]{ifsym}").nl(); }

        if (config.usePifont()) { pack.append("\\usepackage{pifont}").nl(); }

        if (config.useEurosym()) { pack.append("\\usepackage{eurosym}").nl(); }

        // wasysym must be loaded between amsmath and amsfonts!
        if (config.useWasysym()) {
            pack.append("\\usepackage{amsmath,wasysym,amssymb,amsfonts,textcomp}").nl();
        }
        else {
            pack.append("\\usepackage{amsmath,amssymb,amsfonts,textcomp}").nl();
        }

        // \textcyrillic command - deleted as babel defines it!
        //if (bT2A) {
        //    pack.append("\\DeclareRobustCommand{\\textcyrillic}[1]{{\\fontencoding{T2A}\\selectfont\\def\\encodingdefault{T2A}#1}}").nl();
        //}
    }

    public void setDefaultLanguage(String sDefaultLang) {
        this.sDefaultLang = sDefaultLang;
        nDefaultFontenc = getFontenc(sDefaultLang);
    }

    /** <p>Apply language.</p>
     *  @param <code>style</code> the OOo style to read attributesfrom
     *  @param <code>bDecl</code> true if declaration form is required
     *  @param <code>bInherit</code> true if inherited properties should be used
     *  @param <code>ba</code> the <code>BeforeAfter</code> to add LaTeX code to.
     */
    public void applyLanguage(StyleWithProperties style, boolean bDecl, boolean bInherit, BeforeAfter ba) {
        if (bAlwaysUseDefaultLang) { return; }
        if (style==null) { return; }
        String sLang = getBabelLanguage(style.getProperty(XMLString.FO_LANGUAGE,bInherit));
        if (sLang==null) { return; }
        if (bDecl) {
            ba.add("\\selectlanguage{"+sLang+"}","");
            //ba.add("\\begin{otherlanguage}{"+sLang+"}","\\end{otherlanguage}");
        }
        else {
            ba.add("\\foreignlanguage{"+sLang+"}{","}");
        }
    }

    // Return the iso language used in most paragaph styles (in a well-structured
    // document this will be the default language)
    // TODO: Base on content rather than style (move to converter!)
    public String getMajorityLanguage() {
        Hashtable langs = new Hashtable();

        // Read the default language from the default paragraph style
        String sDefaultLang = null;
        StyleWithProperties style = wsc.getDefaultParStyle();
        if (style!=null) {
            sDefaultLang = style.getProperty(XMLString.FO_LANGUAGE);
        }

        // Collect languages from paragraph styles
        Enumeration enumeration = wsc.getParStyles().getStylesEnumeration();
        while (enumeration.hasMoreElements()) {
            style = (StyleWithProperties) enumeration.nextElement();
            String sLang = style.getProperty(XMLString.FO_LANGUAGE);
            if (sLang==null) { sLang = sDefaultLang; }
            if (sLang!=null) {
                int nCount = 1;
                if (langs.containsKey(sLang)) {
                    nCount = ((Integer) langs.get(sLang)).intValue()+1;
                }
                langs.put(sLang,new Integer(nCount));
            }
        }

        // Find the most common language
        int nMaxCount = 0;
        String sMajorityLanguage = null;
        enumeration = langs.keys();
        while (enumeration.hasMoreElements()) {
            String sLang = (String) enumeration.nextElement();
            int nCount = ((Integer) langs.get(sLang)).intValue();
            if (nCount>nMaxCount) {
                nMaxCount = nCount;
                sMajorityLanguage = sLang;
            }
        }
        this.sMajorityLanguage = sMajorityLanguage;
        return sMajorityLanguage;
    }

    private void convertLanguageUsage(LaTeXDocumentPortion ldp) {
        Vector languages = new Vector();
        String sDefaultLanguage = null;
        if (config.multilingual()) {
            // Collect languages from text styles
            Enumeration enumeration = wsc.getTextStyles().getStylesEnumeration();
            while (enumeration.hasMoreElements()) {
                StyleWithProperties style = (StyleWithProperties) enumeration.nextElement();
                String sLang = I18n.getBabelLanguage(style.getProperty(XMLString.FO_LANGUAGE));
                if (sLang!=null && !languages.contains(sLang)) { languages.add(sLang); }
            }
            // Collect languages from paragraph styles
            enumeration = wsc.getParStyles().getStylesEnumeration();
            while (enumeration.hasMoreElements()) {
                StyleWithProperties style = (StyleWithProperties) enumeration.nextElement();
                String sLang = I18n.getBabelLanguage(style.getProperty(XMLString.FO_LANGUAGE));
                if (sLang!=null && !languages.contains(sLang)) { languages.add(sLang); }
            }
            // Read the default language from the default paragraph style
            StyleWithProperties style = wsc.getDefaultParStyle();
            if (style!=null) {
                sDefaultLanguage = I18n.getBabelLanguage(style.getProperty(XMLString.FO_LANGUAGE));
            }
        }
        else { // the most common language is the only language
            sDefaultLanguage = I18n.getBabelLanguage(sMajorityLanguage==null ?
                               getMajorityLanguage() : sMajorityLanguage);
        }
        // If the document contains "anonymous" greek letters we need greek in any case:
        if (this.greek() && !languages.contains("greek")) languages.add("greek");
        // If the document contains "anonymous cyrillic letters we need one of the
        // languages russian, ukrainian or bulgarian
        if (this.cyrillic() && !(languages.contains("ukrainian") ||
            languages.contains("ukrainian") || languages.contains("ukrainian"))) {
            languages.add("russian");
        }
        // Load babel with the used languages
        CSVList babelopt = new CSVList(",");
        Enumeration langenum = languages.elements();
        while (langenum.hasMoreElements()) {
            String sLang = (String) langenum.nextElement();
            if (!sLang.equals(sDefaultLanguage)) {
                if ("greek".equals(sLang) && this.polytonicGreek()) {
                    sLang = "polutonikogreek";
                }
                babelopt.addValue(sLang);
            }
        }
        // The default language must be the last one
        if (sDefaultLanguage!=null) {
            if ("greek".equals(sDefaultLanguage) && this.polytonicGreek()) {
                babelopt.addValue("polutonikogreek");
            }
            else {
                babelopt.addValue(sDefaultLanguage);
            }
        }
        if (!babelopt.isEmpty()) {
            ldp.append("\\usepackage[")
               .append(babelopt.toString())
               .append("]{babel}").nl();
        }
    }

    // The parameter sSymbolsets should contain a sequence of all symbols sets to
    // include eg. "ascii|latin1|dingbats" to include the default symbols,
    // use latin1 inputencoding and support dingbats (pifont.sty).
    private void readSymbols(String sSymbols) {
        tableSet = new Hashtable();
        UnicodeTableHandler handler=new UnicodeTableHandler(tableSet, sSymbols);
        SAXParserFactory factory=SAXParserFactory.newInstance();
        InputStream is = this.getClass().getResourceAsStream("symbols.xml");
        try {
            SAXParser saxParser=factory.newSAXParser();
            saxParser.parse(is,handler);
        }
        catch (Throwable t){
            System.out.println("Oops - Unable to read symbols.xml");
            t.printStackTrace();
        }
        // put root table at top of stack
        tableStack = new Stack();
        tableStack.push((UnicodeTable) tableSet.get("root"));
        table = (UnicodeTable) tableSet.get("root");
    }

    // Did we use cyrillic?
    public boolean cyrillic() { return bT2A; }

    // Did we use greek?
    public boolean greek() { return bGreek; }

    // Did we use polytonic greek?
    public boolean polytonicGreek() { return bPolytonicGreek; }

    // Outside greek text, greek letters may be rendered in math mode,
    // if the user requires that in the configuration.
    private boolean greekMath(char c, int nFontenc) {
        return bGreekMath && nFontenc!=LGR_ENC && table.getFontencs(c)==LGR_ENC;
    }

    // Set cyrillic and greek flags
    private void setFlags(char c, int nFontenc) {
        if ((c>='\u1F00') && (c<='\u1FFF')) bPolytonicGreek = true;
        if (nFontenc==LGR_ENC) bGreek = true;
        if (nFontenc==T2A_ENC) bT2A = true;
    }

    // Missing symbol
    private String notFound(char c,int nFontenc) {
        //String sErrorMsg = "[Warning: Missing symbol " + Integer.toHexString(c).toUpperCase() +"]";
        String sErrorMsg = "["+Integer.toHexString(c).toUpperCase() +"?]";
        if (nFontenc==T1_ENC) return sErrorMsg;
        else return "\\textlatin{"+sErrorMsg+"}";
    }

    // convert a single math character
    private String convertMathChar(char c, int nFontenc) {
        if (table.hasMathChar(c)) {
            return table.getMathChar(c);
        }
        else if (table.hasTextChar(c)) { // use text mode as a fallback
            int nFontencs = table.getFontencs(c);
            if (supportsFontenc(nFontencs,nFontenc)) {
                // The text character is valid in the current font encoding
                setFlags(c,nFontenc);
                if (table.getCharType(c)==UnicodeCharacter.COMBINING) {
                    return "\\text{" + table.getTextChar(c) +"{}}";
                }
                else {
                    return "\\text{" + table.getTextChar(c) +"}";
                }
            }
            else {
                // The text character is valid in another font encoding
                int nFontenc1 = getFontenc(nFontencs);
                setFlags(c,nFontenc1);
                if (table.getCharType(c)==UnicodeCharacter.COMBINING) {
                    return "\\text{" + getFontencCs(nFontenc1) + "{" + table.getTextChar(c) +"{}}}";
                }
                else {
                    return "\\text{" + getFontencCs(nFontenc1) + "{" + table.getTextChar(c) +"}}";
                }
            }
        }
        else {
            return "\\text{" + notFound(c,nFontenc) + "}";
        }
    }

    // Convert a single character
    public String convert(char c, boolean bMathMode, String sLang){
        int nFontenc = bAlwaysUseDefaultLang ? nDefaultFontenc : getFontenc(sLang);
        if (bMathMode) {
            return convertMathChar(c,nFontenc);
        }
        else if (greekMath(c,nFontenc) || (table.hasMathChar(c) && !table.hasTextChar(c))) {
            return "$" + convertMathChar(c,nFontenc) + "$";
        }
        else if (table.hasTextChar(c)) {
            int nFontencs = table.getFontencs(c);
            if (supportsFontenc(nFontencs,nFontenc)) {
                // The text character is valid in the current font encoding
                setFlags(c,nFontenc);
                if (table.getCharType(c)==UnicodeCharacter.COMBINING) {
                    return table.getTextChar(c)+"{}";
                }
                else {
                    return table.getTextChar(c);
                }
            }
            else {
                // The text character is valid in another font encoding
                int nFontenc1 = getFontenc(nFontencs);
                setFlags(c,nFontenc1);
                if (table.getCharType(c)==UnicodeCharacter.COMBINING) {
                    return getFontencCs(nFontenc1) + "{" + table.getTextChar(c) +"{}}";
                }
                else {
                    return getFontencCs(nFontenc1) + "{" + table.getTextChar(c) +"}";
                }
            }
        }
        else {
            return notFound(c,nFontenc);
        }
    }

    // Convert a string of characters
    public String convert(String s, boolean bMathMode, String sLang){
        int nFontenc = bAlwaysUseDefaultLang ? nDefaultFontenc : getFontenc(sLang);
        int nCurFontenc = nFontenc;
        ucparser.reset(table,s);
        StringBuffer buf=new StringBuffer();
        while (ucparser.next()) {
            char c = ucparser.getChar();
            if (bMathMode) {
                buf.append(convertMathChar(c,nFontenc));
            }
            else if (greekMath(c,nFontenc) || (table.hasMathChar(c) && !table.hasTextChar(c))) {
                buf.append("$").append(convertMathChar(c,nFontenc)).append("$");
            }
            else if (table.hasTextChar(c)) {
                int nFontencs = table.getFontencs(c);
                if (supportsFontenc(nFontencs,nCurFontenc)) {
                    // The text character is valid in the current font encoding
                    // Note: Change of font encoding is greedy - change?
                    setFlags(c,nCurFontenc);
                    if (ucparser.hasCombiningChar()) {
                        char cc = ucparser.getCombiningChar();
                        if (supportsFontenc(table.getFontencs(cc),nCurFontenc)) {
                            buf.append(table.getTextChar(cc)).append("{")
                               .append(table.getTextChar(c)).append("}");
                        }
                        else { // ignore combining char if not valid in this font encoding
                            buf.append(table.getTextChar(c));
                        }
                    }
                    else {
                        buf.append(table.getTextChar(c));
                    }
                }
                else {
                    // The text character is valid in another font encoding
                    int nFontenc1 = getFontenc(nFontencs);
                    setFlags(c,nFontenc1);
                    if (nCurFontenc!=nFontenc) { // end "other font encoding"
                        buf.append("}");
                    }
                    if (nFontenc1!=nFontenc) { // start "other font encoding"
                        buf.append(getFontencCs(nFontenc1)).append("{");
                    }

                    if (ucparser.hasCombiningChar()) {
                        char cc = ucparser.getCombiningChar();
                        if (supportsFontenc(table.getFontencs(cc),nCurFontenc)) {
                            buf.append(table.getTextChar(cc)).append("{")
                               .append(table.getTextChar(c)).append("}");
                        }
                        else { // ignore combining char if not valid in this font encoding
                            buf.append(table.getTextChar(c));
                        }
                    }
                    else {
                        buf.append(table.getTextChar(c));
                    }
                    nCurFontenc = nFontenc1;
                }
            }
            else {
                buf.append(notFound(c,nCurFontenc));
            }
        }

        if (nCurFontenc!=nFontenc) { // end unfinished "other font encoding"
            buf.append("}");
        }

        return buf.toString();
    }

    public void pushSpecialTable(String sName) {
        // If no name is specified we should keep the current table
        // Otherwise try to find the table, and use root if it's not available
        if (sName!=null) {
            table = (UnicodeTable) tableSet.get(sName);
            if (table==null) { table = (UnicodeTable) tableSet.get("root"); }
        }
        tableStack.push(table);
    }

    public void popSpecialTable() {
        tableStack.pop();
        table = (UnicodeTable) tableStack.peek();
    }

    public int getCharCount() { return table.getCharCount(); }

}

/*
    // Test app. to print out converted symbols
    // TODO: Rewrite to use LaTeXDocument
    public static void main(String[] args){
        UnicodeTable table;
        try {
            table=new UnicodeTable();
            int nRow=Integer.parseInt(args[0]);
            FileOutputStream f=new FileOutputStream("symbols.tex");
            OutputStreamWriter osw=new OutputStreamWriter(f,"ISO-8859-1");
            osw.write("\\documentclass[landscape]{article}\n");
            osw.write("\\setlength{\\oddsidemargin}{-1cm}\n");
            osw.write("\\setlength{\\topmargin}{-1cm}\n");
            osw.write("\\setlength{\\textwidth}{27cm}\n");
            osw.write("\\setlength{\\textheight}{19cm}\n");
            osw.write("\\usepackage[latin1]{inputenc}\n");
            osw.write("\\usepackage[T1]{fontenc}\n");
            osw.write("\\usepackage{amsmath,amsfonts,amssymb,textcomp}\n");
            osw.write("\\usepackage[english]{babel}");
            osw.write("\\usepackage{writer}\n");
            osw.write("\\begin{document}\n");
            osw.write("\\begin{center}\n");
            osw.write("\\section*{Row number "+nRow+"}\n");
            osw.write("\\begin{tabular}{|*{16}{c|}}\n\\hline\n");
            for (int i=0; i<16; i++){
                for (int j=0; j<16; j++){
                    osw.write("\\small "+Integer.toHexString(256*nRow+16*i+j).toUpperCase());
                    if (j<15) osw.write(" & ");
                }
                osw.write("\\\\\n");
                for (int j=0; j<16; j++){
                    char c=(char) (256*nRow+16*i+j);
                    String st=table.convert(c,false);
                    String sm=table.convert(c,true);
                    if (st.startsWith("[Warning")) osw.write("?");
                    else osw.write(st);
                    osw.write(" ");
                    if (sm.startsWith("[Warning")) osw.write("?");
                    else osw.write("$"+sm+"$");
                    if (j<15) osw.write(" & ");
                }
                osw.write("\\\\\\hline\n");
            }
            osw.write("\\end{tabular}\n");
            osw.write("\\end{center}\n");
            osw.write("\\end{document}\n");
            osw.close();
           f.close();
        }
        catch (Throwable t) {
            t.printStackTrace();
        }
    }
  */
