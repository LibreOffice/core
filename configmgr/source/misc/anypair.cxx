/*************************************************************************
 *
 *  $RCSfile: anypair.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jb $ $Date: 2001-07-06 10:20:50 $
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

#include <anypair.hxx>

#ifndef _UNO_ANY2_H_
#include <uno/any2.h>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#define CFG_PRECOND( expr )  OSL_PRECOND ( expr, "Violated Precondition:  " #expr)
#define CFG_POSTCOND( expr ) OSL_POSTCOND( expr, "Violated Postcondition: " #expr)

namespace configmgr
{
    namespace css = com::sun::star;
    namespace uno = css::uno;

// -----------------------------------------------------------------------------
    static inline bool impl_Any_hasValue(uno_Any const * _pData)
    { return (typelib_TypeClass_VOID != _pData->pType->eTypeClass); }

// -----------------------------------------------------------------------------
    static inline
    void anypair_default_construct_Data( typelib_TypeDescriptionReference ** _ppType,
                                         cfgmgr_AnyPair_Data* _pAnyPairData)
    {
        uno_any_construct(_pAnyPairData, 0,0, uno::cpp_acquire);

        *_ppType = _pAnyPairData->pType;
        ::typelib_typedescriptionreference_acquire( *_ppType );

    }

// -----------------------------------------------------------------------------
    static inline
    void anypair_type_construct_Data(cfgmgr_AnyPair_Data* _pAnyPairData,
                                        typelib_TypeDescriptionReference * /*_pType*/)
    {
        // type is currently unused
        uno_any_construct(_pAnyPairData, 0,0, uno::cpp_acquire);
    }

// -----------------------------------------------------------------------------
    static inline
    void anypair_any_construct_Data(cfgmgr_AnyPair_Data* _pAnyPairData, uno_Any const *_pUnoAny)
    {
        uno_type_any_construct(_pAnyPairData, _pUnoAny->pData, _pUnoAny->pType, uno::cpp_acquire);
    }

// -----------------------------------------------------------------------------
    static inline
    void anypair_copy_construct_Data( cfgmgr_AnyPair_Data* _pAnyPairData,
                                      cfgmgr_AnyPair_Data const * _pAnyPairDataFrom,
                                      typelib_TypeDescriptionReference * /*_pType*/)
    {
        // type is currently unused
        uno_type_any_construct(_pAnyPairData, _pAnyPairDataFrom->pData, _pAnyPairDataFrom->pType, uno::cpp_acquire);
    }

// -----------------------------------------------------------------------------
    static inline
    void anypair_destruct_Data(cfgmgr_AnyPair_Data* _pAnyPairData,
                               typelib_TypeDescriptionReference * /*_pType*/)
    {
        // type is currently unused
        ::uno_any_destruct(_pAnyPairData, uno::cpp_release );
    }

// -----------------------------------------------------------------------------

    static
    sal_Bool anypair_assign_Data( typelib_TypeDescriptionReference** _ppType,
                                  cfgmgr_AnyPair_Data* _pAnyPairData,
                                  uno_Any const *_pUnoAny)
    {
        typelib_TypeDescriptionReference* pNewType = _pUnoAny->pType;

        bool bCanAssign  = false;
        bool bChangeType = false;

        typelib_TypeClass eOldTC = (*_ppType)->eTypeClass;
        if ( pNewType->eTypeClass == typelib_TypeClass_VOID)
            bCanAssign = true;

        else if (eOldTC == typelib_TypeClass_VOID || eOldTC == typelib_TypeClass_ANY )
            bChangeType = bCanAssign = true;

        else if ( typelib_typedescriptionreference_equals(*_ppType,pNewType)  )
            bCanAssign = true;

        else
            bCanAssign = false;

        if (bCanAssign)
        {
            ::uno_type_any_assign(_pAnyPairData, _pUnoAny->pData, _pUnoAny->pType, uno::cpp_acquire, uno::cpp_release );
            if (bChangeType)
            {
                ::typelib_typedescriptionreference_release( *_ppType );
                *_ppType = pNewType; // *_ppType = _pAnyPairData->pType
                ::typelib_typedescriptionreference_acquire( *_ppType );
            }
        }
        else
            OSL_ENSURE(false, "anypair_assign_XXX(): Cannot assign - Type mismatch");

        return bCanAssign;
    }

// -----------------------------------------------------------------------------
    static inline
    void anypair_clear_Data(cfgmgr_AnyPair_Data* _pAnyPairData,
                            typelib_TypeDescriptionReference * /*_pType*/)
    {
        // type is currently unused
        uno_any_assign(_pAnyPairData, 0, 0, uno::cpp_acquire, uno::cpp_release);
    }

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
    void anypair_construct_default(cfgmgr_AnyPair * _pAnyPair)
    {
        CFG_PRECOND( _pAnyPair != NULL );

        anypair_default_construct_Data(&_pAnyPair->pType, &_pAnyPair->m_first);
        anypair_type_construct_Data   (&_pAnyPair->m_second,_pAnyPair->pType);

        CFG_POSTCOND(  cfgmgr_AnyPair_isEmpty(_pAnyPair) );
        CFG_POSTCOND( !cfgmgr_AnyPair_Data_hasValue(&_pAnyPair->m_first) );
        CFG_POSTCOND( !cfgmgr_AnyPair_Data_hasValue(&_pAnyPair->m_second) );
    }

// -----------------------------------------------------------------------------
    void anypair_construct_type(cfgmgr_AnyPair * _pAnyPair, typelib_TypeDescriptionReference* _pType)
    {
        CFG_PRECOND( _pAnyPair != NULL );
        CFG_PRECOND( _pType != NULL );

        _pAnyPair->pType = _pType;
        ::typelib_typedescriptionreference_acquire( _pAnyPair->pType );

        anypair_type_construct_Data(&_pAnyPair->m_first ,_pAnyPair->pType);
        anypair_type_construct_Data(&_pAnyPair->m_second,_pAnyPair->pType);

        CFG_POSTCOND( !cfgmgr_AnyPair_Data_hasValue(&_pAnyPair->m_first) );
        CFG_POSTCOND( !cfgmgr_AnyPair_Data_hasValue(&_pAnyPair->m_second) );
    }

// -----------------------------------------------------------------------------
    void anypair_construct_first(cfgmgr_AnyPair * _pAnyPair, uno_Any const *_pUnoAny)
    {
        CFG_PRECOND( _pAnyPair != NULL );
        CFG_PRECOND( _pUnoAny != NULL );

        _pAnyPair->pType = _pUnoAny->pType;
        ::typelib_typedescriptionreference_acquire( _pAnyPair->pType );

        anypair_any_construct_Data (&_pAnyPair->m_first, _pUnoAny);
        anypair_type_construct_Data(&_pAnyPair->m_second,_pAnyPair->pType);

        CFG_POSTCOND( !cfgmgr_AnyPair_Data_hasValue(&_pAnyPair->m_second) );
    }

// -----------------------------------------------------------------------------
    void anypair_construct_second(cfgmgr_AnyPair * _pAnyPair, uno_Any const *_pUnoAny)
    {
        CFG_PRECOND( _pAnyPair != NULL );
        CFG_PRECOND( _pUnoAny != NULL );

        _pAnyPair->pType = _pUnoAny->pType;
        ::typelib_typedescriptionreference_acquire( _pAnyPair->pType );

        anypair_type_construct_Data(&_pAnyPair->m_first, _pAnyPair->pType);
        anypair_any_construct_Data (&_pAnyPair->m_second,_pUnoAny);

        CFG_POSTCOND( !cfgmgr_AnyPair_Data_hasValue(&_pAnyPair->m_first) );
    }

// -----------------------------------------------------------------------------
    // if type not equal, you got false and the struct contains undefined values
    sal_Bool anypair_construct(cfgmgr_AnyPair * _pAnyPair, uno_Any const * _pFirstAny, uno_Any const *_pSecondAny)
    {
        CFG_PRECOND( _pAnyPair != NULL );
        CFG_PRECOND( _pFirstAny  != NULL );
        CFG_PRECOND( _pSecondAny != NULL );

        bool bHasFirst  = impl_Any_hasValue(_pFirstAny);
        bool bHasSecond = impl_Any_hasValue(_pSecondAny);

        if (bHasFirst && bHasSecond)
        {
            if ( ! typelib_typedescriptionreference_equals(_pFirstAny->pType,_pSecondAny->pType))
            {
                OSL_ENSURE(false, "anypair_construct(): Cannot construct - Different types");
                return false;
            }
        }

        _pAnyPair->pType = bHasFirst ? _pFirstAny->pType : _pSecondAny->pType;
        ::typelib_typedescriptionreference_acquire( _pAnyPair->pType );

        anypair_any_construct_Data (&_pAnyPair->m_first ,_pFirstAny);
        anypair_any_construct_Data (&_pAnyPair->m_second,_pSecondAny);

        CFG_POSTCOND((bHasFirst || bHasSecond) == !cfgmgr_AnyPair_isEmpty(_pAnyPair) );
        CFG_POSTCOND( bHasFirst  == cfgmgr_AnyPair_Data_hasValue(&_pAnyPair->m_first) );
        CFG_POSTCOND( bHasSecond == cfgmgr_AnyPair_Data_hasValue(&_pAnyPair->m_second) );

        return true;
    }
// -----------------------------------------------------------------------------

    void anypair_copy_construct(cfgmgr_AnyPair* _pAnyPair, cfgmgr_AnyPair const * _pAnyPairFrom)
    {
        CFG_PRECOND( _pAnyPair     != NULL );
        CFG_PRECOND( _pAnyPairFrom != NULL );

        _pAnyPair->pType = _pAnyPairFrom->pType;
        ::typelib_typedescriptionreference_acquire( _pAnyPair->pType );

        anypair_copy_construct_Data(&_pAnyPair->m_first,  &_pAnyPairFrom->m_first,  _pAnyPair->pType);
        anypair_copy_construct_Data(&_pAnyPair->m_second, &_pAnyPairFrom->m_second, _pAnyPair->pType);

    }

// -----------------------------------------------------------------------------
    void anypair_destruct(cfgmgr_AnyPair* _pAnyPair)
    {
        CFG_PRECOND( _pAnyPair != NULL );

        anypair_destruct_Data(&_pAnyPair->m_first,  _pAnyPair->pType);
        anypair_destruct_Data(&_pAnyPair->m_second, _pAnyPair->pType );

        ::typelib_typedescriptionreference_release( _pAnyPair->pType );
        OSL_DEBUG_ONLY(_pAnyPair->pType = (typelib_TypeDescriptionReference*)0xdeadbeef);

    }

// -----------------------------------------------------------------------------
    sal_Bool anypair_assign_first(cfgmgr_AnyPair* _pAnyPair, uno_Any const * _pUnoAny)
    {
        CFG_PRECOND( _pAnyPair     != NULL );
        CFG_PRECOND( _pUnoAny != NULL );

        return anypair_assign_Data(&_pAnyPair->pType, &_pAnyPair->m_first, _pUnoAny);
    }

// -----------------------------------------------------------------------------
    sal_Bool anypair_assign_second(cfgmgr_AnyPair* _pAnyPair, uno_Any const * _pUnoAny)
    {
        return anypair_assign_Data(&_pAnyPair->pType, &_pAnyPair->m_second, _pUnoAny);
    }

// -----------------------------------------------------------------------------
    void anypair_assign(cfgmgr_AnyPair* _pAnyPair, cfgmgr_AnyPair const * _pAnyPairFrom)
    {
        if (_pAnyPair != _pAnyPairFrom)
        {
            anypair_destruct(_pAnyPair);
            anypair_copy_construct(_pAnyPair, _pAnyPairFrom);
        }
    }

// -----------------------------------------------------------------------------
    void anypair_clear_first(cfgmgr_AnyPair* _pAnyPair)
    {
        CFG_PRECOND( _pAnyPair != NULL );

        anypair_clear_Data(&_pAnyPair->m_first, _pAnyPair->pType);

        CFG_POSTCOND( !cfgmgr_AnyPair_Data_hasValue(&_pAnyPair->m_first) );
    }

// -----------------------------------------------------------------------------
    void anypair_clear_second(cfgmgr_AnyPair* _pAnyPair)
    {
        CFG_PRECOND( _pAnyPair != NULL );

        anypair_clear_Data(&_pAnyPair->m_second, _pAnyPair->pType );

        CFG_POSTCOND( !cfgmgr_AnyPair_Data_hasValue(&_pAnyPair->m_second) );
    }

// -----------------------------------------------------------------------------
    void anypair_clear_values(cfgmgr_AnyPair* _pAnyPair)
    {
        CFG_PRECOND( _pAnyPair != NULL );

        anypair_clear_Data(&_pAnyPair->m_first, _pAnyPair->pType);
        anypair_clear_Data(&_pAnyPair->m_second, _pAnyPair->pType );

        CFG_POSTCOND( !cfgmgr_AnyPair_Data_hasValue(&_pAnyPair->m_first) );
        CFG_POSTCOND( !cfgmgr_AnyPair_Data_hasValue(&_pAnyPair->m_second) );
    }

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
    static
    inline
    uno::Any anypair_Data_toAny(cfgmgr_AnyPair_Data const * _pAnyPairData,
                                typelib_TypeDescriptionReference * /*_pType*/)
    {
        return uno::Any( _pAnyPairData->pData, _pAnyPairData->pType );
    }

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
    // ctors
    AnyPair::AnyPair()
    {
        anypair_construct_default(&m_aAnyPair);
    }

// -----------------------------------------------------------------------------
    AnyPair::AnyPair(uno::Type const& _aType)                // one Type, any's are null
    {
        anypair_construct_type(&m_aAnyPair, _aType.getTypeLibType());
    }

// -----------------------------------------------------------------------------
    AnyPair::AnyPair(uno::Any const& _aAny)          // one any
    {
        anypair_construct_first(&m_aAnyPair,&_aAny);
    }

// -----------------------------------------------------------------------------
    AnyPair::AnyPair(uno::Any const& _aAny, uno::Any const& _aAny2) SAL_THROW((lang::IllegalArgumentException))
    {
        if (!anypair_construct(&m_aAnyPair,&_aAny, &_aAny2))
        {
            // throw lang::IllegalArgumentException(rtl::OUString::createFromAscii("Types are not equal."));
        }
    }

// -----------------------------------------------------------------------------
    // copy-ctor
    AnyPair::AnyPair(AnyPair const& _aOther)
    {
        anypair_copy_construct(&m_aAnyPair, &_aOther.m_aAnyPair);
    }

// -----------------------------------------------------------------------------
    // assign operator
    AnyPair& AnyPair::operator=(AnyPair const& _aOther)
    {
        anypair_assign(&m_aAnyPair, &_aOther.m_aAnyPair);
        return *this;
    }

// -----------------------------------------------------------------------------
    // d-tor
    AnyPair::~AnyPair()
    {
        anypair_destruct(&m_aAnyPair);
    }


// -----------------------------------------------------------------------------
    sal_Bool AnyPair::setFirst(uno::Any const& _aAny)
    {
        return anypair_assign_first(&m_aAnyPair,&_aAny);
    }

// -----------------------------------------------------------------------------
    sal_Bool AnyPair::setSecond(uno::Any const& _aAny)
    {
        return anypair_assign_second(&m_aAnyPair,&_aAny);
    }

// -----------------------------------------------------------------------------
    uno::Any AnyPair::getFirst() const
    {
        return anypair_Data_toAny( &m_aAnyPair.m_first, m_aAnyPair.pType );
    }
// -----------------------------------------------------------------------------
    uno::Any AnyPair::getSecond() const
    {
        return anypair_Data_toAny( &m_aAnyPair.m_second, m_aAnyPair.pType );
    }

// -----------------------------------------------------------------------------
    uno::Type AnyPair::getValueType() const
    {
        return uno::Type(m_aAnyPair.pType);
    }

// -----------------------------------------------------------------------------

} // namespace

