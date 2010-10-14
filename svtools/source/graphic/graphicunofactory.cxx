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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#include <comphelper/servicedecl.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/graphic/XGraphicObject.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <svtools/grfmgr.hxx>

using namespace com::sun::star;

namespace unographic {

typedef ::cppu::WeakImplHelper1< graphic::XGraphicObject > GObjectAccess_BASE;
 // Simple uno wrapper around the GraphicObject class to allow basic
 // access. ( and solves a horrible cyclic link problem between
 // goodies/toolkit/extensions )
class GObjectImpl : public GObjectAccess_BASE
{
     ::osl::Mutex m_aMutex;
     std::auto_ptr< GraphicObject > mpGObject;
public:
     GObjectImpl( uno::Sequence< uno::Any > const & args, uno::Reference< uno::XComponentContext > const & xComponentContext ) throw (uno::RuntimeException);

     // XGraphicObject
    virtual uno::Reference< graphic::XGraphic > SAL_CALL getGraphic() throw (uno::RuntimeException);
    virtual void SAL_CALL setGraphic( const uno::Reference< graphic::XGraphic >& _graphic ) throw (uno::RuntimeException);
    ::rtl::OUString SAL_CALL getUniqueID() throw (uno::RuntimeException);
};

GObjectImpl::GObjectImpl( uno::Sequence< uno::Any > const & args, uno::Reference< uno::XComponentContext > const & /*xComponentContext*/ ) throw (uno::RuntimeException)
{
    if ( args.getLength() == 1 )
    {
        rtl::OUString sId;
        if ( !( args[ 0 ] >>= sId ) || sId.getLength() == 0 )
            throw lang::IllegalArgumentException();
        ByteString bsId( sId.getStr(), RTL_TEXTENCODING_UTF8 );
        mpGObject.reset( new GraphicObject( bsId ) );
    }
    else
       mpGObject.reset( new GraphicObject() );
}

uno::Reference< graphic::XGraphic > SAL_CALL GObjectImpl::getGraphic() throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !mpGObject.get() )
        throw uno::RuntimeException();
    return mpGObject->GetGraphic().GetXGraphic();
}

void SAL_CALL GObjectImpl::setGraphic( const uno::Reference< graphic::XGraphic >& _graphic ) throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !mpGObject.get() )
        throw uno::RuntimeException();
    Graphic aGraphic( _graphic );
    mpGObject->SetGraphic( aGraphic );
}

::rtl::OUString SAL_CALL GObjectImpl::getUniqueID() throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    rtl::OUString sId;
    if ( mpGObject.get() )
        sId = String( mpGObject->GetUniqueID().GetBuffer(), RTL_TEXTENCODING_ASCII_US );
    return sId;
}


namespace sdecl = comphelper::service_decl;
sdecl::class_<GObjectImpl, sdecl::with_args<true> > serviceBI;
extern sdecl::ServiceDecl const serviceDecl( serviceBI, "com.sun.star.graphic.GraphicObject", "com.sun.star.graphic.GraphicObject" );

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
