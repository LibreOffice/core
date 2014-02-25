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
#include "fmobj.hxx"
#include <svx/fmglob.hxx>
#include <svx/fmdpage.hxx>
#include <svx/unoshape.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/typeprovider.hxx>

using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::form::XFormsSupplier2;

DBG_NAME(SvxFmDrawPage)
SvxFmDrawPage::SvxFmDrawPage( SdrPage* pInPage ) :
    SvxDrawPage( pInPage )
{
    DBG_CTOR(SvxFmDrawPage,NULL);
}

SvxFmDrawPage::~SvxFmDrawPage() throw ()
{
    DBG_DTOR(SvxFmDrawPage,NULL);
}

::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL SvxFmDrawPage::getImplementationId() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    static ::cppu::OImplementationId* pId = 0;
    if (! pId)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if (! pId)
        {
            static ::cppu::OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}

Any SAL_CALL SvxFmDrawPage::queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw(RuntimeException, std::exception)
{
    Any aRet = ::cppu::queryInterface   (   _rType
                                        ,   static_cast< XFormsSupplier2* >( this )
                                        ,   static_cast< XFormsSupplier* >( this )
                                        );
    if ( !aRet.hasValue() )
        aRet = SvxDrawPage::queryAggregation( _rType );

    return aRet;
}

/***********************************************************************
*                                                                      *
***********************************************************************/
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL SvxFmDrawPage::getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > aTypes(SvxDrawPage::getTypes());
    aTypes.realloc(aTypes.getLength() + 1);
    ::com::sun::star::uno::Type* pTypes = aTypes.getArray();

    pTypes[aTypes.getLength()-1] = ::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormsSupplier>*)0);
    return aTypes;
}

SdrObject *SvxFmDrawPage::_CreateSdrObject( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > & xDescr )
    throw (std::exception)
{
    OUString aShapeType( xDescr->getShapeType() );

    if  (   aShapeType == "com.sun.star.drawing.ShapeControl"   // compatibility
        ||  aShapeType == "com.sun.star.drawing.ControlShape"
        )
        return new FmFormObj();
    else
        return SvxDrawPage::_CreateSdrObject( xDescr );

}

::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >  SvxFmDrawPage::_CreateShape( SdrObject *pObj ) const
    throw (std::exception)
{
    if( FmFormInventor == pObj->GetObjInventor() )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >  xShape = (SvxShape*)new SvxShapeControl( pObj );
        return xShape;
    }
    else
        return SvxDrawPage::_CreateShape( pObj );
}

// XFormsSupplier
::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > SAL_CALL SvxFmDrawPage::getForms(void) throw( ::com::sun::star::uno::RuntimeException, std::exception )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >  xForms;

    FmFormPage *pFmPage = PTR_CAST( FmFormPage, GetSdrPage() );
    if( pFmPage )
        xForms.set( pFmPage->GetForms(), css::uno::UNO_QUERY_THROW );

    return xForms;
}

// XFormsSupplier2
sal_Bool SAL_CALL SvxFmDrawPage::hasForms(void) throw( ::com::sun::star::uno::RuntimeException, std::exception )
{
    sal_Bool bHas = sal_False;
    FmFormPage* pFormPage = PTR_CAST( FmFormPage, GetSdrPage() );
    if ( pFormPage )
        bHas = pFormPage->GetForms( false ).is();
    return bHas;
}

// ::com::sun::star::lang::XServiceInfo
::com::sun::star::uno::Sequence< OUString > SAL_CALL SvxFmDrawPage::getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException, std::exception )
{
    return SvxDrawPage::getSupportedServiceNames();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
