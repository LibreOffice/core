/************************************************************************
 *
 *  WriterStyleCollection.java
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
 *  Version 0.3.3f (2004-09-26)
 *
 */

package writer2latex.office;

import java.util.Hashtable;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.w3c.dom.Document;
import writer2latex.util.Misc;

/** <p> Class representing the collection of all the styles (read only) in
  * an OOo Writer document - slightly misnaned since it also covers Calc documents :-)</p>
  */
public class WriterStyleCollection {

    // Font declarations
    private OfficeStyleFamily font = new OfficeStyleFamily(FontDeclaration.class);

    // Styles
    private OfficeStyleFamily text = new OfficeStyleFamily(StyleWithProperties.class);
    private OfficeStyleFamily par = new OfficeStyleFamily(ParStyle.class);
    private OfficeStyleFamily section = new OfficeStyleFamily(SectionStyle.class);
    private OfficeStyleFamily table = new OfficeStyleFamily(StyleWithProperties.class);
    private OfficeStyleFamily column = new OfficeStyleFamily(StyleWithProperties.class);
    private OfficeStyleFamily row = new OfficeStyleFamily(StyleWithProperties.class);
    private OfficeStyleFamily cell = new OfficeStyleFamily(StyleWithProperties.class);
    private OfficeStyleFamily frame = new OfficeStyleFamily(StyleWithProperties.class);
    private OfficeStyleFamily list = new OfficeStyleFamily(ListStyle.class);
    private OfficeStyleFamily pageMaster = new OfficeStyleFamily(PageMaster.class);
    private OfficeStyleFamily masterPage = new OfficeStyleFamily(MasterPage.class);
    private String sFirstMasterPageName = null;

    // Document-wide styles
    private ListStyle outline = new ListStyle();
    private PropertySet footnotes = null;
    private PropertySet endnotes = null;

    public OfficeStyleFamily getFontDeclarations() { return font; }
    public FontDeclaration getFontDeclaration(String sName) {
        return (FontDeclaration) font.getStyle(sName);
    }

    public OfficeStyleFamily getTextStyles() { return text; }
    public StyleWithProperties getTextStyle(String sName) {
        return (StyleWithProperties) text.getStyle(sName);
    }

    public OfficeStyleFamily getParStyles() { return par; }
    public ParStyle getParStyle(String sName) {
        return (ParStyle) par.getStyle(sName);
    }
    public ParStyle getDefaultParStyle() {
        return (ParStyle) par.getDefaultStyle();
    }

    public OfficeStyleFamily getSectionStyles() { return section; }
    public SectionStyle getSectionStyle(String sName) {
        return (SectionStyle) section.getStyle(sName);
    }

    public OfficeStyleFamily getTableStyles() { return table; }
    public StyleWithProperties getTableStyle(String sName) {
        return (StyleWithProperties) table.getStyle(sName);
    }
    public OfficeStyleFamily getColumnStyles() { return column; }
    public StyleWithProperties getColumnStyle(String sName) {
        return (StyleWithProperties) column.getStyle(sName);
    }

    public OfficeStyleFamily getRowStyles() { return row; }
    public StyleWithProperties getRowStyle(String sName) {
        return (StyleWithProperties) row.getStyle(sName);
    }

    public OfficeStyleFamily getCellStyles() { return cell; }
    public StyleWithProperties getCellStyle(String sName) {
        return (StyleWithProperties) cell.getStyle(sName);
    }
    public StyleWithProperties getDefaultCellStyle() {
        return (StyleWithProperties) cell.getDefaultStyle();
    }

    public OfficeStyleFamily getFrameStyles() { return frame; }
    public StyleWithProperties getFrameStyle(String sName) {
        return (StyleWithProperties) frame.getStyle(sName);
    }
    public StyleWithProperties getDefaultFrameStyle() {
        return (StyleWithProperties) frame.getDefaultStyle();
    }

    public OfficeStyleFamily getListStyles() { return list; }
    public ListStyle getListStyle(String sName) {
        return (ListStyle) list.getStyle(sName);
    }

    public OfficeStyleFamily getPageMasters() { return pageMaster; }
    public PageMaster getPageMaster(String sName) {
        return (PageMaster) pageMaster.getStyle(sName);
    }

    public OfficeStyleFamily getMasterPages() { return masterPage; }
    public MasterPage getMasterPage(String sName) {
        return (MasterPage) masterPage.getStyle(sName);
    }
    public String getFirstMasterPageName() {
        return sFirstMasterPageName;
    }

    public ListStyle getOutlineStyle() { return outline; }

    public PropertySet getFootnotesConfiguration() { return footnotes; }

    public PropertySet getEndnotesConfiguration() { return endnotes; }

    private void loadStylesFromDOM(Node node, boolean bAllParagraphsAreSoft) {
        // node should be office:master-styles, office:styles or office:automatic-styles
        boolean bAutomatic = XMLString.OFFICE_AUTOMATIC_STYLES.equals(node.getNodeName());
        if (node.hasChildNodes()){
            NodeList nl = node.getChildNodes();
            int nLen = nl.getLength();
            for (int i = 0; i < nLen; i++ ) {
                Node child=nl.item(i);
                if (child.getNodeType()==Node.ELEMENT_NODE){
                    if (child.getNodeName().equals(XMLString.STYLE_STYLE)){
                        String sFamily = Misc.getAttribute(child,XMLString.STYLE_FAMILY);
                        if ("text".equals(sFamily)){
                            text.loadStyleFromDOM(child,bAutomatic);
                        }
                        else if ("paragraph".equals(sFamily)){
                            par.loadStyleFromDOM(child,bAutomatic && !bAllParagraphsAreSoft);
                        }
                        else if ("section".equals(sFamily)){
                            section.loadStyleFromDOM(child,bAutomatic);
                        }
                        else if ("table".equals(sFamily)){
                            table.loadStyleFromDOM(child,bAutomatic);
                        }
                        else if ("table-column".equals(sFamily)){
                            column.loadStyleFromDOM(child,bAutomatic);
                        }
                        else if ("table-row".equals(sFamily)){
                            row.loadStyleFromDOM(child,bAutomatic);
                        }
                        else if ("table-cell".equals(sFamily)){
                            cell.loadStyleFromDOM(child,bAutomatic);
                        }
                        else if ("graphics".equals(sFamily)){
                            frame.loadStyleFromDOM(child,bAutomatic);
                        }
                    }
                    else if (child.getNodeName().equals(XMLString.STYLE_PAGE_MASTER)) {
                        pageMaster.loadStyleFromDOM(child,bAutomatic);
                    }
                    else if (child.getNodeName().equals(XMLString.STYLE_MASTER_PAGE)) {
                        masterPage.loadStyleFromDOM(child,bAutomatic);
                        if (sFirstMasterPageName==null) {
                            sFirstMasterPageName = Misc.getAttribute(child,XMLString.STYLE_NAME);
                        }
                    }
                    else if (child.getNodeName().equals(XMLString.TEXT_LIST_STYLE)) {
                        list.loadStyleFromDOM(child,bAutomatic);
                    }
                    else if (child.getNodeName().equals(XMLString.TEXT_OUTLINE_STYLE)) {
                        outline.loadStyleFromDOM(child);
                    }
                    else if (child.getNodeName().equals(XMLString.STYLE_DEFAULT_STYLE)){
                        String sFamily = Misc.getAttribute(child,XMLString.STYLE_FAMILY);
                        if ("paragraph".equals(sFamily)) {
                            ParStyle defaultPar = new ParStyle();
                            defaultPar.loadStyleFromDOM(child);
                            par.setDefaultStyle(defaultPar);
                        }
                        else if ("graphics".equals(sFamily)) {
                            StyleWithProperties defaultFrame = new StyleWithProperties();
                            defaultFrame.loadStyleFromDOM(child);
                            frame.setDefaultStyle(defaultFrame);
                        }
                        else if ("table-cell".equals(sFamily)) {
                            StyleWithProperties defaultCell = new StyleWithProperties();
                            defaultCell.loadStyleFromDOM(child);
                            cell.setDefaultStyle(defaultCell);
                        }
                    }
                }
            }
        }
    }

    public void loadStylesFromDOM(Document stylesDOM, Document contentDOM){
        loadStylesFromDOM(stylesDOM, contentDOM, false);
    }

    public void loadStylesFromDOM(Document stylesDOM, Document contentDOM, boolean bAllParagraphsAreSoft){
        // Flat xml: stylesDOM will be null and contentDOM contain everything
        // This is only the case for old versions of xmerge; newer versions
        // creates DOM for styles, content, meta and settings.
        NodeList list;

        // font declarations:
        if (stylesDOM==null) {
            list = contentDOM.getElementsByTagName(XMLString.OFFICE_FONT_DECLS);
        }
        else {
            list = stylesDOM.getElementsByTagName(XMLString.OFFICE_FONT_DECLS);
        }
        if (list.getLength()!=0) {
            Node node = list.item(0);
            if (node.hasChildNodes()){
                NodeList nl = node.getChildNodes();
                int nLen = nl.getLength();
                for (int i = 0; i < nLen; i++ ) {
                    Node child = nl.item(i);
                    if (child.getNodeType()==Node.ELEMENT_NODE){
                        if (child.getNodeName().equals(XMLString.STYLE_FONT_DECL)){
                            font.loadStyleFromDOM(child,false);
                        }
                    }
                }
            }
        }

        // soft formatting:
        if (stylesDOM==null) {
            list = contentDOM.getElementsByTagName(XMLString.OFFICE_STYLES);
        }
        else {
            list = stylesDOM.getElementsByTagName(XMLString.OFFICE_STYLES);
        }
        if (list.getLength()!=0) {
            loadStylesFromDOM(list.item(0),bAllParagraphsAreSoft);
        }

        // master styles:
        if (stylesDOM==null) {
            list = contentDOM.getElementsByTagName(XMLString.OFFICE_MASTER_STYLES);
        }
        else {
            list = stylesDOM.getElementsByTagName(XMLString.OFFICE_MASTER_STYLES);
        }
        if (list.getLength()!=0) {
            loadStylesFromDOM(list.item(0),bAllParagraphsAreSoft);
        }

        // hard formatting:
        // Load from styles.xml first. Problem: There may be name clashes
        // with automatic styles from content.xml
        if (stylesDOM!=null) {
            list = stylesDOM.getElementsByTagName(XMLString.OFFICE_AUTOMATIC_STYLES);
            if (list.getLength()!=0) {
                loadStylesFromDOM(list.item(0),bAllParagraphsAreSoft);
            }
        }
        list = contentDOM.getElementsByTagName(XMLString.OFFICE_AUTOMATIC_STYLES);
        if (list.getLength()!=0) {
            loadStylesFromDOM(list.item(0),bAllParagraphsAreSoft);
        }

        // footnotes configuration:
        if (stylesDOM==null) {
            list = contentDOM.getElementsByTagName(XMLString.TEXT_FOOTNOTES_CONFIGURATION);
        }
        else {
            list = stylesDOM.getElementsByTagName(XMLString.TEXT_FOOTNOTES_CONFIGURATION);
        }
        if (list.getLength()!=0) {
            footnotes = new PropertySet();
            footnotes.loadFromDOM(list.item(0));
        }

        // endnotes configuration:
        if (stylesDOM==null) {
            list = contentDOM.getElementsByTagName(XMLString.TEXT_ENDNOTES_CONFIGURATION);
        }
        else {
            list = stylesDOM.getElementsByTagName(XMLString.TEXT_ENDNOTES_CONFIGURATION);
        }
        if (list.getLength()!=0) {
            endnotes = new PropertySet();
            endnotes.loadFromDOM(list.item(0));
        }

    }

}