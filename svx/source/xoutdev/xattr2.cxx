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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

// include ---------------------------------------------------------------


#include <com/sun/star/drawing/LineJoint.hpp>
#include <com/sun/star/uno/Any.hxx>

#include <svx/dialogs.hrc>
#include "svx/xattr.hxx"
#include <svx/xtable.hxx>
#include <svx/dialmgr.hxx>
#include <editeng/itemtype.hxx>
#include <svx/xdef.hxx>

#define GLOBALOVERFLOW

/************************************************************************/

//------------------------------
// class XLineTransparenceItem
//------------------------------
TYPEINIT1_AUTOFACTORY(XLineTransparenceItem, SfxUInt16Item);

/*************************************************************************
|*
|*    XLineTransparenceItem::XLineTransparenceItem(sal_uInt16)
|*
|*    Beschreibung
|*    Ersterstellung    07.11.95 KA
|*    Letzte Aenderung  07.11.95 KA
|*
*************************************************************************/

XLineTransparenceItem::XLineTransparenceItem(sal_uInt16 nLineTransparence) :
    SfxUInt16Item(XATTR_LINETRANSPARENCE, nLineTransparence)
{
}

/*************************************************************************
|*
|*    XLineTransparenceItem::XLineTransparenceItem(SvStream& rIn)
|*
|*    Beschreibung
|*    Ersterstellung    07.11.95 KA
|*    Letzte Aenderung  07.11.95 KA
|*
*************************************************************************/

XLineTransparenceItem::XLineTransparenceItem(SvStream& rIn) :
    SfxUInt16Item(XATTR_LINETRANSPARENCE, rIn)
{
}

/*************************************************************************
|*
|*    XLineTransparenceItem::Clone(SfxItemPool* pPool) const
|*
|*    Beschreibung
|*    Ersterstellung    07.11.95 KA
|*    Letzte Aenderung  07.11.95 KA
|*
*************************************************************************/

SfxPoolItem* XLineTransparenceItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XLineTransparenceItem(*this);
}

/*************************************************************************
|*
|*    SfxPoolItem* XLineTransparenceItem::Create(SvStream& rIn, sal_uInt16 nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    07.11.95 KA
|*    Letzte Aenderung  07.11.95 KA
|*
*************************************************************************/

SfxPoolItem* XLineTransparenceItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XLineTransparenceItem(rIn);
}

//------------------------------------------------------------------------

SfxItemPresentation XLineTransparenceItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&          rText, const IntlWrapper *
)   const
{
    rText.Erase();

    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            return ePres;
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = XubString( ResId( RID_SVXSTR_TRANSPARENCE, DIALOG_MGR() ) );
            rText.AppendAscii(": ");
        case SFX_ITEM_PRESENTATION_NAMELESS:
            rText += XubString( UniString::CreateFromInt32((sal_uInt16) GetValue()) );
            rText += sal_Unicode('%');
            return ePres;
        default:
            return SFX_ITEM_PRESENTATION_NONE;
    }
}

//-----------------------
// class XLineJointItem -
//-----------------------

TYPEINIT1_AUTOFACTORY(XLineJointItem, SfxEnumItem);

// -----------------------------------------------------------------------------

XLineJointItem::XLineJointItem( XLineJoint eLineJoint ) :
    SfxEnumItem(XATTR_LINEJOINT, sal::static_int_cast< sal_uInt16 >(eLineJoint))
{
}

// -----------------------------------------------------------------------------

XLineJointItem::XLineJointItem( SvStream& rIn ) :
    SfxEnumItem( XATTR_LINEJOINT, rIn )
{
}

// -----------------------------------------------------------------------------

sal_uInt16 XLineJointItem::GetVersion( sal_uInt16 /*nFileFormatVersion*/) const
{
    return 1;
}

// -----------------------------------------------------------------------------

SfxPoolItem* XLineJointItem::Create( SvStream& rIn, sal_uInt16 nVer ) const
{
    XLineJointItem* pRet = new XLineJointItem( rIn );

    if(nVer < 1)
        pRet->SetValue(XLINEJOINT_ROUND);

    return pRet;
}

// -----------------------------------------------------------------------------

SfxPoolItem* XLineJointItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XLineJointItem( *this );
}

// -----------------------------------------------------------------------------

SfxItemPresentation XLineJointItem::GetPresentation( SfxItemPresentation ePres, SfxMapUnit /*eCoreUnit*/,
                                                     SfxMapUnit /*ePresUnit*/, XubString& rText, const IntlWrapper*) const
{
    rText.Erase();

    switch( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE: return ePres;

        case SFX_ITEM_PRESENTATION_COMPLETE:
        case SFX_ITEM_PRESENTATION_NAMELESS:
        {
            sal_uInt16 nId = 0;

            switch( GetValue() )
            {
                case( XLINEJOINT_NONE ):
                    nId = RID_SVXSTR_LINEJOINT_NONE;
                break;

                case( XLINEJOINT_MIDDLE ):
                    nId = RID_SVXSTR_LINEJOINT_MIDDLE;
                break;


                case( XLINEJOINT_BEVEL ):
                    nId = RID_SVXSTR_LINEJOINT_BEVEL;
                break;


                case( XLINEJOINT_MITER ):
                    nId = RID_SVXSTR_LINEJOINT_MITER;
                break;


                case( XLINEJOINT_ROUND ):
                    nId = RID_SVXSTR_LINEJOINT_ROUND;
                break;
            }

            if( nId )
                rText = SVX_RESSTR( nId );

            return ePres;
        }
        default:
            return SFX_ITEM_PRESENTATION_NONE;
    }
}

// -----------------------------------------------------------------------------

sal_Bool XLineJointItem::QueryValue( ::com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    ::com::sun::star::drawing::LineJoint eJoint = ::com::sun::star::drawing::LineJoint_NONE;

    switch( GetValue() )
    {
    case XLINEJOINT_NONE:
        break;
    case XLINEJOINT_MIDDLE:
        eJoint = ::com::sun::star::drawing::LineJoint_MIDDLE;
        break;
    case XLINEJOINT_BEVEL:
        eJoint = ::com::sun::star::drawing::LineJoint_BEVEL;
        break;
    case XLINEJOINT_MITER:
        eJoint = ::com::sun::star::drawing::LineJoint_MITER;
        break;
    case XLINEJOINT_ROUND:
        eJoint = ::com::sun::star::drawing::LineJoint_ROUND;
        break;
    default:
        DBG_ERROR( "Unknown LineJoint enum value!" );
    }

    rVal <<= eJoint;
    return sal_True;
}

// -----------------------------------------------------------------------------

sal_Bool XLineJointItem::PutValue( const ::com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    XLineJoint eJoint = XLINEJOINT_NONE;
    ::com::sun::star::drawing::LineJoint eUnoJoint;

    if(!(rVal >>= eUnoJoint))
    {
        // also try an int (for Basic)
        sal_Int32 nLJ = 0;
        if(!(rVal >>= nLJ))
            return sal_False;
        eUnoJoint = (::com::sun::star::drawing::LineJoint)nLJ;
    }

    switch( eUnoJoint )
    {
    case ::com::sun::star::drawing::LineJoint_MIDDLE:
        eJoint = XLINEJOINT_MIDDLE;
        break;
    case ::com::sun::star::drawing::LineJoint_BEVEL:
        eJoint = XLINEJOINT_BEVEL;
        break;
    case ::com::sun::star::drawing::LineJoint_MITER:
        eJoint = XLINEJOINT_MITER;
        break;
    case ::com::sun::star::drawing::LineJoint_ROUND:
        eJoint = XLINEJOINT_ROUND;
        break;
    default:
        break;
    }

    SetValue( sal::static_int_cast< sal_uInt16 >( eJoint ) );

    return sal_True;
}

// -----------------------------------------------------------------------------

sal_uInt16 XLineJointItem::GetValueCount() const
{
    // don't forget to update the api interface also
    return 5;
}

//------------------------------
// class XFillTransparenceItem
//------------------------------
TYPEINIT1_AUTOFACTORY(XFillTransparenceItem, SfxUInt16Item);

/*************************************************************************
|*
|*    XFillTransparenceItem::XFillTransparenceItem(sal_uInt16)
|*
|*    Beschreibung
|*    Ersterstellung    07.11.95 KA
|*    Letzte Aenderung  07.11.95 KA
|*
*************************************************************************/

XFillTransparenceItem::XFillTransparenceItem(sal_uInt16 nFillTransparence) :
    SfxUInt16Item(XATTR_FILLTRANSPARENCE, nFillTransparence)
{
}

/*************************************************************************
|*
|*    XFillTransparenceItem::XFillTransparenceItem(SvStream& rIn)
|*
|*    Beschreibung
|*    Ersterstellung    07.11.95 KA
|*    Letzte Aenderung  07.11.95 KA
|*
*************************************************************************/

XFillTransparenceItem::XFillTransparenceItem(SvStream& rIn) :
    SfxUInt16Item(XATTR_FILLTRANSPARENCE, rIn)
{
}

/*************************************************************************
|*
|*    XFillTransparenceItem::Clone(SfxItemPool* pPool) const
|*
|*    Beschreibung
|*    Ersterstellung    07.11.95 KA
|*    Letzte Aenderung  07.11.95 KA
|*
*************************************************************************/

SfxPoolItem* XFillTransparenceItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFillTransparenceItem(*this);
}

/*************************************************************************
|*
|*    SfxPoolItem* XFillTransparenceItem::Create(SvStream& rIn, sal_uInt16 nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    07.11.95 KA
|*    Letzte Aenderung  07.11.95 KA
|*
*************************************************************************/

SfxPoolItem* XFillTransparenceItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XFillTransparenceItem(rIn);
}

//------------------------------------------------------------------------

SfxItemPresentation XFillTransparenceItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&          rText, const IntlWrapper *
)   const
{
    rText.Erase();

    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            return ePres;
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = XubString( ResId( RID_SVXSTR_TRANSPARENCE, DIALOG_MGR() ) );
            rText.AppendAscii(": ");
        case SFX_ITEM_PRESENTATION_NAMELESS:
            rText += XubString( UniString::CreateFromInt32((sal_uInt16) GetValue() ));
            rText += sal_Unicode('%');
            return ePres;
        default:
            return SFX_ITEM_PRESENTATION_NONE;
    }
}

//------------------------------
// class XFormTextShadowTranspItem
//------------------------------
TYPEINIT1_AUTOFACTORY(XFormTextShadowTranspItem, SfxUInt16Item);

/*************************************************************************
|*
|*    XFormTextShadowTranspItem::XFormTextShadowTranspItem(sal_uInt16)
|*
|*    Beschreibung
|*    Ersterstellung    09.11.95 KA
|*    Letzte Aenderung  09.11.95 KA
|*
*************************************************************************/

XFormTextShadowTranspItem::XFormTextShadowTranspItem(sal_uInt16 nShdwTransparence) :
    SfxUInt16Item(XATTR_FORMTXTSHDWTRANSP, nShdwTransparence)
{
}

/*************************************************************************
|*
|*    XFormTextShadowTranspItem::XFormTextShadowTranspItem(SvStream& rIn)
|*
|*    Beschreibung
|*    Ersterstellung    09.11.95 KA
|*    Letzte Aenderung  09.11.95 KA
|*
*************************************************************************/

XFormTextShadowTranspItem::XFormTextShadowTranspItem(SvStream& rIn) :
    SfxUInt16Item(XATTR_FORMTXTSHDWTRANSP, rIn)
{
}

/*************************************************************************
|*
|*    XFormTextShadowTranspItem::Clone(SfxItemPool* pPool) const
|*
|*    Beschreibung
|*    Ersterstellung    09.11.95 KA
|*    Letzte Aenderung  09.11.95 KA
|*
*************************************************************************/

SfxPoolItem* XFormTextShadowTranspItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFormTextShadowTranspItem(*this);
}

/*************************************************************************
|*
|*    SfxPoolItem* XFormTextShadowTranspItem::Create(SvStream& rIn, sal_uInt16 nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    09.11.95 KA
|*    Letzte Aenderung  09.11.95 KA
|*
*************************************************************************/

SfxPoolItem* XFormTextShadowTranspItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XFormTextShadowTranspItem(rIn);
}


//------------------------------
// class XFillGradientStepCountItem
//------------------------------
TYPEINIT1_AUTOFACTORY(XGradientStepCountItem, SfxUInt16Item);

/*************************************************************************
|*
|*    XGradientStepCountItem::XGradientStepCountItem( sal_uInt16 )
|*
|*    Beschreibung
|*    Ersterstellung    23.01.96 KA
|*    Letzte Aenderung  23.01.96 KA
|*
*************************************************************************/

XGradientStepCountItem::XGradientStepCountItem( sal_uInt16 nStepCount ) :
    SfxUInt16Item( XATTR_GRADIENTSTEPCOUNT, nStepCount )
{
}

/*************************************************************************
|*
|*    XGradientStepCountItem::XGradientStepCountItem( SvStream& rIn )
|*
|*    Beschreibung
|*    Ersterstellung    23.01.96 KA
|*    Letzte Aenderung  23.01.96 KA
|*
*************************************************************************/

XGradientStepCountItem::XGradientStepCountItem( SvStream& rIn ) :
    SfxUInt16Item( XATTR_GRADIENTSTEPCOUNT, rIn )
{
}

/*************************************************************************
|*
|*    XGradientStepCountItem::Clone( SfxItemPool* pPool ) const
|*
|*    Beschreibung
|*    Ersterstellung    23.01.96 KA
|*    Letzte Aenderung  23.01.96 KA
|*
*************************************************************************/

SfxPoolItem* XGradientStepCountItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new XGradientStepCountItem( *this );
}

/*************************************************************************
|*
|*    SfxPoolItem* XGradientStepCountItem::Create(SvStream& rIn, sal_uInt16 nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    23.01.96 KA
|*    Letzte Aenderung  23.01.96 KA
|*
*************************************************************************/

SfxPoolItem* XGradientStepCountItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XGradientStepCountItem( rIn );
}

//------------------------------------------------------------------------

SfxItemPresentation XGradientStepCountItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&          rText, const IntlWrapper *
)   const
{
    rText.Erase();

    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            return ePres;
        case SFX_ITEM_PRESENTATION_COMPLETE:
//          rText = XubString( ResId( RID_SVXSTR_GRADIENTSTEPCOUNT, DIALOG_MGR() ) );
//          rText += ": ";
        case SFX_ITEM_PRESENTATION_NAMELESS:
            rText += XubString( UniString::CreateFromInt32((sal_uInt16) GetValue() ));
            return ePres;
        default:
            return SFX_ITEM_PRESENTATION_NONE;
    }
}


//------------------------------
// class XFillBmpTileItem
//------------------------------
TYPEINIT1_AUTOFACTORY( XFillBmpTileItem, SfxBoolItem );

/*************************************************************************
|*
|*
|*
|*    Beschreibung
|*    Ersterstellung    28.02.96 KA
|*    Letzte Aenderung  28.02.96 KA
|*
*************************************************************************/

XFillBmpTileItem::XFillBmpTileItem( sal_Bool bTile ) :
            SfxBoolItem( XATTR_FILLBMP_TILE, bTile )
{
}


/*************************************************************************
|*
|*
|*
|*    Beschreibung
|*    Ersterstellung    28.02.96 KA
|*    Letzte Aenderung  28.02.96 KA
|*
*************************************************************************/

XFillBmpTileItem::XFillBmpTileItem( SvStream& rIn ) :
            SfxBoolItem( XATTR_FILLBMP_TILE, rIn )
{
}


/*************************************************************************
|*
|*
|*
|*    Beschreibung
|*    Ersterstellung    28.02.96 KA
|*    Letzte Aenderung  28.02.96 KA
|*
*************************************************************************/

SfxPoolItem* XFillBmpTileItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new XFillBmpTileItem( *this );
}


/*************************************************************************
|*
|*
|*
|*    Beschreibung
|*    Ersterstellung    28.02.96 KA
|*    Letzte Aenderung  28.02.96 KA
|*
*************************************************************************/

SfxPoolItem* XFillBmpTileItem::Create( SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XFillBmpTileItem( rIn );
}


/*************************************************************************
|*
|*
|*
|*    Beschreibung
|*    Ersterstellung    28.02.96 KA
|*    Letzte Aenderung  28.02.96 KA
|*
*************************************************************************/

SfxItemPresentation XFillBmpTileItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&          rText, const IntlWrapper *
)   const
{
    rText.Erase();

    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            return ePres;
        case SFX_ITEM_PRESENTATION_COMPLETE:
        case SFX_ITEM_PRESENTATION_NAMELESS:
            return ePres;
        default:
            return SFX_ITEM_PRESENTATION_NONE;
    }
}


//------------------------------
// class XFillBmpTilePosItem
//------------------------------
TYPEINIT1_AUTOFACTORY( XFillBmpPosItem, SfxEnumItem );

/*************************************************************************
|*
|*
|*
|*    Beschreibung
|*    Ersterstellung    28.02.96 KA
|*    Letzte Aenderung  28.02.96 KA
|*
*************************************************************************/

XFillBmpPosItem::XFillBmpPosItem( RECT_POINT eRP ) :
    SfxEnumItem( XATTR_FILLBMP_POS, sal::static_int_cast< sal_uInt16 >( eRP ) )
{
}


/*************************************************************************
|*
|*
|*
|*    Beschreibung
|*    Ersterstellung    28.02.96 KA
|*    Letzte Aenderung  28.02.96 KA
|*
*************************************************************************/

XFillBmpPosItem::XFillBmpPosItem( SvStream& rIn ) :
            SfxEnumItem( XATTR_FILLBMP_POS, rIn )
{
}


/*************************************************************************
|*
|*
|*
|*    Beschreibung
|*    Ersterstellung    28.02.96 KA
|*    Letzte Aenderung  28.02.96 KA
|*
*************************************************************************/

SfxPoolItem* XFillBmpPosItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new XFillBmpPosItem( *this );
}


/*************************************************************************
|*
|*
|*
|*    Beschreibung
|*    Ersterstellung    28.02.96 KA
|*    Letzte Aenderung  28.02.96 KA
|*
*************************************************************************/

SfxPoolItem* XFillBmpPosItem::Create( SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XFillBmpPosItem( rIn );
}


/*************************************************************************
|*
|*
|*
|*    Beschreibung
|*    Ersterstellung    28.02.96 KA
|*    Letzte Aenderung  28.02.96 KA
|*
*************************************************************************/

SfxItemPresentation XFillBmpPosItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&          rText, const IntlWrapper *
)   const
{
    rText.Erase();

    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            return ePres;
        case SFX_ITEM_PRESENTATION_COMPLETE:
        case SFX_ITEM_PRESENTATION_NAMELESS:
            return ePres;
        default:
            return SFX_ITEM_PRESENTATION_NONE;
    }
}

/******************************************************************************
|*
|*
|*
\******************************************************************************/

sal_uInt16 XFillBmpPosItem::GetValueCount() const
{
    return 9;
}


//------------------------------
// class XFillBmpTileSizeXItem
//------------------------------
TYPEINIT1_AUTOFACTORY( XFillBmpSizeXItem, SfxMetricItem );

/*************************************************************************
|*
|*
|*
|*    Beschreibung
|*    Ersterstellung    28.02.96 KA
|*    Letzte Aenderung  28.02.96 KA
|*
*************************************************************************/

XFillBmpSizeXItem::XFillBmpSizeXItem( long nSizeX ) :
            SfxMetricItem( XATTR_FILLBMP_SIZEX, nSizeX )
{
}


/*************************************************************************
|*
|*
|*
|*    Beschreibung
|*    Ersterstellung    28.02.96 KA
|*    Letzte Aenderung  28.02.96 KA
|*
*************************************************************************/

XFillBmpSizeXItem::XFillBmpSizeXItem( SvStream& rIn ) :
            SfxMetricItem( XATTR_FILLBMP_SIZEX, rIn )
{
}


/*************************************************************************
|*
|*
|*
|*    Beschreibung
|*    Ersterstellung    28.02.96 KA
|*    Letzte Aenderung  28.02.96 KA
|*
*************************************************************************/

SfxPoolItem* XFillBmpSizeXItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new XFillBmpSizeXItem( *this );
}


/*************************************************************************
|*
|*
|*
|*    Beschreibung
|*    Ersterstellung    28.02.96 KA
|*    Letzte Aenderung  28.02.96 KA
|*
*************************************************************************/

SfxPoolItem* XFillBmpSizeXItem::Create( SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XFillBmpSizeXItem( rIn );
}


/*************************************************************************
|*
|*
|*
|*    Beschreibung
|*    Ersterstellung    28.02.96 KA
|*    Letzte Aenderung  28.02.96 KA
|*
*************************************************************************/

SfxItemPresentation XFillBmpSizeXItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&          rText, const IntlWrapper *
)   const
{
    rText.Erase();

    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            return ePres;
        case SFX_ITEM_PRESENTATION_COMPLETE:
        case SFX_ITEM_PRESENTATION_NAMELESS:
            return ePres;
        default:
            return SFX_ITEM_PRESENTATION_NONE;
    }
}


/*************************************************************************
|*
|*    Beschreibung
|*    Ersterstellung    05.11.96 KA
|*    Letzte Aenderung  05.11.96 KA
|*
\*************************************************************************/

FASTBOOL XFillBmpSizeXItem::HasMetrics() const
{
    return GetValue() > 0L;
}


//------------------------------
// class XFillBmpTileSizeYItem
//------------------------------
TYPEINIT1_AUTOFACTORY( XFillBmpSizeYItem, SfxMetricItem );

/*************************************************************************
|*
|*
|*
|*    Beschreibung
|*    Ersterstellung    28.02.96 KA
|*    Letzte Aenderung  28.02.96 KA
|*
*************************************************************************/

XFillBmpSizeYItem::XFillBmpSizeYItem( long nSizeY ) :
            SfxMetricItem( XATTR_FILLBMP_SIZEY, nSizeY )
{
}


/*************************************************************************
|*
|*
|*
|*    Beschreibung
|*    Ersterstellung    28.02.96 KA
|*    Letzte Aenderung  28.02.96 KA
|*
*************************************************************************/

XFillBmpSizeYItem::XFillBmpSizeYItem( SvStream& rIn ) :
            SfxMetricItem( XATTR_FILLBMP_SIZEY, rIn )
{
}


/*************************************************************************
|*
|*
|*
|*    Beschreibung
|*    Ersterstellung    28.02.96 KA
|*    Letzte Aenderung  28.02.96 KA
|*
*************************************************************************/

SfxPoolItem* XFillBmpSizeYItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new XFillBmpSizeYItem( *this );
}


/*************************************************************************
|*
|*
|*
|*    Beschreibung
|*    Ersterstellung    28.02.96 KA
|*    Letzte Aenderung  28.02.96 KA
|*
*************************************************************************/

SfxPoolItem* XFillBmpSizeYItem::Create( SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XFillBmpSizeYItem( rIn );
}


/*************************************************************************
|*
|*
|*
|*    Beschreibung
|*    Ersterstellung    28.02.96 KA
|*    Letzte Aenderung  28.02.96 KA
|*
*************************************************************************/

SfxItemPresentation XFillBmpSizeYItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&          rText, const IntlWrapper *
)   const
{
    rText.Erase();

    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            return ePres;
        case SFX_ITEM_PRESENTATION_COMPLETE:
        case SFX_ITEM_PRESENTATION_NAMELESS:
            return ePres;
        default:
            return SFX_ITEM_PRESENTATION_NONE;
    }
}


/*************************************************************************
|*
|*    Beschreibung
|*    Ersterstellung    05.11.96 KA
|*    Letzte Aenderung  05.11.96 KA
|*
\*************************************************************************/

FASTBOOL XFillBmpSizeYItem::HasMetrics() const
{
    return GetValue() > 0L;
}


//------------------------------
// class XFillBmpTileLogItem
//------------------------------
TYPEINIT1_AUTOFACTORY( XFillBmpSizeLogItem, SfxBoolItem );

/*************************************************************************
|*
|*
|*
|*    Beschreibung
|*    Ersterstellung    28.02.96 KA
|*    Letzte Aenderung  28.02.96 KA
|*
*************************************************************************/

XFillBmpSizeLogItem::XFillBmpSizeLogItem( sal_Bool bLog ) :
            SfxBoolItem( XATTR_FILLBMP_SIZELOG, bLog )
{
}


/*************************************************************************
|*
|*
|*
|*    Beschreibung
|*    Ersterstellung    28.02.96 KA
|*    Letzte Aenderung  28.02.96 KA
|*
*************************************************************************/

XFillBmpSizeLogItem::XFillBmpSizeLogItem( SvStream& rIn ) :
            SfxBoolItem( XATTR_FILLBMP_SIZELOG, rIn )
{
}


/*************************************************************************
|*
|*
|*
|*    Beschreibung
|*    Ersterstellung    28.02.96 KA
|*    Letzte Aenderung  28.02.96 KA
|*
*************************************************************************/

SfxPoolItem* XFillBmpSizeLogItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new XFillBmpSizeLogItem( *this );
}


/*************************************************************************
|*
|*
|*
|*    Beschreibung
|*    Ersterstellung    28.02.96 KA
|*    Letzte Aenderung  28.02.96 KA
|*
*************************************************************************/

SfxPoolItem* XFillBmpSizeLogItem::Create( SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XFillBmpSizeLogItem( rIn );
}


/*************************************************************************
|*
|*
|*
|*    Beschreibung
|*    Ersterstellung    28.02.96 KA
|*    Letzte Aenderung  28.02.96 KA
|*
*************************************************************************/

SfxItemPresentation XFillBmpSizeLogItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&          rText, const IntlWrapper *
)   const
{
    rText.Erase();

    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            return ePres;
        case SFX_ITEM_PRESENTATION_COMPLETE:
        case SFX_ITEM_PRESENTATION_NAMELESS:
            return ePres;
        default:
            return SFX_ITEM_PRESENTATION_NONE;
    }
}


//------------------------------
// class XFillBmpTileOffXItem
//------------------------------
TYPEINIT1_AUTOFACTORY( XFillBmpTileOffsetXItem, SfxUInt16Item );

/*************************************************************************
|*
|*
|*
|*    Beschreibung
|*    Ersterstellung    28.02.96 KA
|*    Letzte Aenderung  28.02.96 KA
|*
*************************************************************************/

XFillBmpTileOffsetXItem::XFillBmpTileOffsetXItem( sal_uInt16 nOffX ) :
            SfxUInt16Item( XATTR_FILLBMP_TILEOFFSETX, nOffX )
{
}


/*************************************************************************
|*
|*
|*
|*    Beschreibung
|*    Ersterstellung    28.02.96 KA
|*    Letzte Aenderung  28.02.96 KA
|*
*************************************************************************/

XFillBmpTileOffsetXItem::XFillBmpTileOffsetXItem( SvStream& rIn ) :
            SfxUInt16Item( XATTR_FILLBMP_TILEOFFSETX, rIn )
{
}


/*************************************************************************
|*
|*
|*
|*    Beschreibung
|*    Ersterstellung    28.02.96 KA
|*    Letzte Aenderung  28.02.96 KA
|*
*************************************************************************/

SfxPoolItem* XFillBmpTileOffsetXItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new XFillBmpTileOffsetXItem( *this );
}


/*************************************************************************
|*
|*
|*
|*    Beschreibung
|*    Ersterstellung    28.02.96 KA
|*    Letzte Aenderung  28.02.96 KA
|*
*************************************************************************/

SfxPoolItem* XFillBmpTileOffsetXItem::Create( SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XFillBmpTileOffsetXItem( rIn );
}


/*************************************************************************
|*
|*
|*
|*    Beschreibung
|*    Ersterstellung    28.02.96 KA
|*    Letzte Aenderung  28.02.96 KA
|*
*************************************************************************/

SfxItemPresentation XFillBmpTileOffsetXItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&          rText, const IntlWrapper *
)   const
{
    rText.Erase();

    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            return ePres;
        case SFX_ITEM_PRESENTATION_COMPLETE:
        case SFX_ITEM_PRESENTATION_NAMELESS:
            return ePres;
        default:
            return SFX_ITEM_PRESENTATION_NONE;
    }
}


//------------------------------
// class XFillBmpTileOffYItem
//------------------------------
TYPEINIT1_AUTOFACTORY( XFillBmpTileOffsetYItem, SfxUInt16Item );

/*************************************************************************
|*
|*
|*
|*    Beschreibung
|*    Ersterstellung    28.02.96 KA
|*    Letzte Aenderung  28.02.96 KA
|*
*************************************************************************/

XFillBmpTileOffsetYItem::XFillBmpTileOffsetYItem( sal_uInt16 nOffY ) :
            SfxUInt16Item( XATTR_FILLBMP_TILEOFFSETY, nOffY )
{
}


/*************************************************************************
|*
|*
|*
|*    Beschreibung
|*    Ersterstellung    28.02.96 KA
|*    Letzte Aenderung  28.02.96 KA
|*
*************************************************************************/

XFillBmpTileOffsetYItem::XFillBmpTileOffsetYItem( SvStream& rIn ) :
            SfxUInt16Item( XATTR_FILLBMP_TILEOFFSETY, rIn )
{
}


/*************************************************************************
|*
|*
|*
|*    Beschreibung
|*    Ersterstellung    28.02.96 KA
|*    Letzte Aenderung  28.02.96 KA
|*
*************************************************************************/

SfxPoolItem* XFillBmpTileOffsetYItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new XFillBmpTileOffsetYItem( *this );
}


/*************************************************************************
|*
|*
|*
|*    Beschreibung
|*    Ersterstellung    28.02.96 KA
|*    Letzte Aenderung  28.02.96 KA
|*
*************************************************************************/

SfxPoolItem* XFillBmpTileOffsetYItem::Create( SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XFillBmpTileOffsetYItem( rIn );
}


/*************************************************************************
|*
|*
|*
|*    Beschreibung
|*    Ersterstellung    28.02.96 KA
|*    Letzte Aenderung  28.02.96 KA
|*
*************************************************************************/

SfxItemPresentation XFillBmpTileOffsetYItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&          rText, const IntlWrapper *
)   const
{
    rText.Erase();

    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            return ePres;
        case SFX_ITEM_PRESENTATION_COMPLETE:
        case SFX_ITEM_PRESENTATION_NAMELESS:
            return ePres;
        default:
            return SFX_ITEM_PRESENTATION_NONE;
    }
}


//------------------------------
// class XFillBmpStretchItem
//------------------------------
TYPEINIT1_AUTOFACTORY( XFillBmpStretchItem, SfxBoolItem );

/*************************************************************************
|*
|*
|*
|*    Beschreibung
|*    Ersterstellung    28.02.96 KA
|*    Letzte Aenderung  28.02.96 KA
|*
*************************************************************************/

XFillBmpStretchItem::XFillBmpStretchItem( sal_Bool bStretch ) :
            SfxBoolItem( XATTR_FILLBMP_STRETCH, bStretch )
{
}


/*************************************************************************
|*
|*
|*
|*    Beschreibung
|*    Ersterstellung    28.02.96 KA
|*    Letzte Aenderung  28.02.96 KA
|*
*************************************************************************/

XFillBmpStretchItem::XFillBmpStretchItem( SvStream& rIn ) :
            SfxBoolItem( XATTR_FILLBMP_STRETCH, rIn )
{
}


/*************************************************************************
|*
|*
|*
|*    Beschreibung
|*    Ersterstellung    28.02.96 KA
|*    Letzte Aenderung  28.02.96 KA
|*
*************************************************************************/

SfxPoolItem* XFillBmpStretchItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new XFillBmpStretchItem( *this );
}


/*************************************************************************
|*
|*
|*
|*    Beschreibung
|*    Ersterstellung    28.02.96 KA
|*    Letzte Aenderung  28.02.96 KA
|*
*************************************************************************/

SfxPoolItem* XFillBmpStretchItem::Create( SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XFillBmpStretchItem( rIn );
}


/*************************************************************************
|*
|*
|*
|*    Beschreibung
|*    Ersterstellung    28.02.96 KA
|*    Letzte Aenderung  28.02.96 KA
|*
*************************************************************************/

SfxItemPresentation XFillBmpStretchItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&          rText, const IntlWrapper *
)   const
{
    rText.Erase();

    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            return ePres;
        case SFX_ITEM_PRESENTATION_COMPLETE:
        case SFX_ITEM_PRESENTATION_NAMELESS:
            return ePres;
        default:
            return SFX_ITEM_PRESENTATION_NONE;
    }
}


//------------------------------
// class XFillBmpTileOffPosXItem
//------------------------------
TYPEINIT1_AUTOFACTORY( XFillBmpPosOffsetXItem, SfxUInt16Item );

/*************************************************************************
|*
|*
|*
|*    Beschreibung
|*    Ersterstellung    KA 29.04.96
|*    Letzte Aenderung  KA 29.04.96
|*
*************************************************************************/

XFillBmpPosOffsetXItem::XFillBmpPosOffsetXItem( sal_uInt16 nOffPosX ) :
            SfxUInt16Item( XATTR_FILLBMP_POSOFFSETX, nOffPosX )
{
}


/*************************************************************************
|*
|*
|*
|*    Beschreibung
|*    Ersterstellung    KA 29.04.96
|*    Letzte Aenderung  KA 29.04.96
|*
*************************************************************************/

XFillBmpPosOffsetXItem::XFillBmpPosOffsetXItem( SvStream& rIn ) :
            SfxUInt16Item( XATTR_FILLBMP_POSOFFSETX, rIn )
{
}


/*************************************************************************
|*
|*
|*
|*    Beschreibung
|*    Ersterstellung    KA 29.04.96
|*    Letzte Aenderung  KA 29.04.96
|*
*************************************************************************/

SfxPoolItem* XFillBmpPosOffsetXItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new XFillBmpPosOffsetXItem( *this );
}


/*************************************************************************
|*
|*
|*
|*    Beschreibung
|*    Ersterstellung    KA 29.04.96
|*    Letzte Aenderung  KA 29.04.96
|*
*************************************************************************/

SfxPoolItem* XFillBmpPosOffsetXItem::Create( SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XFillBmpPosOffsetXItem( rIn );
}


/*************************************************************************
|*
|*
|*
|*    Beschreibung
|*    Ersterstellung    KA 29.04.96
|*    Letzte Aenderung  KA 29.04.96
|*
*************************************************************************/

SfxItemPresentation XFillBmpPosOffsetXItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&          rText, const IntlWrapper *
)   const
{
    rText.Erase();

    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            return ePres;
        case SFX_ITEM_PRESENTATION_COMPLETE:
        case SFX_ITEM_PRESENTATION_NAMELESS:
            return ePres;
        default:
            return SFX_ITEM_PRESENTATION_NONE;
    }
}


//------------------------------
// class XFillBmpTileOffPosYItem
//------------------------------
TYPEINIT1_AUTOFACTORY( XFillBmpPosOffsetYItem, SfxUInt16Item );

/*************************************************************************
|*
|*
|*
|*    Beschreibung
|*    Ersterstellung    KA 29.04.96
|*    Letzte Aenderung  KA 29.04.96
|*
*************************************************************************/

XFillBmpPosOffsetYItem::XFillBmpPosOffsetYItem( sal_uInt16 nOffPosY ) :
            SfxUInt16Item( XATTR_FILLBMP_POSOFFSETY, nOffPosY )
{
}


/*************************************************************************
|*
|*
|*
|*    Beschreibung
|*    Ersterstellung    KA 29.04.96
|*    Letzte Aenderung  KA 29.04.96
|*
*************************************************************************/

XFillBmpPosOffsetYItem::XFillBmpPosOffsetYItem( SvStream& rIn ) :
            SfxUInt16Item( XATTR_FILLBMP_POSOFFSETY, rIn )
{
}


/*************************************************************************
|*
|*
|*
|*    Beschreibung
|*    Ersterstellung    KA 29.04.96
|*    Letzte Aenderung  KA 29.04.96
|*
*************************************************************************/

SfxPoolItem* XFillBmpPosOffsetYItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new XFillBmpPosOffsetYItem( *this );
}


/*************************************************************************
|*
|*
|*
|*    Beschreibung
|*    Ersterstellung    KA 29.04.96
|*    Letzte Aenderung  KA 29.04.96
|*
*************************************************************************/

SfxPoolItem* XFillBmpPosOffsetYItem::Create( SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XFillBmpPosOffsetYItem( rIn );
}


/*************************************************************************
|*
|*
|*
|*    Beschreibung
|*    Ersterstellung    KA 29.04.96
|*    Letzte Aenderung  KA 29.04.96
|*
*************************************************************************/

SfxItemPresentation XFillBmpPosOffsetYItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&          rText, const IntlWrapper *
)   const
{
    rText.Erase();

    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            return ePres;
        case SFX_ITEM_PRESENTATION_COMPLETE:
        case SFX_ITEM_PRESENTATION_NAMELESS:
            return ePres;
        default:
            return SFX_ITEM_PRESENTATION_NONE;
    }
}

//--------------------------
// class XFillBackgroundItem
//--------------------------
TYPEINIT1_AUTOFACTORY(XFillBackgroundItem, SfxBoolItem);

/*************************************************************************
|*
|*    XFillBackgroundItem::XFillBackgroundItem( sal_Bool )
|*
|*    Beschreibung
|*    Ersterstellung    19.11.96 KA
|*    Letzte Aenderung
|*
*************************************************************************/

XFillBackgroundItem::XFillBackgroundItem( sal_Bool bFill ) :
    SfxBoolItem( XATTR_FILLBACKGROUND, bFill )
{
}

/*************************************************************************
|*
|*    XFillBackgroundItem::XFillBackgroundItem( SvStream& rIn )
|*
|*    Beschreibung
|*    Ersterstellung    23.01.96 KA
|*    Letzte Aenderung  23.01.96 KA
|*
*************************************************************************/

XFillBackgroundItem::XFillBackgroundItem( SvStream& rIn ) :
    SfxBoolItem( XATTR_FILLBACKGROUND, rIn )
{
}

/*************************************************************************
|*
|*    XFillBackgroundItem::Clone( SfxItemPool* pPool ) const
|*
|*    Beschreibung
|*    Ersterstellung    23.01.96 KA
|*    Letzte Aenderung  23.01.96 KA
|*
*************************************************************************/

SfxPoolItem* XFillBackgroundItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new XFillBackgroundItem( *this );
}

/*************************************************************************
|*
|*    SfxPoolItem* XFillBackgroundItem::Create(SvStream& rIn, sal_uInt16 nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    23.01.96 KA
|*    Letzte Aenderung  23.01.96 KA
|*
*************************************************************************/

SfxPoolItem* XFillBackgroundItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new XFillBackgroundItem( rIn );
}

//------------------------------------------------------------------------

SfxItemPresentation XFillBackgroundItem::GetPresentation( SfxItemPresentation ePres, SfxMapUnit /*eCoreUnit*/,
                                                          SfxMapUnit /*ePresUnit*/, XubString& rText, const IntlWrapper*) const
{
    rText.Erase();

    switch( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            return ePres;

        case SFX_ITEM_PRESENTATION_COMPLETE:
        case SFX_ITEM_PRESENTATION_NAMELESS:
            return ePres;
        default:
            return SFX_ITEM_PRESENTATION_NONE;
    }
}



