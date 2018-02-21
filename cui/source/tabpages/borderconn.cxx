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

#include "borderconn.hxx"
#include <svx/frmsel.hxx>
#include <svx/svxids.hrc>
#include <editeng/lineitem.hxx>
#include <editeng/boxitem.hxx>
#include <svx/algitem.hxx>
#include <svx/colorbox.hxx>
#include <editeng/shaditem.hxx>

namespace svx {

/* ============================================================================
SvxLineItem connection
----------------------
Connects an SvxLineItem (that contains the style of one line of a cell border)
with one frame border from a svx::FrameSelector control. If this connection is
used, no additional code is needed in the Reset() and FillItemSet() functions
of the tab page.
============================================================================ */

// 1st: item wrappers ---------------------------------------------------------

class LineItemWrapper : public sfx::SingleItemWrapper< SvxLineItem, const editeng::SvxBorderLine* >
{
public:
    explicit     LineItemWrapper( sal_uInt16 nSlot ) : SingleItemWrapperType( nSlot ) {}

    virtual const editeng::SvxBorderLine* GetItemValue( const SvxLineItem& rItem ) const override
                            { return rItem.GetLine(); }
    virtual void        SetItemValue( SvxLineItem& rItem, const editeng::SvxBorderLine* pLine ) const override
                            { rItem.SetLine( pLine ); }
};

// 2nd: control wrappers ------------------------------------------------------

class FrameSelectorWrapper : public sfx::SingleControlWrapper< FrameSelector, const editeng::SvxBorderLine* >
{
public:
    explicit     FrameSelectorWrapper( FrameSelector& rFrameSel, FrameBorderType eBorder ) :
                            SingleControlWrapperType( rFrameSel ), meBorder( eBorder ) {}

    virtual bool        IsControlDontKnow() const override;
    virtual void        SetControlDontKnow( bool bSet ) override;

    virtual const editeng::SvxBorderLine* GetControlValue() const override;
    virtual void        SetControlValue( const editeng::SvxBorderLine* pLine ) override;

private:
    FrameBorderType       meBorder;         /// The line this wrapper works with.
};

bool FrameSelectorWrapper::IsControlDontKnow() const
{
    return GetControl().GetFrameBorderState( meBorder ) == FrameBorderState::DontCare;
}

void FrameSelectorWrapper::SetControlDontKnow( bool bSet )
{
    if( bSet )
        GetControl().SetBorderDontCare( meBorder );
}

const editeng::SvxBorderLine* FrameSelectorWrapper::GetControlValue() const
{
    return GetControl().GetFrameBorderStyle( meBorder );
}

void FrameSelectorWrapper::SetControlValue( const editeng::SvxBorderLine* pLine )
{
    GetControl().ShowBorder( meBorder, pLine );
}

// 3rd: connection ------------------------------------------------------------

/* ============================================================================
SvxMarginItem connection
------------------------
Connects an SvxMarginItem (that contains the inner margin of all cell borders)
with the numerical edit controls of the SvxBorderTabPage. If this connection is
used, no additional code is needed in the Reset() and FillItemSet() functions
of the tab page.
============================================================================ */

// 1st: item wrappers ---------------------------------------------------------

typedef sfx::IdentItemWrapper< SvxMarginItem > MarginItemWrapper;

// 2nd: control wrappers ------------------------------------------------------

class MarginControlsWrapper : public sfx::MultiControlWrapper< SvxMarginItem >
{
public:
    explicit            MarginControlsWrapper(
                            MetricField& rMfLeft, MetricField& rMfRight,
                            MetricField& rMfTop, MetricField& rMfBottom );

    virtual SvxMarginItem GetControlValue() const override;
    virtual void        SetControlValue( SvxMarginItem aItem ) override;

private:
    sfx::MetricFieldWrapper<sal_Int16> maLeftWrp;
    sfx::MetricFieldWrapper<sal_Int16> maRightWrp;
    sfx::MetricFieldWrapper<sal_Int16> maTopWrp;
    sfx::MetricFieldWrapper<sal_Int16> maBottomWrp;
};

MarginControlsWrapper::MarginControlsWrapper(
        MetricField& rMfLeft, MetricField& rMfRight, MetricField& rMfTop, MetricField& rMfBottom ) :
    maLeftWrp( rMfLeft, FUNIT_TWIP ),
    maRightWrp( rMfRight, FUNIT_TWIP ),
    maTopWrp( rMfTop, FUNIT_TWIP ),
    maBottomWrp( rMfBottom, FUNIT_TWIP )
{
    RegisterControlWrapper( maLeftWrp );
    RegisterControlWrapper( maRightWrp );
    RegisterControlWrapper( maTopWrp );
    RegisterControlWrapper( maBottomWrp );
}

SvxMarginItem MarginControlsWrapper::GetControlValue() const
{
    SvxMarginItem aItem( GetDefaultValue() );
    if( !maLeftWrp.IsControlDontKnow() )
        aItem.SetLeftMargin( maLeftWrp.GetControlValue() );
    if( !maRightWrp.IsControlDontKnow() )
        aItem.SetRightMargin( maRightWrp.GetControlValue() );
    if( !maTopWrp.IsControlDontKnow() )
        aItem.SetTopMargin( maTopWrp.GetControlValue() );
    if( !maBottomWrp.IsControlDontKnow() )
        aItem.SetBottomMargin( maBottomWrp.GetControlValue() );
    return aItem;
}

void MarginControlsWrapper::SetControlValue( SvxMarginItem aItem )
{
    maLeftWrp.SetControlValue( aItem.GetLeftMargin() );
    maRightWrp.SetControlValue( aItem.GetRightMargin() );
    maTopWrp.SetControlValue( aItem.GetTopMargin() );
    maBottomWrp.SetControlValue( aItem.GetBottomMargin() );
}

// 3rd: connection ------------------------------------------------------------

class MarginConnection : public sfx::ItemControlConnection< MarginItemWrapper, MarginControlsWrapper >
{
public:
    explicit            MarginConnection( const SfxItemSet& rItemSet,
                            MetricField& rMfLeft, MetricField& rMfRight,
                            MetricField& rMfTop, MetricField& rMfBottom );
};

MarginConnection::MarginConnection( const SfxItemSet& rItemSet,
        MetricField& rMfLeft, MetricField& rMfRight, MetricField& rMfTop, MetricField& rMfBottom ) :
    ItemControlConnectionType( SID_ATTR_ALIGN_MARGIN, new MarginControlsWrapper( rMfLeft, rMfRight, rMfTop, rMfBottom ), ItemConnFlags::NONE )
{
    mxCtrlWrp->SetDefaultValue( maItemWrp.GetDefaultItem( rItemSet ) );
}

/* ============================================================================
SvxShadowItem connection
------------------------
Connects an SvxShadowItem (that contains shadow position, size, and color) with
the controls of the SvxBorderTabPage. If this connection is used, no additional
code is needed in the Reset() and FillItemSet() functions of the tab page.
============================================================================ */

// 1st: item wrappers ---------------------------------------------------------

typedef sfx::IdentItemWrapper< SvxShadowItem > ShadowItemWrapper;

// 2nd: control wrappers ------------------------------------------------------

typedef sfx::ValueSetWrapper< SvxShadowLocation > ShadowPosWrapper;
static const ShadowPosWrapper::MapEntryType s_pShadowPosMap[] =
{
    { 1,                        SvxShadowLocation::NONE         },
    { 2,                        SvxShadowLocation::BottomRight  },
    { 3,                        SvxShadowLocation::TopRight     },
    { 4,                        SvxShadowLocation::BottomLeft   },
    { 5,                        SvxShadowLocation::TopLeft      },
    { WRAPPER_VALUESET_ITEM_NOTFOUND, SvxShadowLocation::NONE   }
};

class ShadowControlsWrapper : public sfx::MultiControlWrapper< SvxShadowItem >
{
public:
    explicit            ShadowControlsWrapper( ValueSet& rVsPos, MetricField& rMfSize, SvxColorListBox& rLbColor );

    virtual SvxShadowItem GetControlValue() const override;
    virtual void        SetControlValue( SvxShadowItem aItem ) override;

private:
    ShadowPosWrapper                    maPosWrp;
    sfx::MetricFieldWrapper<sal_uInt16> maSizeWrp;
    SvxColorListBoxWrapper              maColorWrp;
};

ShadowControlsWrapper::ShadowControlsWrapper(
        ValueSet& rVsPos, MetricField& rMfSize, SvxColorListBox& rLbColor ) :
    maPosWrp( rVsPos, s_pShadowPosMap ),
    maSizeWrp( rMfSize, FUNIT_TWIP ),
    maColorWrp( rLbColor )
{
    RegisterControlWrapper( maPosWrp );
    RegisterControlWrapper( maSizeWrp );
    RegisterControlWrapper( maColorWrp );
}

SvxShadowItem ShadowControlsWrapper::GetControlValue() const
{
    SvxShadowItem aItem( GetDefaultValue() );
    if( !maPosWrp.IsControlDontKnow() )
        aItem.SetLocation( maPosWrp.GetControlValue() );
    // Default value was saved; so don't change the aItem's width if the control
    // has not changed its value, to avoid round-trip errors (like twip->cm->twip)
    // E.g., initial 100 twip will become 0.18 cm, which will return as 102 twip
    if( !maSizeWrp.IsControlDontKnow() && maSizeWrp.IsControlValueChanged() )
        aItem.SetWidth( maSizeWrp.GetControlValue() );
    if( !maColorWrp.IsControlDontKnow() )
        aItem.SetColor( maColorWrp.GetControlValue() );
    return aItem;
}

void ShadowControlsWrapper::SetControlValue( SvxShadowItem aItem )
{
    SetDefaultValue(aItem);
    maPosWrp.SetControlValue( aItem.GetLocation() );
    maSizeWrp.SetControlValue( aItem.GetWidth() );
    maSizeWrp.GetControl().SaveValue();
    maColorWrp.SetControlValue( aItem.GetColor() );
}

// 3rd: connection ------------------------------------------------------------

class ShadowConnection : public sfx::ItemControlConnection< ShadowItemWrapper, ShadowControlsWrapper >
{
public:
    explicit            ShadowConnection( const SfxItemSet& rItemSet,
                                ValueSet& rVsPos, MetricField& rMfSize, SvxColorListBox& rLbColor );
};

ShadowConnection::ShadowConnection( const SfxItemSet& rItemSet,
        ValueSet& rVsPos, MetricField& rMfSize, SvxColorListBox& rLbColor ) :
    ItemControlConnectionType( SID_ATTR_BORDER_SHADOW, new ShadowControlsWrapper( rVsPos, rMfSize, rLbColor ), ItemConnFlags::NONE )
{
    mxCtrlWrp->SetDefaultValue( maItemWrp.GetDefaultItem( rItemSet ) );
}


sfx::ItemConnectionBase* CreateFrameLineConnection( sal_uInt16 nSlot,
        FrameSelector& rFrameSel, FrameBorderType eBorder )
{
    return new sfx::ItemControlConnection< LineItemWrapper, FrameSelectorWrapper >( nSlot, new FrameSelectorWrapper( rFrameSel, eBorder ), ItemConnFlags::NONE );
}

sfx::ItemConnectionBase* CreateMarginConnection( const SfxItemSet& rItemSet,
        MetricField& rMfLeft, MetricField& rMfRight,
        MetricField& rMfTop, MetricField& rMfBottom )
{
    return new MarginConnection( rItemSet, rMfLeft, rMfRight, rMfTop, rMfBottom );
}

sfx::ItemConnectionBase* CreateShadowConnection( const SfxItemSet& rItemSet,
        ValueSet& rVsPos, MetricField& rMfSize, SvxColorListBox& rLbColor )
{
    return new ShadowConnection( rItemSet, rVsPos, rMfSize, rLbColor );
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
