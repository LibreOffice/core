/*************************************************************************
 *
 *  $RCSfile: borderconn.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 17:42:37 $
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
 *  WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SVX_BORDERCONN_HXX
#include "borderconn.hxx"
#endif

#ifndef SVX_FRMSEL_HXX
#include "frmsel.hxx"
#endif

#define ITEMID_LINE     0
#define ITEMID_BOX      0
#define ITEMID_BOXINFO  0
#define ITEMID_MARGIN   SID_ATTR_ALIGN_MARGIN
#define ITEMID_SHADOW   SID_ATTR_BORDER_SHADOW

#ifndef _SVX_BOLNITEM_HXX
#include "bolnitem.hxx"
#endif
#ifndef _SVX_BOXITEM_HXX
#include "boxitem.hxx"
#endif
#ifndef _SVX_ALGITEM_HXX
#include "algitem.hxx"
#endif
#ifndef _SVX_SHADITEM_HXX
#include "shaditem.hxx"
#endif

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

class LineItemWrapper : public sfx::SingleItemWrapper< SvxLineItem, const SvxBorderLine* >
{
public:
    inline explicit     LineItemWrapper( USHORT nSlot ) : SingleItemWrapperType( nSlot ) {}

    virtual const SvxBorderLine* GetItemValue( const SvxLineItem& rItem ) const
                            { return rItem.GetLine(); }
    virtual void        SetItemValue( SvxLineItem& rItem, const SvxBorderLine* pLine ) const
                            { rItem.SetLine( pLine ); }
};

// 2nd: control wrappers ------------------------------------------------------

class FrameSelectorWrapper : public sfx::SingleControlWrapper< FrameSelector, const SvxBorderLine* >
{
public:
    inline explicit     FrameSelectorWrapper( FrameSelector& rFrameSel, FrameBorderType eBorder ) :
                            SingleControlWrapperType( rFrameSel ), meBorder( eBorder ) {}

    virtual bool        IsControlDontKnow() const;
    virtual void        SetControlDontKnow( bool bSet );

    virtual const SvxBorderLine* GetControlValue() const;
    virtual void        SetControlValue( const SvxBorderLine* pLine );

private:
    FrameBorderType       meBorder;         /// The line this wrapper works with.
};

bool FrameSelectorWrapper::IsControlDontKnow() const
{
    return GetControl().GetBorderState( meBorder ) == FRAMESTATE_DONTCARE;
}

void FrameSelectorWrapper::SetControlDontKnow( bool bSet )
{
    if( bSet )
        GetControl().SetBorderDontCare( meBorder );
}

const SvxBorderLine* FrameSelectorWrapper::GetControlValue() const
{
    return GetControl().GetBorderStyle( meBorder );
}

void FrameSelectorWrapper::SetControlValue( const SvxBorderLine* pLine )
{
    GetControl().ShowBorder( meBorder, pLine );
}

// 3rd: connection ------------------------------------------------------------

typedef sfx::ItemControlConnection< LineItemWrapper, FrameSelectorWrapper > FrameLineConnection;

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

    virtual SvxMarginItem GetControlValue() const;
    virtual void        SetControlValue( SvxMarginItem aItem );

private:
    sfx::Int16MetricFieldWrapper maLeftWrp;
    sfx::Int16MetricFieldWrapper maRightWrp;
    sfx::Int16MetricFieldWrapper maTopWrp;
    sfx::Int16MetricFieldWrapper maBottomWrp;
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
                            MetricField& rMfTop, MetricField& rMfBottom,
                            sfx::ItemConnFlags nFlags = sfx::ITEMCONN_DEFAULT );
};

MarginConnection::MarginConnection( const SfxItemSet& rItemSet,
        MetricField& rMfLeft, MetricField& rMfRight, MetricField& rMfTop, MetricField& rMfBottom,
        sfx::ItemConnFlags nFlags ) :
    ItemControlConnectionType( ITEMID_MARGIN, new MarginControlsWrapper( rMfLeft, rMfRight, rMfTop, rMfBottom ), nFlags )
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
    { 1,                        SVX_SHADOW_NONE         },
    { 2,                        SVX_SHADOW_BOTTOMRIGHT  },
    { 3,                        SVX_SHADOW_TOPRIGHT     },
    { 4,                        SVX_SHADOW_BOTTOMLEFT   },
    { 5,                        SVX_SHADOW_TOPLEFT      },
    { VALUESET_ITEM_NOTFOUND,   SVX_SHADOW_NONE         }
};

class ShadowControlsWrapper : public sfx::MultiControlWrapper< SvxShadowItem >
{
public:
    explicit            ShadowControlsWrapper( ValueSet& rVsPos, MetricField& rMfSize, ColorListBox& rLbColor );

    virtual SvxShadowItem GetControlValue() const;
    virtual void        SetControlValue( SvxShadowItem aItem );

private:
    ShadowPosWrapper                maPosWrp;
    sfx::UShortMetricFieldWrapper   maSizeWrp;
    sfx::ColorListBoxWrapper        maColorWrp;
};

ShadowControlsWrapper::ShadowControlsWrapper(
        ValueSet& rVsPos, MetricField& rMfSize, ColorListBox& rLbColor ) :
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
    if( !maSizeWrp.IsControlDontKnow() )
        aItem.SetWidth( maSizeWrp.GetControlValue() );
    if( !maColorWrp.IsControlDontKnow() )
        aItem.SetColor( maColorWrp.GetControlValue() );
    return aItem;
}

void ShadowControlsWrapper::SetControlValue( SvxShadowItem aItem )
{
    maPosWrp.SetControlValue( aItem.GetLocation() );
    maSizeWrp.SetControlValue( aItem.GetWidth() );
    maColorWrp.SetControlValue( aItem.GetColor() );
}

// 3rd: connection ------------------------------------------------------------

class ShadowConnection : public sfx::ItemControlConnection< ShadowItemWrapper, ShadowControlsWrapper >
{
public:
    explicit            ShadowConnection( const SfxItemSet& rItemSet,
                                ValueSet& rVsPos, MetricField& rMfSize, ColorListBox& rLbColor,
                                sfx::ItemConnFlags nFlags = sfx::ITEMCONN_DEFAULT );
};

ShadowConnection::ShadowConnection( const SfxItemSet& rItemSet,
        ValueSet& rVsPos, MetricField& rMfSize, ColorListBox& rLbColor, sfx::ItemConnFlags nFlags ) :
    ItemControlConnectionType( ITEMID_SHADOW, new ShadowControlsWrapper( rVsPos, rMfSize, rLbColor ), nFlags )
{
    mxCtrlWrp->SetDefaultValue( maItemWrp.GetDefaultItem( rItemSet ) );
}

// ============================================================================
// ============================================================================

sfx::ItemConnectionBase* CreateFrameLineConnection( USHORT nSlot,
        FrameSelector& rFrameSel, FrameBorderType eBorder, sfx::ItemConnFlags nFlags )
{
    return new FrameLineConnection( nSlot, new FrameSelectorWrapper( rFrameSel, eBorder ), nFlags );
}

sfx::ItemConnectionBase* CreateFrameBoxConnection( USHORT nBoxSlot, USHORT nBoxInfoSlot,
        FrameSelector& rFrameSel, FrameBorderType eBorder, sfx::ItemConnFlags nFlags )
{
    DBG_ERRORFILE( "svx::CreateFrameBoxConnection - not implemented" );
    return 0;
}

sfx::ItemConnectionBase* CreateMarginConnection( const SfxItemSet& rItemSet,
        MetricField& rMfLeft, MetricField& rMfRight,
        MetricField& rMfTop, MetricField& rMfBottom,
        sfx::ItemConnFlags nFlags )
{
    return new MarginConnection( rItemSet, rMfLeft, rMfRight, rMfTop, rMfBottom, nFlags );
}

sfx::ItemConnectionBase* CreateShadowConnection( const SfxItemSet& rItemSet,
        ValueSet& rVsPos, MetricField& rMfSize, ColorListBox& rLbColor,
        sfx::ItemConnFlags nFlags )
{
    return new ShadowConnection( rItemSet, rVsPos, rMfSize, rLbColor, nFlags );
}

// ============================================================================

} // namespace svx

