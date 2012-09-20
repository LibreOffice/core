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

package com.sun.star.comp.xsltfilter;

// Standard Java classes
import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.PrintStream;
import java.io.StringReader;
import java.lang.ref.WeakReference;
import java.net.URL;
import java.net.URLConnection;
import java.util.Iterator;
import java.util.Map;
import java.util.HashMap;
import java.util.ArrayList;

// Imported TraX classes
import javax.xml.parsers.SAXParserFactory;
import javax.xml.transform.Source;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.URIResolver;
import javax.xml.transform.sax.SAXSource;
import javax.xml.transform.stream.StreamResult;
import javax.xml.transform.stream.StreamSource;
import org.xml.sax.EntityResolver;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;
import org.xml.sax.XMLReader;

//StarOffice Interfaces and UNO
import com.sun.star.beans.NamedValue;
import com.sun.star.comp.loader.FactoryHelper;
import com.sun.star.io.XActiveDataControl;
import com.sun.star.io.XActiveDataSink;
import com.sun.star.io.XActiveDataSource;
import com.sun.star.io.XInputStream;
import com.sun.star.io.XOutputStream;
import com.sun.star.io.XSeekable;
import com.sun.star.io.XStreamListener;
import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XServiceName;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.xml.xslt.XXSLTTransformer;

//Uno to java Adaptor
import com.sun.star.lib.uno.adapter.XInputStreamToInputStreamAdapter;
import com.sun.star.lib.uno.adapter.XOutputStreamToOutputStreamAdapter;
import javax.xml.transform.Templates;

import net.sf.saxon.FeatureKeys;

/** This outer class provides an inner class to implement the service
 * description and a method to instantiate the
 * component on demand (__getServiceFactory()).
 */
public class XSLTransformer
        implements XTypeProvider, XServiceName, XServiceInfo, XXSLTTransformer,
        URIResolver, EntityResolver {

    /**
     * This component provides java based XSL transformations
     * A SAX based interface is not feasible when crossing language bordes
     * since too much time would be wasted by bridging the events between environments
     * example: 190 pages document, 82000 events 8seconds transform 40(!) sec. bridging
     *
     */
    private XInputStream m_xis;
    private XOutputStream m_xos;    // private static HashMap templatecache;
    private static final String STATSPROP = "XSLTransformer.statsfile";
    private static PrintStream statsp;
    private String stylesheeturl;
    private String targeturl;
    private String targetbaseurl;
    private String sourceurl;
    private String sourcebaseurl;
    private String pubtype = new String();
    private String systype = new String();    // processing thread
    private Thread t;    // listeners
    private ArrayList<XStreamListener> listeners = new ArrayList<XStreamListener>();    //
    private XMultiServiceFactory svcfactory;    // cache for transformations by stylesheet
    private static Map<String,WeakReference<Transformation>> xsltReferences = new HashMap<String,WeakReference<Transformation>>();
    // struct for cached stylesheets
    private static class Transformation {

        public Templates cachedXSLT;
        public long lastmod;
    }
    // Resolve URIs to an empty source
    public Source resolve(String href, String base) {
        return new StreamSource(new StringReader(""));
    }

    public InputSource resolveEntity(String publicId, String systemId) throws SAXException, java.io.IOException {
        return new InputSource(new StringReader(""));
    }
    // --- Initialization ---
    public XSLTransformer(XMultiServiceFactory msf) {
        svcfactory = msf;
    }

    public void initialize(Object[] args) throws com.sun.star.uno.Exception {
        // some configurable debugging
        String statsfilepath = null;
        if ((statsfilepath = System.getProperty(STATSPROP)) != null) {
            try {
                File statsfile = new File(statsfilepath);
                statsp = new PrintStream(new FileOutputStream(statsfile.getPath(), false));
            } catch (java.lang.Exception e) {
                System.err.println("XSLTransformer: could not open statsfile'" + statsfilepath + "'");
                System.err.println("   " + e.getClass().getName() + ": " + e.getMessage());
                System.err.println("   output disabled");
            }
        }

        Object[] values = (Object[]) args[0];

        // reading the values
        NamedValue nv = null;
        debug("The transformation's parameters as 'name = value' pairs:\n");

        for (int i = 0; i < values.length; i++) {
            nv = (NamedValue) AnyConverter.toObject(new Type(NamedValue.class), values[i]);

            if (nv.Name != null && !nv.Name.equals("")) {
                debug(nv.Name + " = " + nv.Value);
            }

            if (nv.Name.equals("StylesheetURL")) {
                stylesheeturl = (String) AnyConverter.toObject(
                        new Type(String.class), nv.Value);
            } else if (nv.Name.equals("SourceURL")) {
                sourceurl = (String) AnyConverter.toObject(
                        new Type(String.class), nv.Value);
            } else if (nv.Name.equals("TargetURL")) {
                targeturl = (String) AnyConverter.toObject(
                        new Type(String.class), nv.Value);
            } else if (nv.Name.equals("SourceBaseURL")) {
                sourcebaseurl = (String) AnyConverter.toObject(
                        new Type(String.class), nv.Value);
            } else if (nv.Name.equals("TargetBaseURL")) {
                targetbaseurl = (String) AnyConverter.toObject(
                        new Type(String.class), nv.Value);
            } else if (nv.Name.equals("SystemType")) {
                systype = (String) AnyConverter.toObject(
                        new Type(String.class), nv.Value);
            } else if (nv.Name.equals("PublicType")) {
                pubtype = (String) AnyConverter.toObject(
                        new Type(String.class), nv.Value);
            }
        }
    }

    // --- XActiveDataSink        xistream = aStream;
    public void setInputStream(XInputStream aStream) {
        m_xis = aStream;
    }

    public com.sun.star.io.XInputStream getInputStream() {
        return m_xis;
    }

    // --- XActiveDataSource
    public void setOutputStream(XOutputStream aStream) {
        m_xos = aStream;
    }

    public com.sun.star.io.XOutputStream getOutputStream() {
        return m_xos;
    }

    // --- XActiveDataControl
    public void addListener(XStreamListener aListener) {
        if (aListener != null && !listeners.contains(aListener)) {
            listeners.add(aListener);
        }
    }

    public void removeListener(XStreamListener aListener) {
        if (aListener != null) {
            listeners.remove(aListener);
        }

    }

    public void start() {
        // notify listeners
        t = new Thread() {

            @Override
            public void run() {

                // Local variabes used outside try block in finally block
                InputStream is = null;
                Source source = null;
                BufferedOutputStream os = null;
                PrintStream origOut = System.out;
                PrintStream origErr = System.err;
                if (statsp != null) {
                    System.setErr(statsp);
                    System.setOut(statsp);
                }
                try {
                    debug("\n\nStarting transformation...");

                    // Set up context class loader for SAXParserFactory and
                    // TransformerFactory calls below:
                    setContextClassLoader(this.getClass().getClassLoader());

                    for (Iterator<XStreamListener> e = listeners.iterator(); e.hasNext();) {
                        XStreamListener l = e.next();
                        l.started();
                    }

                    XSeekable xseek = UnoRuntime.queryInterface(XSeekable.class, m_xis);
                    if (xseek != null) {
                        xseek.seek(0);
                    }

                    is = new BufferedInputStream(
                            new XInputStreamToInputStreamAdapter(m_xis));
                    //Source xmlsource = new StreamSource(xmlinput);
                    SAXParserFactory spf = SAXParserFactory.newInstance();
                    spf.setValidating(false);
                    spf.setNamespaceAware(true);
                    XMLReader xmlReader = spf.newSAXParser().getXMLReader();
                    xmlReader.setEntityResolver(XSLTransformer.this);
                    source = new SAXSource(xmlReader, new InputSource(is));

                    // in order to help performance and to remedy a a possible memory
                    // leak in xalan, where it seems, that Transformer instances cannot
                    // be reclaimed though they are no longer referenced here, we use
                    // a cache of weak references (ie. xsltReferences) created for specific
                    // style sheet URLs see also #i48384#

                    Templates xsltTemplate = null;
                    Transformer transformer = null;
                    Transformation transformation = null;
                    // File stylefile = new File(new URI(stylesheeturl));
                    long lastmod = 0;
                    try {
                        URL uStyle = new URL(stylesheeturl);
                        URLConnection c = uStyle.openConnection();
                        lastmod = c.getLastModified();
                    } catch (java.lang.Exception ex) {
                        // lastmod will remain at 0;
                        if (statsp != null) {
                            statsp.println(ex.getClass().getName() + ": " + ex.getMessage());
                            ex.printStackTrace(statsp);
                        }
                    }

                    synchronized (xsltReferences) {
                        WeakReference<Transformation> ref = null;
                        // try to get the xsltTemplate reference from the cache
                        if ((ref = xsltReferences.get(stylesheeturl)) == null ||
                                (transformation = ref.get()) == null ||
                                ref.get().lastmod < lastmod) {
                            // we cannot find a valid reference for this stylesheet
                            // or the stylsheet was updated
                            if (ref != null) {
                                xsltReferences.remove(stylesheeturl);
                            }
                            // create new xsltTemplate for this stylesheet
                            TransformerFactory tfactory = TransformerFactory.newInstance();
                            debug("TransformerFactory is '" + tfactory.getClass().getName() + "'");
                // some external saxons (Debian, Ubuntu, ...) have this disabled
                // per default
                tfactory.setAttribute(FeatureKeys.ALLOW_EXTERNAL_FUNCTIONS, new Boolean(true));
                            xsltTemplate = tfactory.newTemplates(new StreamSource(stylesheeturl));

                            // store the transformation into the cache
                            transformation = new Transformation();
                            transformation.lastmod = lastmod;
                            transformation.cachedXSLT = xsltTemplate;
                            ref = new WeakReference<Transformation>(transformation);
                            xsltReferences.put(stylesheeturl, ref);
                        }
                    }
                    xsltTemplate = transformation.cachedXSLT;
                            transformer = xsltTemplate.newTransformer();
                            transformer.setOutputProperty("encoding", "UTF-8");
                            // transformer.setURIResolver(XSLTransformer.this);

                    // invalid to set 'null' as parameter as 'null' is not a valid Java object
                    if (sourceurl != null) {
                        transformer.setParameter("sourceURL", sourceurl);
                    }
                    if (sourcebaseurl != null) {
                        transformer.setParameter("sourceBaseURL", sourcebaseurl);
                    }
                    if (targeturl != null) {
                        transformer.setParameter("targetURL", targeturl);
                    }
                    if (targetbaseurl != null) {
                        transformer.setParameter("targetBaseURL", targetbaseurl);
                    }
                    if (pubtype != null) {
                        transformer.setParameter("publicType", pubtype);
                    }
                    if (systype != null) {
                        transformer.setParameter("systemType", systype);
                    }
                    if (svcfactory != null) {
                        transformer.setParameter("XMultiServiceFactory", svcfactory);
                    }
                    os = new BufferedOutputStream(
                            new XOutputStreamToOutputStreamAdapter(m_xos));
                    StreamResult sr = new StreamResult(os);
                    long tstart = System.currentTimeMillis();
                    transformer.transform(source, sr);
                    debug("finished transformation in " + (System.currentTimeMillis() - tstart) + "ms");

                } catch (java.lang.Throwable ex) {
                    // notify any listeners about close
                    for (Iterator<XStreamListener> e = listeners.iterator(); e.hasNext();) {

                        XStreamListener l = e.next();
                        l.error(new com.sun.star.uno.Exception(ex.getClass().getName() + ": " + ex.getMessage()));
                    }
                    if (statsp != null) {
                        statsp.println(ex.getClass().getName() + ": " + ex.getMessage());
                        ex.printStackTrace(statsp);
                    }
                } finally {
                    // dereference input buffer
                    source = null;
                    try {
                        if (is != null) {
                            is.close();
                        }
                    } catch (java.lang.Throwable ex) {
                        if (statsp != null) {
                            statsp.println(ex.getClass().getName() + ": " + ex.getMessage());
                            ex.printStackTrace(statsp);
                        }
                    }
                    try {
                        if (os != null) {
                            os.close();
                        }
                    } catch (java.lang.Throwable ex) {
                        if (statsp != null) {
                            statsp.println(ex.getClass().getName() + ": " + ex.getMessage());
                            ex.printStackTrace(statsp);
                        }
                    }
                    try {
                        if (m_xis != null) {
                            m_xis.closeInput();
                        }
                    } catch (java.lang.Throwable ex) {
                        if (statsp != null) {
                            statsp.println(ex.getClass().getName() + ": " + ex.getMessage());
                            ex.printStackTrace(statsp);
                        }
                    }
                    try {
                        if (m_xos != null) {
                            m_xos.closeOutput();
                        }
                    } catch (java.lang.Throwable ex) {
                        if (statsp != null) {
                            statsp.println(ex.getClass().getName() + ": " + ex.getMessage());
                            ex.printStackTrace(statsp);
                        }
                    }

                    // resetting standard input/error streams from logfile to default
                    if (statsp != null) {
                        System.setErr(origErr);
                        System.setOut(origOut);
                    }
                    // try to release references asap...
                    m_xos = null;
                    m_xis = null;
                    is = null;
                    os = null;
                    // notify any listeners about close
                    if (listeners != null) {
                        for (Iterator<XStreamListener> e = listeners.iterator(); e.hasNext();) {
                            XStreamListener l = e.next();
                            l.closed();
                        }
                    }
                }
            }
        };
        t.start();
    } /* a statsfile have to be created as precondition to use this function */


    private static final void debug(String s) {
        if (statsp != null) {
            statsp.println(s);
        }
    }

    public void terminate() {
        try {
            debug("terminate called");
            if (t.isAlive()) {
                t.interrupt();
                for (Iterator<XStreamListener> e = listeners.iterator(); e.hasNext();) {
                    XStreamListener l = e.next();
                    l.terminated();
                }
            }
        } catch (java.lang.Exception ex) {
            if (statsp != null) {
                statsp.println(ex.getClass().getName() + ": " + ex.getMessage());
                ex.printStackTrace(statsp);
            }
        }
    }    // --- component management interfaces... ---
    private final static String _serviceName = "com.sun.star.xml.xslt.XSLT2Transformer";

    // Implement methods from interface XTypeProvider
    public byte[] getImplementationId() {
        byte[] byteReturn = {};
        byteReturn = new String("" + this.hashCode()).getBytes();
        return (byteReturn);
    }

    public com.sun.star.uno.Type[] getTypes() {
        Type[] typeReturn = {};
        try {
            typeReturn = new Type[]{
                        new Type(XTypeProvider.class),
                        new Type(XServiceName.class),
                        new Type(XServiceInfo.class),
                        new Type(XActiveDataSource.class),
                        new Type(XActiveDataSink.class),
                        new Type(XActiveDataControl.class),
                        new Type(XInitialization.class)
                    };
        } catch (java.lang.Exception exception) {
        }

        return (typeReturn);
    }

    // --- Implement method from interface XServiceName ---
    public String getServiceName() {
        return (_serviceName);
    }

    // --- Implement methods from interface XServiceInfo ---
    public boolean supportsService(String stringServiceName) {
        return (stringServiceName.equals(_serviceName));
    }

    public String getImplementationName() {
        return (XSLTransformer.class.getName());
    }

    public String[] getSupportedServiceNames() {
        String[] stringSupportedServiceNames = {_serviceName};
        return stringSupportedServiceNames;
    }

    // --- component registration methods ---
    public static XSingleServiceFactory __getServiceFactory(
            String implName, XMultiServiceFactory multiFactory, XRegistryKey regKey) {
        XSingleServiceFactory xSingleServiceFactory = null;
        if (implName.equals(XSLTransformer.class.getName())) {
            xSingleServiceFactory = FactoryHelper.getServiceFactory(XSLTransformer.class,
                    _serviceName, multiFactory, regKey);
        }
        return xSingleServiceFactory;
    }
}
