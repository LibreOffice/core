/************************************************************************
 *
 *  ConverterPalette.java
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
 *  Version 0.3.3g (2004-11-04)
 *
 */

package writer2latex.latex;

import org.w3c.dom.Document;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Element;

import java.io.File;
import java.io.OutputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.util.zip.ZipException;
import java.util.LinkedList;
import java.util.Hashtable;
import java.util.Vector;
import java.util.Enumeration;
import java.net.URLDecoder;

import org.openoffice.xmerge.ConvertData;
import org.openoffice.xmerge.converter.xml.EmbeddedObject;
import org.openoffice.xmerge.converter.xml.EmbeddedBinaryObject;
import org.openoffice.xmerge.converter.xml.EmbeddedXMLObject;
import org.openoffice.xmerge.converter.xml.sxw.SxwDocument;

import writer2latex.Application;
import writer2latex.util.*;
import writer2latex.xmerge.*;
import writer2latex.office.*;
import writer2latex.latex.style.*;
import writer2latex.latex.content.*;

/**
 *  <p>This class converts a Writer XML file to a LaTeX file<.</p>
 */
public final class ConverterPalette {
    // Basic data
    private Config config;
    private ConvertData convertData;
    private LaTeXDocument texDoc;
    private SxwDocument sxwDoc;
    private String sOutFileName;

    // Styles
    private I18n i18n;
    private WriterStyleCollection wsc = new WriterStyleCollection();
    private CharStyleConverter charSc;
    private ListStyleConverter listSc;
    private ParStyleConverter parSc;
    private PageStyleConverter pageSc;
    private SectionStyleConverter sectionSc;
    private TableStyleConverter tableSc;
    private NoteConfigurationConverter noteCc;

    // Content
    private BlockConverter blockCv;
    private IndexConverter indexCv;
    private NoteConverter noteCv;
    private InlineConverter inlineCv;
    private FieldConverter fieldCv;
    private DrawConverter drawCv;
    private MathmlConverter mathmlCv;
    private Info info;

    // Default context
    private Context mainContext;

    // Meta Data
    private MetaData metaData;

    // Graphics
    private ImageLoader imageLoader;

    // Constructor
    public ConverterPalette(SxwDocument sxwDoc, String sConfigFileName) {
        sOutFileName = sxwDoc.getName();
        config = new Config();
        // Read the specified configuration, if any
        if (sConfigFileName!=null) {
            File f = new File(sConfigFileName);
            if (f.exists()) {
                config.read(sConfigFileName);
            }
            else {
                System.out.println("Oops - the configuration file " + sConfigFileName + " does not exist!");
            }
        }

        this.sxwDoc = sxwDoc;
        convertData = new ConvertData();

        metaData = new MetaData(sxwDoc);

        // Load style info
        wsc.loadStylesFromDOM(sxwDoc.getStyleDOM(),sxwDoc.getContentDOM());

        i18n = new I18n(wsc,config,this);
        charSc = new CharStyleConverter(wsc,config,this);
        listSc = new ListStyleConverter(wsc,config,this);
        pageSc = new PageStyleConverter(wsc,config,this);
        parSc = new ParStyleConverter(wsc,config,this);
        sectionSc = new SectionStyleConverter(wsc,config,this);
        tableSc = new TableStyleConverter(wsc,config,this);
        noteCc = new NoteConfigurationConverter(wsc,config,this);

        blockCv = new BlockConverter(config,this);
        indexCv = new IndexConverter(config,this);
        noteCv = new NoteConverter(config,this);
        inlineCv = new InlineConverter(config,this);
        fieldCv = new FieldConverter(config,this);
        drawCv = new DrawConverter(config,this);
        mathmlCv = new MathmlConverter(config,this);
        info = new Info(config,this);

        if (config.debug()) {
            System.out.println();
            System.out.println("Loaded "+i18n.getCharCount()+" unicode symbols");
        }
    }

    public void setOutFileName(String s) {
        sOutFileName = Misc.trimDocumentName(s,".tex");
    }

    public String getOutFileName() { return sOutFileName; }

    public I18n getI18n() { return i18n; }
    public CharStyleConverter getCharSc() { return charSc; }
    public ParStyleConverter getParSc() { return parSc; }
    public ListStyleConverter getListSc() { return listSc; }
    public PageStyleConverter getPageSc() { return pageSc; }
    public SectionStyleConverter getSectionSc() { return sectionSc; }
    public TableStyleConverter getTableSc() { return tableSc; }
    public NoteConfigurationConverter getNoteCc() { return noteCc; }
    public BlockConverter getBlockCv() { return blockCv; }
    public IndexConverter getIndexCv() { return indexCv; }
    public NoteConverter getNoteCv() { return noteCv; }
    public InlineConverter getInlineCv() { return inlineCv; }
    public FieldConverter getFieldCv() { return fieldCv; }
    public DrawConverter getDrawCv() { return drawCv; }
    public MathmlConverter getMathmlCv() { return mathmlCv; }
    public Info getInfo() { return info; }

    public boolean isPackage() {
        return sxwDoc.getStyleDOM()==null;
    }

    public Context getMainContext() { return mainContext; }

    public MetaData getMetaData() { return metaData; }

    public ImageLoader getImageLoader() { return imageLoader; }

    public void addDocument(org.openoffice.xmerge.Document doc) { convertData.addDocument(doc); }

    public EmbeddedObject getEmbeddedObject(String sHref) {
        if (sHref.startsWith("#")) {
            // Embedded object in package
            sHref = sHref.substring(1);
            if (sHref.startsWith("./")) { sHref = sHref.substring(2); }
            return sxwDoc.getEmbeddedObject(sHref);
        }
        else {
            // TODO: Linked object
            return null;
        }
    }

    public ConvertData convert() {
        // Cannot initialize the imageloader in the constructor, because
        // sOutFileName may be changed later
        imageLoader = new ImageLoader(sxwDoc,sOutFileName,true);
        this.texDoc = new LaTeXDocument(sOutFileName);
        texDoc.setEncoding(I18n.writeJavaEncoding(config.getInputencoding()));

        Document styles = sxwDoc.getStyleDOM();
        Document doc = sxwDoc.getContentDOM();

        i18n.setDefaultLanguage(i18n.getMajorityLanguage());

        // Setup context.
        // The default language is specified in the default paragraph style:
        mainContext = new Context();
        mainContext.resetFormattingFromStyle(wsc.getDefaultParStyle());

        // Create main LaTeXDocumentPortions
        LaTeXDocumentPortion packages = new LaTeXDocumentPortion(false);
        LaTeXDocumentPortion declarations = new LaTeXDocumentPortion(false);
        LaTeXDocumentPortion body = new LaTeXDocumentPortion(true);

        // Traverse the body (in flat HTML there may be several bodies, but
        // the first one is the main body).
        NodeList list;
        list = doc.getElementsByTagName(XMLString.OFFICE_BODY);
        int nLen = list.getLength();
        if (nLen > 0) {
            Element node = (Element) list.item(0);
            prepass(node);
            blockCv.traverseBlockText(node,body,mainContext);
            noteCv.insertEndnotes(body);
        }

        // Add declarations from our helpers
        i18n.appendDeclarations(packages,declarations);
        charSc.appendDeclarations(packages,declarations);
        parSc.appendDeclarations(packages,declarations);
        listSc.appendDeclarations(packages,declarations);
        pageSc.appendDeclarations(packages,declarations);
        sectionSc.appendDeclarations(packages,declarations);
        tableSc.appendDeclarations(packages,declarations);
        noteCc.appendDeclarations(packages,declarations);

        blockCv.appendDeclarations(packages,declarations);
        indexCv.appendDeclarations(packages,declarations);
        noteCv.appendDeclarations(packages,declarations);
        inlineCv.appendDeclarations(packages,declarations);
        fieldCv.appendDeclarations(packages,declarations);
        drawCv.appendDeclarations(packages,declarations);
        mathmlCv.appendDeclarations(packages,declarations);

        // Add custom preamble
        LinkedList customPreamble = config.getCustomPreamble();
        int nCPLen = customPreamble.size();
        for (int i=0; i<nCPLen; i++) {
            declarations.append( (String) customPreamble.get(i) ).nl();
        }

        // Set \title (for \maketitle)
        createMeta("title",metaData.getTitle(),declarations);

        // Create options for documentclass
        CSVList docOptions = new CSVList(',');

        StyleWithProperties dpStyle = wsc.getDefaultParStyle();
        if (dpStyle!=null) {
            String s = dpStyle.getProperty(XMLString.FO_FONT_SIZE);
            if ("10pt".equals(s)) { docOptions.addValue("10pt"); }
            if ("11pt".equals(s)) { docOptions.addValue("11pt"); }
            if ("12pt".equals(s)) { docOptions.addValue("12pt"); }
        }

        // TODO: twoside option should be optional :-)
        if (config.pageFormatting()!=Config.IGNORE_ALL) { docOptions.addValue("twoside"); }

        // Temp solution. TODO: Fix when new CSVList is implemented
        if (config.getGlobalOptions().length()>0) {
            docOptions.addValue(config.getGlobalOptions());
        }

        // Assemble the document
        LaTeXDocumentPortion result = texDoc.getContents();

        if (!config.noPreamble()) {
            // Create document class declaration
            result.append("% This file was converted to LaTeX by Writer2LaTeX ver. "+Application.getVersion()).nl()
                  .append("% see http://www.hj-gym.dk/~hj/writer2latex for more info").nl();
            result.append("\\documentclass");
            if (!docOptions.isEmpty()) {
                result.append("[").append(docOptions.toString()).append("]");
            }
            result.append("{").append(config.getDocumentclass()).append("}").nl();

            result.append(packages)
                  .append(declarations)
                  .append("\\begin{document}").nl();
        }

        result.append(body);

        if (!config.noPreamble()) {
            result.append("\\end{document}").nl();
        }

        // Return the converted data
        convertData.addDocument(texDoc);
        if (indexCv.getBibTeXDocument()!=null) {
            convertData.addDocument(indexCv.getBibTeXDocument());
        }
        return convertData;
    }

    private void createMeta(String sName, String sValue,LaTeXDocumentPortion ldp) {
        if (sValue==null) { return; }
        // Meta data is assumed to be in the default language:
        ldp.append("\\"+sName+"{"+i18n.convert(sValue,false,mainContext.getLang())+"}").nl();
    }

    /* Prepass. Ensures that we onl include \label and \hypertarget
       when it is actually needed.
       An exception is text:reference, where we always include the \label
    */
    private void prepass(Node node) {
        if (node.hasChildNodes()) {
            NodeList list = node.getChildNodes();
            int nLen = list.getLength();
            for (int i=0; i<nLen; i++) {
                Node child = list.item(i);
                if (child.getNodeType()==Node.ELEMENT_NODE) {
                    String sName = child.getNodeName();
                    if (sName.equals(XMLString.TEXT_FOOTNOTE_REF)) {
                        String sRefName = Misc.getAttribute(child,XMLString.TEXT_REF_NAME);
                        if (sRefName!=null) { noteCv.addFootnoteName(sRefName); }
                    }
                    else if (sName.equals(XMLString.TEXT_ENDNOTE_REF)) {
                        String sRefName = Misc.getAttribute(child,XMLString.TEXT_REF_NAME);
                        if (sRefName!=null) { noteCv.addEndnoteName(sRefName); }
                    }
                    else if (sName.equals(XMLString.TEXT_SEQUENCE_REF)) {
                        String sRefName = Misc.getAttribute(child,XMLString.TEXT_REF_NAME);
                        if (sRefName!=null) { fieldCv.addSequenceName(sRefName); }
                    }
                    else if (sName.equals(XMLString.TEXT_BOOKMARK_REF)) {
                        String sRefName = Misc.getAttribute(child,XMLString.TEXT_REF_NAME);
                        if (sRefName!=null) { fieldCv.addBookmarkName(sRefName); }
                    }
                    else if (sName.equals(XMLString.TEXT_A)) {
                        String sHref = Misc.getAttribute(child,XMLString.XLINK_HREF);
                        if (sHref!=null && sHref.startsWith("#")) {
                            fieldCv.addTargetName(sHref.substring(1));
                        }
                    }
                    prepass(child);
                }
            }
        }
    }

}