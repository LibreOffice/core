#include <com/sun/star/ucb/UnsupportedNameClashException.hpp>
#include <com/sun/star/ucb/NameClash.hpp>
#include "ftpintreq.hxx"

using namespace cppu;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::ucb;
using namespace com::sun::star::task;
using namespace ftp;


XInteractionApproveImpl::XInteractionApproveImpl()
    : m_bSelected(false)
{
}


void SAL_CALL
XInteractionApproveImpl::acquire( void )
    throw()
{
    OWeakObject::acquire();
}


void SAL_CALL
XInteractionApproveImpl::release( void )
    throw()
{
    OWeakObject::release();
}



Any SAL_CALL
XInteractionApproveImpl::queryInterface( const Type& rType )
    throw( RuntimeException )
{
    Any aRet = cppu::queryInterface(
        rType,
        SAL_STATIC_CAST( lang::XTypeProvider*, this ),
        SAL_STATIC_CAST( XInteractionApprove*,this) );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}


//////////////////////////////////////////////////////////////////////////////
//  XTypeProvider
//////////////////////////////////////////////////////////////////////////////

XTYPEPROVIDER_IMPL_2( XInteractionApproveImpl,
                      XTypeProvider,
                      XInteractionApprove )


void SAL_CALL XInteractionApproveImpl::select()
    throw (RuntimeException)
{
    m_bSelected = true;
}


bool XInteractionApproveImpl::isSelected() const
{
    return m_bSelected;
}



// XInteractionDisapproveImpl

XInteractionDisapproveImpl::XInteractionDisapproveImpl()
    : m_bSelected(false)
{
}


void SAL_CALL
XInteractionDisapproveImpl::acquire( void )
    throw()
{
    OWeakObject::acquire();
}


void SAL_CALL
XInteractionDisapproveImpl::release( void )
    throw()
{
    OWeakObject::release();
}



Any SAL_CALL
XInteractionDisapproveImpl::queryInterface( const Type& rType )
    throw( RuntimeException )
{
    Any aRet = cppu::queryInterface(
        rType,
        SAL_STATIC_CAST( lang::XTypeProvider*, this ),
        SAL_STATIC_CAST( XInteractionDisapprove*,this) );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}


//////////////////////////////////////////////////////////////////////////////
//  XTypeProvider
//////////////////////////////////////////////////////////////////////////////

XTYPEPROVIDER_IMPL_2( XInteractionDisapproveImpl,
                      XTypeProvider,
                      XInteractionDisapprove )


void SAL_CALL XInteractionDisapproveImpl::select()
    throw (RuntimeException)

{
    m_bSelected = true;
}


bool XInteractionDisapproveImpl::isSelected() const
{
    return m_bSelected;
}



// XInteractionRequestImpl

XInteractionRequestImpl::XInteractionRequestImpl(const rtl::OUString& aName)
    : m_aSeq( 2 ),
      p1( new XInteractionApproveImpl ),
      p2( new XInteractionDisapproveImpl ),
      m_aName(aName)
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
    Any aRet = cppu::queryInterface(
        rType,
        SAL_STATIC_CAST( lang::XTypeProvider*, this ),
        SAL_STATIC_CAST( XInteractionRequest*,this) );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}


//////////////////////////////////////////////////////////////////////////////
//  XTypeProvider
/////////////////////////////////////////////////////////////////////////////

XTYPEPROVIDER_IMPL_2( XInteractionRequestImpl,
                      XTypeProvider,
                      XInteractionRequest )


Any SAL_CALL XInteractionRequestImpl::getRequest(  )
    throw (RuntimeException)
{
    Any aAny;
    UnsupportedNameClashException excep;
    excep.NameClash = NameClash::ERROR;
    aAny <<= excep;
    return aAny;
}


Sequence<Reference<XInteractionContinuation > > SAL_CALL
XInteractionRequestImpl::getContinuations(  )
    throw (RuntimeException)
{
    return m_aSeq;
}


bool XInteractionRequestImpl::aborted() const
{
    return p2->isSelected();
}


bool XInteractionRequestImpl::approved() const
{
    return p1->isSelected();
}

