/** You can find more
 * information on the following web page:
 * http://api.openoffice.org/common/ref/com/sun/star/index.html
 */
import com.sun.star.comp.loader.FactoryHelper;
import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XServiceName;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.uno.Type;
//import com.sun.star.documentconversion.XConverterBridge;
//import com.sun.star.document.XFilterAdapter;
import java.util.Enumeration;
import java.util.Vector;
import java.io.InputStream;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.ByteArrayOutputStream;
import com.sun.star.xml.XImportFilter;
import com.sun.star.xml.XExportFilter;
import java.io.*;


import com.sun.star.uno.AnyConverter;
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
public class XFlatXml {


    private static XMultiServiceFactory xMSF;
    private static XDocumentHandler exportDocHandler=null;
    private static XInputStream xInStream =null;
    private static XOutputStream xOutStream=null;
    private static String  udJarPath=null;
    private static XOutputStream xos = null;
    private static XOutputStreamToOutputStreamAdapter adaptedStream=null;
    private static String sFileName=null;



    /** This inner class provides the component as a concrete implementation
     * of the service description. It implements the needed interfaces.
     * @implements XTypeProvider
     */
    static public class _XFlatXml implements
                                    //XConverterBridge,
     XImportFilter,
     XExportFilter,
        XServiceName,
        XServiceInfo,
    XDocumentHandler,
        XTypeProvider {

        /** The component will be registered under this name.
         */
        static private final String __serviceName = "com.sun.star.documentconversion.XFlatXml";

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
        */
        sFileName=null;
        String sDirectory = null;
        String sURL=null;
        String udConvertClass=msUserData[0];
        udJarPath=msUserData[1];
        String udImport =msUserData[2];
        String udExport =msUserData[3];
        com.sun.star.io.XInputStream xis=null;
        com.sun.star.beans.PropertyValue[] pValue = aSourceData;

        for  (int  i = 0 ; i < pValue.length; i++)
        {
         try{
             //System.out.println("\n"+pValue[i].Name+" "+pValue[i].Value);
             if (pValue[i].Name.compareTo("InputStream")==0){
            xis=(com.sun.star.io.XInputStream)AnyConverter.toObject(new Type(com.sun.star.io.XInputStream.class), pValue[i].Value);
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

        try{

        Object xCfgMgrObj=xMSF.createInstance("com.sun.star.config.SpecialConfigManager");
         XConfigManager xCfgMgr = (XConfigManager) UnoRuntime.queryInterface(
                                            XConfigManager.class , xCfgMgrObj );
        String PathString=xCfgMgr.substituteVariables("$(progurl)" );
        PathString= PathString.concat("/");

        Object xSaxParserObj=xMSF.createInstance("com.sun.star.xml.sax.Parser");

        XParser xParser = (XParser) UnoRuntime.queryInterface(
                        XParser.class , xSaxParserObj );

        InputSource aInput = new InputSource();
        if (sFileName==null)
            sFileName=" ";
        aInput.sSystemId = sFileName;
        aInput.aInputStream =xis;
                xParser.setDocumentHandler ( xDocHandler );
        xParser.parseStream ( aInput );


        }
        catch (Exception e){
        //e.printStackTrace();
        System.out.println("\nException "+e);

        }
        return true;
    }



 public boolean exporter(com.sun.star.beans.PropertyValue[] aSourceData,
                    java.lang.String[] msUserData) throws com.sun.star.uno.RuntimeException,com.sun.star.lang.IllegalArgumentException{
                   /*
        System.out.println("\nFound the Exporter!\n");

        System.out.println("\n"+msUserData[0]);
        System.out.println("\n"+msUserData[1]);
        System.out.println("\n"+msUserData[2]);
        System.out.println("\n"+msUserData[3]);
        */
        sFileName=null;
        String sDirectory = null;
        String sURL=null;
        String udConvertClass=msUserData[0];
        udJarPath=msUserData[1];
        String udImport =msUserData[2];
        String udExport =msUserData[3];


        com.sun.star.beans.PropertyValue[] pValue = aSourceData;

        for  (int  i = 0 ; i < pValue.length; i++)
        {
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
           }
        catch (Exception e){
        System.out.println("Exception "+e);
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
        convert (xInStream,xos,true,udJarPath,sFileName);

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
        //str=str.concat("\n");
        try{
        // xOutStream.writeBytes(str.getBytes());
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



     public void convert (com.sun.star.io.XInputStream xml,
            com.sun.star.io.XOutputStream device,boolean convertFromOffice,String pluginUrl,String FileName) throws com.sun.star.uno.RuntimeException {
        //System.out.println("\nFound the Convert method "+pluginUrl);
         String jarName = pluginUrl;
         String name=getFileName(FileName);

         XInputStreamToInputStreamAdapter xis =new XInputStreamToInputStreamAdapter(xml);

         XOutputStreamToOutputStreamAdapter xos =
                 new XOutputStreamToOutputStreamAdapter(device);

         int b=0;
         ByteArrayOutputStream baos = new ByteArrayOutputStream();
         if (convertFromOffice)
         {
         try{
             while (xis.available()>0)
             {
                 byte[] bytearr = new byte[xis.available()];
                 xis.read(bytearr);
                 xos.write(bytearr);
             }

         }
         catch (Exception e){}
         }
         else{

         try{

               while (xis.available()>0)
             {
                 byte[] bytearr = new byte[xis.available()];
                 xis.read(bytearr);
                 xos.write(bytearr);

             }
           xos.close();
           xis.close();
           //xos.write(-1); //EOF character
          }
          catch (Exception e){}
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
            return( _XFlatXml.class.getName() );
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
        if (implName.equals(_XFlatXml.class.getName()) ) {
            xSingleServiceFactory = FactoryHelper.getServiceFactory(_XFlatXml.class,
            _XFlatXml.__serviceName,
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

        return FactoryHelper.writeRegistryServiceInfo(_XFlatXml.class.getName(),
        _XFlatXml.__serviceName, regKey);
    }
}
