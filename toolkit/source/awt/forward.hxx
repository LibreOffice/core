#ifndef AWT_FORWARD_HXX
#define AWT_FORWARD_HXX

#include <comphelper/uno3.hxx>

#define IMPLEMENT_FORWARD_XTYPEPROVIDER1( classname, baseclass ) \
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL classname::getTypes(  ) throw (::com::sun::star::uno::RuntimeException) \
    { return baseclass::getTypes(); } \
    IMPLEMENT_GET_IMPLEMENTATION_ID( classname )

#define IMPLEMENT_2_FORWARD_XINTERFACE1( classname, refcountbase1, refcountbase2 ) \
    void SAL_CALL classname::acquire() throw() { refcountbase1::acquire(); refcountbase2::acquire(); } \
    void SAL_CALL classname::release() throw() { refcountbase1::release(); refcountbase2::release(); } \
    ::com::sun::star::uno::Any SAL_CALL classname::queryInterface( const ::com::sun::star::uno::Type& _rType ) throw (::com::sun::star::uno::RuntimeException) \
    { \
        ::com::sun::star::uno::Any aReturn = refcountbase1::queryInterface( _rType ); \
        if ( !aReturn.hasValue() ) \
        { \
            aReturn = refcountbase2::queryInterface( _rType ); \
        } \
        return aReturn; \
    }

#define IMPLEMENT_2_FORWARD_XINTERFACE2( classname, refcountbase1, refcountbase2, baseclass3 ) \
    void SAL_CALL classname::acquire() throw() { refcountbase1::acquire(); refcountbase2::acquire(); } \
    void SAL_CALL classname::release() throw() { refcountbase1::release(); refcountbase2::release(); } \
    ::com::sun::star::uno::Any SAL_CALL classname::queryInterface( const ::com::sun::star::uno::Type& _rType ) throw (::com::sun::star::uno::RuntimeException) \
    { \
        ::com::sun::star::uno::Any aReturn = refcountbase1::queryInterface( _rType ); \
        if ( !aReturn.hasValue() ) \
        { \
            aReturn = refcountbase2::queryInterface( _rType ); \
            if ( !aReturn.hasValue() ) \
                aReturn = baseclass3::queryInterface( _rType ); \
        } \
        return aReturn; \
    }

#endif /*AWT_FORWARD_HXX*/
