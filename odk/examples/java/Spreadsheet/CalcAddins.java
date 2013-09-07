/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

// Template for an Office Calc add-in Java implementation file.

/** You can find more
 * information on the following web page:
 * http://api.libreoffice.org/
 */
import com.sun.star.comp.loader.FactoryHelper;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.lib.uno.helper.WeakBase;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XServiceName;
import com.sun.star.sheet.XAddIn;
import com.sun.star.lang.Locale;
import org.openoffice.sheet.addin.XCalcAddins;

/** This outer class provides an inner class to implement the service
 * description, a method to instantiate the
 * component on demand (__getServiceFactory()), and a method to give
 * information about the component (__writeRegistryServiceInfo()).
 */
public class CalcAddins {

/** This inner class provides the component as a concrete implementation
 * of the service description. It implements the needed interfaces.
 * @implements XCalcAddins, XAddIn, XServiceName, XServiceInfo, XTypeProvider
 */
    static public class _CalcAddins extends WeakBase implements
                                            XCalcAddins,
                                            XAddIn,
                                            XServiceName,
                                            XServiceInfo
    {

/** The component will be registered under this name.
 */
        static private final String __serviceName = "org.openoffice.sheet.addin.CalcAddins";

        static private final String ADDIN_SERVICE = "com.sun.star.sheet.AddIn";

        private Locale aFuncLoc;

        private static final String[] stringFunctionName = {
/** TO DO:
 * You should replace these method names by the method names of your interface.
 */
            "getMyFirstValue",
            "getMySecondValue"
        };

        private static final short shortINVALID = -1;

/** TO DO:
 * For each of your methods you should make up a new constant with a different value.
 */
        private static final short shortGETMYFIRSTVALUE = 0;
        private static final short shortGETMYSECONDVALUE = 1;


/** TO DO:
 * This is where you implement all methods of your interface. The parameters have to
 * be the same as in your IDL file and their types have to be the correct
 * IDL-to-Java mappings of their types in the IDL file.
 */
        public int getMyFirstValue(
            com.sun.star.beans.XPropertySet xOptions
        ) {
            return 1;
        }

        public int getMySecondValue(
            com.sun.star.beans.XPropertySet xOptions,
            int intDummy
        ) {
            return( 2 + intDummy );
        }


        // Implement method from interface XServiceName
        public String getServiceName() {
            return( __serviceName );
        }

        // Implement methods from interface XServiceInfo
        public boolean supportsService(String stringServiceName) {
            return( stringServiceName.equals( ADDIN_SERVICE ) ||
                    stringServiceName.equals( __serviceName ) );
        }

        public String getImplementationName() {
            return( _CalcAddins.class.getName() );
        }

        public String[] getSupportedServiceNames() {
            String[] stringSupportedServiceNames = { ADDIN_SERVICE, __serviceName };
            return( stringSupportedServiceNames );
        }

        // Implement methods from interface XAddIn
        public String getDisplayArgumentName(String stringProgrammaticFunctionName,int intArgument) {
            String stringReturn = "";

            switch( this.getFunctionID( stringProgrammaticFunctionName ) ) {
/** TO DO:
 * You should list all argument names for each of your methods, here.
 */
                case shortGETMYFIRSTVALUE:
                    switch( intArgument ) {
                        case 0:
                            stringReturn = "(internal)";
                            break;
                    }
                    break;
                case shortGETMYSECONDVALUE:
                    switch( intArgument ) {
                        case 0:
                            stringReturn = "(internal)";
                            break;
                        case 1:
                            stringReturn = "intDummy";
                            break;
                    }
                    break;
            }
            return( stringReturn );
        }

        public String getDisplayFunctionName(String stringProgrammaticName) {
            String stringReturn = "";

            switch( this.getFunctionID( stringProgrammaticName ) ) {
/** TO DO:
 * Assign the name of each of your methods.
 */
                case shortGETMYFIRSTVALUE:
                    stringReturn = "getMyFirstValue";
                    break;
                case shortGETMYSECONDVALUE:
                    stringReturn = "getMySecondValue";
                    break;
            }

            return( stringReturn );
        }

        public String getProgrammaticCategoryName(String p1) {
            return( "Add-In" );
        }

        public String getDisplayCategoryName(String p1) {
            return( "Add-In" );
        }

        public String getFunctionDescription(String stringProgrammaticName) {
            String stringReturn = "";

            switch( this.getFunctionID( stringProgrammaticName ) ) {
/** TO DO:
 * Enter a description for each of your methods that office users will understand.
 */
                case shortGETMYFIRSTVALUE:
                    stringReturn = "This is your first method.";
                    break;
                case shortGETMYSECONDVALUE:
                    stringReturn = "This is your second method.";
                    break;
            }

            return( stringReturn );
        }

        public String getArgumentDescription(String stringProgrammaticFunctionName,int intArgument) {
            String stringReturn = "";

            switch( this.getFunctionID( stringProgrammaticFunctionName ) ) {
/** TO DO:
 * Enter a description for every argument of every method. Make them so that office users will understand.
 */
                case shortGETMYFIRSTVALUE:
                    switch( intArgument ) {
                        case 0:
                            stringReturn = "(internal)";
                            break;
                    }
                    break;
                case shortGETMYSECONDVALUE:
                    switch( intArgument ) {
                        case 0:
                            stringReturn = "(internal)";
                            break;
                        case 1:
                            stringReturn = "You can add this value.";
                            break;
                    }
                    break;
            }
            return( stringReturn );
        }

        public String getProgrammaticFuntionName(String p1) {
            return( "" );
        }

        // Implement methods from interface XLocalizable
        public Locale getLocale() {
            return( aFuncLoc );
        }

        public void setLocale(Locale p1) {
            aFuncLoc = p1;
        }

        // Auxiliary functions
        private short getFunctionID( String stringProgrammaticName ) {
            for ( int i = 0; i < stringFunctionName.length; i++ ) {
                if ( stringProgrammaticName.equals( stringFunctionName[ i ] ) ) {
                    return( ( short ) i );
                }
            }

            return( -1 );
        }
    }

    /**
     * Returns a factory for creating the service.
     * This method is called by the <code>JavaLoader</code>
     * <p>
     * @return  returns a <code>XSingleServiceFactory</code> for creating the component
     * @param   implName     the name of the implementation for which a service is desired
     * @param   multiFactory the service manager to be used if needed
     * @param   regKey       the registryKey
     * @see                  com.sun.star.comp.loader.JavaLoader
     */
    public static XSingleServiceFactory __getServiceFactory(String implName,
    XMultiServiceFactory multiFactory,
    XRegistryKey regKey) {
        XSingleServiceFactory xSingleServiceFactory = null;

        if (implName.equals(_CalcAddins.class.getName()) )
            xSingleServiceFactory = FactoryHelper.getServiceFactory(_CalcAddins.class,
            _CalcAddins.__serviceName,
            multiFactory,
            regKey);

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
    // This method not longer necessary since OOo 3.4 where the component registration
    // was changed to passive component registration. For more details see
    // http://wiki.openoffice.org/wiki/Passive_Component_Registration

//     public static boolean __writeRegistryServiceInfo(XRegistryKey regKey) {
//         return FactoryHelper.writeRegistryServiceInfo(_CalcAddins.class.getName(),
//         _CalcAddins.__serviceName, regKey)
//         && FactoryHelper.writeRegistryServiceInfo(_CalcAddins.class.getName(),
//         _CalcAddins.ADDIN_SERVICE, regKey);
//     }
}
