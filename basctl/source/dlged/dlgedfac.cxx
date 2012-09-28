/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "dlgedfac.hxx"
#include "dlgedobj.hxx"
#include <dlgeddef.hxx>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/ScrollBarOrientation.hpp>

namespace basctl
{

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
    static bool bNeedsInit = true;
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
        bNeedsInit = false;
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
                        sal_Bool bB = true;
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

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
