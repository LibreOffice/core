#ifndef _RTL_TRES_HXX_
#define _RTL_TRES_HXX_

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _OSL_THREAD_H_
#include    <osl/thread.h>
#endif

namespace rtl {
class tRes {

    sal_Char* m_name;
    sal_Char* m_result;
    sal_Bool m_boom;
    sal_Bool m_state;

    tRes();
    tRes( const tRes& oRes );

    // private string copy method
    sal_Char* cpy( sal_Char** dest, const sal_Char* src ) {

        *dest = new sal_Char[ ln(src)+1 ];
        // set pointer
        sal_Char* pdest = *dest;
        const sal_Char* psrc = src;

        // copy string by char
        while( *pdest++ = *psrc++ );

        return ( *dest );

    }
    // private string cat method
    sal_Char* cat( const sal_Char* str1, const sal_Char* str2 ) {

        // allocate memory for destination string
        sal_Char* dest = new sal_Char[ ln(str1)+ln(str2)+1 ];

        // set pointers
        sal_Char* pdest = dest;
        const sal_Char* psrc = str1;

        // copy string1 by char to dest
        while( *pdest++ = *psrc++ );
        pdest--;
        psrc = str2;
        while( *pdest++ = *psrc++ );

        return ( dest );

    }

    // private strlen method
    sal_uInt32 ln( const sal_Char* str ) {

        sal_uInt32 len = 0;
        const sal_Char* ptr = str;

        if( ptr ) {
            while( *ptr++ ) len++;
        }

        return(len);
    }

public:

    //> c'tor
    tRes( const sal_Char* meth, sal_Bool boom = sal_False )
            : m_name(0)
            , m_boom( boom )
            , m_result(0)
            , m_state(sal_False) {


        cpy( &m_name, meth );
    } ///< c'tor


    //> d'tor
    ~tRes() {
        if( m_name )
            delete( m_name );
        if( m_result )
            delete( m_result );
    } ///< d'tor

    //> state
    inline void state( sal_Bool state, sal_Char* msg = 0 ) {
        m_state = state;
        if( ! state && m_boom ) {
            if(! msg ) {
                cpy( &msg, m_name );
            }
            OSL_ENSURE( m_state, msg );
        }
    } ///< state

    //> getState
    inline sal_Bool getState() {
        return m_state;
    } ///< getState

    //> end
    inline void end( sal_Char* msg = 0 ) {

        sal_Char* okStr = "#OK#";
        sal_Char* fdStr = "#FAILED#";

        if ( ! msg ) {
            msg = "PASSED";
        }

        if( m_state )
            cpy( &m_result, cat( msg, okStr ) );
        else
            cpy( &m_result, cat( msg, fdStr ) );

    } ///< end

    //> getName
    sal_Char* getName() {
        return m_name;
    } ///< getName

    //> getResult
    sal_Char* getResult() {
        return m_result;
    } ///< getResult

};
}

#endif





