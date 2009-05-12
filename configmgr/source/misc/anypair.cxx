/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: anypair.cxx,v $
 * $Revision: 1.13 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"

#include <anypair.hxx>
#include <uno/any2.h>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Type.hxx>

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
    static inline bool impl_Any_storesData(uno_Any const * _pData)
    {
        const void * pAnyData = _pData->pData;

        const bool bSelfReferential = (pAnyData == &_pData->pReserved);
        OSL_ENSURE( bSelfReferential == ( _pData <= pAnyData && pAnyData < _pData+1 ),
                    "uno_Any layout changed: Unreckognized self-referentiality" );

        return bSelfReferential;
    }

// -----------------------------------------------------------------------------
    static inline void * impl_getDataPointer(const void * const * _pAnyPairData)
    {
        const void * const pResult = *_pAnyPairData;

        return const_cast<void*>(pResult);
    }

// -----------------------------------------------------------------------------
    static inline void * impl_getData(const void * const * _pAnyPairData, bool _bStoredData)
    {
        const void * const pResult = _bStoredData ? _pAnyPairData : *_pAnyPairData;

        return const_cast<void*>(pResult);
    }

// -----------------------------------------------------------------------------
    static inline void impl_setDataPointer(const void * * _pAnyPairData, void* _pData)
    {
        *_pAnyPairData = _pData;
    }
// -----------------------------------------------------------------------------
    static const unsigned SHIFT_DATA_FLAG = 4;
// -----------------------------------------------------------------------------
    static
    inline void impl_state_setState(sal_uInt8* _pState, sal_uInt8 _nState, sal_uInt8  _nSelect)
    {
        sal_uInt8 const nSelectMask = _nSelect | (_nSelect<<SHIFT_DATA_FLAG);
        OSL_ENSURE( (_nState & nSelectMask) == _nState, "State specified does not belong to the selector");

        *_pState &= ~nSelectMask;
        *_pState |= _nState;
    }

// -----------------------------------------------------------------------------
    static
    inline void impl_state_setNull(sal_uInt8* _pState, sal_uInt8  _nSelect)
    {
        sal_uInt8 const nSelectMask = _nSelect | (_nSelect<<SHIFT_DATA_FLAG);
        *_pState &= ~nSelectMask;
    }

// -----------------------------------------------------------------------------
    static inline
    void impl_state_setData(sal_uInt8* _pState, sal_uInt8  _nSelect)
    {
        sal_uInt8 const nSelectMask = _nSelect | (_nSelect<<SHIFT_DATA_FLAG);
        *_pState |= nSelectMask;
    }

// -----------------------------------------------------------------------------
    static inline
    void impl_state_setValue(sal_uInt8* _pState, sal_uInt8  _nSelect, bool _bStoresData)
    {
        *_pState |= _nSelect;

        _nSelect <<= SHIFT_DATA_FLAG;
        if (_bStoresData)
            *_pState |=  _nSelect;
        else
            *_pState &= ~_nSelect;
    }

// -----------------------------------------------------------------------------
    static
    inline bool impl_state_isNull(sal_uInt8 const _nState, sal_uInt8  _nSelect)
    {
        return 0 == (_nState & _nSelect);
    }

// -----------------------------------------------------------------------------
    static
    inline bool impl_state_isData(sal_uInt8 const _nState, sal_uInt8  _nSelect)
    {
        return 0 != (_nState & (_nSelect<<SHIFT_DATA_FLAG));
    }


// -----------------------------------------------------------------------------
    static
    typelib_TypeDescriptionReference * impl_getVoidType()
    {
        static const uno::Type aNullType;
        return aNullType.getTypeLibType();
    }

// -----------------------------------------------------------------------------
    static inline
    void anypair_type_construct_Desc(   cfgmgr_AnyPair_Desc* _pAnyPairDesc,
                                        typelib_TypeDescriptionReference * _pType)
    {
        _pAnyPairDesc->nState = 0;
        _pAnyPairDesc->pType  = _pType;

        typelib_typedescriptionreference_acquire( _pAnyPairDesc->pType );
    }

// -----------------------------------------------------------------------------
    static inline
    void anypair_default_construct_Desc( cfgmgr_AnyPair_Desc* _pAnyPairDesc )
    {
        anypair_type_construct_Desc(_pAnyPairDesc, impl_getVoidType());
    }

// -----------------------------------------------------------------------------
    static inline
    void anypair_empty_set_Data( const void ** _pAnyPairData )
    {
        impl_setDataPointer(_pAnyPairData, NULL);
        OSL_DEBUG_ONLY( impl_setDataPointer(_pAnyPairData, reinterpret_cast<void*>(0xdeadbeef)) );
    }

// -----------------------------------------------------------------------------
    // returns a state for the specified selector
    static inline
    sal_uInt8 anypair_any_set_Data( const void ** _pAnyPairData,
                                              sal_uInt8  _nSelect,
                                              uno_Any const *_pUnoAny)
    {
        sal_uInt8 nState = 0;

        bool bValue = impl_Any_hasValue(_pUnoAny);
        if (bValue)
        {
            uno_Any aTmpAny;
            uno_type_any_construct(&aTmpAny, _pUnoAny->pData, _pUnoAny->pType, reinterpret_cast< uno_AcquireFunc >( uno::cpp_acquire ));

            bool bData = impl_Any_storesData(&aTmpAny);

            impl_setDataPointer(_pAnyPairData, bData ? aTmpAny.pReserved : aTmpAny.pData);

            impl_state_setValue(&nState, _nSelect, bData);
        }
        else
            anypair_empty_set_Data(_pAnyPairData);

        return nState;
    }

// -----------------------------------------------------------------------------
    static inline
    sal_uInt8 anypair_copy_Data( const void ** _pAnyPairData,
                                           sal_uInt8  _nSelect,
                                           cfgmgr_AnyPair_Desc const* _pAnyPairDescFrom,
                                           const void * const* _pAnyPairDataFrom )
    {
        sal_uInt8 nState = 0;

        if (impl_state_isNull(_pAnyPairDescFrom->nState, _nSelect))
        {
            anypair_empty_set_Data(_pAnyPairData);
        }

        else
        {
            bool bOldIsData = impl_state_isData(_pAnyPairDescFrom->nState, _nSelect);

            void * pFromData = impl_getData(_pAnyPairDataFrom, bOldIsData);

            uno_Any aTmpAny;
            uno_type_any_construct(&aTmpAny, pFromData, _pAnyPairDescFrom->pType, reinterpret_cast< uno_AcquireFunc >( uno::cpp_acquire ));

            bool bNewIsData = impl_Any_storesData(&aTmpAny);
            OSL_ENSURE(bOldIsData == bNewIsData, "INFO [safe to ignore]: Copy of uno_Any changes directness !?");

            impl_setDataPointer(_pAnyPairData, bNewIsData ? aTmpAny.pReserved : aTmpAny.pData);

            impl_state_setValue(&nState, _nSelect, bNewIsData);
        }

        return nState;
    }

// -----------------------------------------------------------------------------
    static inline
    void anypair_destruct_Desc(cfgmgr_AnyPair_Desc* _pAnyPairDesc)
    {
        typelib_typedescriptionreference_release( _pAnyPairDesc->pType );
        OSL_DEBUG_ONLY(_pAnyPairDesc->nState = 0xDD);
        OSL_DEBUG_ONLY(_pAnyPairDesc->pType  = (typelib_TypeDescriptionReference*)0xdeadbeef);
    }

// -----------------------------------------------------------------------------
    static
    void anypair_clear_Data( const void ** _pAnyPairData,
                             sal_uInt8  _nSelect,
                             cfgmgr_AnyPair_Desc const* _pAnyPairDesc
                           )
    {
        if (!impl_state_isNull(_pAnyPairDesc->nState,_nSelect))
        {
            uno_Any aTmpAny;
            aTmpAny.pType = _pAnyPairDesc->pType;

            if (impl_state_isData(_pAnyPairDesc->nState,_nSelect))
            {
                aTmpAny.pReserved = impl_getDataPointer(_pAnyPairData);
                aTmpAny.pData     = &aTmpAny.pReserved;
            }
            else
            {
                aTmpAny.pReserved   = NULL;
                aTmpAny.pData       = impl_getDataPointer(_pAnyPairData);
            }

            typelib_typedescriptionreference_acquire( aTmpAny.pType );
            uno_any_destruct(&aTmpAny, reinterpret_cast< uno_ReleaseFunc >( uno::cpp_release ));

            impl_setDataPointer(_pAnyPairData, NULL);
            OSL_DEBUG_ONLY(impl_setDataPointer(_pAnyPairData, reinterpret_cast<void*>(0xDeadBeef)));
        }
    }

// -----------------------------------------------------------------------------
    static
    void anypair_Data_fill_Any( uno_Any* _pUnoAny,
                                cfgmgr_AnyPair_Desc const* _pAnyPairDesc,
                                const void * const* _pAnyPairData,
                                sal_uInt8  _nSelect )
    {
        if (impl_state_isNull(_pAnyPairDesc->nState,_nSelect))
        {
            _pUnoAny->pType = impl_getVoidType();
            _pUnoAny->pReserved = NULL;
            _pUnoAny->pData = NULL;
        }
        else if (impl_state_isData(_pAnyPairDesc->nState,_nSelect))
        {
            _pUnoAny->pType     = _pAnyPairDesc->pType;
            _pUnoAny->pReserved = impl_getDataPointer(_pAnyPairData);
            _pUnoAny->pData     = &_pUnoAny->pReserved;
        }
        else
        {
            _pUnoAny->pType     = _pAnyPairDesc->pType;
            _pUnoAny->pReserved = NULL;
            _pUnoAny->pData     = impl_getDataPointer(_pAnyPairData);
        }
    }

// -----------------------------------------------------------------------------
    static inline
    typelib_TypeDescriptionReference*
        anypair_test_assigned_type( typelib_TypeDescriptionReference* _pOldType,
                                    typelib_TypeDescriptionReference* _pNewType)
    {
        typelib_TypeDescriptionReference* pResult;
        if ( _pNewType->eTypeClass == typelib_TypeClass_VOID)
            pResult = _pOldType;

        else if (_pOldType->eTypeClass == typelib_TypeClass_VOID || _pOldType->eTypeClass == typelib_TypeClass_ANY )
            pResult = _pNewType;

        else if ( typelib_typedescriptionreference_equals(_pOldType,_pNewType)  )
            pResult = _pOldType;

        else
            pResult = NULL;

        return pResult;
    }

// -----------------------------------------------------------------------------
    static
    sal_Bool anypair_any_assign_Data(   cfgmgr_AnyPair_Desc* _pAnyPairDesc,
                                        const void ** _pAnyPairData,
                                        sal_uInt8  _nSelect,
                                        uno_Any const *_pUnoAny)
    {
        typelib_TypeDescriptionReference* pOldType = _pAnyPairDesc->pType;
        typelib_TypeDescriptionReference* pNewType = anypair_test_assigned_type(pOldType,_pUnoAny->pType);

        if (pNewType != NULL)
        {
            uno_Any aTmpAny;
            anypair_Data_fill_Any(&aTmpAny,_pAnyPairDesc,_pAnyPairData,_nSelect);

            typelib_typedescriptionreference_acquire(aTmpAny.pType);

            uno_type_any_assign(&aTmpAny,
                                _pUnoAny->pData,
                                _pUnoAny->pType,
                                reinterpret_cast< uno_AcquireFunc >( uno::cpp_acquire ),
                                reinterpret_cast< uno_AcquireFunc >( uno::cpp_release ));

            sal_uInt8 nNewState = anypair_any_set_Data(_pAnyPairData,_nSelect,&aTmpAny);
            impl_state_setState(&_pAnyPairDesc->nState, nNewState, _nSelect);

            uno_any_destruct(
                &aTmpAny,
                reinterpret_cast< uno_ReleaseFunc >(uno::cpp_release));

            if (pNewType != pOldType)
            {
                typelib_typedescriptionreference_acquire(pNewType);
                typelib_typedescriptionreference_release(pOldType);
                _pAnyPairDesc->pType = pNewType;
            }

            CFG_POSTCOND( cfgmgr_AnyPair_isNull(_pAnyPairDesc,_nSelect) == !impl_Any_hasValue(_pUnoAny) );
            CFG_POSTCOND( typelib_typedescriptionreference_equals(_pAnyPairDesc->pType,pNewType) );
        }
        else
            OSL_ENSURE(false, "anypair_assign_XXX(): Cannot assign - Type mismatch");

        return (pNewType != NULL);
    }

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
    void anypair_construct_default(cfgmgr_AnyPair * _pAnyPair)
    {
        CFG_PRECOND( _pAnyPair != NULL );

        anypair_default_construct_Desc(&_pAnyPair->desc);
        anypair_empty_set_Data(&_pAnyPair->first);
        anypair_empty_set_Data(&_pAnyPair->second);

        CFG_POSTCOND( cfgmgr_AnyPair_isNull(&_pAnyPair->desc,cfgmgr_SELECT_BOTH) );
        CFG_POSTCOND( cfgmgr_AnyPair_isEmpty(&_pAnyPair->desc) );
    }

// -----------------------------------------------------------------------------
    void anypair_construct_type(cfgmgr_AnyPair * _pAnyPair, typelib_TypeDescriptionReference* _pType)
    {
        CFG_PRECOND( _pAnyPair != NULL );
        CFG_PRECOND( _pType != NULL );

        anypair_type_construct_Desc(&_pAnyPair->desc, _pType);
        anypair_empty_set_Data(&_pAnyPair->first);
        anypair_empty_set_Data(&_pAnyPair->second);

        CFG_POSTCOND( cfgmgr_AnyPair_isNull(&_pAnyPair->desc,cfgmgr_SELECT_BOTH) );
        CFG_POSTCOND( typelib_typedescriptionreference_equals(_pAnyPair->desc.pType,_pType) );
    }

// -----------------------------------------------------------------------------
    void anypair_construct_first(cfgmgr_AnyPair * _pAnyPair, uno_Any const *_pUnoAny)
    {
        CFG_PRECOND( _pAnyPair != NULL );
        CFG_PRECOND( _pUnoAny != NULL );

        anypair_type_construct_Desc(&_pAnyPair->desc, _pUnoAny->pType);

        _pAnyPair->desc.nState = anypair_any_set_Data (&_pAnyPair->first, cfgmgr_SELECT_FIRST, _pUnoAny);

        anypair_empty_set_Data(&_pAnyPair->second);

        CFG_POSTCOND( cfgmgr_AnyPair_isNull(&_pAnyPair->desc,cfgmgr_SELECT_FIRST) == !impl_Any_hasValue(_pUnoAny) );
        CFG_POSTCOND( cfgmgr_AnyPair_isNull(&_pAnyPair->desc,cfgmgr_SELECT_SECOND) );
        CFG_POSTCOND( typelib_typedescriptionreference_equals(_pAnyPair->desc.pType,_pUnoAny ->pType) );
    }

// -----------------------------------------------------------------------------
    void anypair_construct_second(cfgmgr_AnyPair * _pAnyPair, uno_Any const *_pUnoAny)
    {
        CFG_PRECOND( _pAnyPair != NULL );
        CFG_PRECOND( _pUnoAny != NULL );

        anypair_type_construct_Desc(&_pAnyPair->desc, _pUnoAny->pType);

        anypair_empty_set_Data(&_pAnyPair->first);

        _pAnyPair->desc.nState = anypair_any_set_Data (&_pAnyPair->second, cfgmgr_SELECT_SECOND, _pUnoAny);

        CFG_POSTCOND( cfgmgr_AnyPair_isNull(&_pAnyPair->desc,cfgmgr_SELECT_FIRST) );
        CFG_POSTCOND( cfgmgr_AnyPair_isNull(&_pAnyPair->desc,cfgmgr_SELECT_SECOND) == !impl_Any_hasValue(_pUnoAny) );
        CFG_POSTCOND( typelib_typedescriptionreference_equals(_pAnyPair->desc.pType,_pUnoAny ->pType) );
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

        anypair_type_construct_Desc(&_pAnyPair->desc, bHasFirst ? _pFirstAny->pType : _pSecondAny->pType);

        sal_uInt8 nState = 0;

        nState |= anypair_any_set_Data (&_pAnyPair->first,  cfgmgr_SELECT_FIRST,  _pFirstAny);
        nState |= anypair_any_set_Data (&_pAnyPair->second, cfgmgr_SELECT_SECOND, _pSecondAny);

        _pAnyPair->desc.nState = nState;

        CFG_POSTCOND((bHasFirst || bHasSecond) == !cfgmgr_AnyPair_isEmpty(&_pAnyPair->desc) );
        CFG_POSTCOND( bHasFirst  == !cfgmgr_AnyPair_isNull(&_pAnyPair->desc,cfgmgr_SELECT_FIRST) );
        CFG_POSTCOND( bHasSecond == !cfgmgr_AnyPair_isNull(&_pAnyPair->desc,cfgmgr_SELECT_SECOND) );
        CFG_POSTCOND( !bHasFirst  || typelib_typedescriptionreference_equals(_pAnyPair->desc.pType,_pFirstAny ->pType) );
        CFG_POSTCOND( !bHasSecond || typelib_typedescriptionreference_equals(_pAnyPair->desc.pType,_pSecondAny->pType) );

        return true;
    }

// -----------------------------------------------------------------------------
    void anypair_copy_construct(cfgmgr_AnyPair* _pAnyPair, cfgmgr_AnyPair const * _pAnyPairFrom)
    {
        CFG_PRECOND( _pAnyPair     != NULL );
        CFG_PRECOND( _pAnyPairFrom != NULL );

        anypair_type_construct_Desc(&_pAnyPair->desc, _pAnyPairFrom->desc.pType);

        sal_uInt8 nState = 0;

        nState |= anypair_copy_Data(&_pAnyPair->first,  cfgmgr_SELECT_FIRST,
                                    &_pAnyPairFrom->desc, &_pAnyPairFrom->first );

        nState |= anypair_copy_Data(&_pAnyPair->second,  cfgmgr_SELECT_SECOND,
                                    &_pAnyPairFrom->desc, &_pAnyPairFrom->second );

        _pAnyPair->desc.nState = nState;

        OSL_ENSURE(_pAnyPairFrom->desc.nState == nState, "Unexpected: Copy changes state");
        CFG_POSTCOND( typelib_typedescriptionreference_equals(_pAnyPair->desc.pType,_pAnyPairFrom->desc.pType) );
    }

// -----------------------------------------------------------------------------
    void anypair_destruct(cfgmgr_AnyPair* _pAnyPair)
    {
        CFG_PRECOND( _pAnyPair != NULL );

        anypair_clear_Data(&_pAnyPair->first,  cfgmgr_SELECT_FIRST,  &_pAnyPair->desc);
        anypair_clear_Data(&_pAnyPair->second, cfgmgr_SELECT_SECOND, &_pAnyPair->desc);
        anypair_destruct_Desc(&_pAnyPair->desc );
    }

// -----------------------------------------------------------------------------
    sal_Bool anypair_assign_first(cfgmgr_AnyPair* _pAnyPair, uno_Any const * _pUnoAny)
    {
        CFG_PRECOND( _pAnyPair     != NULL );
        CFG_PRECOND( _pUnoAny != NULL );

        return anypair_any_assign_Data(&_pAnyPair->desc, &_pAnyPair->first, cfgmgr_SELECT_FIRST, _pUnoAny);
    }

// -----------------------------------------------------------------------------
    sal_Bool anypair_assign_second(cfgmgr_AnyPair* _pAnyPair, uno_Any const * _pUnoAny)
    {
        CFG_PRECOND( _pAnyPair     != NULL );
        CFG_PRECOND( _pUnoAny != NULL );

        return anypair_any_assign_Data(&_pAnyPair->desc, &_pAnyPair->second, cfgmgr_SELECT_SECOND, _pUnoAny);
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

        anypair_clear_Data(&_pAnyPair->first, cfgmgr_SELECT_FIRST, &_pAnyPair->desc);
        impl_state_setNull(&_pAnyPair->desc.nState, cfgmgr_SELECT_FIRST);

        CFG_POSTCOND( cfgmgr_AnyPair_isNull(&_pAnyPair->desc,cfgmgr_SELECT_FIRST) );
    }

// -----------------------------------------------------------------------------
    void anypair_clear_second(cfgmgr_AnyPair* _pAnyPair)
    {
        CFG_PRECOND( _pAnyPair != NULL );

        anypair_clear_Data(&_pAnyPair->second, cfgmgr_SELECT_SECOND, &_pAnyPair->desc );
        impl_state_setNull(&_pAnyPair->desc.nState, cfgmgr_SELECT_SECOND);

        CFG_POSTCOND( cfgmgr_AnyPair_isNull(&_pAnyPair->desc,cfgmgr_SELECT_SECOND) );
    }

// -----------------------------------------------------------------------------
    void anypair_clear_values(cfgmgr_AnyPair* _pAnyPair)
    {
        CFG_PRECOND( _pAnyPair != NULL );

        anypair_clear_Data(&_pAnyPair->first, cfgmgr_SELECT_FIRST, &_pAnyPair->desc);
        anypair_clear_Data(&_pAnyPair->second, cfgmgr_SELECT_SECOND, &_pAnyPair->desc );
        impl_state_setNull(&_pAnyPair->desc.nState, cfgmgr_SELECT_BOTH);

        CFG_POSTCOND( cfgmgr_AnyPair_isNull(&_pAnyPair->desc,cfgmgr_SELECT_BOTH) );
    }

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
    static
    inline
    uno::Any anypair_Data_toAny(cfgmgr_AnyPair_Desc const* _pAnyPairDesc,
                                const void * const* _pAnyPairData,
                                sal_uInt8  _nSelect)
    {
        uno_Any aTmpAny;
        anypair_Data_fill_Any(&aTmpAny,_pAnyPairDesc,_pAnyPairData,_nSelect);

        return uno::Any( aTmpAny.pData, aTmpAny.pType );
    }

// -----------------------------------------------------------------------------
    AnyPair::AnyPair(uno::Type const& _aType)                // one Type, any's are null
    {
        anypair_construct_type(&m_aAnyPair, _aType.getTypeLibType());
    }

// -----------------------------------------------------------------------------
    AnyPair::AnyPair(uno::Any const& _aAny, SelectMember _select)
    {
        switch (_select)
        {
        case SELECT_FIRST:  anypair_construct_first(&m_aAnyPair,&_aAny);  break;
        case SELECT_SECOND: anypair_construct_second(&m_aAnyPair,&_aAny); break;
        case SELECT_BOTH:   OSL_VERIFY( anypair_construct(&m_aAnyPair,&_aAny,&_aAny) ); break;

        default:            OSL_ENSURE(false, "AnyPair: Unknown member selector");
                            anypair_construct_default(&m_aAnyPair); break;
        }
    }

// -----------------------------------------------------------------------------
    AnyPair::AnyPair(uno::Any const& _aAny, uno::Any const& _aAny2) SAL_THROW((lang::IllegalArgumentException))
    {
        if (!anypair_construct(&m_aAnyPair,&_aAny, &_aAny2))
        {
            throw lang::IllegalArgumentException(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AnyPair: Type mismatch in constructor.")),NULL,-1);
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
    void AnyPair::clear(SelectMember _select)
    {
        switch (_select)
        {
        case SELECT_FIRST:  anypair_clear_first(&m_aAnyPair);  break;
        case SELECT_SECOND: anypair_clear_second(&m_aAnyPair); break;
        case SELECT_BOTH:   anypair_clear_values(&m_aAnyPair); break;

        default:            OSL_ENSURE(false, "AnyPair: Unknown member selector");
                            break;
        }
    }

// -----------------------------------------------------------------------------
    uno::Type AnyPair::getValueType() const
    {
        return uno::Type(m_aAnyPair.desc.pType);
    }

// -----------------------------------------------------------------------------
    uno::Any AnyPair::getFirst() const
    {
        return anypair_Data_toAny( &m_aAnyPair.desc, &m_aAnyPair.first, cfgmgr_SELECT_FIRST );
    }
// -----------------------------------------------------------------------------
    uno::Any AnyPair::getSecond() const
    {
        return anypair_Data_toAny( &m_aAnyPair.desc, &m_aAnyPair.second, cfgmgr_SELECT_SECOND );
    }

// -----------------------------------------------------------------------------
    uno::Any AnyPair::getValue(SelectMember _select) const
    {
        switch (_select)
        {
        case SELECT_FIRST:  return getFirst();
        case SELECT_SECOND: return getSecond();

        default:            OSL_ENSURE(false, "AnyPair: Unknown member selector");
        case SELECT_BOTH:   OSL_ENSURE(false, "AnyPair: Cannot get value - Invalid selector");
                            return uno::Any();
        }
    }

// -----------------------------------------------------------------------------

} // namespace

