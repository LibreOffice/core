/*
 * AbstractExporter.java
 *
 * Created on 1. Oktober 2003, 16:12
 */

package com.sun.star.wizards.web.export;

import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XStorable;
import com.sun.star.io.IOException;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.wizards.common.Desktop;
import com.sun.star.wizards.common.FileAccess;
import com.sun.star.wizards.common.Properties;
import com.sun.star.wizards.document.OfficeDocument;
import com.sun.star.wizards.text.TextDocument;
import com.sun.star.wizards.web.data.CGArgument;
import com.sun.star.wizards.web.data.CGDocument;
import com.sun.star.wizards.web.data.CGExporter;
import com.sun.star.wizards.web.data.TypeDetection;

/**
 *
 * @author  rpiterman
 */
public abstract class AbstractExporter implements Exporter {
    protected CGExporter exporter;
    protected FileAccess fileAccess;

    protected void storeToURL(Object officeDocument, Properties props, String targetUrl, String filterName, PropertyValue[] filterData)
       throws IOException {

         props = new Properties();
         props.put("FilterName", filterName);

         if (filterData.length>0)
             props.put("FilterData", filterData);

         XStorable xs = ((XStorable)UnoRuntime.queryInterface(XStorable.class,officeDocument));
         PropertyValue[] o = props.getProperties();
         xs.storeToURL(targetUrl, o);
     }

     protected void storeToURL(Object officeDocument, String targetUrl, String filterName, PropertyValue[] filterData)
         throws IOException {

         storeToURL(officeDocument, new Properties(), targetUrl, filterName, filterData);
     }

     protected void storeToURL(Object officeDocument, String targetUrl, String filterName )
             throws IOException {

         storeToURL(officeDocument, new Properties(), targetUrl, filterName, new PropertyValue[0]);

     }

    protected String getArgument(String name, CGExporter p) {
            return ((CGArgument)p.cp_Arguments.getElement(name)).cp_Value;
    }

    protected Object openDocument(CGDocument doc, XMultiServiceFactory xmsf)
        throws com.sun.star.io.IOException
    {
        Object document = null;
        //open the document.
        try {
            XDesktop desktop = Desktop.getDesktop(xmsf);
            Properties props = new Properties();
            props.put("Hidden", Boolean.TRUE);
            document = (
                (XComponentLoader) UnoRuntime.queryInterface(
                    XComponentLoader.class,
                    desktop)).loadComponentFromURL(
                doc.cp_URL,
                "_blank",
                0,
                props.getProperties());
        } catch (com.sun.star.lang.IllegalArgumentException iaex) {
        }
        //try to get the number of pages in the document;
        try {
            pageCount(doc,document);
        }
        catch (Exception ex) {
            //Here i do nothing since pages is not *so* important.
        }
        return document;
    }

    protected void closeDocument(Object doc,XMultiServiceFactory xmsf) {
        OfficeDocument.dispose(
            xmsf,
            (XComponent) UnoRuntime.queryInterface(XComponent.class, doc));
    }

    private void pageCount(CGDocument doc, Object document) {
        if (doc.appType.equals(TypeDetection.WRITER_DOC))
            doc.pages = TextDocument.getPageCount(document);
        else if (doc.appType.equals(TypeDetection.IMPRESS_DOC))
            doc.pages = OfficeDocument.getSlideCount(document);
        else if (doc.appType.equals(TypeDetection.DRAW_DOC))
            doc.pages = OfficeDocument.getSlideCount(document);
    }

    public void init(CGExporter exporter_) {
        exporter = exporter_;
    }

    protected FileAccess getFileAccess(XMultiServiceFactory xmsf) {
        if ( fileAccess == null )
            try {
                fileAccess = new FileAccess(xmsf);
            }
            catch (Exception ex) {}
        return fileAccess;
    }

    protected void calcFileSize(CGDocument doc, String url, XMultiServiceFactory xmsf) {
        /*if the exporter exports to a
         * binary format, get the size of the destination.
         */
        if (exporter.cp_Binary)
            doc.sizeKB = getFileAccess(xmsf).getSize(url);

    }

}
