/*************************************************************************
 *
 *  $RCSfile: xattr2.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:28 $
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

// include ---------------------------------------------------------------

#pragma hdrstop

#ifndef _COM_SUN_STAR_DRAWING_LINEJOINT_HPP_
#include <com/sun/star/drawing/LineJoint.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#include "dialogs.hrc"
#include "xattr.hxx"
#include "xtable.hxx"
#include "xoutx.hxx"
#include "dialmgr.hxx"
#include "itemtype.hxx"
#include "xdef.hxx"

#define GLOBALOVERFLOW

/************************************************************************/

//------------------------------
// class XLineTransparenceItem
//------------------------------
TYPEINIT1_AUTOFACTORY(XLineTransparenceItem, SfxUInt16Item);

/*************************************************************************
|*
|*    XLineTransparenceItem::XLineTransparenceItem(USHORT)
|*
|*    Beschreibung
|*    Ersterstellung    07.11.95 KA
|*    Letzte Aenderung  07.11.95 KA
|*
*************************************************************************/

XLineTransparenceItem::XLineTransparenceItem(USHORT nLineTransparence) :
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

SfxPoolItem* XLineTransparenceItem::Clone(SfxItemPool* pPool) const
{
    return new XLineTransparenceItem(*this);
}

/*************************************************************************
|*
|*    SfxPoolItem* XLineTransparenceItem::Create(SvStream& rIn, USHORT nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    07.11.95 KA
|*    Letzte Aenderung  07.11.95 KA
|*
*************************************************************************/

SfxPoolItem* XLineTransparenceItem::Create(SvStream& rIn, USHORT nVer) const
{
    return new XLineTransparenceItem(rIn);
}

//------------------------------------------------------------------------

SfxItemPresentation XLineTransparenceItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
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
            rText += XubString( UniString::CreateFromInt32((USHORT) GetValue()) );
            rText += sal_Unicode('%');
            return ePres;
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

//-----------------------
// class XLineJointItem -
//-----------------------

TYPEINIT1_AUTOFACTORY(XLineJointItem, SfxEnumItem);

// -----------------------------------------------------------------------------

XLineJointItem::XLineJointItem( XLineJoint eLineJoint ) :
    SfxEnumItem(XATTR_LINEJOINT, eLineJoint)
{
}

// -----------------------------------------------------------------------------

XLineJointItem::XLineJointItem( SvStream& rIn ) :
    SfxEnumItem( XATTR_LINEJOINT, rIn )
{
}

// -----------------------------------------------------------------------------

USHORT XLineJointItem::GetVersion( USHORT nFileFormatVersion ) const
{
    return 1;
}

// -----------------------------------------------------------------------------

SfxPoolItem* XLineJointItem::Create( SvStream& rIn, USHORT nVer ) const
{
    XLineJointItem* pRet = new XLineJointItem( rIn );

    if(nVer < 1)
        pRet->SetValue(XLINEJOINT_ROUND);

    return pRet;
}

// -----------------------------------------------------------------------------

SfxPoolItem* XLineJointItem::Clone(SfxItemPool* pPool) const
{
    return new XLineJointItem( *this );
}

// -----------------------------------------------------------------------------

SfxItemPresentation XLineJointItem::GetPresentation( SfxItemPresentation ePres, SfxMapUnit eCoreUnit,
                                                     SfxMapUnit ePresUnit, XubString& rText, const International*) const
{
    rText.Erase();

    switch( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE: return ePres;

        case SFX_ITEM_PRESENTATION_COMPLETE:
        case SFX_ITEM_PRESENTATION_NAMELESS:
        {
            USHORT nId = 0;

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
    }

    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------------

sal_Bool XLineJointItem::QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
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

BOOL XLineJointItem::PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId )
{
    XLineJoint eJoint = XLINEJOINT_NONE;
    ::com::sun::star::drawing::LineJoint eUnoJoint;

    if(!(rVal >>= eUnoJoint))
        return sal_False;

    switch( eUnoJoint )
    {
    case ::com::sun::star::drawing::LineJoint_NONE:
        break;
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
    }

    SetValue( eJoint );

    return sal_True;
}

// -----------------------------------------------------------------------------

USHORT XLineJointItem::GetValueCount() const
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
|*    XFillTransparenceItem::XFillTransparenceItem(USHORT)
|*
|*    Beschreibung
|*    Ersterstellung    07.11.95 KA
|*    Letzte Aenderung  07.11.95 KA
|*
*************************************************************************/

XFillTransparenceItem::XFillTransparenceItem(USHORT nFillTransparence) :
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

SfxPoolItem* XFillTransparenceItem::Clone(SfxItemPool* pPool) const
{
    return new XFillTransparenceItem(*this);
}

/*************************************************************************
|*
|*    SfxPoolItem* XFillTransparenceItem::Create(SvStream& rIn, USHORT nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    07.11.95 KA
|*    Letzte Aenderung  07.11.95 KA
|*
*************************************************************************/

SfxPoolItem* XFillTransparenceItem::Create(SvStream& rIn, USHORT nVer) const
{
    return new XFillTransparenceItem(rIn);
}

//------------------------------------------------------------------------

SfxItemPresentation XFillTransparenceItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
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
            rText += XubString( UniString::CreateFromInt32((USHORT) GetValue() ));
            rText += sal_Unicode('%');
            return ePres;
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

//------------------------------
// class XFormTextShadowTranspItem
//------------------------------
TYPEINIT1_AUTOFACTORY(XFormTextShadowTranspItem, SfxUInt16Item);

/*************************************************************************
|*
|*    XFormTextShadowTranspItem::XFormTextShadowTranspItem(USHORT)
|*
|*    Beschreibung
|*    Ersterstellung    09.11.95 KA
|*    Letzte Aenderung  09.11.95 KA
|*
*************************************************************************/

XFormTextShadowTranspItem::XFormTextShadowTranspItem(USHORT nShdwTransparence) :
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

SfxPoolItem* XFormTextShadowTranspItem::Clone(SfxItemPool* pPool) const
{
    return new XFormTextShadowTranspItem(*this);
}

/*************************************************************************
|*
|*    SfxPoolItem* XFormTextShadowTranspItem::Create(SvStream& rIn, USHORT nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    09.11.95 KA
|*    Letzte Aenderung  09.11.95 KA
|*
*************************************************************************/

SfxPoolItem* XFormTextShadowTranspItem::Create(SvStream& rIn, USHORT nVer) const
{
    return new XFormTextShadowTranspItem(rIn);
}


//------------------------------
// class XFillGradientStepCountItem
//------------------------------
TYPEINIT1_AUTOFACTORY(XGradientStepCountItem, SfxUInt16Item);

/*************************************************************************
|*
|*    XGradientStepCountItem::XGradientStepCountItem( USHORT )
|*
|*    Beschreibung
|*    Ersterstellung    23.01.96 KA
|*    Letzte Aenderung  23.01.96 KA
|*
*************************************************************************/

XGradientStepCountItem::XGradientStepCountItem( USHORT nStepCount ) :
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

SfxPoolItem* XGradientStepCountItem::Clone( SfxItemPool* pPool ) const
{
    return new XGradientStepCountItem( *this );
}

/*************************************************************************
|*
|*    SfxPoolItem* XGradientStepCountItem::Create(SvStream& rIn, USHORT nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    23.01.96 KA
|*    Letzte Aenderung  23.01.96 KA
|*
*************************************************************************/

SfxPoolItem* XGradientStepCountItem::Create(SvStream& rIn, USHORT nVer) const
{
    return new XGradientStepCountItem( rIn );
}

//------------------------------------------------------------------------

SfxItemPresentation XGradientStepCountItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
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
            rText += XubString( UniString::CreateFromInt32((USHORT) GetValue() ));
            return ePres;
    }
    return SFX_ITEM_PRESENTATION_NONE;
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

XFillBmpTileItem::XFillBmpTileItem( BOOL bTile ) :
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

SfxPoolItem* XFillBmpTileItem::Clone( SfxItemPool* pPool ) const
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

SfxPoolItem* XFillBmpTileItem::Create( SvStream& rIn, USHORT nVer ) const
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
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
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
    }

    return SFX_ITEM_PRESENTATION_NONE;
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
            SfxEnumItem( XATTR_FILLBMP_POS, eRP )
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

SfxPoolItem* XFillBmpPosItem::Clone( SfxItemPool* pPool ) const
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

SfxPoolItem* XFillBmpPosItem::Create( SvStream& rIn, USHORT nVer ) const
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
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
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
    }

    return SFX_ITEM_PRESENTATION_NONE;
}

/******************************************************************************
|*
|*
|*
\******************************************************************************/

USHORT XFillBmpPosItem::GetValueCount() const
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

SfxPoolItem* XFillBmpSizeXItem::Clone( SfxItemPool* pPool ) const
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

SfxPoolItem* XFillBmpSizeXItem::Create( SvStream& rIn, USHORT nVer ) const
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
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
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
    }

    return SFX_ITEM_PRESENTATION_NONE;
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

SfxPoolItem* XFillBmpSizeYItem::Clone( SfxItemPool* pPool ) const
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

SfxPoolItem* XFillBmpSizeYItem::Create( SvStream& rIn, USHORT nVer ) const
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
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
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
    }

    return SFX_ITEM_PRESENTATION_NONE;
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

XFillBmpSizeLogItem::XFillBmpSizeLogItem( BOOL bLog ) :
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

SfxPoolItem* XFillBmpSizeLogItem::Clone( SfxItemPool* pPool ) const
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

SfxPoolItem* XFillBmpSizeLogItem::Create( SvStream& rIn, USHORT nVer ) const
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
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
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
    }

    return SFX_ITEM_PRESENTATION_NONE;
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

XFillBmpTileOffsetXItem::XFillBmpTileOffsetXItem( USHORT nOffX ) :
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

SfxPoolItem* XFillBmpTileOffsetXItem::Clone( SfxItemPool* pPool ) const
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

SfxPoolItem* XFillBmpTileOffsetXItem::Create( SvStream& rIn, USHORT nVer ) const
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
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
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
    }

    return SFX_ITEM_PRESENTATION_NONE;
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

XFillBmpTileOffsetYItem::XFillBmpTileOffsetYItem( USHORT nOffY ) :
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

SfxPoolItem* XFillBmpTileOffsetYItem::Clone( SfxItemPool* pPool ) const
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

SfxPoolItem* XFillBmpTileOffsetYItem::Create( SvStream& rIn, USHORT nVer ) const
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
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
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
    }

    return SFX_ITEM_PRESENTATION_NONE;
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

XFillBmpStretchItem::XFillBmpStretchItem( BOOL bStretch ) :
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

SfxPoolItem* XFillBmpStretchItem::Clone( SfxItemPool* pPool ) const
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

SfxPoolItem* XFillBmpStretchItem::Create( SvStream& rIn, USHORT nVer ) const
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
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
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
    }

    return SFX_ITEM_PRESENTATION_NONE;
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

XFillBmpPosOffsetXItem::XFillBmpPosOffsetXItem( USHORT nOffPosX ) :
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

SfxPoolItem* XFillBmpPosOffsetXItem::Clone( SfxItemPool* pPool ) const
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

SfxPoolItem* XFillBmpPosOffsetXItem::Create( SvStream& rIn, USHORT nVer ) const
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
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
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
    }

    return SFX_ITEM_PRESENTATION_NONE;
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

XFillBmpPosOffsetYItem::XFillBmpPosOffsetYItem( USHORT nOffPosY ) :
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

SfxPoolItem* XFillBmpPosOffsetYItem::Clone( SfxItemPool* pPool ) const
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

SfxPoolItem* XFillBmpPosOffsetYItem::Create( SvStream& rIn, USHORT nVer ) const
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
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText, const International *
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
    }

    return SFX_ITEM_PRESENTATION_NONE;
}

//--------------------------
// class XFillBackgroundItem
//--------------------------
TYPEINIT1_AUTOFACTORY(XFillBackgroundItem, SfxBoolItem);

/*************************************************************************
|*
|*    XFillBackgroundItem::XFillBackgroundItem( BOOL )
|*
|*    Beschreibung
|*    Ersterstellung    19.11.96 KA
|*    Letzte Aenderung
|*
*************************************************************************/

XFillBackgroundItem::XFillBackgroundItem( BOOL bFill ) :
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

SfxPoolItem* XFillBackgroundItem::Clone( SfxItemPool* pPool ) const
{
    return new XFillBackgroundItem( *this );
}

/*************************************************************************
|*
|*    SfxPoolItem* XFillBackgroundItem::Create(SvStream& rIn, USHORT nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    23.01.96 KA
|*    Letzte Aenderung  23.01.96 KA
|*
*************************************************************************/

SfxPoolItem* XFillBackgroundItem::Create(SvStream& rIn, USHORT nVer) const
{
    return new XFillBackgroundItem( rIn );
}

//------------------------------------------------------------------------

SfxItemPresentation XFillBackgroundItem::GetPresentation( SfxItemPresentation ePres, SfxMapUnit eCoreUnit,
                                                          SfxMapUnit ePresUnit, XubString& rText, const International*) const
{
    rText.Erase();

    switch( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            return ePres;
        break;

        case SFX_ITEM_PRESENTATION_COMPLETE:
        case SFX_ITEM_PRESENTATION_NAMELESS:
            return ePres;
        break;
    }

    return SFX_ITEM_PRESENTATION_NONE;
}



