/*************************************************************************
 *
 *  $RCSfile: PageHelper.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-06-10 10:22:10 $
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

// __________ Imports __________

// base classes
import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.*;


// application specific classes
import com.sun.star.drawing.*;

// presentation specific classes
import com.sun.star.presentation.*;

import com.sun.star.beans.XPropertySet;
import com.sun.star.awt.Size;


public class PageHelper
{
    // __________ static helper methods __________

    // __________ draw pages __________

    /** get the page count for standard pages
    */
    static public int getDrawPageCount( XComponent xComponent )
    {
        XDrawPagesSupplier xDrawPagesSupplier =
            (XDrawPagesSupplier)UnoRuntime.queryInterface(
                XDrawPagesSupplier.class, xComponent );
        XDrawPages xDrawPages = xDrawPagesSupplier.getDrawPages();
        return xDrawPages.getCount();
    }

    /** get draw page by index
    */
    static public XDrawPage getDrawPageByIndex( XComponent xComponent, int nIndex )
        throws com.sun.star.lang.IndexOutOfBoundsException,
            com.sun.star.lang.WrappedTargetException
    {
        XDrawPagesSupplier xDrawPagesSupplier =
            (XDrawPagesSupplier)UnoRuntime.queryInterface(
                XDrawPagesSupplier.class, xComponent );
        XDrawPages xDrawPages = xDrawPagesSupplier.getDrawPages();
        return (XDrawPage)UnoRuntime.queryInterface(XDrawPage.class, xDrawPages.getByIndex( nIndex ));
    }

    /** creates and inserts a draw page into the giving position,
        the method returns the new created page
    */
    static public XDrawPage insertNewDrawPageByIndex( XComponent xComponent, int nIndex )
        throws Exception
    {
        XDrawPagesSupplier xDrawPagesSupplier =
            (XDrawPagesSupplier)UnoRuntime.queryInterface(
                XDrawPagesSupplier.class, xComponent );
        XDrawPages xDrawPages = xDrawPagesSupplier.getDrawPages();
        return xDrawPages.insertNewByIndex( nIndex );
    }

    /** removes the given page
    */
    static public void removeDrawPage( XComponent xComponent, XDrawPage xDrawPage )
    {
        XDrawPagesSupplier xDrawPagesSupplier =
            (XDrawPagesSupplier)UnoRuntime.queryInterface(
                XDrawPagesSupplier.class, xComponent );
        XDrawPages xDrawPages = xDrawPagesSupplier.getDrawPages();
        xDrawPages.remove( xDrawPage );
    }

    /** get size of the given page
    */
    static public Size getPageSize( XDrawPage xDrawPage )
        throws com.sun.star.beans.UnknownPropertyException,
            com.sun.star.lang.WrappedTargetException
    {
        XPropertySet xPageProperties = (XPropertySet)
            UnoRuntime.queryInterface( XPropertySet.class, xDrawPage );
        return new Size(
            ((Integer)xPageProperties.getPropertyValue( "Width" )).intValue(),
            ((Integer)xPageProperties.getPropertyValue( "Height" )).intValue() );
    }

    // __________ master pages __________

    /** get the page count for master pages
    */
    static public int getMasterPageCount( XComponent xComponent )
    {
        XMasterPagesSupplier xMasterPagesSupplier =
            (XMasterPagesSupplier)UnoRuntime.queryInterface(
                XMasterPagesSupplier.class, xComponent );
        XDrawPages xDrawPages = xMasterPagesSupplier.getMasterPages();
        return xDrawPages.getCount();
    }

    /** get master page by index
    */
    static public XDrawPage getMasterPageByIndex( XComponent xComponent, int nIndex )
        throws com.sun.star.lang.IndexOutOfBoundsException,
            com.sun.star.lang.WrappedTargetException
    {
        XMasterPagesSupplier xMasterPagesSupplier =
            (XMasterPagesSupplier)UnoRuntime.queryInterface(
                XMasterPagesSupplier.class, xComponent );
        XDrawPages xDrawPages = xMasterPagesSupplier.getMasterPages();
        return (XDrawPage)UnoRuntime.queryInterface(XDrawPage.class, xDrawPages.getByIndex( nIndex ));
    }

    /** creates and inserts a new master page into the giving position,
        the method returns the new created page
    */
    static public XDrawPage insertNewMasterPageByIndex( XComponent xComponent, int nIndex )
    {
        XMasterPagesSupplier xMasterPagesSupplier =
            (XMasterPagesSupplier)UnoRuntime.queryInterface(
                XMasterPagesSupplier.class, xComponent );
        XDrawPages xDrawPages = xMasterPagesSupplier.getMasterPages();
        return xDrawPages.insertNewByIndex( nIndex );
    }

    /** removes the given page
    */
    static public void removeMasterPage( XComponent xComponent, XDrawPage xDrawPage )
    {
        XMasterPagesSupplier xMasterPagesSupplier =
            (XMasterPagesSupplier)UnoRuntime.queryInterface(
                XMasterPagesSupplier.class, xComponent );
        XDrawPages xDrawPages = xMasterPagesSupplier.getMasterPages();
        xDrawPages.remove( xDrawPage );
    }

    /** return the corresponding masterpage for the giving drawpage
    */
    static public XDrawPage getMasterPage( XDrawPage xDrawPage )
    {
        XMasterPageTarget xMasterPageTarget =
            (XMasterPageTarget)UnoRuntime.queryInterface(
                XMasterPageTarget.class, xDrawPage );
        return xMasterPageTarget.getMasterPage();
    }

    /** sets given masterpage at the drawpage
    */
    static public void setMasterPage( XDrawPage xDrawPage, XDrawPage xMasterPage )
    {
        XMasterPageTarget xMasterPageTarget =
            (XMasterPageTarget)UnoRuntime.queryInterface(
                XMasterPageTarget.class, xDrawPage );
        xMasterPageTarget.setMasterPage( xMasterPage );
    }

    // __________ presentation pages __________

    /** test if a Presentation Document is supported.
        This is important, because only presentation documents
        have notes and handout pages
    */
    static public boolean isImpressDocument( XComponent xComponent )
    {
        XServiceInfo xInfo = (XServiceInfo)UnoRuntime.queryInterface(
                XServiceInfo.class, xComponent );
        return xInfo.supportsService( "com.sun.star.presentation.PresentationDocument" );
    }

    /** in impress documents each normal draw page has a corresponding notes page
    */
    static public XDrawPage getNotesPage( XDrawPage xDrawPage )
    {
        XPresentationPage aPresentationPage =
            (XPresentationPage)UnoRuntime.queryInterface(
                XPresentationPage.class, xDrawPage );
        return aPresentationPage.getNotesPage();
    }

    /** in impress each documents has one handout page
    */
    static public XDrawPage getHandoutMasterPage( XComponent xComponent )
    {
        XHandoutMasterSupplier aHandoutMasterSupplier =
            (XHandoutMasterSupplier)UnoRuntime.queryInterface(
                XHandoutMasterSupplier.class, xComponent );
        return aHandoutMasterSupplier.getHandoutMasterPage();
    }
}
