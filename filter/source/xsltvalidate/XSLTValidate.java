/************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


import com.sun.star.comp.loader.FactoryHelper;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XServiceName;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.uno.Type;
import java.util.Enumeration;
import java.util.Vector;
import com.sun.star.xml.XImportFilter;
import com.sun.star.xml.XExportFilter;

// Imported TraX classes
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

import org.xml.sax.ErrorHandler;
import org.xml.sax.SAXException;
import org.xml.sax.SAXParseException;

import com.sun.star.uno.AnyConverter;

//Uno to java Adaptor
import com.sun.star.lib.uno.adapter.*;

/** This outer class provides an inner class to implement the service
 * description, a method to instantiate the
 * component on demand (__getServiceFactory()), and a method to give
 * information about the component (__writeRegistryServiceInfo()).
 */
public class XSLTValidate {

    private static XMultiServiceFactory xMSF;
    private static Vector parseErrors =new Vector();

    /** This inner class provides the component as a concrete implementation
     * of the service description. It implements the needed interfaces.
     * @implements XTypeProvider
     */
    static public class _XSLTValidate implements
        XImportFilter,
        XServiceName,
        XServiceInfo,
        ErrorHandler,
        XTypeProvider {

        private com.sun.star.xml.sax.XErrorHandler xErrorHandler;
        
        /** The component will be registered under this name.
         */
        static private final String __serviceName = "com.sun.star.documentconversion.XSLTValidate";
        
        public _XSLTValidate() {
            xErrorHandler = null;
        }

        public com.sun.star.uno.Type[] getTypes() {
            Type[] typeReturn = {};

            try {
                typeReturn = new Type[] {
                new Type( XTypeProvider.class ),
                new Type( XExportFilter.class ),
                new Type( XImportFilter.class ),
                new Type( XServiceName.class ),
                new Type( XServiceInfo.class ) };
            }
            catch( Exception exception ) {
        
            }

            return( typeReturn );
        }


    public boolean importer(com.sun.star.beans.PropertyValue[] aSourceData, 
                com.sun.star.xml.sax.XDocumentHandler xDocHandler, 
                java.lang.String[] msUserData) throws com.sun.star.uno.RuntimeException,com.sun.star.lang.IllegalArgumentException { 
    
        com.sun.star.io.XInputStream xis=null;
        com.sun.star.beans.PropertyValue[] pValue = aSourceData;
        for  (int  i = 0 ; i < pValue.length; i++)
        {
            try{
                //System.out.println("\n"+pValue[i].Name+" "+pValue[i].Value);
                if (pValue[i].Name.compareTo("InputStream")==0){
                    xis=(com.sun.star.io.XInputStream)AnyConverter.toObject(new Type(com.sun.star.io.XInputStream.class), pValue[i].Value);
                }
                else if (pValue[i].Name.compareTo("ErrorHandler")==0){
                    xErrorHandler=(com.sun.star.xml.sax.XErrorHandler)AnyConverter.toObject(new Type(com.sun.star.xml.sax.XErrorHandler.class), pValue[i].Value);
                }
            }
            catch(com.sun.star.lang.IllegalArgumentException AnyExec){
                System.out.println("\nIllegalArgumentException "+AnyExec);
            }
        }
        try{
            convert (xis);
        }
        catch (Exception AnyExec){
            throw new com.sun.star.uno.RuntimeException(AnyExec.getMessage());
        }
        return true;
    }

     public void convert (com.sun.star.io.XInputStream xml) throws com.sun.star.uno.RuntimeException {
         XInputStreamToInputStreamAdapter xis =new XInputStreamToInputStreamAdapter(xml);	 
         parseErrors =new Vector();
           //String defaultTimeOut = System.getProperty("sun.net.client.defaultConnectTimeout");
           System.getProperties().setProperty("sun.net.client.defaultConnectTimeout", "10000");
         try{
             DocumentBuilderFactory dFactory = DocumentBuilderFactory.newInstance();
             dFactory.setNamespaceAware(true);
             dFactory.setValidating(true);
             DocumentBuilder dBuilder = dFactory.newDocumentBuilder();
             dBuilder.setErrorHandler(this);
             dBuilder.parse(xis);
             if (parseErrors.size()>0){
                 String errString ="";
                 for (Enumeration e = parseErrors.elements() ; e.hasMoreElements() ;) {
                    errString+=e.nextElement();
                    //System.out.println(e.nextElement());
                 }
                 throw new com.sun.star.uno.RuntimeException(errString);
             }
         }
        catch (Exception e){
           //System.out.println("\nException "+ e);
           throw new com.sun.star.uno.RuntimeException(e.getLocalizedMessage());
        } 
     }

    public com.sun.star.uno.Any JavaSAXParseEceptionToUno( SAXParseException e )
    {
        com.sun.star.uno.XInterface xContext = null;

        String aMessage = e.getMessage();
        if( aMessage == null )
            aMessage = new String();

        String aPublicId = e.getPublicId();
        if( aPublicId == null )
            aPublicId = new String();

        String aSystemId = e.getSystemId();
        if( aSystemId == null )
            aSystemId = new String();

        return new com.sun.star.uno.Any( new Type(com.sun.star.xml.sax.SAXParseException.class),
                    new com.sun.star.xml.sax.SAXParseException( aMessage,
                    xContext,
                    com.sun.star.uno.Any.VOID,
                    aPublicId,
                    aSystemId,
                    e.getLineNumber(),
                    e.getColumnNumber() ) );

    }

    //  Warning Event Handler
    public void warning (SAXParseException e)
            throws SAXException 
    {
//		System.out.println("\n_XSLTValidate::warning " + e.toString() );

        if( xErrorHandler != null )
        {
            try
            {
                xErrorHandler.warning( JavaSAXParseEceptionToUno( e ) );
            }
            catch( com.sun.star.xml.sax.SAXException ex )
            {
                throw e;
            }
        }
        else
        {

            //System.err.println ("Warning:  "+e);
            try{
                //parseErrors.write (("\n"+e.getMessage()).getBytes());
            }
            catch(Exception genEx){
                //System.out.print("\n Error while writing ParseErrors"+genEx);
            }
        }
    }
    
    //  Error Event Handler
    public void error (SAXParseException e)
        throws SAXException 
    {
//		System.out.println("\n_XSLTValidate::error " + e.toString() );

        if( xErrorHandler != null )
        {
            try
            {
                xErrorHandler.error( JavaSAXParseEceptionToUno( e ) );
            }
            catch( com.sun.star.xml.sax.SAXException ex )
            {
                throw e;
            }
        }
        else
        {
            //System.err.println ("Error:  "+e);
            try{
                parseErrors.add (e.getLocalizedMessage()+" "+e.getLineNumber()+" ");
            }
            catch(Exception genEx){
                //System.out.print("\n Error while writing ParseErrors"+genEx);
            }
        }
    }

    //  Fatal Error Event Handler
    public void fatalError (SAXParseException e)
    throws SAXException {
//		System.out.println("\n_XSLTValidate::fatalError " + e.toString() );

        if( xErrorHandler != null )
        {
            try
            {
                xErrorHandler.fatalError(  JavaSAXParseEceptionToUno( e ) );
            }
            catch( com.sun.star.xml.sax.SAXException ex )
            {
                throw e;
            }
        }
        else
        {
            //System.err.println ("Fatal Error:  "+e);
            try{
                parseErrors.add (e.getLocalizedMessage()+" "+e.getLineNumber()+" ");
            }
            catch(Exception genEx){
                //System.out.print("\n Error while writing ParseErrors"+genEx);
            }
        }
    }
    
        // Implement methods from interface XTypeProvider
        public byte[] getImplementationId() {
            byte[] byteReturn = {};

            byteReturn = new String( "" + this.hashCode() ).getBytes();

            return( byteReturn );
        }

        // Implement method from interface XServiceName
        public String getServiceName() {
            return( __serviceName );
        }
    
        // Implement methods from interface XServiceInfo
        public boolean supportsService(String stringServiceName) {
            return( stringServiceName.equals( __serviceName ) );
        }
    
        public String getImplementationName() {
            return( _XSLTValidate.class.getName() );
        }
    
        public String[] getSupportedServiceNames() {
            String[] stringSupportedServiceNames = { __serviceName };
            return( stringSupportedServiceNames );
        }
    }

    /**
     * Returns a factory for creating the service.
     * This method is called by the <code>JavaLoader</code>
     *
     * @return  returns a <code>XSingleServiceFactory</code> for creating the
     *          component
     *
     * @param   implName     the name of the implementation for which a
     *                       service is desired
     * @param   multiFactory the service manager to be used if needed
     * @param   regKey       the registryKey
     *
     * @see                  com.sun.star.comp.loader.JavaLoader
     */
    public static XSingleServiceFactory __getServiceFactory(String implName,
    XMultiServiceFactory multiFactory,
    XRegistryKey regKey) {
        XSingleServiceFactory xSingleServiceFactory = null;
        xMSF= multiFactory;
        if (implName.equals(_XSLTValidate.class.getName()) ) {
            xSingleServiceFactory = FactoryHelper.getServiceFactory(_XSLTValidate.class,
            _XSLTValidate.__serviceName,
            multiFactory,
            regKey);
        }
        
        return xSingleServiceFactory;
    }
    
    /**
     * Writes the service information into the given registry key.
     * This method is called by the <code>JavaLoader</code>
     * <p>
     * @return  returns true if the operation succeeded
     * @param   regKey       the registryKey
     * @see                  com.sun.star.comp.loader.JavaLoader
     */
    public static boolean __writeRegistryServiceInfo(XRegistryKey regKey) {

        return FactoryHelper.writeRegistryServiceInfo(_XSLTValidate.class.getName(),
        _XSLTValidate.__serviceName, regKey);
    }
}
