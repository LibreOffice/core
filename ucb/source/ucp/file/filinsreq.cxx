#ifndef _FILINSREQ_HXX_
#include "filinsreq.hxx"
#endif



using namespace cppu;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::task;
using namespace com::sun::star::ucb;
using namespace fileaccess;



void SAL_CALL
XInteractionSupplyNameImpl::acquire( void )
    throw()
{
    OWeakObject::acquire();
}



void SAL_CALL
XInteractionSupplyNameImpl::release( void )
    throw()
{
    OWeakObject::release();
}



Any SAL_CALL
XInteractionSupplyNameImpl::queryInterface( const Type& rType )
    throw( RuntimeException )
{
    Any aRet = cppu::queryInterface( rType,
                                     SAL_STATIC_CAST( lang::XTypeProvider*, this ),
                                     SAL_STATIC_CAST( XInteractionSupplyName*,this) );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}


//////////////////////////////////////////////////////////////////////////////////////////
//  XTypeProvider
//////////////////////////////////////////////////////////////////////////////////////////

XTYPEPROVIDER_IMPL_2( XInteractionSupplyNameImpl,
                      XTypeProvider,
                      XInteractionSupplyName )



void SAL_CALL
XInteractionAbortImpl::acquire( void )
    throw()
{
    OWeakObject::acquire();
}



void SAL_CALL
XInteractionAbortImpl::release( void )
    throw()
{
    OWeakObject::release();
}



Any SAL_CALL
XInteractionAbortImpl::queryInterface( const Type& rType )
    throw( RuntimeException )
{
    Any aRet = cppu::queryInterface( rType,
                                     SAL_STATIC_CAST( lang::XTypeProvider*, this ),
                                     SAL_STATIC_CAST( XInteractionAbort*,this) );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}


//////////////////////////////////////////////////////////////////////////////////////////
//  XTypeProvider
//////////////////////////////////////////////////////////////////////////////////////////

XTYPEPROVIDER_IMPL_2( XInteractionAbortImpl,
                      XTypeProvider,
                      XInteractionAbort )



XInteractionRequestImpl::XInteractionRequestImpl()
    : m_aSeq( 2 ),
      p1( new XInteractionSupplyNameImpl ),
      p2( new XInteractionAbortImpl )
{
    m_aSeq[0] = Reference<XInteractionContinuation>(p1);
    m_aSeq[1] = Reference<XInteractionContinuation>(p2);
}


void SAL_CALL
XInteractionRequestImpl::acquire( void )
    throw()
{
    OWeakObject::acquire();
}



void SAL_CALL
XInteractionRequestImpl::release( void )
    throw()
{
    OWeakObject::release();
}



Any SAL_CALL
XInteractionRequestImpl::queryInterface( const Type& rType )
    throw( RuntimeException )
{
    Any aRet = cppu::queryInterface( rType,
                                     SAL_STATIC_CAST( lang::XTypeProvider*, this ),
                                     SAL_STATIC_CAST( XInteractionRequest*,this) );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}


//////////////////////////////////////////////////////////////////////////////////////////
//  XTypeProvider
//////////////////////////////////////////////////////////////////////////////////////////

XTYPEPROVIDER_IMPL_2( XInteractionRequestImpl,
                      XTypeProvider,
                      XInteractionRequest )
