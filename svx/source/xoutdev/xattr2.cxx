/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

// include ---------------------------------------------------------------


#include <com/sun/star/drawing/LineJoint.hpp>
#include <com/sun/star/drawing/LineCap.hpp>
#include <com/sun/star/uno/Any.hxx>

#include <svx/dialogs.hrc>
#include "svx/xattr.hxx"
#include <svx/xtable.hxx>
#include <svx/dialmgr.hxx>
#include <editeng/itemtype.hxx>
#include <svx/xdef.hxx>
#include <svx/AffineMatrixItem.hxx>

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

XLineJointItem::XLineJointItem( com::sun::star::drawing::LineJoint eLineJoint ) :
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
        pRet->SetValue(com::sun::star::drawing::LineJoint_ROUND);

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
                case( com::sun::star::drawing::LineJoint_NONE ):
                    nId = RID_SVXSTR_LINEJOINT_NONE;
                break;

                case( com::sun::star::drawing::LineJoint_MIDDLE ):
                    nId = RID_SVXSTR_LINEJOINT_MIDDLE;
                break;


                case( com::sun::star::drawing::LineJoint_BEVEL ):
                    nId = RID_SVXSTR_LINEJOINT_BEVEL;
                break;


                case( com::sun::star::drawing::LineJoint_MITER ):
                    nId = RID_SVXSTR_LINEJOINT_MITER;
                break;


                case( com::sun::star::drawing::LineJoint_ROUND ):
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
    const ::com::sun::star::drawing::LineJoint eJoint = GetValue();
    rVal <<= eJoint;
    return sal_True;
}

// -----------------------------------------------------------------------------

sal_Bool XLineJointItem::PutValue( const ::com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    ::com::sun::star::drawing::LineJoint eUnoJoint;

    if(!(rVal >>= eUnoJoint))
    {
        // also try an int (for Basic)
        sal_Int32 nLJ = 0;
        if(!(rVal >>= nLJ))
            return sal_False;
        eUnoJoint = (::com::sun::star::drawing::LineJoint)nLJ;
    }

    SetValue( sal::static_int_cast< sal_uInt16 >( eUnoJoint ) );

    return sal_True;
}

// -----------------------------------------------------------------------------

sal_uInt16 XLineJointItem::GetValueCount() const
{
    // don't forget to update the api interface also
    return 5;
}

//---------------------
// class AffineMatrixItem
//---------------------

TYPEINIT1_AUTOFACTORY(AffineMatrixItem, SfxPoolItem);

AffineMatrixItem::AffineMatrixItem(const com::sun::star::geometry::AffineMatrix2D* pMatrix)
:   SfxPoolItem(SID_ATTR_TRANSFORM_MATRIX)
{
    if(pMatrix)
    {
        maMatrix = *pMatrix;
    }
    else
    {
        maMatrix.m00 = 1.0;
        maMatrix.m01 = 0.0;
        maMatrix.m02 = 0.0;
        maMatrix.m10 = 0.0;
        maMatrix.m11 = 1.0;
        maMatrix.m12 = 0.0;
    }
}

AffineMatrixItem::AffineMatrixItem(SvStream& rIn)
:   SfxPoolItem(SID_ATTR_TRANSFORM_MATRIX)
{
    rIn >> maMatrix.m00;
    rIn >> maMatrix.m01;
    rIn >> maMatrix.m02;
    rIn >> maMatrix.m10;
    rIn >> maMatrix.m11;
    rIn >> maMatrix.m12;
}

AffineMatrixItem::AffineMatrixItem(const AffineMatrixItem& rRef)
:   SfxPoolItem(SID_ATTR_TRANSFORM_MATRIX)
{
    maMatrix = rRef.maMatrix;
}

AffineMatrixItem::~AffineMatrixItem()
{
}

int AffineMatrixItem::operator==(const SfxPoolItem& rRef) const
{
    if(!SfxPoolItem::operator==(rRef))
    {
        return 0;
    }

    const AffineMatrixItem* pRef = dynamic_cast< const AffineMatrixItem* >(&rRef);

    if(!pRef)
    {
        return 0;
    }

    return (maMatrix.m00 == pRef->maMatrix.m00
        && maMatrix.m01 == pRef->maMatrix.m01
        && maMatrix.m02 == pRef->maMatrix.m02
        && maMatrix.m10 == pRef->maMatrix.m10
        && maMatrix.m11 == pRef->maMatrix.m11
        && maMatrix.m12 == pRef->maMatrix.m12);
}

SfxPoolItem* AffineMatrixItem::Clone( SfxItemPool* /*pPool*/ ) const
{
    return new AffineMatrixItem(*this);
}

SfxPoolItem* AffineMatrixItem::Create( SvStream& rIn, sal_uInt16 /*nVer*/ ) const
{
    return new AffineMatrixItem(rIn);
}

SvStream& AffineMatrixItem::Store(SvStream &rStream, sal_uInt16 /*nItemVersion*/ ) const
{
    rStream << maMatrix.m00;
    rStream << maMatrix.m01;
    rStream << maMatrix.m02;
    rStream << maMatrix.m10;
    rStream << maMatrix.m11;
    rStream << maMatrix.m12;
    return rStream;
}

sal_Bool AffineMatrixItem::QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    rVal <<= maMatrix;
    return sal_True;
}

sal_Bool AffineMatrixItem::PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId )
{
    if (rVal >>= maMatrix)
    {
        return sal_True;
    }

    DBG_ERROR( "AffineMatrixItem::PutValue - Wrong type!" );
    return sal_False;
}

const com::sun::star::geometry::AffineMatrix2D& AffineMatrixItem::GetAffineMatrix2D() const
{
    return maMatrix;
}

//-----------------------
// class XLineCapItem -
//-----------------------

TYPEINIT1_AUTOFACTORY(XLineCapItem, SfxEnumItem);

// -----------------------------------------------------------------------------

XLineCapItem::XLineCapItem(com::sun::star::drawing::LineCap eLineCap)
:   SfxEnumItem(XATTR_LINECAP, sal::static_int_cast< sal_uInt16 >(eLineCap))
{
}

// -----------------------------------------------------------------------------

XLineCapItem::XLineCapItem( SvStream& rIn )
:   SfxEnumItem(XATTR_LINECAP, rIn)
{
}

// -----------------------------------------------------------------------------

sal_uInt16 XLineCapItem::GetVersion( sal_uInt16 /*nFileFormatVersion*/) const
{
    return 1;
}

// -----------------------------------------------------------------------------

SfxPoolItem* XLineCapItem::Create( SvStream& rIn, sal_uInt16 nVer ) const
{
    XLineCapItem* pRet = new XLineCapItem( rIn );

    if(nVer < 1)
        pRet->SetValue(com::sun::star::drawing::LineCap_BUTT);

    return pRet;
}

// -----------------------------------------------------------------------------

SfxPoolItem* XLineCapItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XLineCapItem( *this );
}

// -----------------------------------------------------------------------------

SfxItemPresentation XLineCapItem::GetPresentation( SfxItemPresentation ePres, SfxMapUnit /*eCoreUnit*/,
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
                default: /*com::sun::star::drawing::LineCap_BUTT*/
                    nId = RID_SVXSTR_LINECAP_BUTT;
                break;

                case(com::sun::star::drawing::LineCap_ROUND):
                    nId = RID_SVXSTR_LINECAP_ROUND;
                break;

                case(com::sun::star::drawing::LineCap_SQUARE):
                    nId = RID_SVXSTR_LINECAP_SQUARE;
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

sal_Bool XLineCapItem::QueryValue( ::com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    const com::sun::star::drawing::LineCap eCap(GetValue());
    rVal <<= eCap;
    return true;
}

// -----------------------------------------------------------------------------

sal_Bool XLineCapItem::PutValue( const ::com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    com::sun::star::drawing::LineCap eUnoCap;

    if(!(rVal >>= eUnoCap))
    {
        // also try an int (for Basic)
        sal_Int32 nLJ(0);

        if(!(rVal >>= nLJ))
        {
            return false;
        }

        eUnoCap = (com::sun::star::drawing::LineCap)nLJ;
    }

    OSL_ENSURE(com::sun::star::drawing::LineCap_BUTT == eUnoCap
        || com::sun::star::drawing::LineCap_ROUND == eUnoCap
        || com::sun::star::drawing::LineCap_SQUARE == eUnoCap, "Unknown enum value in XATTR_LINECAP (!)");

    SetValue(sal::static_int_cast< sal_uInt16 >(eUnoCap));

    return true;
}

// -----------------------------------------------------------------------------

sal_uInt16 XLineCapItem::GetValueCount() const
{
    // don't forget to update the api interface also
    return 3;
}

// -----------------------------------------------------------------------------

com::sun::star::drawing::LineCap XLineCapItem::GetValue() const
{
    const com::sun::star::drawing::LineCap eRetval((com::sun::star::drawing::LineCap)SfxEnumItem::GetValue());
    OSL_ENSURE(com::sun::star::drawing::LineCap_BUTT == eRetval
        || com::sun::star::drawing::LineCap_ROUND == eRetval
        || com::sun::star::drawing::LineCap_SQUARE == eRetval, "Unknown enum value in XATTR_LINECAP (!)");

    return eRetval;
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



