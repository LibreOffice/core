/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SFX2_CONTROLWRAPPER_HXX
#define INCLUDED_SFX2_CONTROLWRAPPER_HXX

#include <tools/debug.hxx>
#include <sal/config.h>
#include <sfx2/dllapi.h>

#include <memory>

#include <vcl/button.hxx>
#include <vcl/edit.hxx>
#include <vcl/field.hxx>
#include <vcl/lstbox.hxx>
#include <svtools/valueset.hxx>
#include <svtools/ctrlbox.hxx>


namespace sfx {


/** List position type of VCL ListBox. */
typedef sal_uInt16 ListBoxPosType;
/** List position type of SVTOOLS ValueSet. */
typedef sal_uInt16 ValueSetPosType;


// Helpers


/** A helper class for mapping list positions from/to represented values.

    Deriving from this helper class adds the two functions GetValueFromPos()
    and GetPosFromValue(). The constructor receives an array of MapEntryType
    structures that represents the table of positions and values. It is
    possible to pass a null pointer, this results in a direct mapping between
    list positions and values. If the map exists, it MUST be terminated with an
    entry containing the special "not found" list position passed to the
    constructor. The value contained in this last entry is used as default
    value in case of an error.
 */
template< typename PosT, typename ValueT >
class PosValueMapper
{
public:
    typedef PosT                            PosType;
    typedef ValueT                          ValueType;
    typedef PosValueMapper< PosT, ValueT >  MapperType;

    /** A helper struct that contains a list position - value pair. */
    struct MapEntryType
    {
        PosT const          mnPos;      /// Position in the list.
        ValueT const        mnValue;    /// Corresponding value.
    };

    /** Constructs the map helper with the passed map.
        @param nNFPos  This list position is used to represent the
        "not found" or "not existing" state.
        @param pMap  The map of list positions/values. If 0, a direct mapping
        is used (simply casting between list position and values). If the map
        exists, it *MUST* be terminated by an entry containing the special
        "not found" list position. */
    explicit     PosValueMapper( PosT nNFPos, const MapEntryType* pMap ) :
                            mpMap( pMap ), mnNFPos( nNFPos ) {}

    /** Returns the value at the specified list position.
        @return  The found value, or the value of the last map entry on error. */
    ValueT              GetValueFromPos( PosT nPos ) const;
    /** Returns the list position of the specified value.
        @return  The position, or the special "not found" position on error. */
    PosT                GetPosFromValue( ValueT nValue ) const;

    /** Returns the special "not found" list position. */
    PosT         GetNotFoundPos() const { return mnNFPos; }

private:
    const MapEntryType* mpMap;      /// The list position/value map.
    PosT                mnNFPos;    /// Special "not found" list position.
};


// Base control wrapper classes


/** Base class for all control wrappers.

    Control wrappers are used to have an equal interface for various functions
    used in connections for different types of controls.

    The current tree of base classes/templates and standard control wrappers:

    ControlWrapperBase
     |
     +- SingleControlWrapper< ControlT, ValueT >
     |   |
     |   +- DummyWindowWrapper   [1]
     |   +- CheckBoxWrapper   [1]
     |   +- EditWrapper   [1]
     |   +- SvxColorListBoxWrapper   [1]
     |   |
     |   +- MetricFieldWrapper< ValueT >   [1]
     |   |   |
     |   |   +- [ValueType]MetricFieldWrapper   [1] [2]
     |   |
     |   +- ListBoxWrapper< ValueT >   [1]
     |       |
     |       +- [ValueType]ListBoxWrapper   [1] [2]
     |
     +- MultiControlWrapperHelper
         |
         +- MultiControlWrapper< ValueT >

    Notes:
    [1] Standard wrappers ready to use.
    [2] [ValueType] is one of Int16, UInt16, Int32, UInt32, UShort, ULong.

    See documentation of class ItemConnectionBase (itemconnect.hxx) for more
    details.
 */
class SFX2_DLLPUBLIC ControlWrapperBase
{
public:
    explicit     ControlWrapperBase() {}
    virtual             ~ControlWrapperBase();

private:
                        ControlWrapperBase( const ControlWrapperBase& ) = delete;
    ControlWrapperBase& operator=( const ControlWrapperBase& ) = delete;
};


// Single control wrappers


/** Base class template for control wrappers containing one single control.

    Classes created from this template store the reference to a single control
    object. It is not required that the control is derived from VCL's Window
    class. Derived classes have to implement the abstract functions
    ShowControl(), EnableControl(), IsControlDontKnow(), SetControlDontKnow(),
    GetControlValue(), and SetControlValue().

    As already stated, it is not required for ControlT to be a VCL Window.
    Anyway, ControlT must support the following functions:
    - void ControlT::Enable( bool )
    - void ControlT::Show( bool )
 */
template< typename ControlT, typename ValueT >
class SingleControlWrapper : public ControlWrapperBase
{
public:
    typedef ControlT                                 ControlType;
    typedef ValueT                                   ControlValueType;
    typedef SingleControlWrapper< ControlT, ValueT > SingleControlWrapperType;

    explicit     SingleControlWrapper( ControlT& rControl ) : mrControl( rControl ) {}

    /** Returns a reference to the control this connection works on. */
    const ControlT& GetControl() const { return mrControl; }
    /** Returns a reference to the control this connection works on. */
    ControlT&    GetControl() { return mrControl; }

    /** Derived classes return the value the control contains. */
    virtual ValueT      GetControlValue() const = 0;
    /** Derived classes set the contents of the control to the passed value. */
    virtual void        SetControlValue( ValueT aValue ) = 0;

private:
    ControlT&           mrControl;  /// The control of this wrapper.
};


#define WRAPPER_LISTBOX_ENTRY_NOTFOUND  0xFFFF  /* XXX was value of LISTBOX_ENTRY_NOTFOUND */

/** A wrapper for the VCL ListBox.

    If a position<->value map is passed to the constructor, it MUST be
    terminated with an entry containing WRAPPER_LISTBOX_ENTRY_NOTFOUND as list
    position. See documentation of the PosValueMapper template for details.
 */
template< typename ValueT >
class ListBoxWrapper :
        public SingleControlWrapper< ListBox, ValueT >,
        public PosValueMapper< ListBoxPosType, ValueT >
{
    typedef PosValueMapper< ListBoxPosType, ValueT > MapperType;

public:
    typedef typename MapperType::MapEntryType MapEntryType;

    /** @param pMap  Optional list position <-> value map.
        See PosValueMapper documentation for details. */
    explicit     ListBoxWrapper( ListBox& rListBox, const MapEntryType* pMap ) :
                            SingleControlWrapper< ListBox, ValueT >( rListBox ), MapperType( WRAPPER_LISTBOX_ENTRY_NOTFOUND, pMap ) {}

    virtual ValueT      GetControlValue() const override;
    virtual void        SetControlValue( ValueT nValue ) override;
};


//               ***  Implementation of template functions  ***


// Helpers


template< typename PosT, typename ValueT >
ValueT PosValueMapper< PosT, ValueT >::GetValueFromPos( PosT nPos ) const
{
    ValueT nValue;
    if( mpMap )
    {
        const MapEntryType* pEntry = mpMap;
        while( (pEntry->mnPos != nPos) && (pEntry->mnPos != mnNFPos) )
            ++pEntry;
        nValue = pEntry->mnValue;
    }
    else /* if( nPos != mnNFPos ) */
    {
        DBG_ASSERT( nPos != mnNFPos, "sfx2::PosValueMapper< PosT, ValueT >::GetValueFromPos(), previously uninitialized value found!" );
        nValue = static_cast< ValueT >( nPos );
    }

    return nValue;
}

template< typename PosT, typename ValueT >
PosT PosValueMapper< PosT, ValueT >::GetPosFromValue( ValueT nValue ) const
{
    PosT nPos = mnNFPos;
    if( mpMap )
    {
        const MapEntryType* pEntry = mpMap;
        while( (pEntry->mnValue != nValue) && (pEntry->mnPos != mnNFPos) )
            ++pEntry;
        nPos = pEntry->mnPos;
    }
    else if( nValue >= static_cast< ValueT >(0) )
        nPos = static_cast< PosT >( nValue );
    return nPos;
}


// Single control wrappers


template< typename ValueT >
ValueT ListBoxWrapper< ValueT >::GetControlValue() const
{
    return this->GetValueFromPos( this->GetControl().GetSelectedEntryPos() );
}

template< typename ValueT >
void ListBoxWrapper< ValueT >::SetControlValue( ValueT nValue )
{
    sal_uInt16 nPos = this->GetPosFromValue( nValue );
    if( nPos != this->GetNotFoundPos() )
        this->GetControl().SelectEntryPos( nPos );
}

} // namespace sfx

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
