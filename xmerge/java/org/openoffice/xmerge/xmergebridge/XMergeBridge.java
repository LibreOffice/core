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
 ************************************************************************

import com.sun.star.comp.loader.FactoryHelper;
import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XServiceName;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.uno.Type;
import com.sun.star.documentconversion.XConverterBridge;
import org.openoffice.xmerge.util.registry.ConverterInfoReader;
import org.openoffice.xmerge.util.registry.ConverterInfo;
import org.openoffice.xmerge.Convert;
import org.openoffice.xmerge.ConverterFactory;
import java.util.Enumeration;
import java.util.Vector;
import java.io.InputStream;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import org.openoffice.xmerge.Document;
import org.openoffice.xmerge.ConvertData;
import org.openoffice.xmerge.converter.xml.OfficeDocument;

/** This outer class provides an inner class to implement the service
 * description, a method to instantiate the
 * component on demand (__getServiceFactory()), and a method to give
 * information about the component (__writeRegistryServiceInfo()).
 */
public class XMergeBridge {

    /** This inner class provides the component as a concrete implementation
     * of the service description. It implements the needed interfaces.
     * @implements XTypeProvider
     */
    static public class _XMergeBridge implements
        XConverterBridge,
        XServiceName,
        XServiceInfo,
        XTypeProvider {

        /** The component will be registered under this name.
         */
        static private final String __serviceName = "com.sun.star.documentconversion.XMergeBridge";

        public com.sun.star.uno.Type[] getTypes() {
            Type[] typeReturn = {};

            try {
                typeReturn = new Type[] {
                new Type( XTypeProvider.class ),
                new Type( XConverterBridge.class ),
                new Type( XServiceName.class ),
                new Type( XServiceInfo.class ) };
            }
            catch( Exception exception ) {

            }

            return( typeReturn );
        }

     public void convert (com.sun.star.io.XInputStream xml,
            com.sun.star.io.XOutputStream device,boolean convertFromOffice,String pluginUrl,String FileName) throws com.sun.star.uno.RuntimeException {
         System.out.println("\nFound the Convert method "+pluginUrl);
         String jarName = pluginUrl;
         String name=null;
         if(FileName.equalsIgnoreCase(""))
           name = "OutFile";
         else
          name = FileName.substring(0,(FileName.lastIndexOf(".")));

         ConverterInfo converterInfo = null;
         Enumeration ciEnum= null;
         XInputStreamToInputStreamAdapter xis =new XInputStreamToInputStreamAdapter(xml);

             XOutputStreamToOutputStreamAdapter xos =
                 new XOutputStreamToOutputStreamAdapter(device);

         try{
         ConverterInfoReader cir = new ConverterInfoReader(jarName,false);
         ciEnum =cir.getConverterInfoEnumeration();
         }
         catch(Exception e){
         System.out.println("Error:"+e);
         }

         if (convertFromOffice)
         {

         try {

             while (ciEnum.hasMoreElements()) {
             converterInfo = (ConverterInfo)ciEnum.nextElement();


             //System.out.println("\nFound the "+converterInfo.getDisplayName()+" Converter\n");



             ConverterFactory cf = new ConverterFactory();
             Convert cv = cf.getConverter(converterInfo,false);
             if (cv == null) {
                 System.out.println("\nNo plug-in exists to convert from <staroffice/sxw> to <specified format> ");
             }
             cv.addInputStream(name,(InputStream)xis,false);
             ConvertData dataOut = cv.convert();

             Enumeration docEnum = dataOut.getDocumentEnumeration();
             while (docEnum.hasMoreElements()) {
                 Document docOut      = (Document)docEnum.nextElement();
                 String fileName      = docOut.getFileName();

                 docOut.write(xos);
                 xos.flush();
                 xos.close();

             }



             }

         }
         catch (Exception e) {
             System.out.println("Error:"+e);
         }
         }
         else{

         try {

             while (ciEnum.hasMoreElements()) {
             converterInfo = (ConverterInfo)ciEnum.nextElement();

             //System.out.println("\nFound the "+converterInfo.getDisplayName()+" Converter\n");

                 ConverterFactory cf = new ConverterFactory();
             Convert cv = cf.getConverter(converterInfo,true);
             if (cv == null) {
                 System.out.println("\nNo plug-in exists to convert to <staroffice/sxw> from <specified format>");
             }

             cv.addInputStream(name,(InputStream)xis,false);
             ConvertData dataIn = cv.convert();

              Enumeration docEnum = dataIn.getDocumentEnumeration();
             while (docEnum.hasMoreElements()) {
                 OfficeDocument docIn      = (OfficeDocument)docEnum.nextElement();
                 docIn.write(xos,false);

             }
             xos.write(-1); //EOF character
             xos.flush();
             xos.close();


             }

         }
         catch (Exception e) {
             System.out.println("Error:"+e);
         }
         }

         //System.out.println("\nFinished converting\n");
         //System.out.flush();

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
