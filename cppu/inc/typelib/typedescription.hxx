/*************************************************************************
 *
 *  $RCSfile: typedescription.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:25:51 $
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
#ifndef _TYPELIB_TYPEDESCRIPTION_HXX_
#define _TYPELIB_TYPEDESCRIPTION_HXX_

#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif
#ifndef _COM_SUN_STAR_UNO_TYPE_H_
#include <com/sun/star/uno/Type.h>
#endif
#ifndef _TYPELIB_TYPEDESCRIPTION_H
#include <typelib/typedescription.h>
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

/** C++ wrapper for typelib_TypeDescription.
    Constructors by name, type, type description reference will get
    the full type description.
    <br>
*/
class TypeDescription
{
    /** C typelib type description<br>
    */
    mutable typelib_TypeDescription * _pTypeDescr;

public:
    /** Constructor:
        <br>
        @param pTypeDescr a type description
    */
    inline TypeDescription( typelib_TypeDescription * pTypeDescr = 0 );
    /** Constructor:
        <br>
        @param pTypeDescrRef a type description reference
    */
    inline TypeDescription( typelib_TypeDescriptionReference * pTypeDescrRef );
    /** Constructor:
        <br>
        @param rType a type
    */
    inline TypeDescription( const ::com::sun::star::uno::Type & rType );
    /** Copy constructor:
        <br>
        @param rDescr another TypeDescription
    */
    inline TypeDescription( const TypeDescription & rDescr );
    /** Constructor:
        <br>
        @param pTypeName a type name
    */
    inline TypeDescription( rtl_uString * pTypeName );
    /** Constructor:
        <br>
        @param rTypeName a type name
    */
    inline TypeDescription( const ::rtl::OUString & rTypeName );
    /** Destructor:
        <br>
        releases type description
    */
    inline ~TypeDescription();

    /** Assignment operator:
        acquires given type description and releases a set one.
        <br>
        @param pTypeDescr another type description
        @return this TypeDescription
    */
    inline TypeDescription & operator = ( typelib_TypeDescription * pTypeDescr );
    /** Assignment operator:
        acquires given type description and releases a set one.
        <br>
        @param rTypeDescr another type description
        @return this TypeDescription
    */
    inline TypeDescription & operator =( const TypeDescription & rTypeDescr )
        { return this->operator =( rTypeDescr.get() ); }

    /** Tests if two type descriptions are equal.
        <br>
        @param pTypeDescr another type description
        @return true, if both type descriptions are equal, false otherwise
    */
    inline sal_Bool equals( const typelib_TypeDescription * pTypeDescr ) const;
    /** Tests if two type descriptions are equal.
        <br>
        @param rTypeDescr another type description
        @return true, if both type descriptions are equal, false otherwise
    */
    inline sal_Bool equals( const TypeDescription & rTypeDescr ) const
        { return equals( rTypeDescr._pTypeDescr ); }

    /** Makes stored type description complete.
        <br>
    */
    inline void makeComplete() const;

    /** Gets the <b>un</b>acquired type description pointer.
        <br>
        @return stored pointer of type description
    */
    inline typelib_TypeDescription * get() const
        { return _pTypeDescr; }
    /** Tests if a type description is set.
        <br>
        @return true, if a type description is set, false otherwise
    */
    inline sal_Bool is() const
        { return (_pTypeDescr != 0); }
};
//__________________________________________________________________________________________________
inline TypeDescription::TypeDescription( typelib_TypeDescription * pTypeDescr )
    : _pTypeDescr( pTypeDescr )
{
    if (_pTypeDescr)
        typelib_typedescription_acquire( _pTypeDescr );
}
//__________________________________________________________________________________________________
inline TypeDescription::TypeDescription( typelib_TypeDescriptionReference * pTypeDescrRef )
    : _pTypeDescr( 0 )
{
    if (pTypeDescrRef)
        typelib_typedescriptionreference_getDescription( &_pTypeDescr, pTypeDescrRef );
}
//__________________________________________________________________________________________________
inline TypeDescription::TypeDescription( const ::com::sun::star::uno::Type & rType )
    : _pTypeDescr( 0 )
{
    if (rType.getTypeLibType())
        typelib_typedescriptionreference_getDescription( &_pTypeDescr, rType.getTypeLibType() );
}
//__________________________________________________________________________________________________
inline TypeDescription::TypeDescription( const TypeDescription & rTypeDescr )
    : _pTypeDescr( rTypeDescr._pTypeDescr )
{
    if (_pTypeDescr)
        typelib_typedescription_acquire( _pTypeDescr );
}
//__________________________________________________________________________________________________
inline TypeDescription::TypeDescription( rtl_uString * pTypeName )
    : _pTypeDescr( 0 )
{
    typelib_typedescription_getByName( &_pTypeDescr , pTypeName );
}
//__________________________________________________________________________________________________
inline TypeDescription::TypeDescription( const ::rtl::OUString & rTypeName )
    : _pTypeDescr( 0 )
{
    typelib_typedescription_getByName( &_pTypeDescr , rTypeName.pData );
}
//__________________________________________________________________________________________________
inline TypeDescription::~TypeDescription()
{
    if (_pTypeDescr)
        typelib_typedescription_release( _pTypeDescr );
}
//__________________________________________________________________________________________________
inline TypeDescription & TypeDescription::operator =( typelib_TypeDescription * pTypeDescr )
{
    if (_pTypeDescr)
        typelib_typedescription_release( _pTypeDescr );
    if (_pTypeDescr = pTypeDescr)
        typelib_typedescription_acquire( _pTypeDescr );
    return *this;
}
//__________________________________________________________________________________________________
inline sal_Bool TypeDescription::equals( const typelib_TypeDescription * pTypeDescr ) const
{
    return (_pTypeDescr && pTypeDescr &&
            typelib_typedescription_equals( _pTypeDescr, pTypeDescr ));
}
//__________________________________________________________________________________________________
inline void TypeDescription::makeComplete() const
{
    if (_pTypeDescr && !_pTypeDescr->bComplete)
        ::typelib_typedescription_complete( &_pTypeDescr );
}

}
}
}
}

#endif
