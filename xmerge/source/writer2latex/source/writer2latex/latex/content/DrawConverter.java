/************************************************************************
 *
 *  DrawConverter.java
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
 *  Version 0.3.3i (2004-12-14)
 *
 */

package writer2latex.latex.content;

import java.util.LinkedList;

import org.w3c.dom.Element;
import org.w3c.dom.Document;

import org.openoffice.xmerge.converter.xml.EmbeddedObject;
import org.openoffice.xmerge.converter.xml.EmbeddedXMLObject;

import writer2latex.util.Misc;
import writer2latex.util.Config;
import writer2latex.util.CSVList;
import writer2latex.xmerge.BinaryGraphicsDocument;
import writer2latex.office.XMLString;
import writer2latex.office.MIMETypes;
import writer2latex.office.ImageLoader;
import writer2latex.latex.Context;
import writer2latex.latex.LaTeXDocumentPortion;
import writer2latex.latex.ConverterHelper;
import writer2latex.latex.ConverterPalette;

/**
 *  <p>This class handles draw elements.</p>
 */
public class DrawConverter extends ConverterHelper {

    private boolean bNeedGraphicx = false;

    // Keep track of floating frames (images, textboxes...)
    private LinkedList floatingFrames = new LinkedList();

    public DrawConverter(Config config, ConverterPalette palette) {
        super(config,palette);
    }

    public void appendDeclarations(LaTeXDocumentPortion pack, LaTeXDocumentPortion decl) {
        if (bNeedGraphicx) {
            pack.append("\\usepackage");
            if (config.getBackend()==Config.PDFTEX) pack.append("[pdftex]");
            else if (config.getBackend()==Config.DVIPS) pack.append("[dvips]");
            pack.append("{graphicx}").nl();
        }
    }

    public void handleDrawElement(Element node, LaTeXDocumentPortion ldp, Context oc) {
        // node must be an elment in the draw namespace
        String sName = node.getTagName();
        if (sName.equals(XMLString.DRAW_OBJECT)) {
            handleDrawObject(node,ldp,oc);
        }
        else if ((!oc.isInHeaderFooter()) && sName.equals(XMLString.DRAW_IMAGE)) {
            handleDrawImage(node,ldp,oc);
        }
        else if ((!oc.isInHeaderFooter()) && sName.equals(XMLString.DRAW_TEXT_BOX)) {
            handleDrawTextBox(node,ldp,oc);
        }
        else if (sName.equals(XMLString.DRAW_A)) {
            // we handle this like text:a
            palette.getFieldCv().handleAnchor(node,ldp,oc);
        }
        else {
            // Other drawing objects (eg. shapes) are currently not supported
            ldp.append("[Warning: Draw object ignored]");
        }
    }

    //-----------------------------------------------------------------
    // handle draw:object elements (OOo objects such as Chart, Math,...)

    private void handleDrawObject(Element node, LaTeXDocumentPortion ldp, Context oc) {
        String sHref = node.getAttribute(XMLString.XLINK_HREF);
        if (sHref!=null && sHref.length()>0) { // embedded or linked object
            EmbeddedObject object = palette.getEmbeddedObject(sHref);
            if (object!=null) {
                if (MIMETypes.MATH.equals(object.getType())) {
                    try {
                        Document settings = ((EmbeddedXMLObject) object).getSettingsDOM();
                        Document formuladoc = ((EmbeddedXMLObject) object).getContentDOM();
                        Element formula = Misc.getChildByTagName(formuladoc,XMLString.MATH_MATH);
                        ldp.append(" $")
                           .append(palette.getMathmlCv().convert(settings,formula))
                           .append("$ ");
                    }
                    catch (org.xml.sax.SAXException e) {
                        e.printStackTrace();
                    }
                    catch (java.io.IOException e) {
                        e.printStackTrace();
                    }
                }
            }
        }
        else { // flat xml, object is contained in node
            Element formula = Misc.getChildByTagName(node,XMLString.MATH_MATH);
            if (formula!=null) {
                ldp.append(" $")
                   .append(palette.getMathmlCv().convert(null,formula))
                   .append("$ ");
            }
        }
    }

    //--------------------------------------------------------------------------
    // Handle draw:image elements

    private void handleDrawImage(Element node, LaTeXDocumentPortion ldp, Context oc) {
        String sName = node.getAttribute(XMLString.TEXT_NAME);
        palette.getFieldCv().addTarget(node,"|graphics",ldp);
        String sAnchor = node.getAttribute(XMLString.TEXT_ANCHOR_TYPE);
        if (oc.isInFrame() || "as-char".equals(sAnchor)) {
            handleDrawImageAsChar(node,ldp,oc);
        }
        else {
            floatingFrames.add(node);
        }
    }

    private void handleDrawImageAsChar(Element node, LaTeXDocumentPortion ldp, Context oc) {
        ldp.append(" ");
        includeGraphics(node,ldp,oc);
        ldp.append(" ");
    }

    private void handleDrawImageFloat(Element node, LaTeXDocumentPortion ldp, Context oc) {
        ldp.append("\\begin{center}").nl();
        includeGraphics(node,ldp,oc);
        ldp.nl().append("\\end{center}").nl();
    }

    private void includeGraphics(Element node, LaTeXDocumentPortion ldp, Context oc) {
        String sFileName = null;
        boolean bCommentOut = true;
        String sHref = node.getAttribute(XMLString.XLINK_HREF);
        if (sHref!=null && sHref.length()>0 && !sHref.startsWith("#")) {
            // Linked image is not yet handled by ImageLoader. This is a temp.
            // solution (will go away when ImageLoader is finished)
            sFileName = sHref;
            int nExtStart = sHref.lastIndexOf(".");
            String sExt = nExtStart>=0 ? sHref.substring(nExtStart).toLowerCase() : "";
            // Accept only relative filenames and supported filetypes:
            bCommentOut = sFileName.indexOf(":")>-1 || !(
                (config.getBackend()==config.PDFTEX && MIMETypes.JPEG_EXT.equals(sExt)) ||
                (config.getBackend()==config.PDFTEX && MIMETypes.PNG_EXT.equals(sExt)) ||
                (config.getBackend()==config.DVIPS && MIMETypes.EPS_EXT.equals(sExt)));
        }
        else { // embedded or base64 encoded image
            BinaryGraphicsDocument bgd = palette.getImageLoader().getImage(node);
            if (bgd!=null) {
                palette.addDocument(bgd);
                sFileName = bgd.getFileName();
                String sMIME = bgd.getDocumentMIMEType();
                bCommentOut = !(
                    (config.getBackend()==config.PDFTEX && MIMETypes.JPEG.equals(sMIME)) ||
                    (config.getBackend()==config.PDFTEX && MIMETypes.PNG.equals(sMIME)) ||
                    (config.getBackend()==config.DVIPS && MIMETypes.EPS.equals(sMIME)));
            }
        }

        if (sFileName==null) {
            ldp.append("[Warning: Image not found]");
            return;
        }

        // Now for the actual inclusion:
        bNeedGraphicx = true;
        /* TODO (0.4): handle cropping and mirror:
           style:mirror can be none, vertical (lodret), horizontal (vandret),
           horizontal-on-odd, or
           horizontal-on-even (vandret på ulige hhv. lige side).
              mirror is handled with scalebox, eg:
                %\\scalebox{-1}[1]{...}
           can check for even/odd page first!!

          fo:clip="rect(t,r,b,l) svarer til trim
          value can be auto - no clip!
          cropping is handled with clip and trim:
          \\includegraphics[clip,trim=l b r t]{...}
          note the different order from xsl-fo!
         */

        String sWidth = Misc.truncateLength(node.getAttribute(XMLString.SVG_WIDTH));
        String sHeight = Misc.truncateLength(node.getAttribute(XMLString.SVG_HEIGHT));
        if (bCommentOut) {
            ldp.append(" [Warning: Image ignored] ");
            ldp.append("% Unhandled or unsupported graphics:").nl().append("%");
        }
        ldp.append("\\includegraphics");

        CSVList options = new CSVList(',');
        if (sWidth!=null) { options.addValue("width="+sWidth); }
        if (sHeight!=null) { options.addValue("height="+sHeight); }
        if (!options.isEmpty()) {
            ldp.append("[").append(options.toString()).append("]");
        }

        ldp.append("{").append(sFileName).append("}");
        if (bCommentOut) { ldp.nl(); }
    }

    //--------------------------------------------------------------------------
    // handle draw:text-box element

    private void handleDrawTextBox(Element node, LaTeXDocumentPortion ldp, Context oc) {
        String sName = node.getAttribute(XMLString.TEXT_NAME);
        palette.getFieldCv().addTarget(node,"|frame",ldp);
        String sAnchor = node.getAttribute(XMLString.TEXT_ANCHOR_TYPE);
        if (oc.isInFrame() || "as-char".equals(sAnchor)) {
            makeDrawTextBox(node, ldp, oc);
        }
        else {
            floatingFrames.add(node);
        }
    }

    private void handleDrawTextBoxFloat(Element node, LaTeXDocumentPortion ldp, Context oc) {
        ldp.append("\\begin{center}").nl();
        makeDrawTextBox(node, ldp, oc);
        ldp.append("\\end{center}").nl();
    }

    private void makeDrawTextBox(Element node, LaTeXDocumentPortion ldp, Context oc) {
        Context ic = (Context) oc.clone();
        ic.setInFrame(true);
        ic.setNoFootnotes(true);

        String sWidth = Misc.truncateLength(node.getAttribute(XMLString.SVG_WIDTH));
        ldp.append("\\begin{minipage}{").append(sWidth).append("}").nl();
        palette.getBlockCv().traverseBlockText(node,ldp,ic);
        ldp.append("\\end{minipage}");
        if (!oc.isNoFootnotes()) { palette.getNoteCv().flushFootnotes(ldp,oc); }
    }

    //-------------------------------------------------------------------------
    //handle any pending floating frames

    public void flushFloatingFrames(LaTeXDocumentPortion ldp, Context oc) {
        // todo: fix language
        if (oc.isInFrame()) { return; }
        int n = floatingFrames.size();
        if (n==0) { return; }
        for (int i=0; i<n; i++) {
            Element node = (Element) floatingFrames.get(i);
            String sName = node.getNodeName();
            if (sName.equals(XMLString.DRAW_IMAGE)) {
                handleDrawImageFloat(node,ldp,oc);
            }
            else if (sName.equals(XMLString.DRAW_TEXT_BOX)) {
                handleDrawTextBoxFloat(node,ldp,oc);
            }
        }
        floatingFrames.clear();
    }

}