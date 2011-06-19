/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef FORMS_SOURCE_RICHTEXT_RTATTRIBUTES_HXX
#define FORMS_SOURCE_RICHTEXT_RTATTRIBUTES_HXX

#include <tools/solar.h>
#include <sal/types.h>
#include <svl/poolitem.hxx>

//........................................................................
namespace frm
{
//........................................................................

    //====================================================================
    //= misc
    //====================================================================
    /// the id of an attribute
    typedef sal_Int32   AttributeId;
    /// the "which id" of an item in an SfxItemSet
    typedef sal_uInt16      WhichId;
    /// a SFX slot id
    typedef sal_uInt16      SfxSlotId;
    /// a script type
    typedef sal_uInt16      ScriptType;

    //====================================================================
    //= AttributeCheckState
    //====================================================================
    enum AttributeCheckState
    {
        eChecked,
        eUnchecked,
        eIndetermined
    };

    //====================================================================
    //= AttributeState
    //====================================================================
    struct AttributeState
    {
    private:
        SfxItemHandle*      pItemHandle;

    public:
        AttributeCheckState eSimpleState;

        //................................................................
        inline          AttributeState( );
        inline explicit AttributeState( AttributeCheckState _eCheckState );
        inline          AttributeState( const AttributeState& _rSource );

        inline AttributeState& operator=( const AttributeState& _rSource );

        inline bool operator==( const AttributeState& _rRHS );

        inline const SfxPoolItem* getItem() const;
        inline void setItem( const SfxPoolItem* _pItem );
    };

    //====================================================================
    //= AttributeState (inline implementation)
    //====================================================================
    //................................................................
    inline AttributeState::AttributeState( )
        :pItemHandle( NULL )
        ,eSimpleState( eIndetermined )
    {
    }

    //................................................................
    inline AttributeState::AttributeState( AttributeCheckState _eCheckState )
        :pItemHandle( NULL )
        ,eSimpleState( _eCheckState )
    {
    }

    //................................................................
    inline AttributeState::AttributeState( const AttributeState& _rSource )
        :pItemHandle( NULL )
        ,eSimpleState( eIndetermined )
    {
        operator=( _rSource );
    }

    //................................................................
    inline AttributeState& AttributeState::operator=( const AttributeState& _rSource )
    {
        if ( &_rSource == this )
            return *this;

        eSimpleState = _rSource.eSimpleState;
        setItem( _rSource.getItem() );
        return *this;
    }

    //................................................................
    inline const SfxPoolItem* AttributeState::getItem() const
    {
        return pItemHandle ? &pItemHandle->GetItem() : NULL;
    }

    //................................................................
    inline void AttributeState::setItem( const SfxPoolItem* _pItem )
    {
        if ( pItemHandle )
            delete pItemHandle;
        if ( _pItem )
            pItemHandle = new SfxItemHandle( *const_cast< SfxPoolItem* >( _pItem ) );
        else
            pItemHandle = NULL;
    }

    //................................................................
    inline bool AttributeState::operator==( const AttributeState& _rRHS )
    {
        if ( eSimpleState != _rRHS.eSimpleState )
            return false;

        if ( pItemHandle && !_rRHS.pItemHandle )
            return false;

        if ( !pItemHandle && _rRHS.pItemHandle )
            return false;

        if ( !pItemHandle && !_rRHS.pItemHandle )
            return true;

        return ( pItemHandle->GetItem() == _rRHS.pItemHandle->GetItem() );
    }

    //====================================================================
    //= IMultiAttributeDispatcher
    //====================================================================
    class IMultiAttributeDispatcher
    {
    public:
        virtual AttributeState  getState( AttributeId _nAttributeId ) const = 0;
        virtual void            executeAttribute( AttributeId _nAttributeId, const SfxPoolItem* _pArgument ) = 0;
    };

//........................................................................
} // namespace frm
//........................................................................

#endif // FORMS_SOURCE_RICHTEXT_RTATTRIBUTES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
