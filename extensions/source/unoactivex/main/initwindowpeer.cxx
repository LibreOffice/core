
#include "initwindowpeer.hxx"

using namespace ::com::sun::star;


uno::Any SAL_CALL InitWindowPeer::getWindowHandle( const uno::Sequence< sal_Int8 >& ProcessId, sal_Int16 SystemType )
    throw ( uno::RuntimeException )
{
    uno::Any aRes;
    sal_Int32 nHwnd = (sal_Int32)m_hwnd;
    aRes <<= nHwnd;
    return aRes;
}


uno::Reference< awt::XToolkit > SAL_CALL InitWindowPeer::getToolkit()
    throw ( uno::RuntimeException )
{
    return uno::Reference< awt::XToolkit >();
}

void SAL_CALL InitWindowPeer::setPointer( const uno::Reference< awt::XPointer >& Pointer )
    throw ( uno::RuntimeException )
{
}

void SAL_CALL InitWindowPeer::setBackground( sal_Int32 Color )
    throw ( uno::RuntimeException )
{
}

void SAL_CALL InitWindowPeer::invalidate( sal_Int16 Flags )
    throw ( uno::RuntimeException )
{
}

void SAL_CALL InitWindowPeer::invalidateRect( const awt::Rectangle& Rect, sal_Int16 Flags )
    throw ( uno::RuntimeException )
{
}

void SAL_CALL InitWindowPeer::dispose()
    throw ( uno::RuntimeException )
{
}

void SAL_CALL InitWindowPeer::addEventListener( const uno::Reference< lang::XEventListener >& xListener )
    throw ( uno::RuntimeException )
{
}

void SAL_CALL InitWindowPeer::removeEventListener( const uno::Reference< lang::XEventListener >& aListener )
    throw ( uno::RuntimeException )
{
}

