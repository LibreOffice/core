/*************************************************************************
 *
 *  $RCSfile: tres.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: sz $ $Date: 2001-04-12 06:16:12 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _RTL_TRES_HXX_
#define _RTL_TRES_HXX_

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#define TST_BOOM(c, m)  OSL_ENSURE(c, m)


namespace rtl {
class tRes {

    sal_Char* m_name;
    sal_Char* m_result;
    sal_Bool m_state;
    sal_Bool m_boom;

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
            , m_result(0)
            , m_state( sal_False )
            , m_boom( boom ) {


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
            TST_BOOM( m_state, msg );
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





