/*************************************************************************
 *
 *  $RCSfile: environment.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: dbo $ $Date: 2001-03-09 12:10:56 $
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
#ifndef _UNO_ENVIRONMENT_HXX_
#define _UNO_ENVIRONMENT_HXX_

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
    <br>
*/
class Environment
{
    /** binary C uno_Environment<br>
    */
    uno_Environment * _pEnv;

public:
    /** Constructor: acquires given environment
        <br>
        @param pEnv environment
    */
    inline Environment( uno_Environment * pEnv = 0 ) SAL_THROW( () );

    /** Copy constructor: acquires given environment
        <br>
        @param rEnv another environment
    */
    inline Environment( const Environment & rEnv ) SAL_THROW( () );

    /** Destructor:
        <br>
        Releases a set environment.
    */
    inline ~Environment() SAL_THROW( () );

    /** Sets a given environment, i.e. acquires given one and releases a set one.
        <br>
        @param pEnv another environment
        @return this environment
    */
    inline Environment & SAL_CALL operator = ( uno_Environment * pEnv ) SAL_THROW( () );
    /** Sets a given environment, i.e. acquires given one and releases a set one.
        <br>
        @param rEnv another environment
        @return this environment
    */
    inline Environment & SAL_CALL operator = ( const Environment & rEnv ) SAL_THROW( () )
        { return operator = ( rEnv._pEnv ); }

    /** Provides <b>un</b>acquired pointer to the set C environment.
        <br>
        @return <b>un</b>acquired pointer to the C environment struct
    */
    inline uno_Environment * SAL_CALL get() const SAL_THROW( () )
        { return _pEnv; }

    /** Gets type name of set environment.
        <br>
        @return type name of set environment
    */
    inline ::rtl::OUString SAL_CALL getTypeName() const SAL_THROW( () )
        { return _pEnv->pTypeName; }

    /** Gets free context pointer of set environment.
        <br>
        @return free context pointer of set environment
    */
    inline void * SAL_CALL getContext() const SAL_THROW( () )
        { return _pEnv->pContext; }

    /** Tests if a environment is set.
        <br>
        @return true, if a environment is set, false otherwise
    */
    inline sal_Bool SAL_CALL is() const SAL_THROW( () )
        { return (_pEnv != 0); }

    /** Releases a set environment.
        <br>
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
