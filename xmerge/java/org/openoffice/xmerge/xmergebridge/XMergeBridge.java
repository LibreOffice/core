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

/** You can find more
 * information on the following web page:
 * http://api.openoffice.org/common/ref/com/sun/star/index.html
 */


/*Java Uno Helper Classes*/
import com.sun.star.lib.uno.adapters.XInputStreamToInputStreamAdapter;
import com.sun.star.lib.uno.adapters.XOutputStreamToOutputStreamAdapter;

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
import java.util.Vector;
import java.io.InputStream;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.File;
import javax.xml.parsers.*;
import  org.xml.sax.SAXException;
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
        sFileName=null;
        String sURL=null;
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

        //System.out.println("\n"+pValue[i].Name+" "+pValue[i].Value);
        if (pValue[i].Name.compareTo("InputStream")==0){
            xis =(com.sun.star.io.XInputStream) pValue[i].Value;

            //System.out.println(pValue[i].Name+" "+xis);
        }
        if (pValue[i].Name.compareTo("FileName")==0){
            sFileName=(String) pValue[i].Value;
            //System.out.println(pValue[i].Name+" "+sFileName);
        }

        if (pValue[i].Name.compareTo("URL")==0){
            sURL = (String)pValue[i].Value;
            //System.out.println(pValue[i].Name+" "+sURL);
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


        }
        catch (IOException e){
        System.out.println("XMergeBridge IO Exception "+e.getMessage());
          return false;
        }
         catch (Exception e){
        System.out.println("XMergeBridge Exception "+e+" "+e.getMessage());
        //return false;
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
        String sURL=null;
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
        if (pValue[i].Name.compareTo("OutputStream")==0){
            xos =(com.sun.star.io.XOutputStream)pValue[i].Value;
            //System.out.println(pValue[i].Name+" "+xos);
        }
        if (pValue[i].Name.compareTo("FileName")==0){
            sFileName=(String) pValue[i].Value;
            //System.out.println(pValue[i].Name+" "+sFileName);
        }
        if (pValue[i].Name.compareTo("Title")==0){
            title=(String) pValue[i].Value;
            //System.out.println(pValue[i].Name+" "+title);
        }
        if (pValue[i].Name.compareTo("URL")==0){
            sURL = (String)pValue[i].Value;
            // System.out.println("\nMediaDescriptor url "+pValue[i].Name+" "+sURL);
        }
        }


        if (sFileName==null){
        sFileName="";
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



       public void  startDocument ()    {

       }

    public void endDocument()
    {
        try{
        convert (xInStream,xos,true,udJarPath,sFileName,offMime,sdMime);
        }
        catch (IOException e){
        System.out.println("Exception "+e);
        }
         catch (Exception e){
        System.out.println("Exception "+e);
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
            str=str.concat(xattribs.getValueByIndex(i));
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
        //System.out.println(str);

    }
    public void characters(String str){
        //System.out.println(str);
        String tmp=str;
        if (tmp.indexOf("<")!=-1){
        str=tmp.substring(0,tmp.indexOf("<"));
        str=str.concat("&lt;");
        str=str.concat(tmp.substring(tmp.indexOf("<")+1,tmp.length()));
        tmp=str;
        }
        if (tmp.indexOf(">")!=-1){
        str=tmp.substring(0,tmp.indexOf(">"));
        str=str.concat("&gt;");
        str=str.concat(tmp.substring(tmp.indexOf(">")+1,tmp.length()));
        tmp=str;
        }
        try{
         xOutStream.writeBytes(tmp.getBytes("UTF-8"));
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
                     while (docEnum.hasMoreElements() && sFileName.startsWith("file:")) {
                     URI uri=new URI(sFileName);
                     String newFileName = getPath(uri);
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



