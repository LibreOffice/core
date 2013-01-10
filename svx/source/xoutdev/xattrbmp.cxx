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

#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <tools/stream.hxx>
#include <vcl/window.hxx>
#include <vcl/virdev.hxx>
#include <vcl/bitmapex.hxx>
#include <toolkit/unohlp.hxx>
#include <svl/style.hxx>
#include <editeng/memberids.hrc>
#include <svx/dialogs.hrc>
#include "svx/xattr.hxx"
#include <svx/xtable.hxx>
#include <svx/xdef.hxx>
#include <svx/unomid.hxx>
#include <editeng/unoprnms.hxx>
#include <svx/unoapi.hxx>
#include <svx/svdmodel.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <vcl/salbtype.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/dibtools.hxx>

using namespace ::com::sun::star;

// -----------------------
// class XFillBitmapItem
// -----------------------
TYPEINIT1_AUTOFACTORY(XFillBitmapItem, NameOrIndex);

//////////////////////////////////////////////////////////////////////////////

XFillBitmapItem::XFillBitmapItem(long nIndex, const GraphicObject& rGraphicObject)
:   NameOrIndex(XATTR_FILLBITMAP, nIndex),
    maGraphicObject(rGraphicObject)
{
}

//////////////////////////////////////////////////////////////////////////////

XFillBitmapItem::XFillBitmapItem(const XubString& rName, const GraphicObject& rGraphicObject)
:   NameOrIndex(XATTR_FILLBITMAP, rName),
    maGraphicObject(rGraphicObject)
{
}

//////////////////////////////////////////////////////////////////////////////

XFillBitmapItem::XFillBitmapItem(const XFillBitmapItem& rItem)
:   NameOrIndex(rItem),
    maGraphicObject(rItem.maGraphicObject)
{
}

//////////////////////////////////////////////////////////////////////////////

Bitmap createHistorical8x8FromArray(const sal_uInt16* pArray, Color aColorPix, Color aColorBack)
{
    BitmapPalette aPalette(2);

    aPalette[0] = BitmapColor(aColorBack);
    aPalette[1] = BitmapColor(aColorPix);

    Bitmap aBitmap(Size(8, 8), 1, &aPalette);
    BitmapWriteAccess* pContent = aBitmap.AcquireWriteAccess();

    if(pContent)
    {
        for(sal_uInt16 a(0); a < 8; a++)
        {
            for(sal_uInt16 b(0); b < 8; b++)
            {
                if(pArray[(a * 8) + b])
                {
                    pContent->SetPixelIndex(b, a, 1);
                }
                else
                {
                    pContent->SetPixelIndex(b, a, 0);
                }
            }
        }

        aBitmap.ReleaseAccess(pContent);
    }

    return aBitmap;
}

//////////////////////////////////////////////////////////////////////////////

bool SVX_DLLPUBLIC isHistorical8x8(const BitmapEx& rBitmapEx, BitmapColor& o_rBack, BitmapColor& o_rFront)
{
    if(!rBitmapEx.IsTransparent())
    {
        Bitmap aBitmap(rBitmapEx.GetBitmap());

        if(8 == aBitmap.GetSizePixel().Width() && 8 == aBitmap.GetSizePixel().Height())
        {
            if(2 == aBitmap.GetColorCount())
            {
                BitmapReadAccess* pRead = aBitmap.AcquireReadAccess();

                if(pRead)
                {
                    if(pRead->HasPalette() && 2 == pRead->GetPaletteEntryCount())
                    {
                        const BitmapPalette& rPalette = pRead->GetPalette();

                        o_rBack = rPalette[1];
                        o_rFront = rPalette[0];

                        return true;
                    }
                }
            }
        }
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////////

XFillBitmapItem::XFillBitmapItem(SvStream& rIn, sal_uInt16 nVer)
:   NameOrIndex(XATTR_FILLBITMAP, rIn)
{
    if (!IsIndex())
    {
        if(0 == nVer)
        {
            // Behandlung der alten Bitmaps
            Bitmap aBmp;

            ReadDIB(aBmp, rIn, true);
            maGraphicObject = Graphic(aBmp);
        }
        else if(1 == nVer)
        {
            enum XBitmapType
            {
                XBITMAP_IMPORT,
                XBITMAP_8X8
            };

            sal_Int16 iTmp;

            rIn >> iTmp; // former XBitmapStyle
            rIn >> iTmp; // former XBitmapType

            if(XBITMAP_IMPORT == iTmp)
            {
                Bitmap aBmp;

                ReadDIB(aBmp, rIn, true);
                maGraphicObject = Graphic(aBmp);
            }
            else if(XBITMAP_8X8 == iTmp)
            {
                sal_uInt16 aArray[64];

                for(sal_uInt16 i(0); i < 64; i++)
                {
                    rIn >> aArray[i];
                }

                Color aColorPix;
                Color aColorBack;

                rIn >> aColorPix;
                rIn >> aColorBack;

                const Bitmap aBitmap(createHistorical8x8FromArray(aArray, aColorPix, aColorBack));

                maGraphicObject = Graphic(aBitmap);
            }
        }
        else if(2 == nVer)
        {
            BitmapEx aBmpEx;

            ReadDIBBitmapEx(aBmpEx, rIn);
            maGraphicObject = Graphic(aBmpEx);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

XFillBitmapItem::XFillBitmapItem(SfxItemPool* /*pPool*/, const GraphicObject& rGraphicObject)
:   NameOrIndex( XATTR_FILLBITMAP, -1),
    maGraphicObject(rGraphicObject)
{
}

//////////////////////////////////////////////////////////////////////////////

XFillBitmapItem::XFillBitmapItem(SfxItemPool* /*pPool*/)
:   NameOrIndex(XATTR_FILLBITMAP, -1),
    maGraphicObject()
{
}

//////////////////////////////////////////////////////////////////////////////

SfxPoolItem* XFillBitmapItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new XFillBitmapItem(*this);
}

//////////////////////////////////////////////////////////////////////////////

int XFillBitmapItem::operator==(const SfxPoolItem& rItem) const
{
    return (NameOrIndex::operator==(rItem)
        && maGraphicObject == ((const XFillBitmapItem&)rItem).maGraphicObject);
}

//////////////////////////////////////////////////////////////////////////////

SfxPoolItem* XFillBitmapItem::Create(SvStream& rIn, sal_uInt16 nVer) const
{
    return new XFillBitmapItem( rIn, nVer );
}

//////////////////////////////////////////////////////////////////////////////

SvStream& XFillBitmapItem::Store( SvStream& rOut, sal_uInt16 nItemVersion ) const
{
    NameOrIndex::Store(rOut, nItemVersion);

    if(!IsIndex())
    {
        WriteDIBBitmapEx(maGraphicObject.GetGraphic().GetBitmapEx(), rOut);
    }

    return rOut;
}

//////////////////////////////////////////////////////////////////////////////

const GraphicObject& XFillBitmapItem::GetGraphicObject() const
{
    return maGraphicObject;
}

//////////////////////////////////////////////////////////////////////////////

void XFillBitmapItem::SetGraphicObject(const GraphicObject& rGraphicObject)
{
    maGraphicObject = rGraphicObject;
}

//////////////////////////////////////////////////////////////////////////////

sal_uInt16 XFillBitmapItem::GetVersion(sal_uInt16 /*nFileFormatVersion*/) const
{
    // version three
    return(2);
}

//////////////////////////////////////////////////////////////////////////////

SfxItemPresentation XFillBitmapItem::GetPresentation(
    SfxItemPresentation ePres,
    SfxMapUnit /*eCoreUnit*/,
    SfxMapUnit /*ePresUnit*/,
    XubString& rText,
    const IntlWrapper*) const
{
    switch (ePres)
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return ePres;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText += GetName();
            return ePres;
        default:
            return SFX_ITEM_PRESENTATION_NONE;
    }
}

//////////////////////////////////////////////////////////////////////////////

sal_Bool XFillBitmapItem::QueryValue(::com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId) const
{
    nMemberId &= ~CONVERT_TWIPS;

    // needed for MID_NAME
    ::rtl::OUString aApiName;
    // needed for complete item (MID 0)
    ::rtl::OUString aInternalName;

    ::rtl::OUString aURL;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap > xBmp;

    if( nMemberId == MID_NAME )
    {
         SvxUnogetApiNameForItem( Which(), GetName(), aApiName );
    }
    else if( nMemberId == 0  )
    {
        aInternalName = GetName();
    }

    if( nMemberId == MID_GRAFURL ||
        nMemberId == 0 )
    {
        aURL = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(UNO_NAME_GRAPHOBJ_URLPREFIX));
        aURL += ::rtl::OUString::createFromAscii(GetGraphicObject().GetUniqueID().GetBuffer() );
    }
    if( nMemberId == MID_BITMAP ||
        nMemberId == 0  )
    {
        xBmp.set(VCLUnoHelper::CreateBitmap(GetGraphicObject().GetGraphic().GetBitmapEx()));
    }

    if( nMemberId == MID_NAME )
        rVal <<= aApiName;
    else if( nMemberId == MID_GRAFURL )
        rVal <<= aURL;
    else if( nMemberId == MID_BITMAP )
        rVal <<= xBmp;
    else
    {
        // member-id 0 => complete item (e.g. for toolbars)
        DBG_ASSERT( nMemberId == 0, "invalid member-id" );
        uno::Sequence< beans::PropertyValue > aPropSeq( 3 );

        aPropSeq[0].Name  = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Name" ));
        aPropSeq[0].Value = uno::makeAny( aInternalName );
        aPropSeq[1].Name  = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FillBitmapURL" ));
        aPropSeq[1].Value = uno::makeAny( aURL );
        aPropSeq[2].Name  = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Bitmap" ));
        aPropSeq[2].Value = uno::makeAny( xBmp );

        rVal <<= aPropSeq;
    }

    return sal_True;
}

//////////////////////////////////////////////////////////////////////////////

sal_Bool XFillBitmapItem::PutValue( const ::com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;

    ::rtl::OUString aName;
    ::rtl::OUString aURL;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap > xBmp;
    ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > xGraphic;

    bool bSetName   = false;
    bool bSetURL    = false;
    bool bSetBitmap = false;

    if( nMemberId == MID_NAME )
        bSetName = (rVal >>= aName);
    else if( nMemberId == MID_GRAFURL )
        bSetURL = (rVal >>= aURL);
    else if( nMemberId == MID_BITMAP )
    {
        bSetBitmap = (rVal >>= xBmp);
        if ( !bSetBitmap )
            bSetBitmap = (rVal >>= xGraphic );
    }
    else
    {
        DBG_ASSERT( nMemberId == 0, "invalid member-id" );
        uno::Sequence< beans::PropertyValue >   aPropSeq;
        if( rVal >>= aPropSeq )
        {
            for ( sal_Int32 n = 0; n < aPropSeq.getLength(); n++ )
            {
                if( aPropSeq[n].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Name" )))
                    bSetName = (aPropSeq[n].Value >>= aName);
                else if( aPropSeq[n].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "FillBitmapURL" )))
                    bSetURL = (aPropSeq[n].Value >>= aURL);
                else if( aPropSeq[n].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Bitmap" )))
                    bSetBitmap = (aPropSeq[n].Value >>= xBmp);
            }
        }
    }

    if( bSetName )
    {
        SetName( aName );
    }
    if( bSetURL )
    {
        maGraphicObject  = GraphicObject::CreateGraphicObjectFromURL(aURL);

        // #121194# Prefer GraphicObject over bitmap object if both are provided
        if(bSetBitmap && GRAPHIC_NONE != maGraphicObject.GetType())
        {
            bSetBitmap = false;
        }
    }
    if( bSetBitmap )
    {
        if(xBmp.is())
        {
            maGraphicObject = Graphic(VCLUnoHelper::GetBitmap(xBmp));
        }
        else if(xGraphic.is())
        {
            maGraphicObject = Graphic(xGraphic);
        }
    }

    return (bSetName || bSetURL || bSetBitmap);
}

//////////////////////////////////////////////////////////////////////////////

sal_Bool XFillBitmapItem::CompareValueFunc( const NameOrIndex* p1, const NameOrIndex* p2 )
{
    const GraphicObject& aGraphicObjectA(((XFillBitmapItem*)p1)->GetGraphicObject());
    const GraphicObject& aGraphicObjectB(((XFillBitmapItem*)p2)->GetGraphicObject());

    return aGraphicObjectA == aGraphicObjectB;
}

//////////////////////////////////////////////////////////////////////////////

XFillBitmapItem* XFillBitmapItem::checkForUniqueItem( SdrModel* pModel ) const
{
    if( pModel )
    {
        const String aUniqueName = NameOrIndex::CheckNamedItem( this,
                                                                XATTR_FILLBITMAP,
                                                                &pModel->GetItemPool(),
                                                                pModel->GetStyleSheetPool() ? &pModel->GetStyleSheetPool()->GetPool() : NULL,
                                                                XFillBitmapItem::CompareValueFunc,
                                                                RID_SVXSTR_BMP21,
                                                                pModel->GetBitmapList() );

        // if the given name is not valid, replace it!
        if( aUniqueName != GetName() )
        {
            return new XFillBitmapItem(aUniqueName, maGraphicObject);
        }
    }

    return (XFillBitmapItem*)this;
}

//////////////////////////////////////////////////////////////////////////////
// eof
