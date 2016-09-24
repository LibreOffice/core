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

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/graphic/XGraphicObject.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <svtools/grfmgr.hxx>
#include <rtl/ref.hxx>

using namespace com::sun::star;

namespace {

typedef ::cppu::WeakImplHelper< graphic::XGraphicObject, css::lang::XServiceInfo > GObjectAccess_BASE;
 // Simple uno wrapper around the GraphicObject class to allow basic
 // access. ( and solves a horrible cyclic link problem between
 // goodies/toolkit/extensions )
class GObjectImpl : public GObjectAccess_BASE
{
     ::osl::Mutex m_aMutex;
     std::unique_ptr< GraphicObject > mpGObject;
public:
    explicit GObjectImpl(uno::Sequence< uno::Any > const & args) throw (uno::RuntimeException, std::exception);

     // XGraphicObject
    virtual uno::Reference< graphic::XGraphic > SAL_CALL getGraphic() throw (uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setGraphic( const uno::Reference< graphic::XGraphic >& _graphic ) throw (uno::RuntimeException, std::exception) override;
    OUString SAL_CALL getUniqueID() throw (uno::RuntimeException, std::exception) override;

    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override
    {
        return OUString("com.sun.star.graphic.GraphicObject");
    }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
        throw (css::uno::RuntimeException, std::exception) override
    {
        return cppu::supportsService(this, ServiceName);
    }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override
    {
        uno::Sequence<OUString> aRet { "com.sun.star.graphic.GraphicObject" };
        return aRet;
    }
};

GObjectImpl::GObjectImpl(const uno::Sequence< uno::Any >& args) throw (uno::RuntimeException, std::exception)
{
    if ( args.getLength() == 1 )
    {
        OUString sId;
        if ( !( args[ 0 ] >>= sId ) || sId.isEmpty() )
            throw lang::IllegalArgumentException();
        OString bsId(OUStringToOString(sId, RTL_TEXTENCODING_UTF8));
        mpGObject.reset( new GraphicObject( bsId ) );
    }
    else
       mpGObject.reset( new GraphicObject() );
}

uno::Reference< graphic::XGraphic > SAL_CALL GObjectImpl::getGraphic() throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !mpGObject.get() )
        throw uno::RuntimeException();
    return mpGObject->GetGraphic().GetXGraphic();
}

void SAL_CALL GObjectImpl::setGraphic( const uno::Reference< graphic::XGraphic >& _graphic ) throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !mpGObject.get() )
        throw uno::RuntimeException();
    Graphic aGraphic( _graphic );
    mpGObject->SetGraphic( aGraphic );
}

OUString SAL_CALL GObjectImpl::getUniqueID() throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    OUString sId;
    if ( mpGObject.get() )
        sId = OStringToOUString(mpGObject->GetUniqueID(), RTL_TEXTENCODING_ASCII_US);
    return sId;
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_graphic_GraphicObject_get_implementation(
    SAL_UNUSED_PARAMETER css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &arguments)
{
    return cppu::acquire(new GObjectImpl(arguments));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
