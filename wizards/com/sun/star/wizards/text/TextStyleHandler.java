/*************************************************************************
*
*  $RCSfile: TextStyleHandler.java,v $
*
*  $Revision: 1.3 $
*
*  last change: $Author: pjunck $ $Date: 2004-10-27 13:40:10 $
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

import com.sun.star.awt.Size;
import com.sun.star.beans.PropertyVetoException;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameAccess;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.style.XStyleFamiliesSupplier;
import com.sun.star.style.XStyleLoader;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;

public class TextStyleHandler {
    public XStyleFamiliesSupplier xStyleFamiliesSupplier;
    private XMultiServiceFactory xMSFDoc;
    private XTextDocument xTextDocument;

    /** Creates a new instance of TextStyleHandler */
    public TextStyleHandler(com.sun.star.lang.XMultiServiceFactory xMSF, XTextDocument xTextDocument) {
        this.xMSFDoc = xMSF;
        this.xTextDocument = xTextDocument;
        xStyleFamiliesSupplier = (XStyleFamiliesSupplier) UnoRuntime.queryInterface(XStyleFamiliesSupplier.class, xTextDocument);
    }

    public void loadStyleTemplates(String sTemplateUrl, String OptionString) {
        try {
            XStyleLoader xStyleLoader = (XStyleLoader) UnoRuntime.queryInterface(XStyleLoader.class, xStyleFamiliesSupplier.getStyleFamilies());
            com.sun.star.beans.PropertyValue[] StyleOptions = xStyleLoader.getStyleLoaderOptions();
            String CurOptionName = "";
            int PropCount = StyleOptions.length;
            for (int i = 0; i < PropCount; i++) {
                CurOptionName = StyleOptions[i].Name;
                StyleOptions[i].Value = new Boolean((CurOptionName.compareTo(OptionString) == 0) || (CurOptionName.compareTo("OverwriteStyles") == 0));
            }
            xStyleLoader.loadStylesFromURL(sTemplateUrl, StyleOptions);
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
        }
    }

    public XPropertySet getStyleByName(String sStyleFamily, String sStyleName){
    try {
        XPropertySet xPropertySet = null;
        Object oStyleFamily = xStyleFamiliesSupplier.getStyleFamilies().getByName(sStyleFamily);
        XNameAccess xNameAccess = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class, oStyleFamily);
        if (xNameAccess.hasByName(sStyleName))
            xPropertySet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xNameAccess.getByName(sStyleName));
        return xPropertySet;
    } catch (Exception e) {
        e.printStackTrace(System.out);
    }
    return null;
    }


    public Size changePageAlignment(XPropertySet _xPropPageStyle, boolean _bIsLandscape){
        try {
            _xPropPageStyle.setPropertyValue("IsLandscape", new Boolean(_bIsLandscape));
            Size aPageSize = (Size) AnyConverter.toObject(Size.class, _xPropPageStyle.getPropertyValue("Size"));
            int nPageWidth = aPageSize.Width;
            int nPageHeight = aPageSize.Height;
            Size aSize = new Size(nPageHeight, nPageWidth);
            _xPropPageStyle.setPropertyValue("Size", aSize);
            return (Size) AnyConverter.toObject(Size.class,_xPropPageStyle.getPropertyValue("Size"));
        } catch (Exception e) {
            e.printStackTrace(System.out);
            return null;
        }
    }
}
