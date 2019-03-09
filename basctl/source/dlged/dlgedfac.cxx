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

#include <dlgedfac.hxx>
#include <dlgedobj.hxx>
#include <dlgeddef.hxx>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/ScrollBarOrientation.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <comphelper/processfactory.hxx>

namespace basctl
{

using namespace ::com::sun::star;


DlgEdFactory::DlgEdFactory( const css::uno::Reference< css::frame::XModel >& xModel ) : mxModel( xModel )
{
    SdrObjFactory::InsertMakeObjectHdl( LINK(this, DlgEdFactory, MakeObject) );
}


DlgEdFactory::~DlgEdFactory() COVERITY_NOEXCEPT_FALSE
{
    SdrObjFactory::RemoveMakeObjectHdl( LINK(this, DlgEdFactory, MakeObject) );
}


IMPL_LINK( DlgEdFactory, MakeObject, SdrObjCreatorParams, aParams, SdrObject* )
{
    static bool bNeedsInit = true;
    static uno::Reference< lang::XMultiServiceFactory > xDialogSFact;

    if( bNeedsInit )
    {
        uno::Reference< uno::XComponentContext> xContext = ::comphelper::getProcessComponentContext();
        uno::Reference< container::XNameContainer > xC( xContext->getServiceManager()->createInstanceWithContext( "com.sun.star.awt.UnoControlDialogModel", xContext ), uno::UNO_QUERY );
        if( xC.is() )
        {
            uno::Reference< lang::XMultiServiceFactory > xModFact( xC, uno::UNO_QUERY );
            xDialogSFact = xModFact;
        }
        bNeedsInit = false;
    }

    SdrObject* pNewObj = nullptr;
    if( (aParams.nInventor == SdrInventor::BasicDialog) &&
        (aParams.nObjIdentifier >= OBJ_DLG_PUSHBUTTON) &&
        (aParams.nObjIdentifier <= OBJ_DLG_FORMHSCROLL)    )
    {
        switch( aParams.nObjIdentifier )
        {
            case OBJ_DLG_PUSHBUTTON:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, "com.sun.star.awt.UnoControlButtonModel", xDialogSFact );
                 break;
            case OBJ_DLG_RADIOBUTTON:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, "com.sun.star.awt.UnoControlRadioButtonModel", xDialogSFact );
                 break;
            case OBJ_DLG_FORMRADIO:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, "com.sun.star.form.component.RadioButton", xDialogSFact );
                 static_cast< DlgEdObj* >( pNewObj )->MakeDataAware( mxModel );
                 break;
            case OBJ_DLG_CHECKBOX:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, "com.sun.star.awt.UnoControlCheckBoxModel", xDialogSFact );
                 break;
            case OBJ_DLG_FORMCHECK:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, "com.sun.star.form.component.CheckBox", xDialogSFact );
                 static_cast< DlgEdObj* >( pNewObj )->MakeDataAware( mxModel );
                 break;
            case OBJ_DLG_LISTBOX:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, "com.sun.star.awt.UnoControlListBoxModel", xDialogSFact );
                 break;
            case OBJ_DLG_FORMLIST:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, "com.sun.star.form.component.ListBox", xDialogSFact );
                 static_cast< DlgEdObj* >( pNewObj )->MakeDataAware( mxModel );
                 break;
            case OBJ_DLG_FORMCOMBO:
            case OBJ_DLG_COMBOBOX:
            {
                 DlgEdObj* pNew = nullptr;
                 if ( aParams.nObjIdentifier == OBJ_DLG_COMBOBOX )
                     pNew = new DlgEdObj(aParams.rSdrModel, "com.sun.star.awt.UnoControlComboBoxModel", xDialogSFact );
                 else
                 {
                     pNew = new DlgEdObj(aParams.rSdrModel, "com.sun.star.form.component.ComboBox", xDialogSFact );
                     pNew->MakeDataAware( mxModel );
                 }
                 pNewObj = pNew;
                 try
                 {
                    uno::Reference< beans::XPropertySet >  xPSet(pNew->GetUnoControlModel(), uno::UNO_QUERY);
                    if (xPSet.is())
                    {
                        xPSet->setPropertyValue( DLGED_PROP_DROPDOWN, uno::Any(true));
                    }
                 }
                 catch(...)
                 {
                 }
            }
            break;
            case OBJ_DLG_GROUPBOX:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, "com.sun.star.awt.UnoControlGroupBoxModel", xDialogSFact );
                 break;
            case OBJ_DLG_EDIT:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, "com.sun.star.awt.UnoControlEditModel", xDialogSFact );
                 break;
            case OBJ_DLG_FIXEDTEXT:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, "com.sun.star.awt.UnoControlFixedTextModel", xDialogSFact );
                 break;
            case OBJ_DLG_IMAGECONTROL:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, "com.sun.star.awt.UnoControlImageControlModel", xDialogSFact );
                 break;
            case OBJ_DLG_PROGRESSBAR:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, "com.sun.star.awt.UnoControlProgressBarModel", xDialogSFact );
                 break;
            case OBJ_DLG_HSCROLLBAR:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, "com.sun.star.awt.UnoControlScrollBarModel", xDialogSFact );
                 break;
            case OBJ_DLG_FORMHSCROLL:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, "com.sun.star.form.component.ScrollBar", xDialogSFact );
                 static_cast< DlgEdObj* >( pNewObj )->MakeDataAware( mxModel );
                 break;
            case OBJ_DLG_FORMVSCROLL:
            case OBJ_DLG_VSCROLLBAR:
            {
                 DlgEdObj* pNew = nullptr;
                 if ( aParams.nObjIdentifier ==  OBJ_DLG_VSCROLLBAR )
                     pNew = new DlgEdObj(aParams.rSdrModel, "com.sun.star.awt.UnoControlScrollBarModel", xDialogSFact );
                 else
                 {
                     pNew = new DlgEdObj(aParams.rSdrModel, "com.sun.star.form.component.ScrollBar", xDialogSFact );
                     pNew->MakeDataAware( mxModel );
                 }
                 pNewObj = pNew;
                 // set vertical orientation
                 try
                 {
                    uno::Reference< beans::XPropertySet >  xPSet(pNew->GetUnoControlModel(), uno::UNO_QUERY);
                    if (xPSet.is())
                    {
                        xPSet->setPropertyValue( DLGED_PROP_ORIENTATION, uno::Any(sal_Int32(css::awt::ScrollBarOrientation::VERTICAL)) );
                    }
                 }
                 catch(...)
                 {
                 }
            }    break;
            case OBJ_DLG_HFIXEDLINE:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, "com.sun.star.awt.UnoControlFixedLineModel", xDialogSFact );
                 break;
            case OBJ_DLG_VFIXEDLINE:
            {
                 DlgEdObj* pNew = new DlgEdObj(aParams.rSdrModel, "com.sun.star.awt.UnoControlFixedLineModel", xDialogSFact );
                 pNewObj = pNew;
                 // set vertical orientation
                 try
                 {
                    uno::Reference< beans::XPropertySet >  xPSet(pNew->GetUnoControlModel(), uno::UNO_QUERY);
                    if (xPSet.is())
                    {
                        xPSet->setPropertyValue( DLGED_PROP_ORIENTATION, uno::Any(sal_Int32(1)) );
                    }
                 }
                 catch(...)
                 {
                 }
            }    break;
            case OBJ_DLG_DATEFIELD:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, "com.sun.star.awt.UnoControlDateFieldModel", xDialogSFact );
                 break;
            case OBJ_DLG_TIMEFIELD:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, "com.sun.star.awt.UnoControlTimeFieldModel", xDialogSFact );
                 break;
            case OBJ_DLG_NUMERICFIELD:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, "com.sun.star.awt.UnoControlNumericFieldModel", xDialogSFact );
                 break;
            case OBJ_DLG_CURRENCYFIELD:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, "com.sun.star.awt.UnoControlCurrencyFieldModel", xDialogSFact );
                 break;
            case OBJ_DLG_FORMATTEDFIELD:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, "com.sun.star.awt.UnoControlFormattedFieldModel", xDialogSFact );
                 break;
            case OBJ_DLG_PATTERNFIELD:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, "com.sun.star.awt.UnoControlPatternFieldModel", xDialogSFact );
                 break;
            case OBJ_DLG_FILECONTROL:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, "com.sun.star.awt.UnoControlFileControlModel", xDialogSFact );
                 break;
            case OBJ_DLG_SPINBUTTON:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, "com.sun.star.awt.UnoControlSpinButtonModel", xDialogSFact );
                 break;
            case OBJ_DLG_FORMSPIN:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, "com.sun.star.form.component.SpinButton", xDialogSFact );
                 static_cast< DlgEdObj* >( pNewObj )->MakeDataAware( mxModel );
                 break;
            case OBJ_DLG_TREECONTROL:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, "com.sun.star.awt.tree.TreeControlModel", xDialogSFact );
                 break;
            case OBJ_DLG_GRIDCONTROL:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, "com.sun.star.awt.grid.UnoControlGridModel", xDialogSFact );
                 break;
            case OBJ_DLG_HYPERLINKCONTROL:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, "com.sun.star.awt.UnoControlFixedHyperlinkModel", xDialogSFact );
                 break;

        }
    }
    return pNewObj;
}

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
