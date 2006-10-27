/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: environment.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2006-10-27 12:14:57 $
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
#ifndef _UNO_ENVIRONMENT_HXX_
#define _UNO_ENVIRONMENT_HXX_

#ifndef _RTL_ALLOC_H_
#include <rtl/alloc.h>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _UNO_ENVIRONMENT_H_
#include <uno/environment.h>
#endif


/** */ //for docpp
namespace com
{
/** */ //for docpp
namespace sun
{
/** */ //for docpp
namespace star
{
/** */ //for docpp
namespace uno
{

/** C++ wrapper for binary C uno_Environment.

    @see uno_Environment
*/
class Environment
{
    /** binary C uno_Environment
    */
    uno_Environment * _pEnv;

public:
    // these are here to force memory de/allocation to sal lib.
    /** @internal */
    inline static void * SAL_CALL operator new ( size_t nSize ) SAL_THROW( () )
        { return ::rtl_allocateMemory( nSize ); }
    /** @internal */
    inline static void SAL_CALL operator delete ( void * pMem ) SAL_THROW( () )
        { ::rtl_freeMemory( pMem ); }
    /** @internal */
    inline static void * SAL_CALL operator new ( size_t, void * pMem ) SAL_THROW( () )
        { return pMem; }
    /** @internal */
    inline static void SAL_CALL operator delete ( void *, void * ) SAL_THROW( () )
        {}

    /** Constructor: acquires given environment

        @param pEnv environment
    */
    inline Environment( uno_Environment * pEnv = 0 ) SAL_THROW( () );

    /** Gets a specific environment. If the specified environment does not exist, then a default one
        is created and registered.

        @param envTypeName      type name of the environment
        @param pContext         context pointer
    */
    inline explicit Environment( rtl::OUString const & envEypeName, void * pContext = NULL ) SAL_THROW( () );


    /** Copy constructor: acquires given environment

        @param rEnv another environment
    */
    inline Environment( const Environment & rEnv ) SAL_THROW( () );

    /** Destructor: releases a set environment.
    */
    inline ~Environment() SAL_THROW( () );

    /** Sets a given environment, i.e. acquires given one and releases a set one.

        @param pEnv another environment
        @return this environment
    */
    inline Environment & SAL_CALL operator = ( uno_Environment * pEnv ) SAL_THROW( () );
    /** Sets a given environment, i.e. acquires given one and releases a set one.

        @param rEnv another environment
        @return this environment
    */
    inline Environment & SAL_CALL operator = ( const Environment & rEnv ) SAL_THROW( () )
        { return operator = ( rEnv._pEnv ); }

    /** Provides UNacquired pointer to the set C environment.

        @return UNacquired pointer to the C environment struct
    */
    inline uno_Environment * SAL_CALL get() const SAL_THROW( () )
        { return _pEnv; }

    /** Gets type name of set environment.

        @return type name of set environment
    */
    inline ::rtl::OUString SAL_CALL getTypeName() const SAL_THROW( () )
        { return _pEnv->pTypeName; }

    /** Gets free context pointer of set environment.

        @return free context pointer of set environment
    */
    inline void * SAL_CALL getContext() const SAL_THROW( () )
        { return _pEnv->pContext; }

    /** Tests if a environment is set.

        @return true, if a environment is set, false otherwise
    */
    inline sal_Bool SAL_CALL is() const SAL_THROW( () )
        { return (_pEnv != 0); }

    /** Releases a set environment.
    */
    inline void SAL_CALL clear() SAL_THROW( () );
};
//__________________________________________________________________________________________________
inline Environment::Environment( uno_Environment * pEnv ) SAL_THROW( () )
    : _pEnv( pEnv )
{
    if (_pEnv)
        (*_pEnv->acquire)( _pEnv );
}
//__________________________________________________________________________________________________
inline Environment::Environment( rtl::OUString const & rEnvTypeName, void * pContext ) SAL_THROW( () )
    : _pEnv(NULL)
{
    uno_getEnvironment(&_pEnv, rEnvTypeName.pData, pContext);
}
//__________________________________________________________________________________________________
inline Environment::Environment( const Environment & rEnv ) SAL_THROW( () )
    : _pEnv( rEnv._pEnv )
{
    if (_pEnv)
        (*_pEnv->acquire)( _pEnv );
}
//__________________________________________________________________________________________________
inline Environment::~Environment() SAL_THROW( () )
{
    if (_pEnv)
        (*_pEnv->release)( _pEnv );
}
//__________________________________________________________________________________________________
inline void Environment::clear() SAL_THROW( () )
{
    if (_pEnv)
    {
        (*_pEnv->release)( _pEnv );
        _pEnv = 0;
    }
}
//__________________________________________________________________________________________________
inline Environment & Environment::operator = ( uno_Environment * pEnv ) SAL_THROW( () )
{
    if (pEnv != _pEnv)
    {
        if (pEnv)
            (*pEnv->acquire)( pEnv );
        if (_pEnv)
            (*_pEnv->release)( _pEnv );
        _pEnv = pEnv;
    }
    return *this;
}

}
}
}
}

#endif
