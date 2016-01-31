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

#ifndef INCLUDED_VCL_SALNATIVEWIDGETS_HXX
#define INCLUDED_VCL_SALNATIVEWIDGETS_HXX

#include <rtl/ustring.hxx>
#include <vcl/dllapi.h>
#include <tools/gen.hxx>
#include <o3tl/typed_flags_set.hxx>

#include <boost/functional/hash.hpp>

class ControlCacheKey
{
public:
    ControlType mnType;
    ControlPart mnPart;
    ControlState mnState;
    Size maSize;

    ControlCacheKey(ControlType nType, ControlPart nPart, ControlState nState, const Size& rSize)
        : mnType(nType)
        , mnPart(nPart)
        , mnState(nState)
        , maSize(rSize)
    {}

    bool operator==(ControlCacheKey const& aOther) const
    {
        return mnType == aOther.mnType
            && mnPart == aOther.mnPart
            && mnState == aOther.mnState
            && maSize.Width() == aOther.maSize.Width()
            && maSize.Height() == aOther.maSize.Height();
    }

    bool canCacheControl()
    {
        switch(mnType)
        {
            case CTRL_CHECKBOX:
            case CTRL_RADIOBUTTON:
            case CTRL_LISTNODE:
            case CTRL_SLIDER:
                return false;

            case CTRL_MENUBAR:
                if (mnPart == PART_ENTIRE_CONTROL)
                    return false;
                break;
                                                                                                                              296,1         49%
            default:
                break;
        }
        return true;
    }
};


struct ControlCacheHashFunction
{
    std::size_t operator()(ControlCacheKey const& aCache) const
    {
        std::size_t seed = 0;
        boost::hash_combine(seed, aCache.mnType);
        boost::hash_combine(seed, aCache.mnPart);
        boost::hash_combine(seed, aCache.mnState);
        boost::hash_combine(seed, aCache.maSize.Width());
        boost::hash_combine(seed, aCache.maSize.Height());
        return seed;
    }
};
enum ButtonValue {
    BUTTONVALUE_DONTKNOW,
    BUTTONVALUE_ON,
    BUTTONVALUE_OFF,
    BUTTONVALUE_MIXED
};

class VCL_DLLPUBLIC ImplControlValue
{
    friend class SalFrame;

    private:
        ControlType     mType;
        ButtonValue     mTristate;    // Tristate value: on, off, mixed
        long            mNumber;      // numeric value
    protected:
        ImplControlValue( ControlType i_eType, ButtonValue i_eTriState, long i_nNumber )
        : mType( i_eType )
        , mTristate( i_eTriState )
        , mNumber( i_nNumber )
        {}

    public:
        explicit ImplControlValue(ButtonValue nTristate)
            : mType( CTRL_GENERIC ), mTristate(nTristate), mNumber(0) {}
        explicit ImplControlValue( long nNumeric )
            : mType( CTRL_GENERIC ), mTristate(BUTTONVALUE_DONTKNOW), mNumber( nNumeric) {}
        inline ImplControlValue()
            : mType( CTRL_GENERIC ), mTristate(BUTTONVALUE_DONTKNOW), mNumber(0) {}

        virtual ~ImplControlValue();

        virtual ImplControlValue* clone() const;

        ControlType getType() const { return mType; }

        inline ButtonValue      getTristateVal() const { return mTristate; }
        inline void         setTristateVal( ButtonValue nTristate ) { mTristate = nTristate; }

        inline long         getNumericVal() const { return mNumber; }
        inline void         setNumericVal( long nNumeric ) { mNumber = nNumeric; }
};

class VCL_DLLPUBLIC ScrollbarValue : public ImplControlValue
{
    public:
        long            mnMin;
        long            mnMax;
        long            mnCur;
        long            mnVisibleSize;
        Rectangle       maThumbRect;
        Rectangle       maButton1Rect;
        Rectangle       maButton2Rect;
        ControlState    mnButton1State;
        ControlState    mnButton2State;
        ControlState    mnThumbState;
        ControlState    mnPage1State;
        ControlState    mnPage2State;

        inline ScrollbarValue()
        : ImplControlValue( CTRL_SCROLLBAR, BUTTONVALUE_DONTKNOW, 0 )
        {
            mnMin = 0; mnMax = 0; mnCur = 0; mnVisibleSize = 0;
            mnButton1State = ControlState::NONE; mnButton2State = ControlState::NONE;
            mnThumbState = ControlState::NONE; mnPage1State = ControlState::NONE; mnPage2State = ControlState::NONE;
        };
        virtual ~ScrollbarValue();
        virtual ScrollbarValue* clone() const override;
};

class VCL_DLLPUBLIC SliderValue : public ImplControlValue
{
    public:
        long            mnMin;
        long            mnMax;
        long            mnCur;
        Rectangle       maThumbRect;
        ControlState    mnThumbState;

        SliderValue()
        : ImplControlValue( CTRL_SLIDER, BUTTONVALUE_DONTKNOW, 0 )
        , mnMin( 0 ), mnMax( 0 ), mnCur( 0 ), mnThumbState( ControlState::NONE )
        {}
        virtual ~SliderValue();
        virtual SliderValue* clone() const override;
};

enum class TabitemFlags
{
    NONE           = 0x00,
    LeftAligned    = 0x01,   // the tabitem is aligned with the left  border of the TabControl
    RightAligned   = 0x02,   // the tabitem is aligned with the right border of the TabControl
    FirstInGroup   = 0x04,   // the tabitem is the first in group of tabitems
    LastInGroup    = 0x08,   // the tabitem is the last in group of tabitems
};

namespace o3tl
{
    template<> struct typed_flags<TabitemFlags> : is_typed_flags<TabitemFlags, 0x0f> {};
}

class VCL_DLLPUBLIC SpinbuttonValue : public ImplControlValue
{
    public:
        Rectangle       maUpperRect;
        Rectangle       maLowerRect;
        ControlState    mnUpperState;
        ControlState    mnLowerState;
        int         mnUpperPart;
        int         mnLowerPart;

        SpinbuttonValue()
            : ImplControlValue( CTRL_SPINBUTTONS, BUTTONVALUE_DONTKNOW, 0 )
            , mnUpperState(ControlState::NONE)
            , mnLowerState(ControlState::NONE)
            , mnUpperPart(0)
            , mnLowerPart(0)
        {
        }

        virtual ~SpinbuttonValue();
        virtual SpinbuttonValue* clone() const override;
};

class VCL_DLLPUBLIC ToolbarValue : public ImplControlValue
{
public:
    ToolbarValue() : ImplControlValue( CTRL_TOOLBAR, BUTTONVALUE_DONTKNOW, 0 )
    { mbIsTopDockingArea = false; }
    virtual ~ToolbarValue();
    virtual ToolbarValue* clone() const override;
    Rectangle           maGripRect;
    bool                mbIsTopDockingArea; // indicates that this is the top aligned dockingarea
                                            // adjacent to the menubar
};

class VCL_DLLPUBLIC MenubarValue : public ImplControlValue
{
public:
    MenubarValue() : ImplControlValue( CTRL_MENUBAR, BUTTONVALUE_DONTKNOW, 0 )
    { maTopDockingAreaHeight=0; }
    virtual ~MenubarValue();
    virtual MenubarValue* clone() const override;
    int             maTopDockingAreaHeight;
};

class VCL_DLLPUBLIC MenupopupValue : public ImplControlValue
{
public:
    MenupopupValue() : ImplControlValue( CTRL_MENU_POPUP, BUTTONVALUE_DONTKNOW, 0 )
    {}
    MenupopupValue( long i_nGutterWidth, const Rectangle& i_rItemRect )
    : ImplControlValue( CTRL_MENU_POPUP, BUTTONVALUE_DONTKNOW, i_nGutterWidth )
    , maItemRect( i_rItemRect )
    {}
    virtual ~MenupopupValue();
    virtual MenupopupValue* clone() const override;
    Rectangle       maItemRect;
};

class VCL_DLLPUBLIC PushButtonValue : public ImplControlValue
{
public:
    PushButtonValue()
    : ImplControlValue( CTRL_PUSHBUTTON, BUTTONVALUE_DONTKNOW, 0 )
    , mbBevelButton( false ), mbSingleLine( true ) {}
    virtual ~PushButtonValue();
    virtual PushButtonValue* clone() const override;

    bool            mbBevelButton:1;
    bool            mbSingleLine:1;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

