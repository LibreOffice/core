/*************************************************************************
 *
 *  $RCSfile: DesktopTools.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change:$Date: 2003-01-27 16:27:03 $
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

package util;

// access the implementations via names
import com.sun.star.uno.XInterface;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;

import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.beans.PropertyValue;
import com.sun.star.lang.XComponent;

/**
 * contains helper methods for the Desktop
 */

public class DesktopTools {

    /**
     * Queries the XComponentLoader
     *
     * @param xMSF the MultiServiceFactory
     * @return the gained XComponentLoader
    */

    public static XComponentLoader getCLoader( XMultiServiceFactory xMSF ) {
        XDesktop oDesktop = ( XDesktop ) UnoRuntime.queryInterface(
                                    XDesktop.class, createDesktop(xMSF) );

        XComponentLoader oCLoader = ( XComponentLoader )
            UnoRuntime.queryInterface( XComponentLoader.class, oDesktop );

        return oCLoader;
    } // finish getCLoader

    /**
     * Creates an Instance of the Desktop service
     *
     * @param xMSF the MultiServiceFactory
     * @return the gained Object
    */

    public static Object createDesktop( XMultiServiceFactory xMSF ) {
            Object oInterface;
            try {
                oInterface = xMSF.createInstance( "com.sun.star.frame.Desktop" );
            }
            catch( com.sun.star.uno.Exception e ) {
                throw new IllegalArgumentException(
                                            "Desktop Service not available" );
        }
        return oInterface;
    } //finish createDesktop

    /**
     * Opens a new document of a given kind
     * with arguments
     * @param xMSF the MultiServiceFactory
     * @return the XComponent Interface of the document
    */

    public static XComponent openNewDoc( XMultiServiceFactory xMSF, String kind,
                                                        PropertyValue[] Args ) {

        XComponent oDoc = null ;
        try {
            oDoc = getCLoader(xMSF).loadComponentFromURL(
                                "private:factory/"+kind, "_blank", 0, Args );
        }
        catch (com.sun.star.uno.Exception e) {
            throw new IllegalArgumentException( "Document could not be opened" );
        }

        return oDoc;
    } //finish openNewDoc

    /**
     * loads a document of from a given url
     * with arguments
     * @param xMSF the MultiServiceFactory
     * @return the XComponent Interface of the document
    */

    public static XComponent loadDoc( XMultiServiceFactory xMSF, String url,
                                                        PropertyValue[] Args ) {

        XComponent oDoc = null ;
        try {
            oDoc = getCLoader(xMSF).loadComponentFromURL( url, "_blank", 0, Args );
        }
        catch (com.sun.star.uno.Exception e) {
            throw new IllegalArgumentException( "Document could not be loaded" );
        }

        return oDoc;
    } //finish openNewDoc

}