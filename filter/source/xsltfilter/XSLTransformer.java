/************************************************************************
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

//Standard Java classes
import java.util.*;
import java.io.*;

// Imported TraX classes
import javax.xml.transform.*;
import javax.xml.transform.stream.*;
import org.xml.sax.*;
import org.xml.sax.helpers.*;

//StarOffice Interfaces and UNO
import com.sun.star.uno.*;
import com.sun.star.lang.*;
import com.sun.star.comp.loader.FactoryHelper;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.io.*;
import com.sun.star.ucb.*;
import com.sun.star.beans.*;

//Uno to java Adaptor
import com.sun.star.lib.uno.adapter.*;

/** This outer class provides an inner class to implement the service
 * description, a method to instantiate the
 * component on demand (__getServiceFactory()), and a method to give
 * information about the component (__writeRegistryServiceInfo()).
 */
public class XSLTransformer
    implements XTypeProvider, XServiceName, XServiceInfo, XActiveDataSink,
        XActiveDataSource, XActiveDataControl, XInitialization

{

    /**
     * This component provides java based XSL transformations
     * A SAX based interface is not feasible when crossing language bordes
     * since too much time would be wasted by bridging the events between environments
     * example: 190 pages document, 82000 events 8seconds transform 40(!) sec. bridging
     *
     */

    private XInputStream xistream;
    private XOutputStream xostream;
    private BufferedOutputStream ostream;

    // private static HashMap templatecache;

    private static final int STREAM_BUFFER_SIZE = 4000;
    private static final String STATSPROP = "XSLTransformer.statsfile";
    private static PrintStream statsp;

    private String stylesheeturl;
    private String targeturl;
    private String targetbaseurl;
    private String sourceurl;
    private String sourcebaseurl;
    private String pubtype = new String();
    private String systype = new String();

    // processing thread
    private Thread t;

    // listeners
    private Vector listeners = new Vector();

    //
    private XMultiServiceFactory svcfactory;

    // --- Initialization ---

    public XSLTransformer(XMultiServiceFactory msf) {
        svcfactory = msf;
    }

    public void initialize(Object[] values) throws com.sun.star.uno.Exception {
        NamedValue nv = null;
        for (int i=0; i<values.length; i++)
        {
            nv = (NamedValue)AnyConverter.toObject(new Type(NamedValue.class), values[i]);
            if (nv.Name.equals("StylesheetURL"))
                stylesheeturl = (String)AnyConverter.toObject(
                    new Type(String.class), nv.Value);
            else if (nv.Name.equals("SourceURL"))
                sourceurl = (String)AnyConverter.toObject(
                    new Type(String.class), nv.Value);
            else if (nv.Name.equals("TargetURL"))
                targeturl = (String)AnyConverter.toObject(
                    new Type(String.class), nv.Value);
            else if (nv.Name.equals("SourceBaseURL"))
                sourcebaseurl = (String)AnyConverter.toObject(
                    new Type(String.class), nv.Value);
            else if (nv.Name.equals("TargetBaseURL"))
                targetbaseurl = (String)AnyConverter.toObject(
                    new Type(String.class), nv.Value);
            else if (nv.Name.equals("SystemType"))
                systype = (String)AnyConverter.toObject(
                    new Type(String.class), nv.Value);
            else if (nv.Name.equals("PublicType"))
                pubtype = (String)AnyConverter.toObject(
                    new Type(String.class), nv.Value);
        }

        // some configurable debugging
        String statsfilepath = null;
        if ((statsfilepath = System.getProperty(STATSPROP)) != null) {
            try {
                File statsfile = new File(statsfilepath);
                statsp = new PrintStream(new FileOutputStream(statsfile.getPath(), false));
            } catch (java.lang.Exception e)
            {
                System.err.println("XSLTransformer: could not open statsfile'"+statsfilepath+"'");
                System.err.println("   "+e.getClass().getName()+": "+e.getMessage());
                System.err.println("   output disabled");
            }
        }
    }

    // --- XActiveDataSink        xistream = aStream;
    public void setInputStream(XInputStream aStream)
    {
        xistream = aStream;
    }

    public com.sun.star.io.XInputStream getInputStream()
    {
        return xistream;
    }

    // --- XActiveDataSource
    public void setOutputStream(XOutputStream aStream)
    {
        xostream = aStream;
        ostream = new BufferedOutputStream(
            new XOutputStreamToOutputStreamAdapter(xostream), STREAM_BUFFER_SIZE);

    }
    public com.sun.star.io.XOutputStream getOutputStream()
    {
        return xostream;
    }

    // --- XActiveDataControl
    public void addListener(XStreamListener aListener)
    {
        if (aListener != null && !listeners.contains(aListener)) {
            listeners.add(aListener);
        }
    }

    public void removeListener(XStreamListener aListener)
    {
        if (aListener != null ) {
            listeners.removeElement(aListener);
        }

    }

    public void start()
    {
        // notify listeners
        t = new Thread(){
            public void run() {
                try {
                    if (statsp != null) statsp.println("starting transformation...");
                    for (Enumeration e = listeners.elements(); e.hasMoreElements();)
                    {
                        XStreamListener l = (XStreamListener)e.nextElement();
                        l.started();
                    }
                    StreamSource stylesource = new StreamSource(stylesheeturl);

                    // buffer input and modify doctype declaration
                    // remove any dtd references but keep localy defined
                    // entities

                    ByteArrayOutputStream bufstream = new ByteArrayOutputStream();
                    final int bsize = 2000;
                    int rbytes = 0;
                    byte[][] byteBuffer = new byte[1][bsize];
                    // rewind
                    XSeekable xseek = (XSeekable)UnoRuntime.queryInterface(XSeekable.class, xistream);
                    if (xseek != null) {
                        xseek.seek(0);
                    }
                    while ((rbytes = xistream.readSomeBytes(byteBuffer, bsize)) != 0)
                        bufstream.write(byteBuffer[0], 0, rbytes);

                    String xmlFile = bufstream.toString("UTF-8");
                    if (xmlFile.indexOf("<!DOCTYPE")!=-1){
                        String tag = xmlFile.substring(xmlFile.lastIndexOf("/")+1,
                                                       xmlFile.lastIndexOf(">"));
                        String entities = "";
                        if (xmlFile.indexOf("[",xmlFile.indexOf("<!DOCTYPE"))!=-1){
                            if (xmlFile.indexOf("[",xmlFile.indexOf("<!DOCTYPE")) <
                                xmlFile.indexOf(">",xmlFile.indexOf("<!DOCTYPE"))){
                                    entities = xmlFile.substring(
                                        xmlFile.indexOf("[",xmlFile.indexOf("<!DOCTYPE")),
                                        xmlFile.indexOf("]",xmlFile.indexOf("<!DOCTYPE"))+1);
                            }
                        }
                        String newDocType =
                            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<!DOCTYPE "
                            +tag+" "+entities+">";
                        xmlFile= xmlFile.substring(
                            xmlFile.indexOf("<"+tag,0),
                            xmlFile.lastIndexOf(">")+1);
                        xmlFile= newDocType.concat(xmlFile);
                    }
                    StreamSource xmlsource = new StreamSource(
                        new ByteArrayInputStream(xmlFile.getBytes("UTF-8")));

                    ByteArrayOutputStream resultbuf = new ByteArrayOutputStream();
                    StreamResult xmlresult = new StreamResult(resultbuf);
                    TransformerFactory tfactory = TransformerFactory.newInstance();
                    Transformer transformer = tfactory.newTransformer(stylesource);

                    transformer.setParameter("sourceURL", sourceurl);
                    transformer.setParameter("targetURL", targeturl);
                    transformer.setParameter("targetBaseURL", targetbaseurl);
                    transformer.setParameter("publicType", pubtype);
                    transformer.setParameter("systemType", systype);

                    long tstart = System.currentTimeMillis();
                    transformer.transform(xmlsource, xmlresult);
                    long time = System.currentTimeMillis() - tstart;
                    if (statsp != null) {
                        statsp.println("finished transformation in "+time+"ms");
                    }
                    String resultstring = resultbuf.toString();
                    xostream.writeBytes(resultbuf.toByteArray());
                    xostream.closeOutput();

                    // notify any listeners about close
                    for (Enumeration e = listeners.elements(); e.hasMoreElements();)
                    {
                        XStreamListener l = (XStreamListener)e.nextElement();
                        l.closed();
                    }

                } catch (java.lang.Throwable ex)
                {
                    // notify any listeners about close
                    for (Enumeration e = listeners.elements(); e.hasMoreElements();)
                    {
                        XStreamListener l = (XStreamListener)e.nextElement();
                        l.error(new com.sun.star.uno.Exception(ex.getClass().getName()+": "+ex.getMessage()));
                    }
                    if (statsp != null)
                    {
                            statsp.println(ex.getClass().getName()+": "+ex.getMessage());
                            ex.printStackTrace(statsp);
                    }
                }
            }
        };
        t.start();
    }

    public void terminate()
    {
        try {
            if (statsp != null){
                statsp.println("terminate called");
            }
            if(t.isAlive()){
                t.interrupt();
                for (Enumeration e = listeners.elements(); e.hasMoreElements();)
                {
                    XStreamListener l = (XStreamListener)e.nextElement();
                    l.terminated();
                }
            }
        } catch (java.lang.Exception ex) {
            if (statsp != null){
                statsp.println(ex.getClass().getName()+": "+ex.getMessage());
                ex.printStackTrace(statsp);
            }
        }
    }

    // --- component management interfaces... ---
    private final static String _serviceName = "com.sun.star.comp.JAXTHelper";

    // Implement methods from interface XTypeProvider
    public byte[] getImplementationId() {
        byte[] byteReturn = {};
        byteReturn = new String( "" + this.hashCode() ).getBytes();
        return( byteReturn );
    }

    public com.sun.star.uno.Type[] getTypes() {
        Type[] typeReturn = {};
        try {
            typeReturn = new Type[] {
            new Type( XTypeProvider.class ),
            new Type( XServiceName.class ),
            new Type( XServiceInfo.class ),
            new Type( XActiveDataSource.class ),
            new Type( XActiveDataSink.class ),
            new Type( XActiveDataControl.class ),
            new Type( XInitialization.class )
            };
        }
        catch( java.lang.Exception exception ) {
        }

        return( typeReturn );
    }

    // --- Implement method from interface XServiceName ---
    public String getServiceName() {
        return( _serviceName );
    }

    // --- Implement methods from interface XServiceInfo ---
    public boolean supportsService(String stringServiceName) {
        return( stringServiceName.equals(_serviceName));
    }

    public String getImplementationName() {
        return( XSLTransformer.class.getName());
    }

    public String[] getSupportedServiceNames() {
        String[] stringSupportedServiceNames = { _serviceName };
        return stringSupportedServiceNames;
    }

    // --- component registration methods ---
    public static XSingleServiceFactory __getServiceFactory(
        String implName, XMultiServiceFactory multiFactory, XRegistryKey regKey)
    {
        XSingleServiceFactory xSingleServiceFactory = null;
        if (implName.equals(XSLTransformer.class.getName()) ) {
            xSingleServiceFactory = FactoryHelper.getServiceFactory(XSLTransformer.class,
            _serviceName, multiFactory, regKey);
        }
        return xSingleServiceFactory;
    }

    public static boolean __writeRegistryServiceInfo(XRegistryKey regKey) {
        return FactoryHelper.writeRegistryServiceInfo(XSLTransformer.class.getName(),
        _serviceName, regKey);
    }

}
