/*************************************************************************
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

/** You can find more
 * information on the following web page:
 * http://api.openoffice.org/common/ref/com/sun/star/index.html
 */


/*Java Uno Helper Classes*/
import com.sun.star.lib.uno.adapter.XInputStreamToInputStreamAdapter;
import com.sun.star.lib.uno.adapter.XOutputStreamToOutputStreamAdapter;

/*StarOffice/Uno Classes*/
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
import com.sun.star.uno.AnyConverter;

/* Generated from Idls */
import com.sun.star.xml.XImportFilter;
import com.sun.star.xml.XExportFilter;

/* XMerge Classes */
import org.openoffice.xmerge.util.registry.ConverterInfoReader;
import org.openoffice.xmerge.util.registry.ConverterInfo;
import org.openoffice.xmerge.util.registry.ConverterInfoMgr;
import org.openoffice.xmerge.Convert;
import org.openoffice.xmerge.ConverterFactory;
import org.openoffice.xmerge.Document;
import org.openoffice.xmerge.ConvertData;
import org.openoffice.xmerge.converter.xml.OfficeDocument;
/*Java Classes*/
import java.util.Enumeration;
import java.io.*;
import javax.xml.parsers.*;
import java.net.URI;


/** This outer class provides an inner class to implement the service
 * description, a method to instantiate the
 * component on demand (__getServiceFactory()), and a method to give
 * information about the component (__writeRegistryServiceInfo()).
 */
public class XMergeBridge {
    
    
    private static XMultiServiceFactory xMSF;
    private static XDocumentHandler exportDocHandler=null;
    private static XInputStream xInStream =null;
    private static XOutputStream xOutStream=null;
    private static String  udJarPath=null;
    private static XOutputStream xos = null;
    private static XOutputStreamToOutputStreamAdapter adaptedStream=null;
    private static String offMime=null;
    private static String sdMime=null;
    private static String sFileName=null; 
    private static String sURL="";

    //private static FileOutputStream adaptedStream =null;

    /** This inner class provides the component as a concrete implementation
     * of the service description. It implements the needed interfaces.
     * @implements XTypeProvider
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
                /*    
        System.out.println("\nFound the Importer!\n");
        
        System.out.println("\n"+msUserData[0]);
        System.out.println("\n"+msUserData[1]);
        System.out.println("\n"+msUserData[2]);
        System.out.println("\n"+msUserData[3]);
        System.out.println("\n"+msUserData[4]);
        System.out.println("\n"+msUserData[5]);
        */
        sFileName="";			    
        sURL="";
        String sDirectory = null;
        String udConvertClass=msUserData[0];
        udJarPath=msUserData[1];
        String udImport =msUserData[2];
        String udExport =msUserData[3];
        offMime =msUserData[4];
        sdMime = msUserData[5];
        com.sun.star.io.XInputStream xis=null;	
        com.sun.star.beans.PropertyValue[] pValue = aSourceData;
        
        for  (int  i = 0 ; i < pValue.length; i++)
        {

        //System.out.println("\n"+pValue[i].Name+" "+pValue[i].Value;

         try{
             //System.out.println("\n"+pValue[i].Name+" "+pValue[i].Value);
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
         XConfigManager xCfgMgr = (XConfigManager) UnoRuntime.queryInterface(
                                            XConfigManager.class , xCfgMgrObj );	
        String PathString=xCfgMgr.substituteVariables("$(progurl)" );
        PathString= PathString.concat("/");
        udJarPath= PathString.concat(udJarPath);

        Object xPipeObj=xMSF.createInstance("com.sun.star.io.Pipe");
        xInStream = (XInputStream) UnoRuntime.queryInterface(
                        XInputStream.class , xPipeObj );
            xOutStream = (XOutputStream) UnoRuntime.queryInterface(
                        XOutputStream.class , xPipeObj );
        convert (xis,xOutStream,false,udJarPath,sFileName,offMime,sdMime);
        Object xSaxParserObj=xMSF.createInstance("com.sun.star.xml.sax.Parser");
        
        XParser xParser = (XParser) UnoRuntime.queryInterface(
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
        //System.out.println("XMergeBridge IO Exception "+e.getMessage());
          return false;
        }
         catch (Exception e){
        //System.out.println("XMergeBridge Exception "+e+" "+e.getMessage());
        return false;
        }
        return true;
    }

       public boolean exporter(com.sun.star.beans.PropertyValue[] aSourceData, 
                   java.lang.String[] msUserData) throws com.sun.star.uno.RuntimeException{
                  
                   /*		   	   	   		   
        System.out.println("\nFound the Exporter!\n");
         
        System.out.println("\n"+msUserData[0]);
        System.out.println("\n"+msUserData[1]);
        System.out.println("\n"+msUserData[2]);
        System.out.println("\n"+msUserData[3]);
        System.out.println("\n"+msUserData[4]);
        System.out.println("\n"+msUserData[5]);
        */
        sFileName=null;		   
        sURL=null;
        String sDirectory = null;
        String title=null;
        String udConvertClass=msUserData[0];
        udJarPath=msUserData[1];
        String udImport =msUserData[2];
        String udExport =msUserData[3];
        offMime =msUserData[4];
        sdMime = msUserData[5];
        
        com.sun.star.beans.PropertyValue[] pValue = aSourceData;
        for  (int  i = 0 ; i < pValue.length; i++)
        {
        
        //System.out.println("\n"+pValue[i].Name+" "+pValue[i].Value);
        

        try{
            //System.out.println("\n"+pValue[i].Name+" "+pValue[i].Value);
            if (pValue[i].Name.compareTo("OutputStream")==0){
            xos=(com.sun.star.io.XOutputStream)AnyConverter.toObject(new Type(com.sun.star.io.XOutputStream.class), pValue[i].Value);
            //  System.out.println(pValue[i].Name+" "+xos); 
            }
            if (pValue[i].Name.compareTo("FileName")==0){
            sFileName=(String)AnyConverter.toObject(new Type(java.lang.String.class), pValue[i].Value);
            //System.out.println(pValue[i].Name+" "+sFileName); 
            }
            if (pValue[i].Name.compareTo("URL")==0){
            sURL=(String)AnyConverter.toObject(new Type(java.lang.String.class), pValue[i].Value);
            // System.out.println("\nMediaDescriptor url "+pValue[i].Name+" "+sURL);
            
            }
            if (pValue[i].Name.compareTo("Title")==0){
           
            title=(String)AnyConverter.toObject(new Type(java.lang.String.class), pValue[i].Value);
            //System.out.println(pValue[i].Name+" "+title); 
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
         XConfigManager xCfgMgr = (XConfigManager) UnoRuntime.queryInterface(
                                            XConfigManager.class , xCfgMgrObj );
        
        String PathString=xCfgMgr.substituteVariables("$(progurl)" );
        PathString= PathString.concat("/");
        udJarPath= PathString.concat(udJarPath); 

        Object xPipeObj=xMSF.createInstance("com.sun.star.io.Pipe");
        xInStream = (XInputStream) UnoRuntime.queryInterface(
                        XInputStream.class , xPipeObj );
        xOutStream = (XOutputStream) UnoRuntime.queryInterface(
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

       
    
       public void  startDocument ()	{
       //System.out.println("\nStart Document!");
       }
    
    public void endDocument()throws com.sun.star.uno.RuntimeException
    {	   
        
        try{
        xOutStream.closeOutput();	
        convert (xInStream,xos,true,udJarPath,sURL,offMime,sdMime);
        
        }
        catch (IOException e){
        //System.out.println("Exception "+e);
        throw new com.sun.star.uno.RuntimeException(e.getMessage());

        }
         catch (Exception e){
        //System.out.println("Exception "+e);
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
        // System.out.println(str); 
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
      
         ConverterInfo converterInfo = null;
         Enumeration ciEnum= null;
        
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
                 cv.addInputStream(name,(InputStream)xis,false);
                 ConvertData dataOut = cv.convert();
               
                 Enumeration docEnum = dataOut.getDocumentEnumeration();
               
                 if (docEnum.hasMoreElements()){
                     Document docOut      = (Document)docEnum.nextElement();
                     String fileName      = docOut.getFileName();
                     docOut.write(newxos);
                     
                     newxos.flush();
                     newxos.close();
                     
                  
                     int i=1;
                     while (docEnum.hasMoreElements() && sURL.startsWith("file:")) {
                     //URI uri=new URI(sFileName);
                     URI uri=new URI(sURL);
                     String  newFileName= getPath(uri);
                     
                         
                     //System.out.println("\nURI: "+uri.getPath());
                     File newFile=null;
                     if (newFileName.lastIndexOf(".")!=-1){
                         newFile =new File(newFileName.substring(0,newFileName.lastIndexOf("."))+String.valueOf(i)+newFileName.substring(newFileName.lastIndexOf(".")));
                     }
                     else{
                        newFile =new File(newFileName.concat(String.valueOf(i)));
                     }
                       
                     FileOutputStream fos = new FileOutputStream(newFile);
                     docOut      = (Document)docEnum.nextElement();
                     fileName      = docOut.getFileName();
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
                             /*
                             ByteArrayOutputStream bout = new ByteArrayOutputStream();
                             byte[][] buf = new byte[1][4096];
                             int n=0;
                             while ((n=xml.readSomeBytes(buf, 4096))>0)
                                 bout.write(buf[0], 0, n);
                             ByteArrayInputStream bin = new ByteArrayInputStream(bout.toByteArray());
                             cv.addInputStream(name, bin, false);
                            */
                 cv.addInputStream(name,(InputStream)xis,false);
                 //System.out.println("\nConverting");
                 ConvertData dataIn = cv.convert();
                 //System.out.println("\nFinished Converting");
                 Enumeration docEnum = dataIn.getDocumentEnumeration();
                 while (docEnum.hasMoreElements()) {
                 OfficeDocument docIn      = (OfficeDocument)docEnum.nextElement();		   
                
                 docIn.write(newxos,false);
                 }
                 //newxos.write(-1); //EOF character
                               //newxos.flush();
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
    
    /**
     * Writes the service information into the given registry key.
     * This method is called by the <code>JavaLoader</code>
     * <p>
     * @return  returns true if the operation succeeded
     * @param   regKey       the registryKey
     * @see                  com.sun.star.comp.loader.JavaLoader
     */
    public static boolean __writeRegistryServiceInfo(XRegistryKey regKey) {

        return FactoryHelper.writeRegistryServiceInfo(_XMergeBridge.class.getName(),
        _XMergeBridge.__serviceName, regKey);
    }
}



