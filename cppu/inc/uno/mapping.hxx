/*************************************************************************
 *
 *  $RCSfile: mapping.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:25:52 $
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
#ifndef _UNO_MAPPING_HXX_
#define _UNO_MAPPING_HXX_

#ifndef _CPPU_MACROS_HXX_
#include <cppu/macros.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _UNO_MAPPING_H_
#include <uno/mapping.h>
#endif
#ifndef _COM_SUN_STAR_UNO_TYPE_HXX_
#include <com/sun/star/uno/Type.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

typedef struct _typelib_TypeDescription typelib_TypeDescription;
typedef struct _typelib_InterfaceTypeDescription typelib_InterfaceTypeDescription;
typedef struct _uno_Interface uno_Interface;

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

/** C++ wrapper for C uno_Mapping.
    <br>
*/
class Mapping
{
    uno_Mapping * _pMapping;

public:
    /** Holds a mapping from the specified source to the specified destination by
        environment type names.
        <br>
        @param rFrom        type name of source environment
        @param rTo          type name of destination environment
        @param rAddPurpose  additional purpose
    */
    inline Mapping( const ::rtl::OUString & rFrom, const ::rtl::OUString & rTo,
                    const ::rtl::OUString & rAddPurpose = ::rtl::OUString() );

    /** Holds a mapping from the specified source to the specified destination.
        <br>
        @param pFrom        source environment
        @param pTo          destination environment
        @param rAddPurpose  additional purpose
    */
    inline Mapping( uno_Environment * pFrom, uno_Environment * pTo,
                    const ::rtl::OUString & rAddPurpose = ::rtl::OUString() );

    /** Constructor.
        <br>
        @param pMapping another mapping
    */
    inline Mapping( uno_Mapping * pMapping = 0 );

    /** Copy constructor.
        <br>
        @param rMapping another mapping
    */
    inline Mapping( const Mapping & rMapping );

    /** Destructor.
        <br>
    */
    inline ~Mapping();

    /** Sets a given mapping.
        <br>
        @param pMapping another mapping
        @return this mapping
    */
    inline Mapping & SAL_CALL operator = ( uno_Mapping * pMapping );
    /** Sets a given mapping.
        <br>
        @param rMapping another mapping
        @return this mapping
    */
    inline Mapping & SAL_CALL operator = ( const Mapping & rMapping )
        { return operator = ( rMapping._pMapping ); }

    /** Provides a pointer to the C mapping. The returned mapping is <b>not</b> acquired!
        <br>
        @return <b>un</b>acquired C mapping
    */
    inline uno_Mapping * SAL_CALL get() const
        { return _pMapping; }

    /** Tests if a mapping is set.
        <br>
        @return true if a mapping is set
    */
    inline sal_Bool SAL_CALL is() const
        { return (_pMapping != 0); }

    /** Releases a set mapping.
        <br>
    */
    inline void SAL_CALL clear();

    /** Maps an interface from one environment to another.
        <br>
        @param pInterface       source interface
        @param pTypeDescr       type description of interface
        @return                 mapped interface
    */
    inline void * SAL_CALL mapInterface( void * pInterface, typelib_InterfaceTypeDescription * pTypeDescr ) const;
    /** Maps an interface from one environment to another.
        <br>
        @param pInterface       source interface
        @param pTypeDescr       type description of interface
        @return                 mapped interface
    */
    inline void * SAL_CALL mapInterface( void * pInterface, typelib_TypeDescription * pTypeDescr ) const
        { return mapInterface( pInterface, (typelib_InterfaceTypeDescription *)pTypeDescr ); }

    /** Maps an interface from one environment to another.
        <br>
        @param pInterface       source interface
        @param rType            type of interface
        @return                 mapped interface
    */
    inline void * SAL_CALL mapInterface( void * pInterface, const ::com::sun::star::uno::Type & rType ) const;

    /** Maps an interface from one environment to another.
        <br>
        @param ppOut            inout mapped interface
        @param pInterface       source interface
        @param pTypeDescr       type description of interface
    */
    inline void SAL_CALL mapInterface( void ** ppOut, void * pInterface, typelib_InterfaceTypeDescription * pTypeDescr ) const
        { (*_pMapping->mapInterface)( _pMapping, ppOut, pInterface, pTypeDescr ); }
    /** Maps an interface from one environment to another.
        <br>
        @param ppOut            inout mapped interface
        @param pInterface       source interface
        @param pTypeDescr       type description of interface
    */
    inline void SAL_CALL mapInterface( void ** ppOut, void * pInterface, typelib_TypeDescription * pTypeDescr ) const
        { (*_pMapping->mapInterface)( _pMapping, ppOut, pInterface, (typelib_InterfaceTypeDescription *)pTypeDescr ); }

    /** Maps an interface from one environment to another.
        <br>
        @param ppOut            inout mapped interface
        @param pInterface       source interface
        @param rType            type of interface to be mapped
     */
    inline void SAL_CALL mapInterface( void ** ppOut, void * pInterface, const ::com::sun::star::uno::Type & rType ) const;
};
//__________________________________________________________________________________________________
inline Mapping::Mapping( const ::rtl::OUString & rFrom, const ::rtl::OUString & rTo,
                         const ::rtl::OUString & rAddPurpose )
    : _pMapping( 0 )
{
    uno_getMappingByName( &_pMapping, rFrom.pData, rTo.pData, rAddPurpose.pData );
}
//__________________________________________________________________________________________________
inline Mapping::Mapping( uno_Environment * pFrom, uno_Environment * pTo,
                         const ::rtl::OUString & rAddPurpose )
    : _pMapping( 0 )
{
    uno_getMapping( &_pMapping, pFrom, pTo, rAddPurpose.pData );
}
//__________________________________________________________________________________________________
inline Mapping::Mapping( uno_Mapping * pMapping )
    : _pMapping( pMapping )
{
    if (_pMapping)
        (*_pMapping->acquire)( _pMapping );
}
//__________________________________________________________________________________________________
inline Mapping::Mapping( const Mapping & rMapping )
    : _pMapping( rMapping._pMapping )
{
    if (_pMapping)
        (*_pMapping->acquire)( _pMapping );
}
//__________________________________________________________________________________________________
inline Mapping::~Mapping()
{
    if (_pMapping)
        (*_pMapping->release)( _pMapping );
}
//__________________________________________________________________________________________________
inline void Mapping::clear()
{
    if (_pMapping)
    {
        (*_pMapping->release)( _pMapping );
        _pMapping = 0;
    }
}
//__________________________________________________________________________________________________
inline Mapping & Mapping::operator = ( uno_Mapping * pMapping )
{
    if (pMapping)
        (*pMapping->acquire)( pMapping );
    if (_pMapping)
        (*_pMapping->release)( _pMapping );
    _pMapping = pMapping;
    return *this;
}
//__________________________________________________________________________________________________
inline void Mapping::mapInterface( void ** ppOut, void * pInterface,
                                   const ::com::sun::star::uno::Type & rType ) const
{
    typelib_TypeDescription * pTD = 0;
    TYPELIB_DANGER_GET( &pTD, rType.getTypeLibType() );
    if (pTD)
    {
        (*_pMapping->mapInterface)( _pMapping, ppOut, pInterface, (typelib_InterfaceTypeDescription *)pTD );
        TYPELIB_DANGER_RELEASE( pTD );
    }
}
//__________________________________________________________________________________________________
inline void * Mapping::mapInterface( void * pInterface,
                                     typelib_InterfaceTypeDescription * pTypeDescr ) const
{
    void * pOut = 0;
    (*_pMapping->mapInterface)( _pMapping, &pOut, pInterface, pTypeDescr );
    return pOut;
}
//__________________________________________________________________________________________________
inline void * Mapping::mapInterface( void * pInterface,
                                     const ::com::sun::star::uno::Type & rType ) const
{
    void * pOut = 0;
    mapInterface( &pOut, pInterface, rType );
    return pOut;
}

//--------------------------------------------------------------------------------------------------
template< class C >
inline sal_Bool mapToCpp( Reference< C > * ppRet, uno_Interface * pUnoI )
{
    Mapping aMapping(
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(UNO_LB_UNO) ),
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(CPPU_CURRENT_LANGUAGE_BINDING_NAME) ) );
    OSL_ASSERT( aMapping.is() );
    aMapping.mapInterface( (void **)ppRet, pUnoI, ::getCppuType( ppRet ) );
    return (0 != *ppRet);
}
//--------------------------------------------------------------------------------------------------
template< class C >
inline sal_Bool mapToUno( uno_Interface ** ppRet, const Reference< C > & x )
{
    Mapping aMapping(
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(CPPU_CURRENT_LANGUAGE_BINDING_NAME) ),
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(UNO_LB_UNO) ) );
    OSL_ASSERT( aMapping.is() );
    aMapping.mapInterface( (void **)ppRet, x.get(), ::getCppuType( &x ) );
    return (0 != *ppRet);
}

}
}
}
}

#endif
