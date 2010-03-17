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

//Standard Java classes
import java.util.Enumeration;
import java.util.Vector;
import java.io.InputStream;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.ByteArrayOutputStream;
import java.io.ByteArrayInputStream;
import com.sun.star.xml.XImportFilter;
import com.sun.star.xml.XExportFilter;
import java.io.*;
import java.util.regex.*;

// Imported TraX classes
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.Transformer;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerConfigurationException;
import javax.xml.transform.stream.StreamResult;
import javax.xml.transform.stream.StreamSource;
import javax.xml.transform.URIResolver;
import javax.xml.transform.Source;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerConfigurationException;
import javax.xml.transform.OutputKeys;

//StarOffice Interfaces and UNO
import com.sun.star.uno.AnyConverter;
import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.comp.loader.FactoryHelper;
import com.sun.star.lang.XServiceName;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.frame.XConfigManager;
import com.sun.star.xml.sax.InputSource;
import com.sun.star.xml.sax.XParser;
import com.sun.star.io.XInputStream;
import com.sun.star.io.XOutputStream;
import com.sun.star.xml.sax.XDocumentHandler;

//Uno to java Adaptor
import com.sun.star.lib.uno.adapter.*;

/** This outer class provides an inner class to implement the service
 * description, a method to instantiate the
 * component on demand (__getServiceFactory()), and a method to give
 * information about the component (__writeRegistryServiceInfo()).
 */
public class DBFilter {


    private static XMultiServiceFactory xMSF;

    /** This inner class provides the component as a concrete implementation
     * of the service description. It implements the needed interfaces.
     * @implements XTypeProvider
     */
     public static class _DBFilter implements
    XImportFilter,
    XExportFilter,
        XServiceName,
        XServiceInfo,
    XDocumentHandler,
        XTypeProvider {

        private boolean indent;
        private XInputStream xInStream;
        private XOutputStream xOutStream;
        private XOutputStream xos;
        private String sExportStyleSheet;
        private String doctype_public;
        private String doctype_system;

        public _DBFilter()
        {
            indent = false;
            xInStream = null;
            xOutStream = null;
            xos = null;
            sExportStyleSheet = null;
            doctype_public = null;
            doctype_system = null;
        }

        /** The component will be registered under this name.
         */
        static private final String __serviceName = "com.sun.star.documentconversion.DBFilter";

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
        /*
        System.out.println("\nFound the  Java Importer!\n");

        System.out.println("\n"+msUserData[0]);
        System.out.println("\n"+msUserData[1]);
        System.out.println("\n"+msUserData[2]);
        System.out.println("\n"+msUserData[3]);
        System.out.println("\n"+xDocHandler);
        System.out.println("\n"+msUserData[4]);
        System.out.println("\n"+msUserData[5]);
        */
        String sFileName=null;
        String udImport =msUserData[2];
        String sImportStyleSheet =msUserData[4];
        sExportStyleSheet =msUserData[5];
        com.sun.star.io.XInputStream xis=null;
        com.sun.star.beans.PropertyValue[] pValue = aSourceData;

        for  (int  i = 0 ; i < pValue.length; i++)
        {
         try{
             //System.out.println("\n"+pValue[i].Name+" "+pValue[i].Value);
             if (pValue[i].Name.compareTo("InputStream")==0){
            xis=(com.sun.star.io.XInputStream)AnyConverter.toObject(new Type(com.sun.star.io.XInputStream.class), pValue[i].Value);
             }
             else if (pValue[i].Name.compareTo("FileName")==0){
             sFileName=(String)AnyConverter.toObject(new Type(java.lang.String.class), pValue[i].Value);
             }
             else if (pValue[i].Name.compareTo("Indent")==0){  //to be changed to new value for indentation from XSLT UI
             indent=(boolean)AnyConverter.toBoolean(pValue[i].Value);
             }
             else if (pValue[i].Name.compareTo("DocType_Public")==0){
             doctype_public = AnyConverter.toString(pValue[i].Value);
             }
             else if (pValue[i].Name.compareTo("DocType_System")==0){
             doctype_system = AnyConverter.toString(pValue[i].Value);
             }       }
         catch(com.sun.star.lang.IllegalArgumentException AnyExec){
             System.out.println("\nIllegalArgumentException "+AnyExec);
         }

        }

        try{

        Object xCfgMgrObj=xMSF.createInstance("com.sun.star.config.SpecialConfigManager");
        XConfigManager xCfgMgr = (XConfigManager) UnoRuntime.queryInterface(
                    XConfigManager.class , xCfgMgrObj );
        String PathString=xCfgMgr.substituteVariables("$(progurl)" );
        PathString= PathString.concat("/");
        Object xPipeObj=xMSF.createInstance("com.sun.star.io.Pipe");
        xInStream = (XInputStream) UnoRuntime.queryInterface(
                        XInputStream.class , xPipeObj );
        xOutStream = (XOutputStream) UnoRuntime.queryInterface(
                        XOutputStream.class , xPipeObj );
        if (!sImportStyleSheet.equals("")){
            if (!sImportStyleSheet.startsWith("file:")&&!sImportStyleSheet.startsWith("http:")
                &&!sExportStyleSheet.startsWith("shttp:")
                &&!sExportStyleSheet.startsWith("jar:")){
                sImportStyleSheet=PathString.concat(sImportStyleSheet);
            }
        }
        convert (xis,xOutStream,sImportStyleSheet,true);
        xOutStream.closeOutput();
        Object xSaxParserObj=xMSF.createInstance("com.sun.star.xml.sax.Parser");
        XParser xParser = (XParser) UnoRuntime.queryInterface(
                        XParser.class , xSaxParserObj );
        InputSource aInput = new InputSource();
        if (sFileName==null)
            sFileName=" ";
        aInput.sSystemId = sFileName;
        aInput.aInputStream =xInStream;
                xParser.setDocumentHandler ( xDocHandler );
        xParser.parseStream ( aInput );
        xInStream.closeInput();

        }
        catch (Exception AnyExec){
        //e.printStackTrace();
        System.out.println("\nException "+AnyExec);
        throw new com.sun.star.uno.RuntimeException(AnyExec.getMessage());
        }
        return true;
    }



    public boolean exporter(com.sun.star.beans.PropertyValue[] aSourceData,
                    java.lang.String[] msUserData) throws com.sun.star.uno.RuntimeException,com.sun.star.lang.IllegalArgumentException {
        /*
        System.out.println("\nFound the Exporter!\n");

        System.out.println("\n0"+msUserData[0]);
        System.out.println("\n1"+msUserData[1]);
        System.out.println("\n2"+msUserData[2]);
        System.out.println("\n3"+msUserData[3]);

        System.out.println("\n4"+msUserData[4]);
        System.out.println("\n5"+msUserData[5]);
        */
        String udImport =msUserData[2];
        sExportStyleSheet =msUserData[5];
        com.sun.star.beans.PropertyValue[] pValue = aSourceData;
        for  (int  i = 0 ; i < pValue.length; i++)
        {
        try{
            //System.out.println("\n"+pValue[i].Name+" "+pValue[i].Value);
            if (pValue[i].Name.compareTo("OutputStream")==0){
            xos=(com.sun.star.io.XOutputStream)AnyConverter.toObject(new Type(com.sun.star.io.XOutputStream.class), pValue[i].Value);
            //  System.out.println(pValue[i].Name+" "+xos);
            }
            else if (pValue[i].Name.compareTo("Indent")==0){  //to be changed to new value for indentation from XSLT UI
            indent=(boolean)AnyConverter.toBoolean(pValue[i].Value);
            }
            else if (pValue[i].Name.compareTo("DocType_Public")==0){
            doctype_public = AnyConverter.toString(pValue[i].Value);
            }
            else if (pValue[i].Name.compareTo("DocType_System")==0){
            doctype_system = AnyConverter.toString(pValue[i].Value);
            }

        }
        catch(com.sun.star.lang.IllegalArgumentException AnyExec){
             System.out.println("\nIllegalArgumentException "+AnyExec);
        }
        }


        try{

        Object xCfgMgrObj=xMSF.createInstance("com.sun.star.config.SpecialConfigManager");
        XConfigManager xCfgMgr = (XConfigManager) UnoRuntime.queryInterface(
                        XConfigManager.class , xCfgMgrObj );

        String PathString=xCfgMgr.substituteVariables("$(progurl)" );
        PathString= PathString.concat("/");
        Object xPipeObj=xMSF.createInstance("com.sun.star.io.Pipe");
        xInStream = (XInputStream) UnoRuntime.queryInterface(
                        XInputStream.class , xPipeObj );
        xOutStream = (XOutputStream) UnoRuntime.queryInterface(
                        XOutputStream.class , xPipeObj );
        if (!sExportStyleSheet.equals("")){
            if (!sExportStyleSheet.startsWith("file:")&&!sExportStyleSheet.startsWith("http:")
                &&!sExportStyleSheet.startsWith("shttp:")
                &&!sExportStyleSheet.startsWith("jar:")){
                sExportStyleSheet=PathString.concat(sExportStyleSheet);
            }
        }
           }
        catch (Exception AnyExec){
        System.out.println("Exception "+AnyExec);
          throw new com.sun.star.uno.RuntimeException(AnyExec.getMessage());
        }
        return true;
       }

       public String replace(String origString, String origChar, String replaceChar){
           String tmp="";
           int index=origString.indexOf(origChar);
           if(index !=-1){
           while (index !=-1){
               String first =origString.substring(0,index);
               first=first.concat(replaceChar);
               tmp=tmp.concat(first);
               origString=origString.substring(index+1,origString.length());
               index=origString.indexOf(origChar);
               if(index==-1) {
               tmp=tmp.concat(origString);
               }
           }
           }
           return tmp;
    }

    public String needsMask(String origString){

        if (origString.indexOf("&")!=-1){
        origString=replace(origString,"&","&amp;");
        }
        if (origString.indexOf("\"")!=-1){
        origString=replace(origString,"\"","&quot;");
        }
        if (origString.indexOf("<")!=-1){
        origString=replace(origString,"<","&lt;");
        }
        if (origString.indexOf(">")!=-1){
        origString=replace(origString,">","&gt;");
        }
        return origString;
    }



    public void  startDocument (){
    }

    public void endDocument()
    {
        convert (xInStream,xos,sExportStyleSheet,false);
    }

    public void startElement (String str, com.sun.star.xml.sax.XAttributeList xattribs)
    {

        str="<".concat(str);
        if (xattribs !=null)
        {
        str= str.concat(" ");
        int len=xattribs.getLength();
        for (short i=0;i<len;i++)
            {
            str=str.concat(xattribs.getNameByIndex(i));
            str=str.concat("=\"");
            //str=str.concat(xattribs.getValueByIndex(i));
            str=str.concat(needsMask(xattribs.getValueByIndex(i)));
            str=str.concat("\" ");
            }
        }
        str=str.concat(">");
        //System.out.println(str);
        try{
        //xOutStream.writeBytes(str.getBytes());
        xOutStream.writeBytes(str.getBytes("UTF-8"));
        }
        catch (Exception e){
        System.out.println("\n"+e);
        }

    }

    public void endElement(String str){
        str="</".concat(str);
        str=str.concat(">");
        str=str.concat("\n");
        try{
        xOutStream.writeBytes(str.getBytes("UTF-8"));
        }
        catch (Exception e){
        System.out.println("\n"+e);
        }
        // System.out.println(str);

    }
    public void characters(String str){
        str=needsMask(str);
        try{
         xOutStream.writeBytes(str.getBytes("UTF-8"));
        }
        catch (Exception e){
           System.out.println("\n"+e);
        }

    }

    public void ignorableWhitespace(String str){


    }
    public void processingInstruction(String aTarget, String aData){

    }

    public void setDocumentLocator(com.sun.star.xml.sax.XLocator xLocator){

    }

    private String  maskEntities(String xmlString){
             Pattern testpattern = Pattern.compile("<!ENTITY [a-zA-Z0-9#]* ");
        Matcher testmatch= testpattern.matcher(xmlString);
        //System.out.println("\nStarting replace");
        int offset=0;
        while (testmatch.find(offset)){
            String newstring = xmlString.substring(testmatch.start()+9,testmatch.end()-1);
            offset= testmatch.end();
            //System.out.println("\nReplacing " +newstring);

           xmlString=xmlString.replaceAll("&"+newstring+";","<entity name=\""+newstring+"\">"+"&"+newstring+";"+"</entity>");
           testmatch = testmatch.reset();
              testmatch= testpattern.matcher(xmlString);
            //System.out.println("\nFound Pattern "+testmatch.replaceFirst("<entity>"+newstring+"</entity>"));
            newstring= "";
        }
         return xmlString;
    }

     public void convert (com.sun.star.io.XInputStream xml,
            com.sun.star.io.XOutputStream device,String sStyleSheet,boolean importing ) throws com.sun.star.uno.RuntimeException {
         XInputStreamToInputStreamAdapter xis =new XInputStreamToInputStreamAdapter(xml);
         XOutputStreamToOutputStreamAdapter xos =
             new XOutputStreamToOutputStreamAdapter(device);
         ByteArrayOutputStream baos = new ByteArrayOutputStream();
        String xmlFile=null;
         try{
        //call the tranformer using the XSL, Source and Result dom.
        TransformerFactory tFactory = TransformerFactory.newInstance();
        //tFactory.setURIResolver(this);
        Transformer transformer =null;
        transformer = tFactory.newTransformer( new StreamSource(sStyleSheet));
        if(indent){  // required for displaying XML correctly in XSLT UI
            transformer.setOutputProperty("indent", "yes");
             transformer.setOutputProperty("{http://xml.apache.org/xslt}indent-amount", "1");
        }
        if(null != doctype_public){
            transformer.setOutputProperty(OutputKeys.DOCTYPE_PUBLIC, doctype_public);
        }
        if(null != doctype_system){
            transformer.setOutputProperty(OutputKeys.DOCTYPE_SYSTEM, doctype_system);
        }
        if (importing)
        {
            byte tmpArr[]=new byte[1000];
            while (xis.available()>0){
                int read = xis.read(tmpArr);
                baos.write(tmpArr,0,read);
                tmpArr =new byte[1000];
            }
            xmlFile = maskEntities(baos.toString("UTF-8"));

              //xmlFile = baos.toString("UTF-8");
            if (xmlFile.indexOf("<!DOCTYPE")!=-1){
                    String tag= xmlFile.substring(xmlFile.lastIndexOf("/")+1,xmlFile.lastIndexOf(">"));
                    String entities = "";
                    if(xmlFile.indexOf("[",xmlFile.indexOf("<!DOCTYPE"))!=-1){
                        if(xmlFile.indexOf("[",xmlFile.indexOf("<!DOCTYPE")) < xmlFile.indexOf(">",xmlFile.indexOf("<!DOCTYPE"))){
                            entities = xmlFile.substring(xmlFile.indexOf("[",xmlFile.indexOf("<!DOCTYPE")),xmlFile.indexOf("]",xmlFile.indexOf("<!DOCTYPE"))+1);
                        }
                    }
                    String newDocType = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<!DOCTYPE "+tag+" "+entities+">";
                    xmlFile= xmlFile.substring(xmlFile.indexOf("<"+tag,0), xmlFile.lastIndexOf(">")+1);
                    xmlFile= newDocType.concat(xmlFile);
                    //throw new com.sun.star.uno.RuntimeException(xmlFile);
            }
            ByteArrayInputStream bais = new ByteArrayInputStream(xmlFile.getBytes("UTF-8"));
            transformer.transform(new StreamSource(bais),new StreamResult(xos));
        }
        else
            transformer.transform(new StreamSource(xis),new StreamResult(xos));
         }
         catch (TransformerConfigurationException transConfExc)
         {
          throw new com.sun.star.uno.RuntimeException(transConfExc.getMessage());
         }
         catch (TransformerException transExc){
             //System.out.println("\nException "+ e);
           throw new com.sun.star.uno.RuntimeException(transExc.getMessage());
        }
        catch (Exception e){
           System.out.println("\nException "+ e);
           throw new com.sun.star.uno.RuntimeException(e.getMessage());
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
            return( _DBFilter.class.getName() );
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
        if (implName.equals(_DBFilter.class.getName()) ) {
            xSingleServiceFactory = FactoryHelper.getServiceFactory(_DBFilter.class,
            _DBFilter.__serviceName,
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

        return FactoryHelper.writeRegistryServiceInfo(_DBFilter.class.getName(),
        _DBFilter.__serviceName, regKey);
    }
}
