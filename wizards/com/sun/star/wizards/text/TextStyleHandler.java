/*************************************************************************
*
*  $RCSfile: TextStyleHandler.java,v $
*
*  $Revision: 1.2 $
*
*  last change: $Author: kz $ $Date: 2004-05-19 12:49:26 $
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
*/
package com.sun.star.wizards.text;
import com.sun.star.style.XStyleFamiliesSupplier;

public class TextStyleHandler {
    public XStyleFamiliesSupplier xStyleFamiliesSupplier;
    private com.sun.star.lang.XMultiServiceFactory xMSFDoc;
    private com.sun.star.text.XTextDocument xTextDocument;

    /** Creates a new instance of TextStyleHandler */
    public TextStyleHandler(com.sun.star.lang.XMultiServiceFactory xMSF, com.sun.star.text.XTextDocument xTextDocument) {
        this.xMSFDoc = xMSF;
        this.xTextDocument = xTextDocument;
        xStyleFamiliesSupplier = (XStyleFamiliesSupplier) com.sun.star.uno.UnoRuntime.queryInterface(XStyleFamiliesSupplier.class, xTextDocument);
    }

    public void loadStyleTemplates(String sTemplateUrl, String OptionString) {
        try {
            XStyleFamiliesSupplier xStyleFamiliesSupplier = (XStyleFamiliesSupplier) com.sun.star.uno.UnoRuntime.queryInterface(XStyleFamiliesSupplier.class, xTextDocument);
            com.sun.star.style.XStyleLoader xStyleLoader = (com.sun.star.style.XStyleLoader) com.sun.star.uno.UnoRuntime.queryInterface(com.sun.star.style.XStyleLoader.class, xStyleFamiliesSupplier.getStyleFamilies());
            com.sun.star.beans.PropertyValue[] StyleOptions = xStyleLoader.getStyleLoaderOptions();
            String CurOptionName = "";
            int PropCount = StyleOptions.length;
            for (int i = 0; i < PropCount; i++) {
                CurOptionName = StyleOptions[i].Name;
                StyleOptions[i].Value = new Boolean((CurOptionName.compareTo(OptionString) == 0) || (CurOptionName.compareTo("OverwriteStyles") == 0));
            }
            xStyleLoader.loadStylesFromURL(sTemplateUrl, StyleOptions);
        } catch (com.sun.star.uno.Exception exception) {
            exception.printStackTrace(System.out);
        }
    }

}
