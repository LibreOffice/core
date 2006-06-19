/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: rtattributes.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 13:02:08 $
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

#ifndef FORMS_SOURCE_RICHTEXT_RTATTRIBUTES_HXX
#define FORMS_SOURCE_RICHTEXT_RTATTRIBUTES_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _SFXPOOLITEM_HXX
#include <svtools/poolitem.hxx>
#endif

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
    typedef USHORT      WhichId;
    /// a SFX slot id
    typedef USHORT      SfxSlotId;
    /// a script type
    typedef USHORT      ScriptType;

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

