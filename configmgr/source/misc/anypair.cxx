/*************************************************************************
 *
 *  $RCSfile: anypair.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jb $ $Date: 2001-06-20 20:16:02 $
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

#ifndef _UNO_ANY2_H_
#include <uno/any2.h>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_H_
#include <com/sun/star/uno/Any.h>
#endif

#include <anypair.hxx>

#include <rtl/alloc.h>

namespace configmgr
{
    namespace css = com::sun::star;
    namespace uno = css::uno;



// Dirty Hack
    inline uno_Any * firstAny(cfgmgr_AnyPair* _pPair)
    {
        return reinterpret_cast<uno_Any*>(_pPair);
    }

// -----------------------------------------------------------------------------
    void anypair_construct_default(cfgmgr_AnyPair *_pAnyPair)
    {
        _pAnyPair->m_pFirst = _pAnyPair->m_pSecond = NULL;
        uno_any_construct(firstAny(_pAnyPair),0,0, uno::cpp_acquire);
        OSL_ASSERT(_pAnyPair->m_pFirst == NULL);
        _pAnyPair->m_pSecond = NULL;
    }

// -----------------------------------------------------------------------------
    void anypair_construct_type(cfgmgr_AnyPair *_pAnyPair, typelib_TypeDescriptionReference* _pType)
    {
        _pAnyPair->m_pFirst = _pAnyPair->m_pSecond = NULL;
        _pAnyPair->m_pType = _pType;
        ::typelib_typedescriptionreference_acquire( _pAnyPair->m_pType );
    }
// -----------------------------------------------------------------------------
    void anypair_assign_type(cfgmgr_AnyPair *_pAnyPair, typelib_TypeDescriptionReference* _pType)
    {
        typelib_typedescriptionreference_assign( &_pAnyPair->m_pType, _pType );
    }

// -----------------------------------------------------------------------------
    void anypair_construct_first(cfgmgr_AnyPair *_pAnyPair, const uno_Any *_pUnoAny)
    {
        _pAnyPair->m_pFirst = _pAnyPair->m_pSecond = NULL;

        uno_type_any_construct(firstAny(_pAnyPair), _pUnoAny->pData, _pUnoAny->pType, uno::cpp_acquire);
        _pAnyPair->m_pSecond = NULL;
    }

// -----------------------------------------------------------------------------
    void anypair_construct_second(cfgmgr_AnyPair *_pAnyPair, const uno_Any *_pUnoAny)
    {
        _pAnyPair->m_pFirst = _pAnyPair->m_pSecond = NULL;

        uno_Any aTmp;
        uno_type_any_construct(&aTmp,_pUnoAny->pData, _pUnoAny->pType, uno::cpp_acquire);
        _pAnyPair->m_pType = aTmp.pType;
        _pAnyPair->m_pSecond = aTmp.pData;
        _pAnyPair->m_pFirst = NULL;
    }

// -----------------------------------------------------------------------------
    // if type not equal, you got false and the struct contains {0,0,0}
    sal_Bool anypair_construct(cfgmgr_AnyPair *_pAnyPair, const uno_Any* _pFirstAny, const uno_Any *_pSecondAny)
    {
        _pAnyPair->m_pFirst = _pAnyPair->m_pSecond = NULL;
        bool bHasFirst  = (_pFirstAny ->pType->eTypeClass != typelib_TypeClass_VOID);
        bool bHasSecond = (_pSecondAny->pType->eTypeClass != typelib_TypeClass_VOID);

        if (bHasFirst)
        {
            if (bHasSecond && ! typelib_typedescriptionreference_equals(_pFirstAny->pType,_pSecondAny->pType))
            {
                OSL_ENSURE(false, "anypair_construct(): Cannot construct - Different types");
                return false;
            }

            // construct first value
            uno_type_any_construct(firstAny(_pAnyPair), _pFirstAny->pData, _pFirstAny->pType, uno::cpp_acquire);

            if (bHasSecond && _pSecondAny->pData != NULL)
            {
                // construct second value
                uno_Any aTmp;
                uno_type_any_construct(&aTmp,_pSecondAny->pData, _pSecondAny->pType, uno::cpp_acquire);
                _pAnyPair->m_pSecond = aTmp.pData;

                OSL_ASSERT(typelib_typedescriptionreference_equals(aTmp.pType,_pAnyPair->m_pType));
                typelib_typedescriptionreference_release(aTmp.pType);
            }
            // else
            //  _pAnyPair->m_pSecond = NULL;
        }
        else
        {
            if (bHasSecond)
            {
                anypair_construct_second(_pAnyPair,_pSecondAny);
            }
            else
            {
                _pAnyPair->m_pType = _pFirstAny->pType;
                typelib_typedescriptionreference_acquire(_pAnyPair->m_pType);
            //  _pAnyPair->m_pFirst = NULL;
            }
        }

        return true;
    }

// -----------------------------------------------------------------------------
    void anypair_clear_first(cfgmgr_AnyPair* _pAnyPair)
    {
        if (_pAnyPair->m_pFirst)
        {
            uno_Any aTmp;
            aTmp.pData = _pAnyPair->m_pFirst;
            aTmp.pType = _pAnyPair->m_pType;
            typelib_typedescriptionreference_acquire(aTmp.pType);

            uno_any_destruct(&aTmp, uno::cpp_release);

            _pAnyPair->m_pFirst = NULL;
        }
    }

// -----------------------------------------------------------------------------
    void anypair_clear_second(cfgmgr_AnyPair* _pAnyPair)
    {
        if (_pAnyPair->m_pSecond)
        {
            uno_Any aTmp;
            aTmp.pData = _pAnyPair->m_pSecond;
            aTmp.pType = _pAnyPair->m_pType;
            typelib_typedescriptionreference_acquire(aTmp.pType);

            uno_any_destruct(&aTmp, uno::cpp_release);

            _pAnyPair->m_pSecond = NULL;
        }
    }

// -----------------------------------------------------------------------------
    void anypair_destruct(cfgmgr_AnyPair* _pAnyPair)
    {
        anypair_clear_first(_pAnyPair);
        anypair_clear_second(_pAnyPair);

        ::typelib_typedescriptionreference_release( _pAnyPair->m_pType );
        OSL_DEBUG_ONLY(_pAnyPair->m_pType = (typelib_TypeDescriptionReference*)0xdeadbeef);

    }

// -----------------------------------------------------------------------------
    void anypair_copy_construct(cfgmgr_AnyPair* _pAnyPair, const cfgmgr_AnyPair* _pAnyPairFrom)
    {
        _pAnyPair->m_pType = _pAnyPairFrom->m_pType;
        _pAnyPair->m_pFirst = _pAnyPair->m_pSecond = NULL;

        typelib_typedescriptionreference_acquire(_pAnyPair->m_pType);

        if (_pAnyPairFrom->m_pFirst)
        {
            uno_Any aTmp;
            uno_type_any_construct(&aTmp,_pAnyPairFrom->m_pFirst, _pAnyPairFrom->m_pType, uno::cpp_acquire);
            _pAnyPair->m_pFirst = aTmp.pData;
            OSL_ASSERT(typelib_typedescriptionreference_equals(_pAnyPair->m_pType,aTmp.pType));
            typelib_typedescriptionreference_release(aTmp.pType);
        }
        if (_pAnyPairFrom->m_pSecond)
        {
            uno_Any aTmp;
            uno_type_any_construct(&aTmp,_pAnyPairFrom->m_pSecond, _pAnyPairFrom->m_pType, uno::cpp_acquire);
            _pAnyPair->m_pSecond = aTmp.pData;
            OSL_ASSERT(typelib_typedescriptionreference_equals(_pAnyPair->m_pType,aTmp.pType));
            typelib_typedescriptionreference_release(aTmp.pType);
        }
    }

    static
    sal_Bool anypair_canassign(typelib_TypeDescriptionReference* _ppType, const uno_Any *_pUnoAny)
    {
        typelib_TypeClass eTC = _ppType->eTypeClass;
        if  (eTC == typelib_TypeClass_VOID || eTC == typelib_TypeClass_ANY )
            return true;

        else if ( typelib_typedescriptionreference_equals(_ppType,_pUnoAny->pType)  )
            return true;

        else if (_pUnoAny->pData == NULL && _pUnoAny->pType->eTypeClass == typelib_TypeClass_VOID)
            return true;

        else
            return false;
    }

    static
    sal_Bool anypair_assign_helper(typelib_TypeDescriptionReference** _ppType, void** _ppData , const uno_Any *_pUnoAny)
    {
        if (!anypair_canassign(*_ppType,_pUnoAny))
            return false;

        bool bOldNull = (*_ppData == NULL);
        bool bNewNull = (_pUnoAny->pData == NULL);

        if (bOldNull)
        {
            if (!bNewNull)
            {
                uno_Any aTmp;
                uno_type_any_construct(&aTmp,_pUnoAny->pData, _pUnoAny->pType, uno::cpp_acquire);

                typelib_typedescriptionreference_release(*_ppType);

                *_ppData = aTmp.pData;
                *_ppType = aTmp.pType;
            }
        }
        else
        {
            uno_Any aTmp;
            aTmp.pData = *_ppData;
            aTmp.pType = *_ppType;

            if (bNewNull)
                typelib_typedescriptionreference_acquire(*_ppType);

            uno_type_any_assign(&aTmp,_pUnoAny->pData, _pUnoAny->pType, uno::cpp_acquire, uno::cpp_release);

            *_ppData = aTmp.pData;
            if (bNewNull)
            {
                OSL_ASSERT(aTmp.pData == NULL);
                typelib_typedescriptionreference_release(aTmp.pType);
            }
            else
            {
                *_ppType = aTmp.pType;
            }
        }

        return true;
    }
// -----------------------------------------------------------------------------
    sal_Bool anypair_assign_first(cfgmgr_AnyPair* _pAnyPair, const uno_Any* _pAny)
    {
        return anypair_assign_helper(&_pAnyPair->m_pType, &_pAnyPair->m_pFirst, _pAny);
    }

// -----------------------------------------------------------------------------
    sal_Bool anypair_assign_second(cfgmgr_AnyPair* _pAnyPair, const uno_Any* _pAny)
    {
        return anypair_assign_helper(&_pAnyPair->m_pType, &_pAnyPair->m_pSecond, _pAny);
    }


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
    AnyPair::AnyPair(AnyPair const& _aAny)
    {
        anypair_copy_construct(&m_aAnyPair, &_aAny.m_aAnyPair);
    }

// -----------------------------------------------------------------------------
    // assign operator
    AnyPair& AnyPair::operator=(AnyPair const& _aAny)
    {
        if (this != &_aAny)
        {
            anypair_destruct(&m_aAnyPair);
            anypair_copy_construct(&m_aAnyPair, &_aAny.m_aAnyPair);
        }
        return *this;
    }

// -----------------------------------------------------------------------------
    // d-tor
    AnyPair::~AnyPair()
    {
        anypair_destruct(&m_aAnyPair);
    }


// -----------------------------------------------------------------------------
    void AnyPair::setFirst(uno::Any const& _aAny)
    {
        OSL_VERIFY(anypair_assign_first(&m_aAnyPair,&_aAny));
    }

// -----------------------------------------------------------------------------
    void AnyPair::setSecond(uno::Any const& _aAny)
    {
        OSL_VERIFY(anypair_assign_second(&m_aAnyPair,&_aAny));
    }

// -----------------------------------------------------------------------------
    uno::Any AnyPair::getFirst() const
    {
        // BACK: null any, if any not set.
        if (m_aAnyPair.m_pFirst)
        {
            return uno::Any( m_aAnyPair.m_pFirst, m_aAnyPair.m_pType );
        }
        else
            return uno::Any();
    }
// -----------------------------------------------------------------------------
    uno::Any AnyPair::getSecond() const
    {
        // BACK: null any, if any not set.
        if (m_aAnyPair.m_pSecond)
        {
            return uno::Any( m_aAnyPair.m_pSecond, m_aAnyPair.m_pType );
        }
        else
            return uno::Any();
    }

// -----------------------------------------------------------------------------
    uno::Type AnyPair::getValueType() const
    {
        return uno::Type(m_aAnyPair.m_pType);
    }

// -----------------------------------------------------------------------------

    // in Header
    // bool hasFirst() const {return m_pFirst  ? true : false;}
    // bool hasSecond() const {return m_pSecond ? true : false;}
    // bool isNull() const   {return m_pFirst == NULL && m_pSecond == 0;}
    void AnyPair::check_init()
    {
        if (this->hasFirst() || this->hasSecond())
        {
            OSL_ASSERT(this->getValueType() != ::getVoidCppuType());
        }
    }
// -----------------------------------------------------------------------------
    void AnyPair::init()
    {
        if (hasSecond())
        {
            OSL_ASSERT(this->getValueType() != ::getVoidCppuType());
        }
        else if (hasFirst())
        {
            OSL_ASSERT(this->getValueType() != ::getVoidCppuType());
        }
        else
        {
            // no type set, we must be void.
            OSL_ASSERT(this->getValueType() == ::getVoidCppuType()); // at init time ValueType must be void
        }
    }





} // namespace

