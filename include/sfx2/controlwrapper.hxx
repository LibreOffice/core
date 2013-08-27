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

#ifndef SFX_CONTROLWRAPPER_HXX
#define SFX_CONTROLWRAPPER_HXX

#include <tools/debug.hxx>
#include "sal/config.h"
#include "sfx2/dllapi.h"

#include <memory>

#include <vcl/button.hxx>
#include <vcl/edit.hxx>
#include <vcl/field.hxx>
#include <vcl/lstbox.hxx>
#include <svtools/valueset.hxx>
#include <svtools/ctrlbox.hxx>

// ============================================================================

namespace sfx {

// ============================================================================

/** List position type of VCL ListBox. */
typedef sal_uInt16 ListBoxPosType;
/** List position type of SVTOOLS ValueSet. */
typedef sal_uInt16 ValueSetPosType;

// ============================================================================
// Helpers
// ============================================================================

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
        PosT                mnPos;      /// Position in the list.
        ValueT              mnValue;    /// Corresponding value.
    };

    /** Constructs the map helper with the passed map.
        @param nNFPos  This list position is used to represent the
        "not found" or "not existing" state.
        @param pMap  The map of list positions/values. If 0, a direct mapping
        is used (simply casting between list position and values). If the map
        exists, it *MUST* be terminated by an entry containing the special
        "not found" list position. */
    inline explicit     PosValueMapper( PosT nNFPos, const MapEntryType* pMap = 0 ) :
                            mpMap( pMap ), mnNFPos( nNFPos ) {}

    /** Returns the value at the specified list position.
        @return  The found value, or the value of the last map entry on error. */
    ValueT              GetValueFromPos( PosT nPos ) const;
    /** Returns the list position of the specified value.
        @return  The position, or the special "not found" position on error. */
    PosT                GetPosFromValue( ValueT nValue ) const;

    /** Returns the special "not found" list position. */
    inline PosT         GetNotFoundPos() const { return mnNFPos; }

private:
    const MapEntryType* mpMap;      /// The list position/value map.
    PosT                mnNFPos;    /// Special "not found" list position.
};

// ============================================================================
// Base control wrapper classes
// ============================================================================

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
     |   +- ColorListBoxWrapper   [1]
     |   |
     |   +- NumericFieldWrapper< ValueT >   [1]
     |   |   |
     |   |   +- [ValueType]NumericFieldWrapper   [1] [2]
     |   |
     |   +- MetricFieldWrapper< ValueT >   [1]
     |   |   |
     |   |   +- [ValueType]MetricFieldWrapper   [1] [2]
     |   |
     |   +- ListBoxWrapper< ValueT >   [1]
     |   |   |
     |   |   +- [ValueType]ListBoxWrapper   [1] [2]
     |   |
     |   +- ValueSetWrapper< ValueT >   [1]
     |       |
     |       +- [ValueType]ValueSetWrapper   [1] [2]
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
    inline explicit     ControlWrapperBase() {}
    virtual             ~ControlWrapperBase();

    /** Derived classes enable, disable, show, or hide control(s).
        @descr  Will do nothing, if the corresponding parameter is STATE_DONTKNOW. */
    virtual void        ModifyControl( TriState eEnable, TriState eShow ) = 0;

    /** Derived classes return true if the control is in "don't know" state. */
    virtual bool        IsControlDontKnow() const = 0;
    /** Derived classes set the control to "don't know" state. */
    virtual void        SetControlDontKnow( bool bSet ) = 0;

private:
    /* Disable copy c'tor and assignment. */
                        ControlWrapperBase( const ControlWrapperBase& );
    ControlWrapperBase& operator=( const ControlWrapperBase& );
};

// ============================================================================
// Single control wrappers
// ============================================================================

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

    inline explicit     SingleControlWrapper( ControlT& rControl ) : mrControl( rControl ) {}

    /** Returns a reference to the control this connection works on. */
    inline const ControlT& GetControl() const { return mrControl; }
    /** Returns a reference to the control this connection works on. */
    inline ControlT&    GetControl() { return mrControl; }

    /** Enables, disables, shows, or hides the control.
        @descr  Does nothing, if the corresponding parameter is STATE_DONTKNOW. */
    virtual void        ModifyControl( TriState eEnable, TriState eShow );

    /** Derived classes return the value the control contains. */
    virtual ValueT      GetControlValue() const = 0;
    /** Derived classes set the contents of the control to the passed value. */
    virtual void        SetControlValue( ValueT aValue ) = 0;

private:
    ControlT&           mrControl;  /// The control of this wrapper.
};

// ============================================================================

/** A dummy wrapper for a VCL Window that does nothing special.

    This wrapper is used to implement the DummyItemConnection. It does not
    connect an item to a control, but handles the special flags to disable or
    hide a control, if an item is unknown.
 */
class SFX2_DLLPUBLIC DummyWindowWrapper:
    public SingleControlWrapper< Window, void* >
{
public:
    explicit            DummyWindowWrapper( Window& rWindow );

    virtual bool        IsControlDontKnow() const;
    virtual void        SetControlDontKnow( bool );

    virtual void*       GetControlValue() const;
    virtual void        SetControlValue( void* );
};

// ----------------------------------------------------------------------------

/** A wrapper for the VCL CheckBox. */
class SFX2_DLLPUBLIC CheckBoxWrapper:
    public SingleControlWrapper< CheckBox, sal_Bool >
{
public:
    explicit            CheckBoxWrapper( CheckBox& rCheckBox );

    virtual bool        IsControlDontKnow() const;
    virtual void        SetControlDontKnow( bool bSet );

    virtual sal_Bool        GetControlValue() const;
    virtual void        SetControlValue( sal_Bool bValue );
};

// ----------------------------------------------------------------------------

/** A wrapper for the SVTOOLS ColorListBox. */
class SFX2_DLLPUBLIC ColorListBoxWrapper:
    public SingleControlWrapper< ColorListBox, Color >
{
    /*  Note: cannot use 'const Color&' as template argument, because the
        SVTOOLS ColorListBox returns the color by value and not by reference,
        therefore GetControlValue() must return a temporary object too. */
public:
    explicit ColorListBoxWrapper(ColorListBox & rListBox);

    virtual ~ColorListBoxWrapper();

    virtual bool        IsControlDontKnow() const;
    virtual void        SetControlDontKnow( bool bSet );

    virtual Color       GetControlValue() const;
    virtual void        SetControlValue( Color aColor );
};

// ============================================================================

/** A wrapper for the VCL NumericField. */
template< typename ValueT >
class NumericFieldWrapper : public SingleControlWrapper< NumericField, ValueT >
{
public:
    inline explicit     NumericFieldWrapper( NumericField& rField ) :
                            SingleControlWrapper< NumericField, ValueT >( rField ) {}

    virtual bool        IsControlDontKnow() const;
    virtual void        SetControlDontKnow( bool bSet );

    virtual ValueT      GetControlValue() const;
    virtual void        SetControlValue( ValueT nValue );
};

// ----------------------------------------------------------------------------

typedef NumericFieldWrapper< sal_Int16 >  Int16NumericFieldWrapper;
typedef NumericFieldWrapper< sal_uInt16 > UInt16NumericFieldWrapper;
typedef NumericFieldWrapper< sal_Int32 >  Int32NumericFieldWrapper;
typedef NumericFieldWrapper< sal_uInt32 > UInt32NumericFieldWrapper;

typedef NumericFieldWrapper< sal_uInt16 > UShortNumericFieldWrapper;
typedef NumericFieldWrapper< sal_uIntPtr >  ULongNumericFieldWrapper;

// ============================================================================

/** A wrapper for the VCL MetricField.

    Adds support for field units during accessing the control value. The
    wrapper respects the field unit set at the control itself and converts it
    from/to the field unit passed to the constructor.
 */
template< typename ValueT >
class MetricFieldWrapper : public SingleControlWrapper< MetricField, ValueT >
{
public:
    inline explicit     MetricFieldWrapper( MetricField& rField, FieldUnit eUnit = FUNIT_NONE ) :
                            SingleControlWrapper< MetricField, ValueT >( rField ), meUnit( eUnit ) {}

    virtual bool        IsControlDontKnow() const;
    virtual void        SetControlDontKnow( bool bSet );

    virtual ValueT      GetControlValue() const;
    virtual void        SetControlValue( ValueT nValue );

private:
    FieldUnit           meUnit;
};

// ----------------------------------------------------------------------------

typedef MetricFieldWrapper< sal_Int16 >  Int16MetricFieldWrapper;
typedef MetricFieldWrapper< sal_uInt16 > UInt16MetricFieldWrapper;
typedef MetricFieldWrapper< sal_Int32 >  Int32MetricFieldWrapper;
typedef MetricFieldWrapper< sal_uInt32 > UInt32MetricFieldWrapper;

typedef MetricFieldWrapper< sal_uInt16 > UShortMetricFieldWrapper;
typedef MetricFieldWrapper< sal_uIntPtr >  ULongMetricFieldWrapper;

// ============================================================================

/** A wrapper for the VCL ListBox.

    If a position<->value map is passed to the constructor, it MUST be
    terminated with an entry containing LISTBOX_ENTRY_NOTFOUND as list
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
    inline explicit     ListBoxWrapper( ListBox& rListBox, const MapEntryType* pMap = 0 ) :
                            SingleControlWrapper< ListBox, ValueT >( rListBox ), MapperType( LISTBOX_ENTRY_NOTFOUND, pMap ) {}

    virtual bool        IsControlDontKnow() const
                            { return this->GetControl().GetSelectEntryCount() == 0; }
    virtual void        SetControlDontKnow( bool bSet )
                            { if( bSet ) this->GetControl().SetNoSelection(); }

    virtual ValueT      GetControlValue() const;
    virtual void        SetControlValue( ValueT nValue );
};

// ----------------------------------------------------------------------------

typedef ListBoxWrapper< sal_Int16 >  Int16ListBoxWrapper;
typedef ListBoxWrapper< sal_uInt16 > UInt16ListBoxWrapper;
typedef ListBoxWrapper< sal_Int32 >  Int32ListBoxWrapper;
typedef ListBoxWrapper< sal_uInt32 > UInt32ListBoxWrapper;

typedef ListBoxWrapper< sal_uInt16 > UShortListBoxWrapper;
typedef ListBoxWrapper< sal_uIntPtr >  ULongListBoxWrapper;

// ============================================================================

/** A wrapper for the SVTOOLS ValueSet.

    If a position<->value map is passed to the constructor, it MUST be
    terminated with an entry containing VALUESET_ITEM_NOTFOUND as list
    position. See documentation of the PosValueMapper template for details.
 */
template< typename ValueT >
class ValueSetWrapper :
        public SingleControlWrapper< ValueSet, ValueT >,
        public PosValueMapper< ValueSetPosType, ValueT >
{
    typedef PosValueMapper< ValueSetPosType, ValueT > MapperType;

public:
    typedef typename MapperType::MapEntryType MapEntryType;

    /** @param pMap  Optional position <-> value map.
        See PosValueMapper documentation for details. */
    inline explicit     ValueSetWrapper( ValueSet& rValueSet, const MapEntryType* pMap = 0 ) :
                            SingleControlWrapper< ValueSet, ValueT >( rValueSet ), MapperType( VALUESET_ITEM_NOTFOUND, pMap ) {}

    virtual bool        IsControlDontKnow() const
                            { return this->GetControl().IsNoSelection(); }
    virtual void        SetControlDontKnow( bool bSet )
                            { if( bSet ) this->GetControl().SetNoSelection(); }

    virtual ValueT      GetControlValue() const;
    virtual void        SetControlValue( ValueT nValue );
};

// ----------------------------------------------------------------------------

typedef ValueSetWrapper< sal_Int16 >  Int16ValueSetWrapper;
typedef ValueSetWrapper< sal_uInt16 > UInt16ValueSetWrapper;
typedef ValueSetWrapper< sal_Int32 >  Int32ValueSetWrapper;
typedef ValueSetWrapper< sal_uInt32 > UInt32ValueSetWrapper;

typedef ValueSetWrapper< sal_uInt16 > UShortValueSetWrapper;
typedef ValueSetWrapper< sal_uIntPtr >  ULongValueSetWrapper;

// ============================================================================
// Multi control wrappers
// ============================================================================

struct MultiControlWrapperHelper_Impl;

/** A container of control wrappers.

    Derived classes should define control wrapper members and register them in
    their constructor, using the function RegisterControlWrapper().

    This wrapper implements the abstract functions of the ControlWrapperBase
    base class by calling the functions of all registered wrappers.
 */
class SFX2_DLLPUBLIC MultiControlWrapperHelper : public ControlWrapperBase
{
public:
    explicit            MultiControlWrapperHelper();
    virtual             ~MultiControlWrapperHelper();

    /** Registers a control wrapper (should be a member of a derived class). */
    void                RegisterControlWrapper( ControlWrapperBase& rWrapper );

    /** Enables, disables, shows, or hides the registered controls. */
    virtual void        ModifyControl( TriState eEnable, TriState eShow );

    /** Returns true if all registered controls are in "don't know" state. */
    virtual bool        IsControlDontKnow() const;
    /** Sets all registered controls to "don't know" state. */
    virtual void        SetControlDontKnow( bool bSet );

private:
    std::auto_ptr< MultiControlWrapperHelper_Impl > mxImpl;
};

// ----------------------------------------------------------------------------

/** A multi control wrapper with extended interface.

    This template class extends the MultiControlWrapperHelper class by the
    functions GetControlValue() and SetControlValue(), known from the
    SingleControlWrapper template. This makes it possible to use this template
    in item connections expecting a single control wrapper. The type ValueT
    should be able to contain the values of all controls handled in this
    wrapper. In most cases, the easiest way to achieve this is to use the
    related item type directly, using the IdentItemWrapper template
    (itemwrapper.hxx).
 */
template< typename ValueT >
class MultiControlWrapper : public MultiControlWrapperHelper
{
public:
    typedef MultiControlWrapperHelper       ControlType;
    typedef ValueT                          ControlValueType;
    typedef MultiControlWrapper< ValueT >   MultiControlWrapperType;

    MultiControlWrapper() : maDefValue( 0 ){}

    /** Returns the default value that can be used in GetControlValue(). */
    inline const ValueT& GetDefaultValue() const { return maDefValue; }
    /** Sets a default value that can be used in GetControlValue(). */
    inline void         SetDefaultValue( const ValueT& rDefValue ) { maDefValue = rDefValue; }

    /** Derived classes return the value the control contains. */
    virtual ValueT      GetControlValue() const = 0;
    /** Derived classes set the contents of the control to the passed value. */
    virtual void        SetControlValue( ValueT aValue ) = 0;

private:
    ValueT              maDefValue;
};

// ============================================================================


// ============================================================================
//               ***  Implementation of template functions  ***
// ============================================================================

// ============================================================================
// Helpers
// ============================================================================

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
    else if( nValue >= 0 )
        nPos = static_cast< PosT >( nValue );
    return nPos;
}

// ============================================================================
// Single control wrappers
// ============================================================================

template< typename ControlT, typename ValueT >
inline void SingleControlWrapper< ControlT, ValueT >::ModifyControl( TriState eEnable, TriState eShow )
{
    if( eEnable != STATE_DONTKNOW )
        mrControl.Enable( eEnable == STATE_CHECK );
    if( eShow != STATE_DONTKNOW )
        mrControl.Show( eShow == STATE_CHECK );
}

// ============================================================================

template< typename ValueT >
bool NumericFieldWrapper< ValueT >::IsControlDontKnow() const
{
    return this->GetControl().GetText().Len() == 0;
}

template< typename ValueT >
void NumericFieldWrapper< ValueT >::SetControlDontKnow( bool bSet )
{
    if( bSet )
        this->GetControl().SetText( OUString() );
}

template< typename ValueT >
ValueT NumericFieldWrapper< ValueT >::GetControlValue() const
{
    return static_cast< ValueT >( this->GetControl().Denormalize( this->GetControl().GetValue() ) );
}

template< typename ValueT >
void NumericFieldWrapper< ValueT >::SetControlValue( ValueT nValue )
{
    this->GetControl().SetValue( this->GetControl().Normalize( static_cast< sal_Int64 >( nValue ) ) );
}

// ============================================================================

template< typename ValueT >
bool MetricFieldWrapper< ValueT >::IsControlDontKnow() const
{
    return this->GetControl().GetText().isEmpty();
}

template< typename ValueT >
void MetricFieldWrapper< ValueT >::SetControlDontKnow( bool bSet )
{
    if( bSet )
        this->GetControl().SetText( OUString() );
}

template< typename ValueT >
ValueT MetricFieldWrapper< ValueT >::GetControlValue() const
{
    return static_cast< ValueT >( this->GetControl().Denormalize( this->GetControl().GetValue( meUnit ) ) );
}

template< typename ValueT >
void MetricFieldWrapper< ValueT >::SetControlValue( ValueT nValue )
{
    this->GetControl().SetValue( this->GetControl().Normalize( static_cast< sal_Int64 >( nValue ) ), meUnit );
}

// ============================================================================

template< typename ValueT >
ValueT ListBoxWrapper< ValueT >::GetControlValue() const
{
    return this->GetValueFromPos( this->GetControl().GetSelectEntryPos() );
}

template< typename ValueT >
void ListBoxWrapper< ValueT >::SetControlValue( ValueT nValue )
{
    sal_uInt16 nPos = this->GetPosFromValue( nValue );
    if( nPos != this->GetNotFoundPos() )
        this->GetControl().SelectEntryPos( nPos );
}

// ----------------------------------------------------------------------------

template< typename ValueT >
ValueT ValueSetWrapper< ValueT >::GetControlValue() const
{
    return this->GetValueFromPos( this->GetControl().GetSelectItemId() );
}

template< typename ValueT >
void ValueSetWrapper< ValueT >::SetControlValue( ValueT nValue )
{
    sal_uInt16 nPos = this->GetPosFromValue( nValue );
    if( nPos != this->GetNotFoundPos() )
        this->GetControl().SelectItem( nPos );
}

// ============================================================================


} // namespace sfx

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
