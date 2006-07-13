/************************************************************************
 *
 *  Config.java
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
 *  Copyright: 2002-2004 by Henrik Just
 *
 *  All Rights Reserved.
 *
 *  Version 0.3.3i (2004-12-28)
 *
 */

package writer2latex.util;

/* Configuration
*/

import java.util.Hashtable;
import java.util.Enumeration;
import java.util.LinkedList;
import java.io.*;
import java.net.URI;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import org.w3c.dom.Document;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.w3c.dom.Element;
import org.w3c.dom.DOMImplementation;

import writer2latex.xmerge.NewDOMDocument;
import writer2latex.latex.style.I18n;
import writer2latex.latex.style.StyleMap;
import writer2latex.latex.HeadingMap;
//import writer2latex.xhtml.XhtmlStyleMap;
//import writer2latex.xhtml.XhtmlDocument;

// The mother of all options; reads and writes string values
class Option {
    protected String sValue;
    private String sName;

    void setString(String sValue) { this.sValue = sValue; }

    String getString() { return sValue; }

    String getName() { return sName; }

    Option(String sName, String sDefaultValue) {
       this.sName = sName;
       setString(sDefaultValue);
    }
}

// A simple variant is a BooleanOption, which interprets the values as booleans
class BooleanOption extends Option {
    boolean bValue;

    boolean getValue() { return bValue; }

    void setString(String sValue) {
        super.setString(sValue);
        bValue = "true".equals(sValue);
    }

    BooleanOption(String sName, String sDefaultValue) {
        super(sName,sDefaultValue);
    }
}

// Next is the IntegerOption, which must always be subclassed (must override setString)
abstract class IntegerOption extends Option {
    int nValue;

    int getValue() { return nValue; }

    IntegerOption(String sName, String sDefaultValue) {
        super(sName,sDefaultValue);
    }
}

public class Config {
    public static final int GENERIC = 0;
    public static final int DVIPS = 1;
    public static final int PDFTEX = 2;

    // Formatting (must be ordered)
    public static final int IGNORE_ALL = 0;
    public static final int IGNORE_MOST = 1;
    public static final int CONVERT_BASIC = 2;
    public static final int CONVERT_MOST = 3;
    public static final int CONVERT_ALL = 4;
    // Page formatting
    public static final int CONVERT_HEADER_FOOTER = 5;

    private static final int CREATE_USER_CONFIG = 0;
    private static final int BACKEND = 1;
    private static final int NO_PREAMBLE = 2;
    private static final int DOCUMENTCLASS = 3;
    private static final int GLOBAL_OPTIONS = 4;
    private static final int INPUTENCODING = 5;
    private static final int MULTILINGUAL = 6;
    private static final int GREEK_MATH = 7;
    private static final int USE_OOOMATH = 8;
    private static final int USE_PIFONT = 9;
    private static final int USE_IFSYM = 10;
    private static final int USE_WASYSYM = 11;
    private static final int USE_BBDING = 12;
    private static final int USE_EUROSYM = 13;
    private static final int USE_TIPA = 14;
    private static final int USE_COLOR = 15;
    private static final int USE_HYPERREF = 16;
    private static final int USE_ENDNOTES = 17;
    private static final int USE_ULEM = 18;
    private static final int USE_LASTPAGE = 19;
    private static final int USE_BIBTEX = 20;
    private static final int BIBTEX_STYLE = 21;
    private static final int FORMATTING = 22;
    private static final int PAGE_FORMATTING = 23;
    private static final int IGNORE_HARD_PAGE_BREAKS = 24;
    private static final int IGNORE_HARD_LINE_BREAKS = 25;
    private static final int IGNORE_EMPTY_PARAGRAPHS = 26;
    private static final int IGNORE_DOUBLE_SPACES = 27;
    private static final int DEBUG = 28;
//    private static final int XHTML_NO_DOCTYPE = 28;
//    private static final int XHTML_CUSTOM_STYLESHEET = 29;
//    private static final int XHTML_IGNORE_STYLES = 30;
//    private static final int XHTML_USE_DUBLIN_CORE = 31;
//    private static final int XHTML_CONVERT_TO_PX = 32;
//    private static final int XHTML_SCALING = 33;
//    private static final int XHTML_COLUMN_SCALING = 34;
//    private static final int XHTML_SPLIT_LEVEL = 35;
//    private static final int XHTML_CALC_SPLIT = 36;

    // due to XHTML code removal, changed count from 37 to 29
    // usage of enum (java 5) otherwise initialized array is recommended
    private static final int OPTION_COUNT = 29;
    Option[] options = new Option[OPTION_COUNT];

    protected boolean bDebug = false;

    protected LinkedList customPreamble = new LinkedList();
    protected StyleMap par = new StyleMap();
    protected StyleMap parBlock = new StyleMap();
    protected StyleMap text = new StyleMap();
    protected StyleMap list = new StyleMap();
    protected StyleMap listItem = new StyleMap();
    protected HeadingMap headingMap = new HeadingMap(5);
//    protected XhtmlStyleMap xpar = new XhtmlStyleMap();
//    protected XhtmlStyleMap xtext = new XhtmlStyleMap();
//    protected XhtmlStyleMap xframe = new XhtmlStyleMap();
//    protected XhtmlStyleMap xlist = new XhtmlStyleMap();
//    protected XhtmlStyleMap xattr = new XhtmlStyleMap();
    protected Hashtable mathSymbols = new Hashtable();

    public Config() {
        // create options with default values
        options[CREATE_USER_CONFIG] = new BooleanOption("create_user_config","true");
        options[NO_PREAMBLE] = new BooleanOption("no_preamble","false");
        options[DOCUMENTCLASS] = new Option("documentclass","article");
        options[GLOBAL_OPTIONS] = new Option("global_options","");
        options[BACKEND] = new IntegerOption("backend","generic") {
            void setString(String sValue) {
                super.setString(sValue);
                if ("generic".equals(sValue)) nValue = GENERIC;
                else if ("dvips".equals(sValue)) nValue = DVIPS;
                else if ("pdftex".equals(sValue)) nValue = PDFTEX;
            }
        };
        options[INPUTENCODING] = new IntegerOption("inputencoding",I18n.writeInputenc(I18n.ASCII)) {
            void setString(String sValue) {
                super.setString(sValue);
                nValue = I18n.readInputenc(sValue);
            }
        };
        options[MULTILINGUAL] = new BooleanOption("multilingual","true");
        options[GREEK_MATH] = new BooleanOption("greek_math","true");
        options[USE_OOOMATH] = new BooleanOption("use_ooomath","false");
        options[USE_PIFONT] = new BooleanOption("use_pifont","false");
        options[USE_IFSYM] = new BooleanOption("use_ifsym","false");
        options[USE_WASYSYM] = new BooleanOption("use_wasysym","false");
        options[USE_BBDING] = new BooleanOption("use_bbding","false");
        options[USE_EUROSYM] = new BooleanOption("use_eurosym","false");
        options[USE_TIPA] = new BooleanOption("use_tipa","false");
        options[USE_COLOR] = new BooleanOption("use_color","true");
        options[USE_HYPERREF] = new BooleanOption("use_hyperref","true");
        options[USE_ENDNOTES] = new BooleanOption("use_endnotes","false");
        options[USE_ULEM] = new BooleanOption("use_ulem","false");
        options[USE_LASTPAGE] = new BooleanOption("use_lastpage","false");
        options[USE_BIBTEX] = new BooleanOption("use_bibtex","false");
        options[BIBTEX_STYLE] = new Option("bibtex_style","plain");
        options[FORMATTING] = new IntegerOption("formatting","convert_basic") {
            void setString(String sValue) {
                super.setString(sValue);
                if ("convert_all".equals(sValue)) nValue = CONVERT_ALL;
                else if ("convert_most".equals(sValue)) nValue = CONVERT_MOST;
                else if ("convert_basic".equals(sValue)) nValue = CONVERT_BASIC;
                else if ("ignore_most".equals(sValue)) nValue = IGNORE_MOST;
                else if ("ignore_all".equals(sValue)) nValue = IGNORE_ALL;
            }
        };
        options[PAGE_FORMATTING] = new IntegerOption("page_formatting","convert_all") {
            void setString(String sValue) {
                super.setString(sValue);
                if ("convert_all".equals(sValue)) nValue = CONVERT_ALL;
                else if ("convert_header_footer".equals(sValue)) nValue = CONVERT_HEADER_FOOTER;
                else if ("ignore_all".equals(sValue)) nValue = IGNORE_ALL;
            }
        };
        options[IGNORE_HARD_PAGE_BREAKS] = new BooleanOption("ignore_hard_page_breaks","false");
        options[IGNORE_HARD_LINE_BREAKS] = new BooleanOption("ignore_hard_line_breaks","false");
        options[IGNORE_EMPTY_PARAGRAPHS] = new BooleanOption("ignore_empty_paragraphs","false");
        options[IGNORE_DOUBLE_SPACES] = new BooleanOption("ignore_double_spaces","false");

//        options[XHTML_NO_DOCTYPE] = new BooleanOption("xhtml_no_doctype","false");
//        options[XHTML_CUSTOM_STYLESHEET] = new Option("xhtml_custom_stylesheet","");
//        options[XHTML_IGNORE_STYLES] = new BooleanOption("xhtml_ignore_styles","false");
//        options[XHTML_USE_DUBLIN_CORE] = new BooleanOption("xhtml_use_dublin_core","true");
//        options[XHTML_CONVERT_TO_PX] = new BooleanOption("xhtml_convert_to_px","true");
//        options[XHTML_SCALING] = new Option("xhtml_scaling","100%");
//        options[XHTML_COLUMN_SCALING] = new Option("xhtml_column_scaling","100%");
//        options[XHTML_SPLIT_LEVEL] = new IntegerOption("xhtml_split_level","0") {
//            void setString(String sValue) {
//                super.setString(sValue);
//                nValue = Misc.getPosInteger(sValue,0);
//            }
//        };
//        options[XHTML_CALC_SPLIT] = new BooleanOption("xhtml_calc_split","false");
        options[DEBUG] = new BooleanOption("debug","false");
        // Headings for article class:
        headingMap.setLevelData(1,"section",1);
        headingMap.setLevelData(2,"subsection",2);
        headingMap.setLevelData(3,"subsubsection",3);
        headingMap.setLevelData(4,"paragraph",4);
        headingMap.setLevelData(5,"subparagraph",5);
        // read system wide default configuration, if available
        // TODO: Is there a more elegant approach to find the jar file name???
        //String s = this.getClass().getResource("symbols.xml").toString();
        String s = this.getClass().getResource("Config.class").toString();
        String sPath = s.substring(s.indexOf("file:"),s.indexOf("writer2latex.jar!"));
        String sDefaultConfig = sPath+"writer2latex.xml";
        try {
            File f = new File(new URI(sDefaultConfig));
            if (f.exists()) read(f.getPath());
        }
        catch (Exception e) {
            e.printStackTrace();
        }
        // read user configuration, if available
        String sUserConfig = System.getProperty("user.home")
               + System.getProperty("file.separator") + "writer2latex.xml";
        File f = new File(sUserConfig);
        if (f.exists()) read(sUserConfig);
        else if (createUserConfig()) write(sUserConfig);
    }

    public void read(String sFileName) {
        NewDOMDocument doc = new NewDOMDocument(sFileName,".xml");
        try {
            doc.read(new FileInputStream(sFileName));
        } catch(IOException e) {
            System.out.println("Oops - I cannot read the configuration file "+sFileName);
            e.printStackTrace();
            return; // give up and continue without the configuration
        }
        Document dom = doc.getContentDOM();
        if (dom==null) {
            System.out.println("Oops - I cannot understand the contents of the configuration file "+sFileName);
            return; // give up and continue without the configuration
        }
        Node root = dom.getDocumentElement();
        if (!root.hasChildNodes()) { return; }
        NodeList nl = root.getChildNodes();
        int nLen = nl.getLength();
        for (int i=0; i<nLen; i++) {
            Node child = nl.item(i);
            if (child.getNodeType()==Node.ELEMENT_NODE) {
                String sChildName = child.getNodeName();
                if (sChildName.equals("option")) {
                    String sName = Misc.getAttribute(child,"name");
                    String sValue = Misc.getAttribute(child,"value");
                    for (int j=0; j<OPTION_COUNT; j++) {
                        if (options[j].getName().equals(sName)) {
                            options[j].setString(sValue);
                            break;
                        }
                    }
                }
                else if (sChildName.equals("style-map")) {
                    String sName = Misc.getAttribute(child,"name");
                    String sClass = Misc.getAttribute(child,"class");
                    String sBefore = Misc.getAttribute(child,"before");
                    String sAfter = Misc.getAttribute(child,"after");
                    boolean bLineBreak = !"false".equals(Misc.getAttribute(child,"line-break"));
                    boolean bVerbatim = "true".equals(Misc.getAttribute(child,"verbatim"));
                    if ("paragraph".equals(sClass)) {
                        par.put(sName,sBefore,sAfter,bLineBreak,bVerbatim);
                    }
                    if ("paragraph-block".equals(sClass)) {
                        String sNext = Misc.getAttribute(child,"next");
                        parBlock.put(sName,sBefore,sAfter,sNext);
                    }
                    else if ("text".equals(sClass)) {
                        text.put(sName,sBefore,sAfter,false,bVerbatim);
                    }
                    else if ("list".equals(sClass)) {
                        list.put(sName,sBefore,sAfter);
                    }
                    else if ("listitem".equals(sClass)) {
                        listItem.put(sName,sBefore,sAfter);
                    }
                }
                else if (sChildName.equals("heading-map")) {
                    readHeadingMap(child);
                }
                else if (sChildName.equals("custom-preamble")) {
                    if (child.hasChildNodes()) {
                        NodeList subNl = child.getChildNodes();
                        int nSubLen = subNl.getLength();
                        for (int j=0; j<nSubLen; j++) {
                            Node subChild = subNl.item(j);
                            if (subChild.getNodeType()==Node.TEXT_NODE) {
                                customPreamble.add(subChild.getNodeValue());
                            }
                        }
                    }
                }
//                else if (sChildName.equals("xhtml-style-map")) {
//                    String sName = Misc.getAttribute(child,"name");
//                    String sClass = Misc.getAttribute(child,"class");
//                    String sBlockElement = Misc.getAttribute(child,"block-element");
//                    if (sBlockElement==null) { sBlockElement=""; }
//                    String sBlockCss = Misc.getAttribute(child,"block-css");
//                    if (sBlockCss==null) { sBlockCss="(none)"; }
//                    String sElement = Misc.getAttribute(child,"element");
//                    if (sElement==null) { sElement=""; }
//                    String sCss = Misc.getAttribute(child,"css");
//                    if (sCss==null) { sCss="(none)"; }
//                    if ("paragraph".equals(sClass)) {
//                        xpar.put(sName,sBlockElement,sBlockCss,sElement,sCss);
//                    }
//                    else if ("text".equals(sClass)) {
//                        xtext.put(sName,sBlockElement,sBlockCss,sElement,sCss);
//                    }
//                    else if ("frame".equals(sClass)) {
//                        xframe.put(sName,sBlockElement,sBlockCss,sElement,sCss);
//                    }
//                    else if ("list".equals(sClass)) {
//                        xlist.put(sName,sBlockElement,sBlockCss,sElement,sCss);
//                    }
//                    else if ("attribute".equals(sClass)) {
//                        xattr.put(sName,sBlockElement,sBlockCss,sElement,sCss);
//                    }
//                }
                else if (sChildName.equals("math-symbol-map")) {
                    String sName = Misc.getAttribute(child,"name");
                    String sLatex = Misc.getAttribute(child,"latex");
                    mathSymbols.put(sName,sLatex);
                }
            }
        }
    }

    public void readHeadingMap(Node node) {
        int nMaxLevel = Misc.getPosInteger(Misc.getAttribute(node,"max-level"),0);
        headingMap.reset(nMaxLevel);
        NodeList nl = node.getChildNodes();
        int nLen = nl.getLength();
        for (int i=0; i<nLen; i++) {
            Node child = nl.item(i);
            if (child.getNodeType()==Node.ELEMENT_NODE) {
                String sChildName = child.getNodeName();
                if (sChildName.equals("heading-level-map")) {
                    int nWriterLevel = Misc.getPosInteger(Misc.getAttribute(child,"writer-level"),1);
                    String sName = Misc.getAttribute(child,"name");
                    int nLevel = Misc.getPosInteger(Misc.getAttribute(child,"level"),0);
                    headingMap.setLevelData(nWriterLevel,sName,nLevel);
                }
            }
        }
    }

    public void write(String sFileName) {
        NewDOMDocument doc = new NewDOMDocument(sFileName,".xml");
        Document dom = null;
        try {
            DocumentBuilderFactory builderFactory = DocumentBuilderFactory.newInstance();
            DocumentBuilder builder = builderFactory.newDocumentBuilder();
            DOMImplementation domImpl = builder.getDOMImplementation();
            dom = domImpl.createDocument("","config",null);
        }
        catch (Throwable t) {
            t.printStackTrace();
        }
        Element rootElement = dom.getDocumentElement();

        for (int i=0; i<OPTION_COUNT; i++) {
            Element optionNode = dom.createElement("option");
            optionNode.setAttribute("name",options[i].getName());
            optionNode.setAttribute("value",options[i].getString());
            rootElement.appendChild(optionNode);
        }

        // Write math symbol map
        Enumeration msEnum = mathSymbols.keys();
        while (msEnum.hasMoreElements()) {
            String sName = (String) msEnum.nextElement();
            String sLatex = (String) mathSymbols.get(sName);
            Element msNode = dom.createElement("math-symbol-map");
            msNode.setAttribute("name",sName);
            msNode.setAttribute("latex",sLatex);
            rootElement.appendChild(msNode);
        }

        writeStyleMap(dom,par,"paragraph");
        writeStyleMap(dom,parBlock,"paragraph-block");
        writeStyleMap(dom,text,"text");
        writeStyleMap(dom,list,"list");
        writeStyleMap(dom,listItem,"listitem");

        Element hmNode = dom.createElement("heading-map");
        hmNode.setAttribute("max-level",Integer.toString(headingMap.getMaxLevel()));
        rootElement.appendChild(hmNode);
        for (int i=1; i<=headingMap.getMaxLevel(); i++) {
            Element hlmNode = dom.createElement("heading-level-map");
            hlmNode.setAttribute("writer-level",Integer.toString(i));
            hlmNode.setAttribute("name",headingMap.getName(i));
            hlmNode.setAttribute("level",Integer.toString(headingMap.getLevel(i)));
            hmNode.appendChild(hlmNode);
        }


//        writeXStyleMap(dom,xpar,"paragraph");
//        writeXStyleMap(dom,xtext,"text");
//        writeXStyleMap(dom,xlist,"list");
//        writeXStyleMap(dom,xframe,"frame");
//        writeXStyleMap(dom,xframe,"attribute");

        writeContent(dom,customPreamble,"custom-preamble");

        doc.setContentDOM(dom);
        try {
            doc.write(new FileOutputStream(sFileName));
        } catch(IOException e) {
            System.out.println("Oops - problem writing the configuration");
            e.printStackTrace();
        }
    }

    // Common options
    public boolean createUserConfig() { return ((BooleanOption) options[CREATE_USER_CONFIG]).getValue(); }
    public boolean debug() { return ((BooleanOption) options[DEBUG]).getValue(); }

    // General options
    public String getDocumentclass() { return options[DOCUMENTCLASS].getString(); }
    public String getGlobalOptions() { return options[GLOBAL_OPTIONS].getString(); }
    public int getBackend() { return ((IntegerOption) options[BACKEND]).getValue(); }
    public int getInputencoding() { return ((IntegerOption) options[INPUTENCODING]).getValue(); }
    public boolean multilingual() { return ((BooleanOption) options[MULTILINGUAL]).getValue(); }
    public boolean greekMath() { return ((BooleanOption) options[GREEK_MATH]).getValue(); }

    // Package options
    public boolean noPreamble() { return ((BooleanOption) options[NO_PREAMBLE]).getValue(); }
    public boolean useOoomath() { return ((BooleanOption) options[USE_OOOMATH]).getValue(); }
    public boolean usePifont() { return ((BooleanOption) options[USE_PIFONT]).getValue(); }
    public boolean useIfsym() { return ((BooleanOption) options[USE_IFSYM]).getValue(); }
    public boolean useWasysym() { return ((BooleanOption) options[USE_WASYSYM]).getValue(); }
    public boolean useBbding() { return ((BooleanOption) options[USE_BBDING]).getValue(); }
    public boolean useEurosym() { return ((BooleanOption) options[USE_EUROSYM]).getValue(); }
    public boolean useTipa() { return ((BooleanOption) options[USE_TIPA]).getValue(); }
    public boolean useColor() { return ((BooleanOption) options[USE_COLOR]).getValue(); }
    public boolean useHyperref() { return ((BooleanOption) options[USE_HYPERREF]).getValue(); }
    public boolean useEndnotes() { return ((BooleanOption) options[USE_ENDNOTES]).getValue(); }
    public boolean useUlem() { return ((BooleanOption) options[USE_ULEM]).getValue(); }
    public boolean useLastpage() { return ((BooleanOption) options[USE_LASTPAGE]).getValue(); }
    public boolean useBibtex() { return ((BooleanOption) options[USE_BIBTEX]).getValue(); }
    public String bibtexStyle() { return options[BIBTEX_STYLE].getString(); }

    // Formatting options
    public int formatting() { return ((IntegerOption) options[FORMATTING]).getValue(); }
    public int pageFormatting() { return ((IntegerOption) options[PAGE_FORMATTING]).getValue(); }
    public boolean ignoreHardPageBreaks() { return ((BooleanOption) options[IGNORE_HARD_PAGE_BREAKS]).getValue(); }
    public boolean ignoreHardLineBreaks() { return ((BooleanOption) options[IGNORE_HARD_LINE_BREAKS]).getValue(); }
    public boolean ignoreEmptyParagraphs() { return ((BooleanOption) options[IGNORE_EMPTY_PARAGRAPHS]).getValue(); }
    public boolean ignoreDoubleSpaces() { return ((BooleanOption) options[IGNORE_DOUBLE_SPACES]).getValue(); }

//    // XHTML options
//    public boolean xhtmlNoDoctype() { return ((BooleanOption) options[XHTML_NO_DOCTYPE]).getValue(); }
//    public String xhtmlCustomStylesheet() { return options[XHTML_CUSTOM_STYLESHEET].getString(); }
//    public boolean xhtmlIgnoreStyles() { return ((BooleanOption) options[XHTML_IGNORE_STYLES]).getValue(); }
//    public boolean xhtmlUseDublinCore() { return ((BooleanOption) options[XHTML_USE_DUBLIN_CORE]).getValue(); }
//    public boolean xhtmlConvertToPx() { return ((BooleanOption) options[XHTML_CONVERT_TO_PX]).getValue(); }
//    public String getXhtmlScaling() { return options[XHTML_SCALING].getString(); }
//    public String getXhtmlColumnScaling() { return options[XHTML_COLUMN_SCALING].getString(); }
//    public int getXhtmlSplitLevel() { return ((IntegerOption) options[XHTML_SPLIT_LEVEL]).getValue(); }
//    public boolean xhtmlCalcSplit() { return ((BooleanOption) options[XHTML_CALC_SPLIT]).getValue(); }

    public Hashtable getMathSymbols() { return mathSymbols; }

    public StyleMap getParStyleMap() { return par; }
    public StyleMap getParBlockStyleMap() { return parBlock; }
    public StyleMap getTextStyleMap() { return text; }
    public StyleMap getListStyleMap() { return list; }
    public StyleMap getListItemStyleMap() { return listItem; }
    public HeadingMap getHeadingMap() { return headingMap; }
    public LinkedList getCustomPreamble() { return customPreamble; }

//    public XhtmlStyleMap getXParStyleMap() { return xpar; }
//    public XhtmlStyleMap getXTextStyleMap() { return xtext; }
//    public XhtmlStyleMap getXFrameStyleMap() { return xframe; }
//    public XhtmlStyleMap getXListStyleMap() { return xlist; }
//    public XhtmlStyleMap getXAttrStyleMap() { return xattr; }

    private void writeStyleMap(Document dom, StyleMap sm, String sClass) {
        Enumeration smEnum = sm.getNames();
        while (smEnum.hasMoreElements()) {
            String sName = (String) smEnum.nextElement();
            Element smNode = dom.createElement("style-map");
            smNode.setAttribute("name",sName);
            smNode.setAttribute("class",sClass);
            smNode.setAttribute("before",sm.getBefore(sName));
            smNode.setAttribute("after",sm.getAfter(sName));
            if (sm.getNext(sName)!=null) {
                smNode.setAttribute("next",sm.getNext(sName));
            }
            if (!sm.getLineBreak(sName)) {
                smNode.setAttribute("line-break","false");
            }
            if (sm.getVerbatim(sName)) {
                smNode.setAttribute("verbatim","true");
            }
            dom.getDocumentElement().appendChild(smNode);
        }
    }

//    private void writeXStyleMap(Document dom, XhtmlStyleMap sm, String sClass) {
//        Enumeration smEnum = sm.getNames();
//        while (smEnum.hasMoreElements()) {
//            String sName = (String) smEnum.nextElement();
//            Element smNode = dom.createElement("xhtml-style-map");
//            smNode.setAttribute("name",sName);
//          smNode.setAttribute("class",sClass);
//            smNode.setAttribute("element",sm.getElement(sName));
//            smNode.setAttribute("css",sm.getCss(sName));
//            String sBlockElement = sm.getBlockElement(sName);
//            if (sBlockElement!=null) { smNode.setAttribute("block-element",sm.getCss(sBlockElement)); }
//            String sBlockCss = sm.getBlockCss(sName);
//            if (sBlockCss!=null) { smNode.setAttribute("block-css",sm.getCss(sBlockCss)); }
//            dom.getDocumentElement().appendChild(smNode);
//        }
//    }
    private void writeContent(Document dom, LinkedList list, String sElement) {
        Element node = dom.createElement(sElement);
        int nLen = list.size();
        for (int i=0; i<nLen; i++) {
            node.appendChild( dom.createTextNode( (String) list.get(i) ) );
        }
        dom.getDocumentElement().appendChild(node);
    }

}

