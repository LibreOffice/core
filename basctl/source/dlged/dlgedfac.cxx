/*************************************************************************
 *
 *  $RCSfile: dlgedfac.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: tbe $ $Date: 2001-08-17 13:57:40 $
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


#ifndef _BASCTL_DLGEDFAC_HXX
#include "dlgedfac.hxx"
#endif

#ifndef _BASCTL_DLGEDOBJ_HXX
#include "dlgedobj.hxx"
#endif

#ifndef _BASCTL_DLGEDDEF_HXX
#include <dlgeddef.hxx>
#endif

#ifndef _BASCTL_PROPBRW_HXX
#include "propbrw.hxx"
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_SCROLLBARORIENTATION_HPP_
#include <com/sun/star/awt/ScrollBarOrientation.hpp>
#endif

using namespace ::com::sun::star;

//----------------------------------------------------------------------------

DlgEdFactory::DlgEdFactory()
{
    SdrObjFactory::InsertMakeObjectHdl( LINK(this, DlgEdFactory, MakeObject) );

    PropBrwMgr::RegisterChildWindow();
}

//----------------------------------------------------------------------------

DlgEdFactory::~DlgEdFactory()
{
    SdrObjFactory::RemoveMakeObjectHdl( LINK(this, DlgEdFactory, MakeObject) );
}

//----------------------------------------------------------------------------

IMPL_LINK( DlgEdFactory, MakeObject, SdrObjFactory *, pObjFactory )
{
    static sal_Bool bNeedsInit = sal_True;
    static uno::Reference< lang::XMultiServiceFactory > xDialogSFact;

    if( bNeedsInit )
    {
        uno::Reference< lang::XMultiServiceFactory > xMSF = ::comphelper::getProcessServiceFactory();
        uno::Reference< container::XNameContainer > xC( xMSF->createInstance( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlDialogModel" ) ) ), uno::UNO_QUERY );
        if( xC.is() )
        {
            uno::Reference< lang::XMultiServiceFactory > xModFact( xC, uno::UNO_QUERY );
            xDialogSFact = xModFact;
        }
        bNeedsInit = sal_False;
    }

    if( (pObjFactory->nInventor == DlgInventor) &&
        (pObjFactory->nIdentifier >= OBJ_DLG_PUSHBUTTON) &&
        (pObjFactory->nIdentifier <= OBJ_DLG_VFIXEDLINE)    )
    {
        switch( pObjFactory->nIdentifier )
        {
            case OBJ_DLG_PUSHBUTTON:
                 pObjFactory->pNewObj = new DlgEdObj( ::rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlButtonModel") , xDialogSFact );
                  break;
            case OBJ_DLG_RADIOBUTTON:
                 pObjFactory->pNewObj = new DlgEdObj( ::rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlRadioButtonModel") , xDialogSFact );
                 break;
            case OBJ_DLG_CHECKBOX:
                 pObjFactory->pNewObj = new DlgEdObj( ::rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlCheckBoxModel") , xDialogSFact );
                 break;
            case OBJ_DLG_LISTBOX:
                 pObjFactory->pNewObj = new DlgEdObj( ::rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlListBoxModel") , xDialogSFact );
                 break;
            case OBJ_DLG_COMBOBOX:
            {
                 DlgEdObj* pNew = new DlgEdObj( ::rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlComboBoxModel") , xDialogSFact );
                 pObjFactory->pNewObj = pNew;
                 try
                 {
                    uno::Reference< beans::XPropertySet >  xPSet(pNew->GetUnoControlModel(), uno::UNO_QUERY);
                    if (xPSet.is())
                    {
                        sal_Bool bB = sal_True;
                        xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Dropdown" ) ), uno::Any(&bB,::getBooleanCppuType()));
                    }
                 }
                 catch(...)
                 {
                 }
            }    break;
            case OBJ_DLG_GROUPBOX:
                 pObjFactory->pNewObj = new DlgEdObj( ::rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlGroupBoxModel") , xDialogSFact );
                 break;
            case OBJ_DLG_EDIT:
                 pObjFactory->pNewObj = new DlgEdObj( ::rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlEditModel") , xDialogSFact );
                 break;
            case OBJ_DLG_FIXEDTEXT:
                 pObjFactory->pNewObj = new DlgEdObj( ::rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlFixedTextModel") , xDialogSFact );
                 break;
            case OBJ_DLG_IMAGECONTROL:
                 pObjFactory->pNewObj = new DlgEdObj( ::rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlImageControlModel") , xDialogSFact );
                 break;
            case OBJ_DLG_PROGRESSBAR:
                 pObjFactory->pNewObj = new DlgEdObj( ::rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlProgressBarModel") , xDialogSFact );
                 break;
            case OBJ_DLG_HSCROLLBAR:
                 pObjFactory->pNewObj = new DlgEdObj( ::rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlScrollBarModel") , xDialogSFact );
                 break;
            case OBJ_DLG_VSCROLLBAR:
            {
                 DlgEdObj* pNew = new DlgEdObj( ::rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlScrollBarModel") , xDialogSFact );
                 pObjFactory->pNewObj = pNew;
                 // set vertical orientation
                 try
                 {
                    uno::Reference< beans::XPropertySet >  xPSet(pNew->GetUnoControlModel(), uno::UNO_QUERY);
                    if (xPSet.is())
                    {
                        uno::Any aValue;
                        aValue <<= (sal_Int32) ::com::sun::star::awt::ScrollBarOrientation::VERTICAL;
                        xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Orientation" ) ), aValue );
                    }
                 }
                 catch(...)
                 {
                 }
            }    break;
            case OBJ_DLG_HFIXEDLINE:
                 pObjFactory->pNewObj = new DlgEdObj( ::rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlFixedLineModel") , xDialogSFact );
                 break;
            case OBJ_DLG_VFIXEDLINE:
            {
                 DlgEdObj* pNew = new DlgEdObj( ::rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlFixedLineModel") , xDialogSFact );
                 pObjFactory->pNewObj = pNew;
                 // set vertical orientation
                 try
                 {
                    uno::Reference< beans::XPropertySet >  xPSet(pNew->GetUnoControlModel(), uno::UNO_QUERY);
                    if (xPSet.is())
                    {
                        uno::Any aValue;
                        aValue <<= (sal_Int32) 1;
                        xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Orientation" ) ), aValue );
                    }
                 }
                 catch(...)
                 {
                 }
            }    break;
        }

        DlgEdObj* pDlgEdObj = PTR_CAST(DlgEdObj, pObjFactory->pNewObj);
        if (pDlgEdObj)
        {
            pDlgEdObj->StartListening();
        }
    }

    return 0;
}

//----------------------------------------------------------------------------

