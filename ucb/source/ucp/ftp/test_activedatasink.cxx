#ifndef _TEST_ACTIVEDATASINK_HXX_
#include "test_activedatasink.hxx"
#endif


using namespace test_ftp;
using namespace com::sun::star::uno;
using namespace com::sun::star::io;


Any SAL_CALL Test_ActiveDataSink::queryInterface( const Type& rType ) throw( RuntimeException ) {
    Any aRet = ::cppu::queryInterface(rType,
                                      SAL_STATIC_CAST( XActiveDataSink*,this ));

    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}



void SAL_CALL Test_ActiveDataSink::acquire( void ) throw() {
    OWeakObject::acquire();
}



void SAL_CALL Test_ActiveDataSink::release( void ) throw() {
    OWeakObject::release();
}
