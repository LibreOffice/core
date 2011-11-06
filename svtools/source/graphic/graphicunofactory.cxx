/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
