/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: macros.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 22:57:21 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _TOOLKIT_HELPER_MACROS_HXX_
#define _TOOLKIT_HELPER_MACROS_HXX_

// -------------------------------------------------------------------------------------

#define IMPL_XUNOTUNNEL( ClassName ) \
sal_Int64 ClassName::getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rIdentifier ) throw(::com::sun::star::uno::RuntimeException) \
{ \
    if( ( rIdentifier.getLength() == 16 ) && ( 0 == rtl_compareMemory( ClassName::GetUnoTunnelId().getConstArray(), rIdentifier.getConstArray(), 16 ) ) ) \
    { \
        return sal::static_int_cast< sal_Int64 >(reinterpret_cast< sal_IntPtr >(this)); \
    } \
    return 0; \
} \
const ::com::sun::star::uno::Sequence< sal_Int8 >& ClassName::GetUnoTunnelId() throw() \
{ \
    static ::com::sun::star::uno::Sequence< sal_Int8 > * pSeq = NULL; \
    if( !pSeq ) \
    { \
        ::osl::Guard< ::osl::Mutex > aGuard( ::osl::Mutex::getGlobalMutex() ); \
        if( !pSeq ) \
        { \
            static ::com::sun::star::uno::Sequence< sal_Int8 > aSeq( 16 ); \
            rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True ); \
            pSeq = &aSeq; \
        } \
    } \
    return *pSeq; \
} \
ClassName* ClassName::GetImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rxIFace ) throw() \
{ \
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XUnoTunnel > xUT( rxIFace, ::com::sun::star::uno::UNO_QUERY ); \
    return xUT.is() ? reinterpret_cast<ClassName*>(sal::static_int_cast<sal_IntPtr>(xUT->getSomething( ClassName::GetUnoTunnelId() ))) : NULL; \
}

#define IMPL_XUNOTUNNEL2( ClassName, BaseClass ) \
sal_Int64 ClassName::getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rIdentifier ) throw(::com::sun::star::uno::RuntimeException) \
{ \
    if( ( rIdentifier.getLength() == 16 ) && ( 0 == rtl_compareMemory( ClassName::GetUnoTunnelId().getConstArray(), rIdentifier.getConstArray(), 16 ) ) ) \
    { \
        return sal::static_int_cast< sal_Int64 >(reinterpret_cast< sal_IntPtr >(this)); \
    } \
    return BaseClass::getSomething( rIdentifier ); \
} \
const ::com::sun::star::uno::Sequence< sal_Int8 >& ClassName::GetUnoTunnelId() throw() \
{ \
    static ::com::sun::star::uno::Sequence< sal_Int8 > * pSeq = NULL; \
    if( !pSeq ) \
    { \
        ::osl::Guard< ::osl::Mutex > aGuard( ::osl::Mutex::getGlobalMutex() ); \
        if( !pSeq ) \
        { \
            static ::com::sun::star::uno::Sequence< sal_Int8 > aSeq( 16 ); \
            rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True ); \
            pSeq = &aSeq; \
        } \
    } \
    return *pSeq; \
} \
ClassName* ClassName::GetImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rxIFace ) throw() \
{ \
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XUnoTunnel > xUT( rxIFace, ::com::sun::star::uno::UNO_QUERY ); \
    return xUT.is() ? reinterpret_cast<ClassName*>(sal::static_int_cast<sal_IntPtr>(xUT->getSomething( ClassName::GetUnoTunnelId() ))) : NULL; \
}

// -------------------------------------------------------------------------------------

#define IMPL_IMPLEMENTATION_ID( ClassName ) \
::com::sun::star::uno::Sequence< sal_Int8 > ClassName::getImplementationId() throw(::com::sun::star::uno::RuntimeException) \
{ \
    static ::cppu::OImplementationId* pId = NULL; \
    if( !pId ) \
    { \
        ::osl::Guard< ::osl::Mutex > aGuard( ::osl::Mutex::getGlobalMutex() ); \
        if( ! pId ) \
        { \
            static ::cppu::OImplementationId id( sal_False ); \
            pId = &id; \
        } \
    } \
    return (*pId).getImplementationId(); \
}

#define IMPL_XTYPEPROVIDER_START( ClassName )   \
IMPL_IMPLEMENTATION_ID( ClassName ) \
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > ClassName::getTypes() throw(::com::sun::star::uno::RuntimeException) \
{ \
    static ::cppu::OTypeCollection* pCollection = NULL; \
    if( !pCollection ) \
    { \
        ::osl::Guard< ::osl::Mutex > aGuard( ::osl::Mutex::getGlobalMutex() ); \
        if( !pCollection ) \
        { \
            static ::cppu::OTypeCollection collection( \
            getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XTypeProvider>* ) NULL ),

// -------------------------------------------------------------------------------------

#define IMPL_XTYPEPROVIDER_END \
            ); \
            pCollection = &collection; \
        } \
    } \
    return (*pCollection).getTypes(); \
}

// -------------------------------------------------------------------------------------

#define DECL_LISTENERMULTIPLEXER_START( ClassName, InterfaceName ) \
class ClassName : public ListenerMultiplexerBase, public InterfaceName \
{ \
public: \
    ClassName( ::cppu::OWeakObject& rSource ); \
    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException); \
    void                        SAL_CALL acquire() throw()  { ListenerMultiplexerBase::acquire(); } \
    void                        SAL_CALL release() throw()  { ListenerMultiplexerBase::release(); } \
    void                        SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

// -------------------------------------------------------------------------------------

#define DECL_LISTENERMULTIPLEXER_START_DLLPUB( ClassName, InterfaceName ) \
class TOOLKIT_DLLPUBLIC ClassName : public ListenerMultiplexerBase, public InterfaceName \
{ \
public: \
    ClassName( ::cppu::OWeakObject& rSource ); \
    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException); \
    void                        SAL_CALL acquire() throw()  { ListenerMultiplexerBase::acquire(); } \
    void                        SAL_CALL release() throw()  { ListenerMultiplexerBase::release(); } \
    void                        SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

// -------------------------------------------------------------------------------------

#define DECL_LISTENERMULTIPLEXER_END \
};

// -------------------------------------------------------------------------------------

#define IMPL_LISTENERMULTIPLEXER_BASEMETHODS( ClassName, InterfaceName ) \
ClassName::ClassName( ::cppu::OWeakObject& rSource ) \
    : ListenerMultiplexerBase( rSource ) \
{ \
} \
::com::sun::star::uno::Any ClassName::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) \
{ \
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType, \
                                        SAL_STATIC_CAST( ::com::sun::star::lang::XEventListener*, this ), \
                                        SAL_STATIC_CAST( InterfaceName*, this ) ); \
    return (aRet.hasValue() ? aRet : ListenerMultiplexerBase::queryInterface( rType )); \
} \
void ClassName::disposing( const ::com::sun::star::lang::EventObject& ) throw(::com::sun::star::uno::RuntimeException) \
{ \
}

// -------------------------------------------------------------------------------------

#if OSL_DEBUG_LEVEL > 0
    #define DISPLAY_EXCEPTION( ClassName, MethodName, e )    \
        ::rtl::OString sMessage( #ClassName "::" #MethodName ": caught an exception!\n" ); \
        sMessage += ::rtl::OString( e.Message.getStr(), e.Message.getLength(), RTL_TEXTENCODING_ASCII_US ); \
        OSL_ENSURE( sal_False, sMessage.getStr() );
#else
    #define DISPLAY_EXCEPTION( ClassName, MethodName, e )
#endif

#define IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( ClassName, InterfaceName, MethodName, EventType ) \
void ClassName::MethodName( const EventType& evt ) throw(::com::sun::star::uno::RuntimeException) \
{ \
    EventType aMulti( evt ); \
    aMulti.Source = &GetContext(); \
    ::cppu::OInterfaceIteratorHelper aIt( *this ); \
    while( aIt.hasMoreElements() ) \
    { \
        ::com::sun::star::uno::Reference< InterfaceName > xListener( \
            static_cast< InterfaceName* >( aIt.next() ) ); \
        try \
        { \
            xListener->MethodName( aMulti ); \
        } \
        catch( ::com::sun::star::lang::DisposedException e ) \
        { \
            OSL_ENSURE( e.Context.is(), "caught DisposedException with empty Context field" ); \
            if ( e.Context == xListener || !e.Context.is() ) \
                aIt.remove(); \
        } \
        catch( ::com::sun::star::uno::RuntimeException e ) \
        { \
            DISPLAY_EXCEPTION( ClassName, MethodName, e ) \
        } \
    } \
}

// -------------------------------------------------------------------------------------

#define DECLIMPL_SUPPORTS_SERVICE( ) \
    sal_Bool SAL_CALL supportsService( const ::rtl::OUString& rServiceName ) throw(::com::sun::star::uno::RuntimeException) \
    { \
        ::com::sun::star::uno::Sequence< ::rtl::OUString > aServiceNames( getSupportedServiceNames() ); \
        const ::rtl::OUString* pSupported = aServiceNames.getConstArray(); \
        const ::rtl::OUString* pSupportedEnd = pSupported + aServiceNames.getLength(); \
        for ( ; pSupported != pSupportedEnd; ++pSupported ) \
            if ( *pSupported == rServiceName ) \
                return sal_True; \
        return sal_False; \
    }

// -------------------------------------------------------------------------------------

#define DECLIMPL_SERVICEINFO_DERIVED( ImplName, BaseClass, ServiceName ) \
    ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException) { return ::rtl::OUString::createFromAscii( "stardiv.Toolkit." #ImplName ); } \
    ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException)   \
                            { \
                                ::com::sun::star::uno::Sequence< ::rtl::OUString > aNames = BaseClass::getSupportedServiceNames( ); \
                                aNames.realloc( aNames.getLength() + 1 ); \
                                aNames[ aNames.getLength() - 1 ] = ::rtl::OUString::createFromAscii( ServiceName ); \
                                return aNames; \
                            } \

// -------------------------------------------------------------------------------------

#define DECLIMPL_SERVICEINFO( ImplName, ServiceName ) \
    ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException) { return ::rtl::OUString::createFromAscii( "stardiv.Toolkit." #ImplName ); } \
    ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException)   \
                            { \
                                ::com::sun::star::uno::Sequence< ::rtl::OUString > aNames( 1 ); \
                                aNames[ 0 ] = ::rtl::OUString::createFromAscii( ServiceName ); \
                                return aNames; \
                            } \
    DECLIMPL_SUPPORTS_SERVICE( )






#endif // _TOOLKIT_HELPER_MACROS_HXX_

