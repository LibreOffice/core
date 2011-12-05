/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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



#include "dlgedfac.hxx"
#include "dlgedobj.hxx"
#include <dlgeddef.hxx>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/ScrollBarOrientation.hpp>

using namespace ::com::sun::star;


DlgEdFactory::DlgEdFactory( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xModel ) : mxModel( xModel )
{
    SdrObjFactory::InsertMakeObjectHdl( LINK(this, DlgEdFactory, MakeObject) );
}


DlgEdFactory::~DlgEdFactory()
{
    SdrObjFactory::RemoveMakeObjectHdl( LINK(this, DlgEdFactory, MakeObject) );
}


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
        (pObjFactory->nIdentifier <= OBJ_DLG_FORMHSCROLL)    )
    {
        switch( pObjFactory->nIdentifier )
        {
            case OBJ_DLG_PUSHBUTTON:
                 pObjFactory->pNewObj = new DlgEdObj( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlButtonModel" )) , xDialogSFact );
                  break;
            case OBJ_DLG_FORMRADIO:
            case OBJ_DLG_RADIOBUTTON:
                 if ( pObjFactory->nIdentifier == OBJ_DLG_RADIOBUTTON )
                     pObjFactory->pNewObj = new DlgEdObj( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlRadioButtonModel" )) , xDialogSFact );
                 else
                 {
                     pObjFactory->pNewObj = new DlgEdObj( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.form.component.RadioButton" )) , xDialogSFact );
                     static_cast< DlgEdObj* >( pObjFactory->pNewObj )->MakeDataAware( mxModel );
                 }
                 break;
            case OBJ_DLG_FORMCHECK:
            case OBJ_DLG_CHECKBOX:
                 if ( pObjFactory->nIdentifier == OBJ_DLG_CHECKBOX )
                     pObjFactory->pNewObj = new DlgEdObj( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlCheckBoxModel" )) , xDialogSFact );
                 else
                     pObjFactory->pNewObj = new DlgEdObj( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.form.component.CheckBox" )) , xDialogSFact );
                     static_cast< DlgEdObj* >( pObjFactory->pNewObj )->MakeDataAware( mxModel );
                 break;
            case OBJ_DLG_FORMLIST:
            case OBJ_DLG_LISTBOX:
                 if ( pObjFactory->nIdentifier == OBJ_DLG_LISTBOX )
                     pObjFactory->pNewObj = new DlgEdObj( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlListBoxModel" )) , xDialogSFact );
                 else
                 {
                     pObjFactory->pNewObj = new DlgEdObj( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.form.component.ListBox" )) , xDialogSFact );
                     static_cast< DlgEdObj* >( pObjFactory->pNewObj )->MakeDataAware( mxModel );
                 }
                 break;
            case OBJ_DLG_FORMCOMBO:
            case OBJ_DLG_COMBOBOX:
            {
                 DlgEdObj* pNew = NULL;
                 if ( pObjFactory->nIdentifier == OBJ_DLG_COMBOBOX )
                     pNew = new DlgEdObj( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlComboBoxModel" )) , xDialogSFact );
                 else
                 {
                     pNew = new DlgEdObj( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.form.component.ComboBox" )) , xDialogSFact );
                     static_cast< DlgEdObj* >( pNew )->MakeDataAware( mxModel );
                 }
                 pObjFactory->pNewObj = pNew;
                 try
                 {
                    uno::Reference< beans::XPropertySet >  xPSet(pNew->GetUnoControlModel(), uno::UNO_QUERY);
                    if (xPSet.is())
                    {
                        sal_Bool bB = sal_True;
                        xPSet->setPropertyValue( DLGED_PROP_DROPDOWN, uno::Any(&bB,::getBooleanCppuType()));
                    }
                 }
                 catch(...)
                 {
                 }
            }    break;
            case OBJ_DLG_GROUPBOX:
                 pObjFactory->pNewObj = new DlgEdObj( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlGroupBoxModel" )) , xDialogSFact );
                 break;
            case OBJ_DLG_EDIT:
                 pObjFactory->pNewObj = new DlgEdObj( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlEditModel" )) , xDialogSFact );
                 break;
            case OBJ_DLG_FIXEDTEXT:
                 pObjFactory->pNewObj = new DlgEdObj( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlFixedTextModel" )) , xDialogSFact );
                 break;
            case OBJ_DLG_IMAGECONTROL:
                 pObjFactory->pNewObj = new DlgEdObj( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlImageControlModel" )) , xDialogSFact );
                 break;
            case OBJ_DLG_PROGRESSBAR:
                 pObjFactory->pNewObj = new DlgEdObj( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlProgressBarModel" )) , xDialogSFact );
                 break;
            case OBJ_DLG_FORMHSCROLL:
            case OBJ_DLG_HSCROLLBAR:
                 if ( pObjFactory->nIdentifier == OBJ_DLG_HSCROLLBAR )
                     pObjFactory->pNewObj = new DlgEdObj( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlScrollBarModel" )) , xDialogSFact );
                 else
                 {
                     pObjFactory->pNewObj = new DlgEdObj( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.form.component.ScrollBar" )) , xDialogSFact );
                     static_cast< DlgEdObj* >( pObjFactory->pNewObj )->MakeDataAware( mxModel );
                 }
                 break;
            case OBJ_DLG_FORMVSCROLL:
            case OBJ_DLG_VSCROLLBAR:
            {
                 DlgEdObj* pNew = NULL;
                 if ( pObjFactory->nIdentifier ==  OBJ_DLG_VSCROLLBAR )
                     pNew = new DlgEdObj( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlScrollBarModel" )) , xDialogSFact );
                 else
                 {
                     pNew = new DlgEdObj( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.form.component.ScrollBar" )) , xDialogSFact );
                     static_cast< DlgEdObj* >( pNew )->MakeDataAware( mxModel );
                 }
                 pObjFactory->pNewObj = pNew;
                 // set vertical orientation
                 try
                 {
                    uno::Reference< beans::XPropertySet >  xPSet(pNew->GetUnoControlModel(), uno::UNO_QUERY);
                    if (xPSet.is())
                    {
                        uno::Any aValue;
                        aValue <<= (sal_Int32) ::com::sun::star::awt::ScrollBarOrientation::VERTICAL;
                        xPSet->setPropertyValue( DLGED_PROP_ORIENTATION, aValue );
                    }
                 }
                 catch(...)
                 {
                 }
            }    break;
            case OBJ_DLG_HFIXEDLINE:
                 pObjFactory->pNewObj = new DlgEdObj( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlFixedLineModel" )) , xDialogSFact );
                 break;
            case OBJ_DLG_VFIXEDLINE:
            {
                 DlgEdObj* pNew = new DlgEdObj( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlFixedLineModel" )) , xDialogSFact );
                 pObjFactory->pNewObj = pNew;
                 // set vertical orientation
                 try
                 {
                    uno::Reference< beans::XPropertySet >  xPSet(pNew->GetUnoControlModel(), uno::UNO_QUERY);
                    if (xPSet.is())
                    {
                        uno::Any aValue;
                        aValue <<= (sal_Int32) 1;
                        xPSet->setPropertyValue( DLGED_PROP_ORIENTATION, aValue );
                    }
                 }
                 catch(...)
                 {
                 }
            }    break;
            case OBJ_DLG_DATEFIELD:
                 pObjFactory->pNewObj = new DlgEdObj( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlDateFieldModel" )) , xDialogSFact );
                 break;
            case OBJ_DLG_TIMEFIELD:
                 pObjFactory->pNewObj = new DlgEdObj( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlTimeFieldModel" )) , xDialogSFact );
                 break;
            case OBJ_DLG_NUMERICFIELD:
                 pObjFactory->pNewObj = new DlgEdObj( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlNumericFieldModel" )) , xDialogSFact );
                 break;
            case OBJ_DLG_CURRENCYFIELD:
                 pObjFactory->pNewObj = new DlgEdObj( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlCurrencyFieldModel" )) , xDialogSFact );
                 break;
            case OBJ_DLG_FORMATTEDFIELD:
                 pObjFactory->pNewObj = new DlgEdObj( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlFormattedFieldModel" )) , xDialogSFact );
                 break;
            case OBJ_DLG_PATTERNFIELD:
                 pObjFactory->pNewObj = new DlgEdObj( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlPatternFieldModel" )) , xDialogSFact );
                 break;
            case OBJ_DLG_FILECONTROL:
                 pObjFactory->pNewObj = new DlgEdObj( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlFileControlModel" )) , xDialogSFact );
                 break;
            case OBJ_DLG_FORMSPIN:
            case OBJ_DLG_SPINBUTTON:
                 if ( pObjFactory->nIdentifier == OBJ_DLG_SPINBUTTON )
                     pObjFactory->pNewObj = new DlgEdObj( ::rtl::OUString::createFromAscii("com.sun.star.awt.UnoControlSpinButtonModel") , xDialogSFact );
                 else
                 {
                     pObjFactory->pNewObj = new DlgEdObj( ::rtl::OUString::createFromAscii("com.sun.star.form.component.SpinButton") , xDialogSFact );
                     static_cast< DlgEdObj* >( pObjFactory->pNewObj )->MakeDataAware( mxModel );
                 }
                 break;
            case OBJ_DLG_TREECONTROL:
                 DlgEdObj* pNew = new DlgEdObj( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.tree.TreeControlModel" )) , xDialogSFact );
                 pObjFactory->pNewObj = pNew;
                 break;
        }
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
