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

#include <svx/fmpage.hxx>
#include <fmobj.hxx>
#include <svx/fmdpage.hxx>
#include <svx/fmmodel.hxx>
#include <svx/unoshape.hxx>
#include <vcl/svapp.hxx>
#include <cppuhelper/queryinterface.hxx>

using ::com::sun::star::uno::Any;
using ::com::sun::star::form::XFormsSupplier2;

SvxFmDrawPage::SvxFmDrawPage( SdrPage* pInPage ) :
    SvxDrawPage( pInPage )
{
}

SvxFmDrawPage::~SvxFmDrawPage() noexcept
{
}

css::uno::Sequence< sal_Int8 > SAL_CALL SvxFmDrawPage::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

Any SAL_CALL SvxFmDrawPage::queryAggregation( const css::uno::Type& _rType )
{
    Any aRet = ::cppu::queryInterface   (   _rType
                                        ,   static_cast< XFormsSupplier2* >( this )
                                        ,   static_cast< XFormsSupplier* >( this )
                                        );
    if ( !aRet.hasValue() )
        aRet = SvxDrawPage::queryAggregation( _rType );

    return aRet;
}

css::uno::Sequence< css::uno::Type > SAL_CALL SvxFmDrawPage::getTypes(  )
{
    return comphelper::concatSequences(SvxDrawPage::getTypes(),
        css::uno::Sequence { cppu::UnoType<css::form::XFormsSupplier>::get() });
}

SdrObject *FmFormModel::CreateSdrObject( const css::uno::Reference< css::drawing::XShape > & xDescr, SvxDrawPage* pDrawPage )
{
    OUString aShapeType( xDescr->getShapeType() );

    if  (   aShapeType == "com.sun.star.drawing.ShapeControl"   // compatibility
        ||  aShapeType == "com.sun.star.drawing.ControlShape"
        )
    {
        return new FmFormObj(*this);
    }
    else
    {
        return SdrModel::CreateSdrObject( xDescr, pDrawPage );
    }
}

css::uno::Reference< css::drawing::XShape >  FmFormModel::CreateShape( SdrObject *pObj )
{
    if( SdrInventor::FmForm == pObj->GetObjInventor() )
    {
        css::uno::Reference< css::drawing::XShape >  xShape = static_cast<SvxShape*>(new SvxShapeControl( pObj ));
        return xShape;
    }
    else
        return SdrModel::CreateShape( pObj );
}

// XFormsSupplier
css::uno::Reference< css::container::XNameContainer > SAL_CALL SvxFmDrawPage::getForms()
{
    SolarMutexGuard g;

    css::uno::Reference< css::container::XNameContainer >  xForms;

    FmFormPage *pFmPage = dynamic_cast<FmFormPage*>( GetSdrPage()  );
    if( pFmPage )
        xForms.set( pFmPage->GetForms(), css::uno::UNO_QUERY_THROW );

    return xForms;
}

// XFormsSupplier2
sal_Bool SAL_CALL SvxFmDrawPage::hasForms()
{
    SolarMutexGuard g;

    bool bHas = false;
    FmFormPage* pFormPage = dynamic_cast<FmFormPage*>( GetSdrPage()  );
    if ( pFormPage )
        bHas = pFormPage->GetForms( false ).is();
    return bHas;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
