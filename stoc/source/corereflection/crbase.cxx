/*************************************************************************
 *
 *  $RCSfile: crbase.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-12 15:32:15 $
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

#ifndef _CPPUHELPER_QUERYINTERFACE_HXX_
#include <cppuhelper/queryinterface.hxx>
#endif
#ifndef _UNO_ANY2_H_
#include <uno/any2.h>
#endif

#include "base.hxx"

namespace stoc_corefl
{

#ifdef TEST_LIST_CLASSES
ClassNameList g_aClassNames;
#endif

//--------------------------------------------------------------------------------------------------
Mutex & getMutexAccess()
{
    static Mutex * s_pMutex = 0;
    if (! s_pMutex)
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if (! s_pMutex)
        {
            static Mutex s_aMutex;
            s_pMutex = &s_aMutex;
        }
    }
    return *s_pMutex;
}

//__________________________________________________________________________________________________
IdlClassImpl::IdlClassImpl( IdlReflectionServiceImpl * pReflection,
                            const OUString & rName, typelib_TypeClass eTypeClass,
                            typelib_TypeDescription * pTypeDescr )
    : _pReflection( pReflection )
    , _aName( rName )
    , _eTypeClass( (TypeClass)eTypeClass )
    , _pTypeDescr( pTypeDescr )
{
    if (_pReflection)
        _pReflection->acquire();
    if (_pTypeDescr)
    {
        typelib_typedescription_acquire( _pTypeDescr );
        if (! _pTypeDescr->bComplete)
            typelib_typedescription_complete( &_pTypeDescr );
    }

#ifdef TEST_LIST_CLASSES
    ClassNameList::const_iterator iFind( find( g_aClassNames.begin(), g_aClassNames.end(), _aName ) );
    OSL_ENSURE( iFind == g_aClassNames.end(), "### idl class already exists!" );
    g_aClassNames.push_front( _aName );
#endif
}
//__________________________________________________________________________________________________
IdlClassImpl::~IdlClassImpl()
{
    if (_pTypeDescr)
        typelib_typedescription_release( _pTypeDescr );
    if (_pReflection)
        _pReflection->release();

#ifdef TEST_LIST_CLASSES
    ClassNameList::iterator iFind( find( g_aClassNames.begin(), g_aClassNames.end(), _aName ) );
    OSL_ENSURE( iFind != g_aClassNames.end(), "### idl class does not exist!" );
    g_aClassNames.erase( iFind );
#endif
}

// XIdlClassImpl default implementation
//__________________________________________________________________________________________________
TypeClass IdlClassImpl::getTypeClass()
    throw(::com::sun::star::uno::RuntimeException)
{
    return _eTypeClass;
}
//__________________________________________________________________________________________________
OUString IdlClassImpl::getName()
    throw(::com::sun::star::uno::RuntimeException)
{
    return _aName;
}
//__________________________________________________________________________________________________
sal_Bool IdlClassImpl::equals( const Reference< XIdlClass >& xType )
    throw(::com::sun::star::uno::RuntimeException)
{
    return (xType.is() &&
            (xType->getTypeClass() == _eTypeClass) && (xType->getName() == _aName));
}

static sal_Bool s_aAssignableFromTab[11][11] =
{
                         /* from CH,BO,BY,SH,US,LO,UL,HY,UH,FL,DO */
/* TypeClass_CHAR */            { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
/* TypeClass_BOOLEAN */         { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
/* TypeClass_BYTE */            { 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
/* TypeClass_SHORT */           { 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0 },
/* TypeClass_UNSIGNED_SHORT */  { 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0 },
/* TypeClass_LONG */            { 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0 },
/* TypeClass_UNSIGNED_LONG */   { 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0 },
/* TypeClass_HYPER */           { 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0 },
/* TypeClass_UNSIGNED_HYPER */  { 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0 },
/* TypeClass_FLOAT */           { 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0 },
/* TypeClass_DOUBLE */          { 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1 }
};
//__________________________________________________________________________________________________
sal_Bool IdlClassImpl::isAssignableFrom( const Reference< XIdlClass > & xType )
    throw(::com::sun::star::uno::RuntimeException)
{
    TypeClass eAssign = getTypeClass();
    if (equals( xType ) || eAssign == TypeClass_ANY) // default shot
    {
        return sal_True;
    }
    else
    {
        TypeClass eFrom   = xType->getTypeClass();
        if (eAssign > TypeClass_VOID && eAssign < TypeClass_STRING &&
            eFrom > TypeClass_VOID && eFrom < TypeClass_STRING)
        {
            return s_aAssignableFromTab[eAssign-1][eFrom-1];
        }
    }
    return sal_False;
}
//__________________________________________________________________________________________________
void IdlClassImpl::createObject( Any & rObj )
    throw(::com::sun::star::uno::RuntimeException)
{
    rObj.clear();
    uno_any_destruct( &rObj, cpp_release );
    uno_any_construct( &rObj, 0, getTypeDescr(), 0 );
}

// what TODO ????
//__________________________________________________________________________________________________
Sequence< Reference< XIdlClass > > IdlClassImpl::getClasses()
    throw(::com::sun::star::uno::RuntimeException)
{
    OSL_ENSURE( sal_False, "### unexpected use!" );
    return Sequence< Reference< XIdlClass > >();
}
//__________________________________________________________________________________________________
Reference< XIdlClass > IdlClassImpl::getClass( const OUString & rName )
    throw(::com::sun::star::uno::RuntimeException)
{
    OSL_ENSURE( sal_False, "### unexpected use!" );
    return Reference< XIdlClass >();
}
//__________________________________________________________________________________________________
Sequence< Reference< XIdlClass > > IdlClassImpl::getInterfaces()
    throw(::com::sun::star::uno::RuntimeException)
{
//      OSL_ENSURE( sal_False, "### unexpected use!" );
    return Sequence< Reference< XIdlClass > >();
}

// structs, interfaces
//__________________________________________________________________________________________________
Sequence< Reference< XIdlClass > > IdlClassImpl::getSuperclasses() throw(::com::sun::star::uno::RuntimeException)
{
    return Sequence< Reference< XIdlClass > >();
}
// structs
//__________________________________________________________________________________________________
Reference< XIdlField > IdlClassImpl::getField( const OUString & rName )
    throw(::com::sun::star::uno::RuntimeException)
{
    return Reference< XIdlField >();
}
//__________________________________________________________________________________________________
Sequence< Reference< XIdlField > > IdlClassImpl::getFields()
    throw(::com::sun::star::uno::RuntimeException)
{
    return Sequence< Reference< XIdlField > >();
}
// interfaces
//__________________________________________________________________________________________________
Uik IdlClassImpl::getUik()
    throw(::com::sun::star::uno::RuntimeException)
{
    return Uik();
}
//__________________________________________________________________________________________________
Reference< XIdlMethod > IdlClassImpl::getMethod( const OUString & rName )
    throw(::com::sun::star::uno::RuntimeException)
{
    return Reference< XIdlMethod >();
}
//__________________________________________________________________________________________________
Sequence< Reference< XIdlMethod > > IdlClassImpl::getMethods()
    throw(::com::sun::star::uno::RuntimeException)
{
    return Sequence< Reference< XIdlMethod > >();
}
// array
//__________________________________________________________________________________________________
Reference< XIdlClass > IdlClassImpl::getComponentType()
    throw(::com::sun::star::uno::RuntimeException)
{
    return Reference< XIdlClass >();
}
//__________________________________________________________________________________________________
Reference< XIdlArray > IdlClassImpl::getArray()
    throw(::com::sun::star::uno::RuntimeException)
{
    return Reference< XIdlArray >();
}


//##################################################################################################
//##################################################################################################
//##################################################################################################


//__________________________________________________________________________________________________
IdlMemberImpl::IdlMemberImpl( IdlReflectionServiceImpl * pReflection, const OUString & rName,
                              typelib_TypeDescription * pTypeDescr,
                              typelib_TypeDescription * pDeclTypeDescr )
    : _pReflection( pReflection )
    , _aName( rName )
    , _pTypeDescr( pTypeDescr )
    , _pDeclTypeDescr( pDeclTypeDescr )
{
    _pReflection->acquire();
    typelib_typedescription_acquire( _pTypeDescr );
    typelib_typedescription_acquire( _pDeclTypeDescr );
}
//__________________________________________________________________________________________________
IdlMemberImpl::~IdlMemberImpl()
{
    typelib_typedescription_release( _pDeclTypeDescr );
    typelib_typedescription_release( _pTypeDescr );
    _pReflection->release();
}

// XIdlMember
//__________________________________________________________________________________________________
Reference< XIdlClass > IdlMemberImpl::getDeclaringClass()
    throw(::com::sun::star::uno::RuntimeException)
{
    if (! _xDeclClass.is())
        _xDeclClass = getReflection()->forType( getDeclTypeDescr() );
    return _xDeclClass;
}
//__________________________________________________________________________________________________
OUString IdlMemberImpl::getName()
    throw(::com::sun::star::uno::RuntimeException)
{
    return _aName;
}

}


