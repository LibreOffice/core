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
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/ScrollBarOrientation.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <comphelper/processfactory.hxx>
#include <utility>

namespace basctl
{

using namespace ::com::sun::star;


DlgEdFactory::DlgEdFactory( css::uno::Reference< css::frame::XModel > xModel ) : mxModel(std::move( xModel ))
{
    SdrObjFactory::InsertMakeObjectHdl( LINK(this, DlgEdFactory, MakeObject) );
}


DlgEdFactory::~DlgEdFactory() COVERITY_NOEXCEPT_FALSE
{
    SdrObjFactory::RemoveMakeObjectHdl( LINK(this, DlgEdFactory, MakeObject) );
}


IMPL_LINK( DlgEdFactory, MakeObject, SdrObjCreatorParams, aParams, rtl::Reference<SdrObject> )
{
    static const uno::Reference<lang::XMultiServiceFactory> xDialogSFact = [] {
        uno::Reference<lang::XMultiServiceFactory> xFact;
        uno::Reference< uno::XComponentContext> xContext = ::comphelper::getProcessComponentContext();
        uno::Reference< container::XNameContainer > xC( xContext->getServiceManager()->createInstanceWithContext( u"com.sun.star.awt.UnoControlDialogModel"_ustr, xContext ), uno::UNO_QUERY );
        if (xC.is())
            xFact.set(xC, uno::UNO_QUERY);
        return xFact;
    }();

    rtl::Reference<SdrObject> pNewObj;
    if( (aParams.nInventor == SdrInventor::BasicDialog) &&
        (aParams.nObjIdentifier >= SdrObjKind::BasicDialogPushButton) &&
        (aParams.nObjIdentifier <= SdrObjKind::BasicDialogFormHorizontalScroll)    )
    {
        switch( aParams.nObjIdentifier )
        {
            case SdrObjKind::BasicDialogPushButton:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, u"com.sun.star.awt.UnoControlButtonModel"_ustr, xDialogSFact );
                 break;
            case SdrObjKind::BasicDialogRadioButton:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, u"com.sun.star.awt.UnoControlRadioButtonModel"_ustr, xDialogSFact );
                 break;
            case SdrObjKind::BasicDialogFormRadio:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, u"com.sun.star.form.component.RadioButton"_ustr, xDialogSFact );
                 static_cast< DlgEdObj* >( pNewObj.get() )->MakeDataAware( mxModel );
                 break;
            case SdrObjKind::BasicDialogCheckbox:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, u"com.sun.star.awt.UnoControlCheckBoxModel"_ustr, xDialogSFact );
                 break;
            case SdrObjKind::BasicDialogFormCheck:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, u"com.sun.star.form.component.CheckBox"_ustr, xDialogSFact );
                 static_cast< DlgEdObj* >( pNewObj.get() )->MakeDataAware( mxModel );
                 break;
            case SdrObjKind::BasicDialogListbox:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, u"com.sun.star.awt.UnoControlListBoxModel"_ustr, xDialogSFact );
                 break;
            case SdrObjKind::BasicDialogFormList:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, u"com.sun.star.form.component.ListBox"_ustr, xDialogSFact );
                 static_cast< DlgEdObj* >( pNewObj.get() )->MakeDataAware( mxModel );
                 break;
            case SdrObjKind::BasicDialogFormCombo:
            case SdrObjKind::BasicDialogCombobox:
            {
                 rtl::Reference<DlgEdObj> pNew;
                 if ( aParams.nObjIdentifier == SdrObjKind::BasicDialogCombobox )
                     pNew = new DlgEdObj(aParams.rSdrModel, u"com.sun.star.awt.UnoControlComboBoxModel"_ustr, xDialogSFact );
                 else
                 {
                     pNew = new DlgEdObj(aParams.rSdrModel, u"com.sun.star.form.component.ComboBox"_ustr, xDialogSFact );
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
            case SdrObjKind::BasicDialogGroupBox:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, u"com.sun.star.awt.UnoControlGroupBoxModel"_ustr, xDialogSFact );
                 break;
            case SdrObjKind::BasicDialogEdit:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, u"com.sun.star.awt.UnoControlEditModel"_ustr, xDialogSFact );
                 break;
            case SdrObjKind::BasicDialogFixedText:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, u"com.sun.star.awt.UnoControlFixedTextModel"_ustr, xDialogSFact );
                 break;
            case SdrObjKind::BasicDialogImageControl:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, u"com.sun.star.awt.UnoControlImageControlModel"_ustr, xDialogSFact );
                 break;
            case SdrObjKind::BasicDialogProgressbar:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, u"com.sun.star.awt.UnoControlProgressBarModel"_ustr, xDialogSFact );
                 break;
            case SdrObjKind::BasicDialogHorizontalScrollbar:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, u"com.sun.star.awt.UnoControlScrollBarModel"_ustr, xDialogSFact );
                 break;
            case SdrObjKind::BasicDialogFormHorizontalScroll:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, u"com.sun.star.form.component.ScrollBar"_ustr, xDialogSFact );
                 static_cast< DlgEdObj* >( pNewObj.get() )->MakeDataAware( mxModel );
                 break;
            case SdrObjKind::BasicDialogFormVerticalScroll:
            case SdrObjKind::BasicDialogVerticalScrollbar:
            {
                 rtl::Reference<DlgEdObj> pNew;
                 if ( aParams.nObjIdentifier == SdrObjKind::BasicDialogVerticalScrollbar )
                     pNew = new DlgEdObj(aParams.rSdrModel, u"com.sun.star.awt.UnoControlScrollBarModel"_ustr, xDialogSFact );
                 else
                 {
                     pNew = new DlgEdObj(aParams.rSdrModel, u"com.sun.star.form.component.ScrollBar"_ustr, xDialogSFact );
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
            case SdrObjKind::BasicDialogHorizontalFixedLine:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, u"com.sun.star.awt.UnoControlFixedLineModel"_ustr, xDialogSFact );
                 break;
            case SdrObjKind::BasicDialogVerticalFixedLine:
            {
                 rtl::Reference<DlgEdObj> pNew = new DlgEdObj(aParams.rSdrModel, u"com.sun.star.awt.UnoControlFixedLineModel"_ustr, xDialogSFact );
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
            case SdrObjKind::BasicDialogDateField:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, u"com.sun.star.awt.UnoControlDateFieldModel"_ustr, xDialogSFact );
                 break;
            case SdrObjKind::BasicDialogTimeField:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, u"com.sun.star.awt.UnoControlTimeFieldModel"_ustr, xDialogSFact );
                 break;
            case SdrObjKind::BasicDialogNumericField:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, u"com.sun.star.awt.UnoControlNumericFieldModel"_ustr, xDialogSFact );
                 break;
            case SdrObjKind::BasicDialogCurencyField:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, u"com.sun.star.awt.UnoControlCurrencyFieldModel"_ustr, xDialogSFact );
                 break;
            case SdrObjKind::BasicDialogFormattedField:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, u"com.sun.star.awt.UnoControlFormattedFieldModel"_ustr, xDialogSFact );
                 break;
            case SdrObjKind::BasicDialogPatternField:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, u"com.sun.star.awt.UnoControlPatternFieldModel"_ustr, xDialogSFact );
                 break;
            case SdrObjKind::BasicDialogFileControl:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, u"com.sun.star.awt.UnoControlFileControlModel"_ustr, xDialogSFact );
                 break;
            case SdrObjKind::BasicDialogSpinButton:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, u"com.sun.star.awt.UnoControlSpinButtonModel"_ustr, xDialogSFact );
                 break;
            case SdrObjKind::BasicDialogFormSpin:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, u"com.sun.star.form.component.SpinButton"_ustr, xDialogSFact );
                 static_cast< DlgEdObj* >( pNewObj.get() )->MakeDataAware( mxModel );
                 break;
            case SdrObjKind::BasicDialogTreeControl:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, u"com.sun.star.awt.tree.TreeControlModel"_ustr, xDialogSFact );
                 break;
            case SdrObjKind::BasicDialogGridControl:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, u"com.sun.star.awt.grid.UnoControlGridModel"_ustr, xDialogSFact );
                 break;
            case SdrObjKind::BasicDialogHyperlinkControl:
                 pNewObj = new DlgEdObj(aParams.rSdrModel, u"com.sun.star.awt.UnoControlFixedHyperlinkModel"_ustr, xDialogSFact );
                 break;
            default:
                 break;

        }
    }
    return pNewObj;
}

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
