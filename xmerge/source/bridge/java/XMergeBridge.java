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

/** You can find more
 * information on the following web page:
 * http://api.libreoffice.org/docs/common/ref/com/sun/star/module-ix.html
 */


/*Java Uno Helper Classes*/
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.URI;
import java.util.Iterator;

import javax.xml.parsers.ParserConfigurationException;

import org.openoffice.xmerge.Convert;
import org.openoffice.xmerge.ConvertData;
import org.openoffice.xmerge.ConverterFactory;
import org.openoffice.xmerge.Document;
import org.openoffice.xmerge.converter.xml.OfficeDocument;
import org.openoffice.xmerge.util.registry.ConverterInfo;
import org.openoffice.xmerge.util.registry.ConverterInfoMgr;
import org.openoffice.xmerge.util.registry.ConverterInfoReader;

import com.sun.star.comp.loader.FactoryHelper;
import com.sun.star.frame.XConfigManager;
import com.sun.star.io.XInputStream;
import com.sun.star.io.XOutputStream;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XServiceName;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.lib.uno.adapter.XInputStreamToInputStreamAdapter;
import com.sun.star.lib.uno.adapter.XOutputStreamToOutputStreamAdapter;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.xml.XExportFilter;
import com.sun.star.xml.XImportFilter;
import com.sun.star.xml.sax.InputSource;
import com.sun.star.xml.sax.XDocumentHandler;
import com.sun.star.xml.sax.XParser;


/** This outer class provides an inner class to implement the service
 * description and a method to instantiate the
 * component on demand (__getServiceFactory()).
 */
public class XMergeBridge {


    private static XMultiServiceFactory xMSF;
    private static XInputStream xInStream =null;
    private static XOutputStream xOutStream=null;
    private static String  udJarPath=null;
    private static XOutputStream xos = null;
    private static String offMime=null;
    private static String sdMime=null;
    private static String sFileName=null;
    private static String sURL="";

    /** This inner class provides the component as a concrete implementation
     * of the service description. It implements the needed interfaces.
     */
    static public class _XMergeBridge implements
                                               XImportFilter,
     XExportFilter,
        XServiceName,
        XServiceInfo,
    XDocumentHandler,
        XTypeProvider {

        /** The component will be registered under this name.
         */
        static private final String __serviceName = "com.sun.star.documentconversion.XMergeBridge";



        public com.sun.star.uno.Type[] getTypes() {
            Type[] typeReturn = {};

            try {
                typeReturn = new Type[] {
                new Type( XTypeProvider.class ),
                new Type( XImportFilter.class ),
        new Type( XExportFilter.class ),
                new Type( XServiceName.class ),
                new Type( XServiceInfo.class ) };
            }
            catch( Exception exception ) {

            }

            return( typeReturn );
        }

    String getFileName(String origName)
    {
        String name=null;
        if (origName !=null)
        {
        if(origName.equalsIgnoreCase(""))
            name = "OutFile";
        else {
            if (origName.lastIndexOf("/")>=0){
                origName=origName.substring(origName.lastIndexOf("/")+1,origName.length());
            }
            if (origName.lastIndexOf(".")>=0){
            name = origName.substring(0,(origName.lastIndexOf(".")));
            }
            else{
            name=origName;
            }
        }
        }
        else{
        name = "OutFile";
        }
        return name;
    }



    public boolean importer(com.sun.star.beans.PropertyValue[] aSourceData,
                com.sun.star.xml.sax.XDocumentHandler xDocHandler,
                java.lang.String[] msUserData) throws com.sun.star.uno.RuntimeException {

        sFileName="";
        sURL="";
        udJarPath=msUserData[1];
        offMime =msUserData[4];
        sdMime = msUserData[5];
        com.sun.star.io.XInputStream xis=null;
        com.sun.star.beans.PropertyValue[] pValue = aSourceData;

        for  (int  i = 0 ; i < pValue.length; i++)
        {

         try{
             if (pValue[i].Name.compareTo("InputStream")==0){
            xis=(com.sun.star.io.XInputStream)AnyConverter.toObject(new Type(com.sun.star.io.XInputStream.class), pValue[i].Value);
             }
             if (pValue[i].Name.compareTo("FileName")==0){
             sFileName=(String)AnyConverter.toObject(new Type(java.lang.String.class), pValue[i].Value);
             }

         }
         catch(com.sun.star.lang.IllegalArgumentException AnyExec){
             System.out.println("\nIllegalArgumentException "+AnyExec);
         }



        }


        try{

        Object xCfgMgrObj=xMSF.createInstance("com.sun.star.config.SpecialConfigManager");
         XConfigManager xCfgMgr = UnoRuntime.queryInterface(
                                            XConfigManager.class , xCfgMgrObj );
        String PathString=xCfgMgr.substituteVariables("$(progurl)" );
        PathString= PathString.concat("/");
        udJarPath= PathString.concat(udJarPath);

        Object xPipeObj=xMSF.createInstance("com.sun.star.io.Pipe");
        xInStream = UnoRuntime.queryInterface(
                        XInputStream.class , xPipeObj );
            xOutStream = UnoRuntime.queryInterface(
                        XOutputStream.class , xPipeObj );
        convert (xis,xOutStream,false,udJarPath,sFileName,offMime,sdMime);
        Object xSaxParserObj=xMSF.createInstance("com.sun.star.xml.sax.Parser");

        XParser xParser = UnoRuntime.queryInterface(
                        XParser.class , xSaxParserObj );
        if (xParser==null){
            System.out.println("\nParser creation Failed");
        }
        xOutStream.closeOutput();
        InputSource aInput = new InputSource();
        if (sFileName==null){
            sFileName="";
            }
        aInput.sSystemId = sFileName;
        aInput.aInputStream =xInStream;
                xParser.setDocumentHandler ( xDocHandler );

        xParser.parseStream ( aInput );
        xOutStream.closeOutput();
        xInStream.closeInput();

        }
        catch (IOException e){
          return false;
        }
         catch (Exception e){
        return false;
        }
        return true;
    }

       public boolean exporter(com.sun.star.beans.PropertyValue[] aSourceData,
                   java.lang.String[] msUserData) throws com.sun.star.uno.RuntimeException{

        sFileName=null;
        sURL=null;
        udJarPath=msUserData[1];
        offMime =msUserData[4];
        sdMime = msUserData[5];

        com.sun.star.beans.PropertyValue[] pValue = aSourceData;
        for  (int  i = 0 ; i < pValue.length; i++)
        {


        try{
            if (pValue[i].Name.compareTo("OutputStream")==0){
            xos=(com.sun.star.io.XOutputStream)AnyConverter.toObject(new Type(com.sun.star.io.XOutputStream.class), pValue[i].Value);
            }

            if (pValue[i].Name.compareTo("FileName")==0){
            sFileName=(String)AnyConverter.toObject(new Type(java.lang.String.class), pValue[i].Value);
            }

            if (pValue[i].Name.compareTo("URL")==0){
            sURL=(String)AnyConverter.toObject(new Type(java.lang.String.class), pValue[i].Value);
            }
        }
        catch(com.sun.star.lang.IllegalArgumentException AnyExec){
             System.out.println("\nIllegalArgumentException "+AnyExec);
        }
        }


        if (sURL==null){
        sURL="";
        }

         try{

         Object xCfgMgrObj=xMSF.createInstance("com.sun.star.config.SpecialConfigManager");
         XConfigManager xCfgMgr = UnoRuntime.queryInterface(
                                            XConfigManager.class , xCfgMgrObj );

        String PathString=xCfgMgr.substituteVariables("$(progurl)" );
        PathString= PathString.concat("/");
        udJarPath= PathString.concat(udJarPath);

        Object xPipeObj=xMSF.createInstance("com.sun.star.io.Pipe");
        xInStream = UnoRuntime.queryInterface(
                        XInputStream.class , xPipeObj );
        xOutStream = UnoRuntime.queryInterface(
                        XOutputStream.class , xPipeObj );
           }
          catch (Exception e){
        System.out.println("Exception "+e);
          return false;
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



       public void  startDocument ()    {
       }

    public void endDocument()throws com.sun.star.uno.RuntimeException
    {

        try{
        xOutStream.closeOutput();
        convert (xInStream,xos,true,udJarPath,sURL,offMime,sdMime);

        }
        catch (IOException e){
        throw new com.sun.star.uno.RuntimeException(e.getMessage());

        }
         catch (Exception e){
        throw new com.sun.star.uno.RuntimeException("Xmerge Exception");

        }
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
            str=str.concat(needsMask(xattribs.getValueByIndex(i)));
            str=str.concat("\" ");
            }
        }
        str=str.concat(">");

        try{
         xOutStream.writeBytes(str.getBytes("UTF-8"));
        }
        catch (Exception e){
        System.out.println("\n"+e);
        }

    }

    public void endElement(String str){

        str="</".concat(str);
        str=str.concat(">");
        try{
         xOutStream.writeBytes(str.getBytes("UTF-8"));

        }
        catch (Exception e){
        System.out.println("\n"+e);
        }


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






    public void convert (com.sun.star.io.XInputStream xml,com.sun.star.io.XOutputStream device,
             boolean convertFromOffice,String pluginUrl,String FileName,String offMime,String sdMime) throws com.sun.star.uno.RuntimeException, IOException {

         String jarName = pluginUrl;
         String name= getFileName(FileName);

         Iterator<ConverterInfo> ciEnum= null;

         XInputStreamToInputStreamAdapter xis =new XInputStreamToInputStreamAdapter(xml);


         XOutputStreamToOutputStreamAdapter newxos =new XOutputStreamToOutputStreamAdapter(device);
         try{
         ConverterInfoReader cir = new ConverterInfoReader(jarName,false);
         ciEnum =cir.getConverterInfoEnumeration();
         }
         catch (ParserConfigurationException pexc){
          System.out.println("Error:"+pexc);
         }
          catch ( org.xml.sax.SAXException pexc){
          System.out.println("Error:"+pexc);
         }
         catch(Exception e){
         System.out.println("Error:"+e);
         }
         ConverterInfoMgr. removeByJar(jarName);
         if (convertFromOffice)
         {

         try {

             //Check to see if jar contains a plugin Impl

                 ConverterInfoMgr.addPlugIn(ciEnum);
             ConverterFactory cf = new ConverterFactory();

             Convert cv = cf.getConverter(ConverterInfoMgr.findConverterInfo(sdMime,offMime),false);
             if (cv == null) {
                 System.out.println("\nNo plug-in exists to convert from <staroffice/sxw> to <specified format> ");

             }
             else
             {
                 cv.addInputStream(name,xis,false);
                 ConvertData dataOut = cv.convert();

                 Iterator<Object> docEnum = dataOut.getDocumentEnumeration();

                 if (docEnum.hasNext()){
                     Document docOut      = (Document)docEnum.next();
                     docOut.write(newxos);

                     newxos.flush();
                     newxos.close();


                     int i=1;
                     while (docEnum.hasNext() && sURL.startsWith("file:")) {

                     URI uri=new URI(sURL);
                     String  newFileName= getPath(uri);

                     File newFile=null;
                     if (newFileName.lastIndexOf(".")!=-1){
                         newFile =new File(newFileName.substring(0,newFileName.lastIndexOf("."))+String.valueOf(i)+newFileName.substring(newFileName.lastIndexOf(".")));
                     }
                     else{
                        newFile =new File(newFileName.concat(String.valueOf(i)));
                     }

                     FileOutputStream fos = new FileOutputStream(newFile);
                     docOut      = (Document)docEnum.next();
                     docOut.write(fos);
                     fos.flush();
                     fos.close();
                     i++;

                     }

                 }
             }
             ConverterInfoMgr.removeByJar(jarName);
         }
         catch (StackOverflowError sOE){
             System.out.println("\nERROR : Stack OverFlow. \n Increase of the JRE by adding the following line to the end of the javarc file \n \"-Xss1m\"\n");

         }
         catch (Exception e) {
             System.out.println("Error:"+e);
              throw new IOException("Xmerge Exception");
             }
         }
         else{

         try {
              //Check to see if jar contains a plugin Impl
                 ConverterInfoMgr.addPlugIn(ciEnum);
                 ConverterFactory cf = new ConverterFactory();
             Convert cv = cf.getConverter(ConverterInfoMgr.findConverterInfo(sdMime,offMime),true);
             if (cv == null) {
                 System.out.println("\nNo plug-in exists to convert to <staroffice/sxw> from <specified format>");
             }
             else
             {

                 cv.addInputStream(name,xis,false);
                 ConvertData dataIn = cv.convert();
                 Iterator<Object> docEnum = dataIn.getDocumentEnumeration();
                 while (docEnum.hasNext()) {
                 OfficeDocument docIn      = (OfficeDocument)docEnum.next();

                 docIn.write(newxos,false);
                 }
                 newxos.close();
             }
             ConverterInfoMgr.removeByJar(jarName);
         }
         catch (StackOverflowError sOE){
              System.out.println("\nERROR : Stack OverFlow. \n Increase of the JRE by adding the following line to the end of the javarc file \n \"-Xss1m\"\n");
         }
         catch (Exception e) {
             System.out.println("Error:"+e);
             throw new IOException("Xmerge Exception");
         }


         }

    }

        private String getPath(URI uri){
        String path = uri.getPath();
        String opSys=System.getProperty("os.name");
        if(opSys.indexOf("Windows")!=-1){
        path= path.replace('/','\\');
        path = path.substring(1);
        }
        return path;
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
            return( _XMergeBridge.class.getName() );
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
    xMSF= multiFactory;
        XSingleServiceFactory xSingleServiceFactory = null;
        if (implName.equals(_XMergeBridge.class.getName()) ) {
            xSingleServiceFactory = FactoryHelper.getServiceFactory(_XMergeBridge.class,
            _XMergeBridge.__serviceName,
            multiFactory,
            regKey);
        }

        return xSingleServiceFactory;
    }
}
