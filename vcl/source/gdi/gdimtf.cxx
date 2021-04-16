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

#include <cstdlib>
#include <memory>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <tools/diagnose_ex.h>
#include <tools/helpers.hxx>
#include <tools/stream.hxx>
#include <tools/vcompat.hxx>
#include <tools/fract.hxx>
#include <vcl/BitmapPalette.hxx>
#include <vcl/metaact.hxx>
#include <vcl/outdev.hxx>
#include <vcl/window.hxx>
#include <vcl/virdev.hxx>
#include <vcl/svapp.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/graphictools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/mtfxmldump.hxx>

#include <vcl/TypeSerializer.hxx>

#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <com/sun/star/rendering/MtfRenderer.hpp>
#include <com/sun/star/rendering/XBitmapCanvas.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <comphelper/processfactory.hxx>

using namespace com::sun::star;

namespace {

struct ImplColAdjustParam
{
    std::unique_ptr<sal_uInt8[]>  pMapR;
    std::unique_ptr<sal_uInt8[]>  pMapG;
    std::unique_ptr<sal_uInt8[]>  pMapB;
};

struct ImplBmpAdjustParam
{
    short   nLuminancePercent;
    short   nContrastPercent;
    short   nChannelRPercent;
    short   nChannelGPercent;
    short   nChannelBPercent;
    double  fGamma;
    bool    bInvert;
};

struct ImplColConvertParam
{
    MtfConversion   eConversion;
};

struct ImplBmpConvertParam
{
    BmpConversion   eConversion;
};

struct ImplColMonoParam
{
    Color aColor;
};

struct ImplBmpMonoParam
{
    Color aColor;
};

struct ImplColReplaceParam
{
    std::unique_ptr<sal_uLong[]>     pMinR;
    std::unique_ptr<sal_uLong[]>     pMaxR;
    std::unique_ptr<sal_uLong[]>     pMinG;
    std::unique_ptr<sal_uLong[]>     pMaxG;
    std::unique_ptr<sal_uLong[]>     pMinB;
    std::unique_ptr<sal_uLong[]>     pMaxB;
    const Color *                  pDstCols;
    sal_uLong                      nCount;
};

struct ImplBmpReplaceParam
{
    const Color*        pSrcCols;
    const Color*        pDstCols;
    sal_uLong           nCount;
};

}

GDIMetaFile::GDIMetaFile() :
    m_nCurrentActionElement( 0 ),
    m_aPrefSize   ( 1, 1 ),
    m_pPrev       ( nullptr ),
    m_pNext       ( nullptr ),
    m_pOutDev     ( nullptr ),
    m_bPause      ( false ),
    m_bRecord     ( false ),
    m_bUseCanvas  ( false )
{
}

GDIMetaFile::GDIMetaFile( const GDIMetaFile& rMtf ) :
    m_nCurrentActionElement( rMtf.m_nCurrentActionElement ),
    m_aPrefMapMode    ( rMtf.m_aPrefMapMode ),
    m_aPrefSize       ( rMtf.m_aPrefSize ),
    m_pPrev           ( rMtf.m_pPrev ),
    m_pNext           ( rMtf.m_pNext ),
    m_pOutDev         ( nullptr ),
    m_bPause          ( false ),
    m_bRecord         ( false ),
    m_bUseCanvas      ( rMtf.m_bUseCanvas )
{
    for( size_t i = 0, n = rMtf.GetActionSize(); i < n; ++i )
    {
        m_aList.push_back( rMtf.GetAction( i ) );
    }

    if( rMtf.m_bRecord )
    {
        Record( rMtf.m_pOutDev );

        if ( rMtf.m_bPause )
            Pause( true );
    }
}

GDIMetaFile::~GDIMetaFile()
{
    Clear();
}

bool GDIMetaFile::HasTransparentActions() const
{
    MetaAction* pCurrAct;

    // watch for transparent drawing actions
    for(pCurrAct = const_cast<GDIMetaFile*>(this)->FirstAction();
        pCurrAct;
        pCurrAct = const_cast<GDIMetaFile*>(this)->NextAction())
    {
        // #i10613# determine if the action is transparency capable

        // #107169# Also examine metafiles with masked bitmaps in
        // detail. Further down, this is optimized in such a way
        // that there's no unnecessary painting of masked bitmaps
        // (which are _always_ subdivided into rectangular regions
        // of uniform opacity): if a masked bitmap is printed over
        // empty background, we convert to a plain bitmap with
        // white background.
        if (pCurrAct->IsTransparent())
            return true;
    }

    return false;
}

size_t GDIMetaFile::GetActionSize() const
{
    return m_aList.size();
}

MetaAction* GDIMetaFile::GetAction( size_t nAction ) const
{
    return (nAction < m_aList.size()) ? m_aList[ nAction ].get() : nullptr;
}

MetaAction* GDIMetaFile::FirstAction()
{
    m_nCurrentActionElement = 0;
    return m_aList.empty() ? nullptr : m_aList[ 0 ].get();
}

MetaAction* GDIMetaFile::NextAction()
{
    return ( m_nCurrentActionElement + 1 < m_aList.size() ) ? m_aList[ ++m_nCurrentActionElement ].get() : nullptr;
}

void GDIMetaFile::ReplaceAction( rtl::Reference<MetaAction> pAction, size_t nAction )
{
    if ( nAction >= m_aList.size() )
    {
        return;
    }
    //fdo#39995 This doesn't increment the incoming action ref-count nor does it
    //decrement the outgoing action ref-count
    std::swap(pAction, m_aList[nAction]);
}

GDIMetaFile& GDIMetaFile::operator=( const GDIMetaFile& rMtf )
{
    if( this != &rMtf )
    {
        Clear();

        // Increment RefCount of MetaActions
        for( size_t i = 0, n = rMtf.GetActionSize(); i < n; ++i )
        {
            m_aList.push_back( rMtf.GetAction( i ) );
        }

        m_aPrefMapMode = rMtf.m_aPrefMapMode;
        m_aPrefSize = rMtf.m_aPrefSize;
        m_pPrev = rMtf.m_pPrev;
        m_pNext = rMtf.m_pNext;
        m_pOutDev = nullptr;
        m_bPause = false;
        m_bRecord = false;
        m_bUseCanvas = rMtf.m_bUseCanvas;

        if( rMtf.m_bRecord )
        {
            Record( rMtf.m_pOutDev );

            if( rMtf.m_bPause )
                Pause( true );
        }
    }

    return *this;
}

bool GDIMetaFile::operator==( const GDIMetaFile& rMtf ) const
{
    const size_t    nObjCount = m_aList.size();
    bool        bRet = false;

    if( this == &rMtf )
        bRet = true;
    else if( rMtf.GetActionSize()  == nObjCount &&
             rMtf.GetPrefSize()    == m_aPrefSize &&
             rMtf.GetPrefMapMode() == m_aPrefMapMode )
    {
        bRet = true;

        for( size_t n = 0; n < nObjCount; n++ )
        {
            if( m_aList[ n ] != rMtf.GetAction( n ) )
            {
                bRet = false;
                break;
            }
        }
    }

    return bRet;
}

void GDIMetaFile::Clear()
{
    if( m_bRecord )
        Stop();

    m_aList.clear();
}

void GDIMetaFile::Linker( OutputDevice* pOut, bool bLink )
{
    if( bLink )
    {
        m_pNext = nullptr;
        m_pPrev = pOut->GetConnectMetaFile();
        pOut->SetConnectMetaFile( this );

        if( m_pPrev )
            m_pPrev->m_pNext = this;
    }
    else
    {
        if( m_pNext )
        {
            m_pNext->m_pPrev = m_pPrev;

            if( m_pPrev )
                m_pPrev->m_pNext = m_pNext;
        }
        else
        {
            if( m_pPrev )
                m_pPrev->m_pNext = nullptr;

            pOut->SetConnectMetaFile( m_pPrev );
        }

        m_pPrev = nullptr;
        m_pNext = nullptr;
    }
}

void GDIMetaFile::Record( OutputDevice* pOut )
{
    if( m_bRecord )
        Stop();

    m_nCurrentActionElement = m_aList.empty() ? 0 : (m_aList.size() - 1);
    m_pOutDev = pOut;
    m_bRecord = true;
    Linker( pOut, true );
}

void GDIMetaFile::Play( GDIMetaFile& rMtf )
{
    if (m_bRecord || rMtf.m_bRecord)
        return;

    MetaAction* pAction = GetCurAction();
    const size_t nObjCount = m_aList.size();

    rMtf.UseCanvas( rMtf.GetUseCanvas() || m_bUseCanvas );

    for( size_t nCurPos = m_nCurrentActionElement; nCurPos < nObjCount; nCurPos++ )
    {
        if( pAction )
        {
            rMtf.AddAction( pAction );
        }

        pAction = NextAction();
    }
}

void GDIMetaFile::Play(OutputDevice& rOut, size_t nPos)
{
    if( m_bRecord )
        return;

    MetaAction* pAction = GetCurAction();
    const size_t nObjCount = m_aList.size();
    size_t  nSyncCount = rOut.GetSyncCount();

    if( nPos > nObjCount )
        nPos = nObjCount;

    // #i23407# Set backwards-compatible text language and layout mode
    // This is necessary, since old metafiles don't even know of these
    // recent add-ons. Newer metafiles must of course explicitly set
    // those states.
    rOut.Push(vcl::PushFlags::TEXTLAYOUTMODE|vcl::PushFlags::TEXTLANGUAGE);
    rOut.SetLayoutMode(vcl::text::ComplexTextLayoutFlags::Default);
    rOut.SetDigitLanguage(LANGUAGE_SYSTEM);

    SAL_INFO( "vcl.gdi", "GDIMetaFile::Play on device of size: " << rOut.GetOutputSizePixel().Width() << " " << rOut.GetOutputSizePixel().Height());

    if (!ImplPlayWithRenderer(rOut, Point(0,0), rOut.GetOutputSize())) {
        size_t  i  = 0;
        for( size_t nCurPos = m_nCurrentActionElement; nCurPos < nPos; nCurPos++ )
        {
            if( pAction )
            {
                pAction->Execute(&rOut);

                // flush output from time to time
                if( i++ > nSyncCount )
                {
                    rOut.Flush();
                    i = 0;
                }
            }

            pAction = NextAction();
        }
    }
    rOut.Pop();
}

bool GDIMetaFile::ImplPlayWithRenderer(OutputDevice& rOut, const Point& rPos, Size rLogicDestSize)
{
    if (!m_bUseCanvas)
        return false;

    Size rDestSize(rOut.LogicToPixel(rLogicDestSize));

    const vcl::Window* win = rOut.GetOwnerWindow();

    if (!win)
        win = Application::GetActiveTopWindow();
    if (!win)
        win = Application::GetFirstTopLevelWindow();

    if (!win)
        return false;

    try
    {
        uno::Reference<rendering::XCanvas> xCanvas = win->GetOutDev()->GetCanvas ();

        if (!xCanvas.is())
            return false;

        Size aSize (rDestSize.Width () + 1, rDestSize.Height () + 1);
        uno::Reference<rendering::XBitmap> xBitmap = xCanvas->getDevice ()->createCompatibleAlphaBitmap (vcl::unotools::integerSize2DFromSize( aSize));
        if( xBitmap.is () )
        {
            uno::Reference< rendering::XBitmapCanvas > xBitmapCanvas( xBitmap, uno::UNO_QUERY );
            if( xBitmapCanvas.is() )
            {
                uno::Reference< uno::XComponentContext > xContext = comphelper::getProcessComponentContext();
                uno::Reference< rendering::XMtfRenderer > xMtfRenderer = rendering::MtfRenderer::createWithBitmapCanvas( xContext, xBitmapCanvas );

                xBitmapCanvas->clear();
                uno::Reference< beans::XFastPropertySet > xMtfFastPropertySet( xMtfRenderer, uno::UNO_QUERY );
                if( xMtfFastPropertySet.is() )
                    // set this metafile to the renderer to
                    // speedup things (instead of copying data to
                    // sequence of bytes passed to renderer)
                    xMtfFastPropertySet->setFastPropertyValue( 0, uno::Any( reinterpret_cast<sal_Int64>( this ) ) );

                xMtfRenderer->draw( rDestSize.Width(), rDestSize.Height() );

                BitmapEx aBitmapEx;
                if( aBitmapEx.Create( xBitmapCanvas, aSize ) )
                {
                    if (rOut.GetMapMode().GetMapUnit() == MapUnit::MapPixel)
                        rOut.DrawBitmapEx( rPos, aBitmapEx );
                    else
                        rOut.DrawBitmapEx( rPos, rLogicDestSize, aBitmapEx );
                    return true;
                }
            }
        }
    }
    catch (const uno::RuntimeException& )
    {
        throw; // runtime errors are fatal
    }
    catch (const uno::Exception&)
    {
        // ignore errors, no way of reporting them here
        TOOLS_WARN_EXCEPTION("vcl.gdi", "GDIMetaFile::ImplPlayWithRenderer");
    }

    return false;
}

void GDIMetaFile::Play(OutputDevice& rOut, const Point& rPos,
                       const Size& rSize)
{
    MapMode aDrawMap( GetPrefMapMode() );
    Size    aDestSize(rOut.LogicToPixel(rSize));

    if( !aDestSize.Width() || !aDestSize.Height() )
        return;

    GDIMetaFile* pMtf = rOut.GetConnectMetaFile();

    if (ImplPlayWithRenderer(rOut, rPos, rSize))
        return;

    Size aTmpPrefSize(rOut.LogicToPixel(GetPrefSize(), aDrawMap));

    if( !aTmpPrefSize.Width() )
        aTmpPrefSize.setWidth( aDestSize.Width() );

    if( !aTmpPrefSize.Height() )
        aTmpPrefSize.setHeight( aDestSize.Height() );

    Fraction aScaleX( aDestSize.Width(), aTmpPrefSize.Width() );
    Fraction aScaleY( aDestSize.Height(), aTmpPrefSize.Height() );

    aScaleX *= aDrawMap.GetScaleX(); aDrawMap.SetScaleX( aScaleX );
    aScaleY *= aDrawMap.GetScaleY(); aDrawMap.SetScaleY( aScaleY );

    // #i47260# Convert logical output position to offset within
    // the metafile's mapmode. Therefore, disable pixel offset on
    // outdev, it's inverse mnOutOffLogicX/Y is calculated for a
    // different mapmode (the one currently set on rOut, that is)
    // - thus, aDrawMap's origin would generally be wrong. And
    // even _if_ aDrawMap is similar to pOutDev's current mapmode,
    // it's _still_ undesirable to have pixel offset unequal zero,
    // because one would still get round-off errors (the
    // round-trip error for LogicToPixel( PixelToLogic() ) was the
    // reason for having pixel offset in the first place).
    const Size& rOldOffset(rOut.GetPixelOffset());
    const Size  aEmptySize;
    rOut.SetPixelOffset(aEmptySize);
    aDrawMap.SetOrigin(rOut.PixelToLogic(rOut.LogicToPixel(rPos), aDrawMap));
    rOut.SetPixelOffset(rOldOffset);

    rOut.Push();

    bool bIsRecord = (pMtf && pMtf->IsRecord());
    rOut.SetMetafileMapMode(aDrawMap, bIsRecord);

    // #i23407# Set backwards-compatible text language and layout mode
    // This is necessary, since old metafiles don't even know of these
    // recent add-ons. Newer metafiles must of course explicitly set
    // those states.
    rOut.SetLayoutMode(vcl::text::ComplexTextLayoutFlags::Default);
    rOut.SetDigitLanguage(LANGUAGE_SYSTEM);

    Play(rOut);

    rOut.Pop();
}

void GDIMetaFile::Pause( bool _bPause )
{
    if( !m_bRecord )
        return;

    if( _bPause )
    {
        if( !m_bPause )
            Linker( m_pOutDev, false );
    }
    else
    {
        if( m_bPause )
            Linker( m_pOutDev, true );
    }

    m_bPause = _bPause;
}

void GDIMetaFile::Stop()
{
    if( m_bRecord )
    {
        m_bRecord = false;

        if( !m_bPause )
            Linker( m_pOutDev, false );
        else
            m_bPause = false;
    }
}

void GDIMetaFile::WindStart()
{
    if( !m_bRecord )
        m_nCurrentActionElement = 0;
}

void GDIMetaFile::WindPrev()
{
    if( !m_bRecord )
        if ( m_nCurrentActionElement > 0 )
            --m_nCurrentActionElement;
}

void GDIMetaFile::AddAction(const rtl::Reference<MetaAction>& pAction)
{
    m_aList.push_back( pAction );

    if( m_pPrev )
    {
        m_pPrev->AddAction( pAction );
    }
}

void GDIMetaFile::AddAction(const rtl::Reference<MetaAction>& pAction, size_t nPos)
{
    if ( nPos < m_aList.size() )
    {
        m_aList.insert( m_aList.begin() + nPos, pAction );
    }
    else
    {
        m_aList.push_back( pAction );
    }

    if( m_pPrev )
    {
        m_pPrev->AddAction( pAction, nPos );
    }
}

void GDIMetaFile::push_back(const rtl::Reference<MetaAction>& pAction)
{
    m_aList.push_back( pAction );
}

void GDIMetaFile::Mirror( BmpMirrorFlags nMirrorFlags )
{
    const Size  aOldPrefSize( GetPrefSize() );
    tools::Long        nMoveX, nMoveY;
    double      fScaleX, fScaleY;

    if( nMirrorFlags & BmpMirrorFlags::Horizontal )
    {
        nMoveX = std::abs( aOldPrefSize.Width() ) - 1;
        fScaleX = -1.0;
    }
    else
    {
        nMoveX = 0;
        fScaleX = 1.0;
    }

    if( nMirrorFlags & BmpMirrorFlags::Vertical )
    {
        nMoveY = std::abs( aOldPrefSize.Height() ) - 1;
        fScaleY = -1.0;
    }
    else
    {
        nMoveY = 0;
        fScaleY = 1.0;
    }

    if( ( fScaleX != 1.0 ) || ( fScaleY != 1.0 ) )
    {
        Scale( fScaleX, fScaleY );
        Move( nMoveX, nMoveY );
        SetPrefSize( aOldPrefSize );
    }
}

void GDIMetaFile::Move( tools::Long nX, tools::Long nY )
{
    const Size      aBaseOffset( nX, nY );
    Size            aOffset( aBaseOffset );
    ScopedVclPtrInstance< VirtualDevice > aMapVDev;

    aMapVDev->EnableOutput( false );
    aMapVDev->SetMapMode( GetPrefMapMode() );

    for( MetaAction* pAct = FirstAction(); pAct; pAct = NextAction() )
    {
        const MetaActionType nType = pAct->GetType();
        MetaAction* pModAct;

        if( pAct->GetRefCount() > 1 )
        {
            m_aList[ m_nCurrentActionElement ] = pAct->Clone();
            pModAct = m_aList[ m_nCurrentActionElement ].get();
        }
        else
            pModAct = pAct;

        if( ( MetaActionType::MAPMODE == nType ) ||
            ( MetaActionType::PUSH == nType ) ||
            ( MetaActionType::POP == nType ) )
        {
            pModAct->Execute( aMapVDev.get() );
            aOffset = OutputDevice::LogicToLogic( aBaseOffset, GetPrefMapMode(), aMapVDev->GetMapMode() );
        }

        pModAct->Move( aOffset.Width(), aOffset.Height() );
    }
}

void GDIMetaFile::Move( tools::Long nX, tools::Long nY, tools::Long nDPIX, tools::Long nDPIY )
{
    const Size      aBaseOffset( nX, nY );
    Size            aOffset( aBaseOffset );
    ScopedVclPtrInstance< VirtualDevice > aMapVDev;

    aMapVDev->EnableOutput( false );
    aMapVDev->SetReferenceDevice( nDPIX, nDPIY );
    aMapVDev->SetMapMode( GetPrefMapMode() );

    for( MetaAction* pAct = FirstAction(); pAct; pAct = NextAction() )
    {
        const MetaActionType nType = pAct->GetType();
        MetaAction* pModAct;

        if( pAct->GetRefCount() > 1 )
        {
            m_aList[ m_nCurrentActionElement ] = pAct->Clone();
            pModAct = m_aList[ m_nCurrentActionElement ].get();
        }
        else
            pModAct = pAct;

        if( ( MetaActionType::MAPMODE == nType ) ||
            ( MetaActionType::PUSH == nType ) ||
            ( MetaActionType::POP == nType ) )
        {
            pModAct->Execute( aMapVDev.get() );
            if( aMapVDev->GetMapMode().GetMapUnit() == MapUnit::MapPixel )
            {
                aOffset = aMapVDev->LogicToPixel( aBaseOffset, GetPrefMapMode() );
                MapMode aMap( aMapVDev->GetMapMode() );
                aOffset.setWidth( static_cast<tools::Long>(aOffset.Width() * static_cast<double>(aMap.GetScaleX())) );
                aOffset.setHeight( static_cast<tools::Long>(aOffset.Height() * static_cast<double>(aMap.GetScaleY())) );
            }
            else
                aOffset = OutputDevice::LogicToLogic( aBaseOffset, GetPrefMapMode(), aMapVDev->GetMapMode() );
        }

        pModAct->Move( aOffset.Width(), aOffset.Height() );
    }
}

void GDIMetaFile::Scale( double fScaleX, double fScaleY )
{
    for( MetaAction* pAct = FirstAction(); pAct; pAct = NextAction() )
    {
        MetaAction* pModAct;

        if( pAct->GetRefCount() > 1 )
        {
            m_aList[ m_nCurrentActionElement ] = pAct->Clone();
            pModAct = m_aList[ m_nCurrentActionElement ].get();
        }
        else
            pModAct = pAct;

        pModAct->Scale( fScaleX, fScaleY );
    }

    m_aPrefSize.setWidth( FRound( m_aPrefSize.Width() * fScaleX ) );
    m_aPrefSize.setHeight( FRound( m_aPrefSize.Height() * fScaleY ) );
}

void GDIMetaFile::Scale( const Fraction& rScaleX, const Fraction& rScaleY )
{
    Scale( static_cast<double>(rScaleX), static_cast<double>(rScaleY) );
}

void GDIMetaFile::Clip( const tools::Rectangle& i_rClipRect )
{
    tools::Rectangle aCurRect( i_rClipRect );
    ScopedVclPtrInstance< VirtualDevice > aMapVDev;

    aMapVDev->EnableOutput( false );
    aMapVDev->SetMapMode( GetPrefMapMode() );

    for( MetaAction* pAct = FirstAction(); pAct; pAct = NextAction() )
    {
        const MetaActionType nType = pAct->GetType();

        if( ( MetaActionType::MAPMODE == nType ) ||
            ( MetaActionType::PUSH == nType ) ||
            ( MetaActionType::POP == nType ) )
        {
            pAct->Execute( aMapVDev.get() );
            aCurRect = OutputDevice::LogicToLogic( i_rClipRect, GetPrefMapMode(), aMapVDev->GetMapMode() );
        }
        else if( nType == MetaActionType::CLIPREGION )
        {
            MetaClipRegionAction* pOldAct = static_cast<MetaClipRegionAction*>(pAct);
            vcl::Region aNewReg( aCurRect );
            if( pOldAct->IsClipping() )
                aNewReg.Intersect( pOldAct->GetRegion() );
            MetaClipRegionAction* pNewAct = new MetaClipRegionAction( aNewReg, true );
            m_aList[ m_nCurrentActionElement ] = pNewAct;
        }
    }
}

Point GDIMetaFile::ImplGetRotatedPoint( const Point& rPt, const Point& rRotatePt,
                                        const Size& rOffset, double fSin, double fCos )
{
    const tools::Long nX = rPt.X() - rRotatePt.X();
    const tools::Long nY = rPt.Y() - rRotatePt.Y();

    return Point( FRound( fCos * nX + fSin * nY ) + rRotatePt.X() + rOffset.Width(),
                  -FRound( fSin * nX - fCos * nY ) + rRotatePt.Y() + rOffset.Height() );
}

tools::Polygon GDIMetaFile::ImplGetRotatedPolygon( const tools::Polygon& rPoly, const Point& rRotatePt,
                                                   const Size& rOffset, double fSin, double fCos )
{
    tools::Polygon aRet( rPoly );

    aRet.Rotate( rRotatePt, fSin, fCos );
    aRet.Move( rOffset.Width(), rOffset.Height() );

    return aRet;
}

tools::PolyPolygon GDIMetaFile::ImplGetRotatedPolyPolygon( const tools::PolyPolygon& rPolyPoly, const Point& rRotatePt,
                                                    const Size& rOffset, double fSin, double fCos )
{
    tools::PolyPolygon aRet( rPolyPoly );

    aRet.Rotate( rRotatePt, fSin, fCos );
    aRet.Move( rOffset.Width(), rOffset.Height() );

    return aRet;
}

void GDIMetaFile::ImplAddGradientEx( GDIMetaFile&         rMtf,
                                     const OutputDevice&  rMapDev,
                                     const tools::PolyPolygon&   rPolyPoly,
                                     const Gradient&      rGrad     )
{
    // Generate comment, GradientEx and Gradient actions (within DrawGradient)
    ScopedVclPtrInstance< VirtualDevice > aVDev(rMapDev, DeviceFormat::DEFAULT);
    aVDev->EnableOutput( false );
    GDIMetaFile aGradMtf;

    aGradMtf.Record( aVDev.get() );
    aVDev->DrawGradient( rPolyPoly, rGrad );
    aGradMtf.Stop();

    size_t i, nAct( aGradMtf.GetActionSize() );
    for( i=0; i < nAct; ++i )
    {
        MetaAction* pMetaAct = aGradMtf.GetAction( i );
        rMtf.AddAction( pMetaAct );
    }
}

void GDIMetaFile::Rotate( Degree10 nAngle10 )
{
    nAngle10 %= 3600_deg10;
    nAngle10 = ( nAngle10 < 0_deg10 ) ? ( Degree10(3599) + nAngle10 ) : nAngle10;

    if( !nAngle10 )
        return;

    GDIMetaFile     aMtf;
    ScopedVclPtrInstance< VirtualDevice > aMapVDev;
    const double    fAngle = toRadians(nAngle10);
    const double    fSin = sin( fAngle );
    const double    fCos = cos( fAngle );
    tools::Rectangle aRect( Point(), GetPrefSize() );
    tools::Polygon aPoly( aRect );

    aPoly.Rotate( Point(), fSin, fCos );

    aMapVDev->EnableOutput( false );
    aMapVDev->SetMapMode( GetPrefMapMode() );

    const tools::Rectangle aNewBound( aPoly.GetBoundRect() );

    const Point aOrigin( GetPrefMapMode().GetOrigin().X(), GetPrefMapMode().GetOrigin().Y() );
    const Size  aOffset( -aNewBound.Left(), -aNewBound.Top() );

    Point     aRotAnchor( aOrigin );
    Size      aRotOffset( aOffset );

    for( MetaAction* pAction = FirstAction(); pAction; pAction = NextAction() )
    {
        const MetaActionType nActionType = pAction->GetType();

        switch( nActionType )
        {
            case MetaActionType::PIXEL:
            {
                MetaPixelAction* pAct = static_cast<MetaPixelAction*>(pAction);
                aMtf.AddAction( new MetaPixelAction( ImplGetRotatedPoint( pAct->GetPoint(), aRotAnchor, aRotOffset, fSin, fCos ),
                                                                          pAct->GetColor() ) );
            }
            break;

            case MetaActionType::POINT:
            {
                MetaPointAction* pAct = static_cast<MetaPointAction*>(pAction);
                aMtf.AddAction( new MetaPointAction( ImplGetRotatedPoint( pAct->GetPoint(), aRotAnchor, aRotOffset, fSin, fCos ) ) );
            }
            break;

            case MetaActionType::LINE:
            {
                MetaLineAction* pAct = static_cast<MetaLineAction*>(pAction);
                aMtf.AddAction( new MetaLineAction( ImplGetRotatedPoint( pAct->GetStartPoint(), aRotAnchor, aRotOffset, fSin, fCos ),
                                                    ImplGetRotatedPoint( pAct->GetEndPoint(), aRotAnchor, aRotOffset, fSin, fCos ),
                                                    pAct->GetLineInfo() ) );
            }
            break;

            case MetaActionType::RECT:
            {
                MetaRectAction* pAct = static_cast<MetaRectAction*>(pAction);
                aMtf.AddAction( new MetaPolygonAction( ImplGetRotatedPolygon( pAct->GetRect(), aRotAnchor, aRotOffset, fSin, fCos ) ) );
            }
            break;

            case MetaActionType::ROUNDRECT:
            {
                MetaRoundRectAction*    pAct = static_cast<MetaRoundRectAction*>(pAction);
                const tools::Polygon aRoundRectPoly( pAct->GetRect(), pAct->GetHorzRound(), pAct->GetVertRound() );

                aMtf.AddAction( new MetaPolygonAction( ImplGetRotatedPolygon( aRoundRectPoly, aRotAnchor, aRotOffset, fSin, fCos ) ) );
            }
            break;

            case MetaActionType::ELLIPSE:
            {
                MetaEllipseAction*      pAct = static_cast<MetaEllipseAction*>(pAction);
                const tools::Polygon aEllipsePoly( pAct->GetRect().Center(), pAct->GetRect().GetWidth() >> 1, pAct->GetRect().GetHeight() >> 1 );

                aMtf.AddAction( new MetaPolygonAction( ImplGetRotatedPolygon( aEllipsePoly, aRotAnchor, aRotOffset, fSin, fCos ) ) );
            }
            break;

            case MetaActionType::ARC:
            {
                MetaArcAction*  pAct = static_cast<MetaArcAction*>(pAction);
                const tools::Polygon aArcPoly( pAct->GetRect(), pAct->GetStartPoint(), pAct->GetEndPoint(), PolyStyle::Arc );

                aMtf.AddAction( new MetaPolygonAction( ImplGetRotatedPolygon( aArcPoly, aRotAnchor, aRotOffset, fSin, fCos ) ) );
            }
            break;

            case MetaActionType::PIE:
            {
                MetaPieAction*  pAct = static_cast<MetaPieAction*>(pAction);
                const tools::Polygon aPiePoly( pAct->GetRect(), pAct->GetStartPoint(), pAct->GetEndPoint(), PolyStyle::Pie );

                aMtf.AddAction( new MetaPolygonAction( ImplGetRotatedPolygon( aPiePoly, aRotAnchor, aRotOffset, fSin, fCos ) ) );
            }
            break;

            case MetaActionType::CHORD:
            {
                MetaChordAction*    pAct = static_cast<MetaChordAction*>(pAction);
                const tools::Polygon aChordPoly( pAct->GetRect(), pAct->GetStartPoint(), pAct->GetEndPoint(), PolyStyle::Chord );

                aMtf.AddAction( new MetaPolygonAction( ImplGetRotatedPolygon( aChordPoly, aRotAnchor, aRotOffset, fSin, fCos ) ) );
            }
            break;

            case MetaActionType::POLYLINE:
            {
                MetaPolyLineAction* pAct = static_cast<MetaPolyLineAction*>(pAction);
                aMtf.AddAction( new MetaPolyLineAction( ImplGetRotatedPolygon( pAct->GetPolygon(), aRotAnchor, aRotOffset, fSin, fCos ), pAct->GetLineInfo() ) );
            }
            break;

            case MetaActionType::POLYGON:
            {
                MetaPolygonAction* pAct = static_cast<MetaPolygonAction*>(pAction);
                aMtf.AddAction( new MetaPolygonAction( ImplGetRotatedPolygon( pAct->GetPolygon(), aRotAnchor, aRotOffset, fSin, fCos ) ) );
            }
            break;

            case MetaActionType::POLYPOLYGON:
            {
                MetaPolyPolygonAction* pAct = static_cast<MetaPolyPolygonAction*>(pAction);
                aMtf.AddAction( new MetaPolyPolygonAction( ImplGetRotatedPolyPolygon( pAct->GetPolyPolygon(), aRotAnchor, aRotOffset, fSin, fCos ) ) );
            }
            break;

            case MetaActionType::TEXT:
            {
                MetaTextAction* pAct = static_cast<MetaTextAction*>(pAction);
                aMtf.AddAction( new MetaTextAction( ImplGetRotatedPoint( pAct->GetPoint(), aRotAnchor, aRotOffset, fSin, fCos ),
                                                                         pAct->GetText(), pAct->GetIndex(), pAct->GetLen() ) );
            }
            break;

            case MetaActionType::TEXTARRAY:
            {
                MetaTextArrayAction* pAct = static_cast<MetaTextArrayAction*>(pAction);
                aMtf.AddAction( new MetaTextArrayAction( ImplGetRotatedPoint( pAct->GetPoint(), aRotAnchor, aRotOffset, fSin, fCos ),
                                                                              pAct->GetText(), pAct->GetDXArray(), pAct->GetIndex(), pAct->GetLen() ) );
            }
            break;

            case MetaActionType::STRETCHTEXT:
            {
                MetaStretchTextAction* pAct = static_cast<MetaStretchTextAction*>(pAction);
                aMtf.AddAction( new MetaStretchTextAction( ImplGetRotatedPoint( pAct->GetPoint(), aRotAnchor, aRotOffset, fSin, fCos ),
                                                                                pAct->GetWidth(), pAct->GetText(), pAct->GetIndex(), pAct->GetLen() ) );
            }
            break;

            case MetaActionType::TEXTLINE:
            {
                MetaTextLineAction* pAct = static_cast<MetaTextLineAction*>(pAction);
                aMtf.AddAction( new MetaTextLineAction( ImplGetRotatedPoint( pAct->GetStartPoint(), aRotAnchor, aRotOffset, fSin, fCos ),
                                                                             pAct->GetWidth(), pAct->GetStrikeout(), pAct->GetUnderline(), pAct->GetOverline() ) );
            }
            break;

            case MetaActionType::BMPSCALE:
            {
                MetaBmpScaleAction* pAct = static_cast<MetaBmpScaleAction*>(pAction);
                tools::Polygon aBmpPoly( ImplGetRotatedPolygon( tools::Rectangle( pAct->GetPoint(), pAct->GetSize() ), aRotAnchor, aRotOffset, fSin, fCos ) );
                tools::Rectangle           aBmpRect( aBmpPoly.GetBoundRect() );
                BitmapEx            aBmpEx( pAct->GetBitmap() );

                aBmpEx.Rotate( nAngle10, COL_TRANSPARENT );
                aMtf.AddAction( new MetaBmpExScaleAction( aBmpRect.TopLeft(), aBmpRect.GetSize(),
                                                          aBmpEx ) );
            }
            break;

            case MetaActionType::BMPSCALEPART:
            {
                MetaBmpScalePartAction* pAct = static_cast<MetaBmpScalePartAction*>(pAction);
                tools::Polygon aBmpPoly( ImplGetRotatedPolygon( tools::Rectangle( pAct->GetDestPoint(), pAct->GetDestSize() ), aRotAnchor, aRotOffset, fSin, fCos ) );
                tools::Rectangle               aBmpRect( aBmpPoly.GetBoundRect() );
                BitmapEx                aBmpEx( pAct->GetBitmap() );

                aBmpEx.Crop( tools::Rectangle( pAct->GetSrcPoint(), pAct->GetSrcSize() ) );
                aBmpEx.Rotate( nAngle10, COL_TRANSPARENT );

                aMtf.AddAction( new MetaBmpExScaleAction( aBmpRect.TopLeft(), aBmpRect.GetSize(), aBmpEx ) );
            }
            break;

            case MetaActionType::BMPEXSCALE:
            {
                MetaBmpExScaleAction*   pAct = static_cast<MetaBmpExScaleAction*>(pAction);
                tools::Polygon aBmpPoly( ImplGetRotatedPolygon( tools::Rectangle( pAct->GetPoint(), pAct->GetSize() ), aRotAnchor, aRotOffset, fSin, fCos ) );
                tools::Rectangle               aBmpRect( aBmpPoly.GetBoundRect() );
                BitmapEx                aBmpEx( pAct->GetBitmapEx() );

                aBmpEx.Rotate( nAngle10, COL_TRANSPARENT );

                aMtf.AddAction( new MetaBmpExScaleAction( aBmpRect.TopLeft(), aBmpRect.GetSize(), aBmpEx ) );
            }
            break;

            case MetaActionType::BMPEXSCALEPART:
            {
                MetaBmpExScalePartAction*   pAct = static_cast<MetaBmpExScalePartAction*>(pAction);
                tools::Polygon aBmpPoly( ImplGetRotatedPolygon( tools::Rectangle( pAct->GetDestPoint(), pAct->GetDestSize() ), aRotAnchor, aRotOffset, fSin, fCos ) );
                tools::Rectangle                   aBmpRect( aBmpPoly.GetBoundRect() );
                BitmapEx                    aBmpEx( pAct->GetBitmapEx() );

                aBmpEx.Crop( tools::Rectangle( pAct->GetSrcPoint(), pAct->GetSrcSize() ) );
                aBmpEx.Rotate( nAngle10, COL_TRANSPARENT );

                aMtf.AddAction( new MetaBmpExScaleAction( aBmpRect.TopLeft(), aBmpRect.GetSize(), aBmpEx ) );
            }
            break;

            case MetaActionType::GRADIENT:
            {
                MetaGradientAction* pAct = static_cast<MetaGradientAction*>(pAction);

                ImplAddGradientEx( aMtf, *aMapVDev,
                                   ImplGetRotatedPolygon( pAct->GetRect(), aRotAnchor, aRotOffset, fSin, fCos ),
                                   pAct->GetGradient() );
            }
            break;

            case MetaActionType::GRADIENTEX:
            {
                MetaGradientExAction* pAct = static_cast<MetaGradientExAction*>(pAction);
                aMtf.AddAction( new MetaGradientExAction( ImplGetRotatedPolyPolygon( pAct->GetPolyPolygon(), aRotAnchor, aRotOffset, fSin, fCos ),
                                                          pAct->GetGradient() ) );
            }
            break;

            // Handle gradientex comment block correctly
            case MetaActionType::COMMENT:
            {
                MetaCommentAction* pCommentAct = static_cast<MetaCommentAction*>(pAction);
                if( pCommentAct->GetComment() == "XGRAD_SEQ_BEGIN" )
                {
                    int nBeginComments( 1 );
                    pAction = NextAction();

                    // skip everything, except gradientex action
                    while( pAction )
                    {
                        const MetaActionType nType = pAction->GetType();

                        if( MetaActionType::GRADIENTEX == nType )
                        {
                            // Add rotated gradientex
                            MetaGradientExAction* pAct = static_cast<MetaGradientExAction*>(pAction);
                            ImplAddGradientEx( aMtf, *aMapVDev,
                                               ImplGetRotatedPolyPolygon( pAct->GetPolyPolygon(), aRotAnchor, aRotOffset, fSin, fCos ),
                                               pAct->GetGradient() );
                        }
                        else if( MetaActionType::COMMENT == nType)
                        {
                            MetaCommentAction* pAct = static_cast<MetaCommentAction*>(pAction);
                            if( pAct->GetComment() == "XGRAD_SEQ_END" )
                            {
                                // handle nested blocks
                                --nBeginComments;

                                // gradientex comment block: end reached, done.
                                if( !nBeginComments )
                                    break;
                            }
                            else if( pAct->GetComment() == "XGRAD_SEQ_BEGIN" )
                            {
                                // handle nested blocks
                                ++nBeginComments;
                            }

                        }

                        pAction =NextAction();
                    }
                }
                else
                {
                    bool bPathStroke = (pCommentAct->GetComment() == "XPATHSTROKE_SEQ_BEGIN");
                    if ( bPathStroke || pCommentAct->GetComment() == "XPATHFILL_SEQ_BEGIN" )
                    {
                        if ( pCommentAct->GetDataSize() )
                        {
                            SvMemoryStream aMemStm( const_cast<sal_uInt8 *>(pCommentAct->GetData()), pCommentAct->GetDataSize(), StreamMode::READ );
                            SvMemoryStream aDest;
                            if ( bPathStroke )
                            {
                                SvtGraphicStroke aStroke;
                                ReadSvtGraphicStroke( aMemStm, aStroke );
                                tools::Polygon aPath;
                                aStroke.getPath( aPath );
                                aStroke.setPath( ImplGetRotatedPolygon( aPath, aRotAnchor, aRotOffset, fSin, fCos ) );
                                WriteSvtGraphicStroke( aDest, aStroke );
                                aMtf.AddAction( new MetaCommentAction( "XPATHSTROKE_SEQ_BEGIN", 0,
                                                    static_cast<const sal_uInt8*>( aDest.GetData()), aDest.Tell() ) );
                            }
                            else
                            {
                                SvtGraphicFill aFill;
                                ReadSvtGraphicFill( aMemStm, aFill );
                                tools::PolyPolygon aPath;
                                aFill.getPath( aPath );
                                aFill.setPath( ImplGetRotatedPolyPolygon( aPath, aRotAnchor, aRotOffset, fSin, fCos ) );
                                WriteSvtGraphicFill( aDest, aFill );
                                aMtf.AddAction( new MetaCommentAction( "XPATHFILL_SEQ_BEGIN", 0,
                                                    static_cast<const sal_uInt8*>( aDest.GetData()), aDest.Tell() ) );
                            }
                        }
                    }
                    else if ( pCommentAct->GetComment() == "XPATHSTROKE_SEQ_END"
                           || pCommentAct->GetComment() == "XPATHFILL_SEQ_END" )
                    {
                        pAction->Execute( aMapVDev.get() );
                        aMtf.AddAction( pAction );
                    }
                }
            }
            break;

            case MetaActionType::HATCH:
            {
                MetaHatchAction*    pAct = static_cast<MetaHatchAction*>(pAction);
                Hatch               aHatch( pAct->GetHatch() );

                aHatch.SetAngle( aHatch.GetAngle() + nAngle10 );
                aMtf.AddAction( new MetaHatchAction( ImplGetRotatedPolyPolygon( pAct->GetPolyPolygon(), aRotAnchor, aRotOffset, fSin, fCos ),
                                                                                aHatch ) );
            }
            break;

            case MetaActionType::Transparent:
            {
                MetaTransparentAction* pAct = static_cast<MetaTransparentAction*>(pAction);
                aMtf.AddAction( new MetaTransparentAction( ImplGetRotatedPolyPolygon( pAct->GetPolyPolygon(), aRotAnchor, aRotOffset, fSin, fCos ),
                                                                                      pAct->GetTransparence() ) );
            }
            break;

            case MetaActionType::FLOATTRANSPARENT:
            {
                MetaFloatTransparentAction* pAct = static_cast<MetaFloatTransparentAction*>(pAction);
                GDIMetaFile                 aTransMtf( pAct->GetGDIMetaFile() );
                tools::Polygon aMtfPoly( ImplGetRotatedPolygon( tools::Rectangle( pAct->GetPoint(), pAct->GetSize() ), aRotAnchor, aRotOffset, fSin, fCos ) );
                tools::Rectangle                   aMtfRect( aMtfPoly.GetBoundRect() );

                aTransMtf.Rotate( nAngle10 );
                aMtf.AddAction( new MetaFloatTransparentAction( aTransMtf, aMtfRect.TopLeft(), aMtfRect.GetSize(),
                                                                pAct->GetGradient() ) );
            }
            break;

            case MetaActionType::EPS:
            {
                MetaEPSAction*  pAct = static_cast<MetaEPSAction*>(pAction);
                GDIMetaFile     aEPSMtf( pAct->GetSubstitute() );
                tools::Polygon aEPSPoly( ImplGetRotatedPolygon( tools::Rectangle( pAct->GetPoint(), pAct->GetSize() ), aRotAnchor, aRotOffset, fSin, fCos ) );
                tools::Rectangle       aEPSRect( aEPSPoly.GetBoundRect() );

                aEPSMtf.Rotate( nAngle10 );
                aMtf.AddAction( new MetaEPSAction( aEPSRect.TopLeft(), aEPSRect.GetSize(),
                                                   pAct->GetLink(), aEPSMtf ) );
            }
            break;

            case MetaActionType::CLIPREGION:
            {
                MetaClipRegionAction* pAct = static_cast<MetaClipRegionAction*>(pAction);

                if( pAct->IsClipping() && pAct->GetRegion().HasPolyPolygonOrB2DPolyPolygon() )
                    aMtf.AddAction( new MetaClipRegionAction( vcl::Region( ImplGetRotatedPolyPolygon( pAct->GetRegion().GetAsPolyPolygon(), aRotAnchor, aRotOffset, fSin, fCos ) ), true ) );
                else
                {
                    aMtf.AddAction( pAction );
                }
            }
            break;

            case MetaActionType::ISECTRECTCLIPREGION:
            {
                MetaISectRectClipRegionAction*  pAct = static_cast<MetaISectRectClipRegionAction*>(pAction);
                aMtf.AddAction( new MetaISectRegionClipRegionAction(vcl::Region(
                    ImplGetRotatedPolygon( pAct->GetRect(), aRotAnchor,
                        aRotOffset, fSin, fCos )) ) );
            }
            break;

            case MetaActionType::ISECTREGIONCLIPREGION:
            {
                MetaISectRegionClipRegionAction*    pAct = static_cast<MetaISectRegionClipRegionAction*>(pAction);
                const vcl::Region&                  rRegion = pAct->GetRegion();

                if( rRegion.HasPolyPolygonOrB2DPolyPolygon() )
                    aMtf.AddAction( new MetaISectRegionClipRegionAction( vcl::Region( ImplGetRotatedPolyPolygon( rRegion.GetAsPolyPolygon(), aRotAnchor, aRotOffset, fSin, fCos ) ) ) );
                else
                {
                    aMtf.AddAction( pAction );
                }
            }
            break;

            case MetaActionType::REFPOINT:
            {
                MetaRefPointAction* pAct = static_cast<MetaRefPointAction*>(pAction);
                aMtf.AddAction( new MetaRefPointAction( ImplGetRotatedPoint( pAct->GetRefPoint(), aRotAnchor, aRotOffset, fSin, fCos ), pAct->IsSetting() ) );
            }
            break;

            case MetaActionType::FONT:
            {
                MetaFontAction* pAct = static_cast<MetaFontAction*>(pAction);
                vcl::Font       aFont( pAct->GetFont() );

                aFont.SetOrientation( aFont.GetOrientation() + nAngle10 );
                aMtf.AddAction( new MetaFontAction( aFont ) );
            }
            break;

            case MetaActionType::BMP:
            case MetaActionType::BMPEX:
            case MetaActionType::MASK:
            case MetaActionType::MASKSCALE:
            case MetaActionType::MASKSCALEPART:
            case MetaActionType::WALLPAPER:
            case MetaActionType::TEXTRECT:
            case MetaActionType::MOVECLIPREGION:
            {
                OSL_FAIL( "GDIMetaFile::Rotate(): unsupported action" );
            }
            break;

            default:
            {
                pAction->Execute( aMapVDev.get() );
                aMtf.AddAction( pAction );

                // update rotation point and offset, if necessary
                if( ( MetaActionType::MAPMODE == nActionType ) ||
                    ( MetaActionType::PUSH == nActionType ) ||
                    ( MetaActionType::POP == nActionType ) )
                {
                    aRotAnchor = OutputDevice::LogicToLogic( aOrigin, m_aPrefMapMode, aMapVDev->GetMapMode() );
                    aRotOffset = OutputDevice::LogicToLogic( aOffset, m_aPrefMapMode, aMapVDev->GetMapMode() );
                }
            }
            break;
        }
    }

    aMtf.m_aPrefMapMode = m_aPrefMapMode;
    aMtf.m_aPrefSize = aNewBound.GetSize();

    *this = aMtf;

}

static void ImplActionBounds( tools::Rectangle& o_rOutBounds,
                              const tools::Rectangle& i_rInBounds,
                              const std::vector<tools::Rectangle>& i_rClipStack,
                              tools::Rectangle* o_pHairline )
{
    tools::Rectangle aBounds( i_rInBounds );
    if( ! i_rInBounds.IsEmpty() && ! i_rClipStack.empty() && ! i_rClipStack.back().IsEmpty() )
        aBounds.Intersection( i_rClipStack.back() );
    if(  aBounds.IsEmpty() )
        return;

    if( ! o_rOutBounds.IsEmpty() )
        o_rOutBounds.Union( aBounds );
    else
        o_rOutBounds = aBounds;

    if(o_pHairline)
    {
        if( ! o_pHairline->IsEmpty() )
            o_pHairline->Union( aBounds );
        else
            *o_pHairline = aBounds;
    }
}

tools::Rectangle GDIMetaFile::GetBoundRect( OutputDevice& i_rReference, tools::Rectangle* pHairline ) const
{
    ScopedVclPtrInstance< VirtualDevice > aMapVDev(  i_rReference  );

    aMapVDev->EnableOutput( false );
    aMapVDev->SetMapMode( GetPrefMapMode() );

    std::vector<tools::Rectangle> aClipStack( 1, tools::Rectangle() );
    std::vector<vcl::PushFlags> aPushFlagStack;

    tools::Rectangle aBound;

    if(pHairline)
        *pHairline = tools::Rectangle();

    const sal_uLong nCount(GetActionSize());

    for(sal_uLong a(0); a < nCount; a++)
    {
        MetaAction* pAction = GetAction(a);
        const MetaActionType nActionType = pAction->GetType();
        tools::Rectangle* pUseHairline = (pHairline && aMapVDev->IsLineColor()) ? pHairline : nullptr;

        switch( nActionType )
        {
        case MetaActionType::PIXEL:
        {
            MetaPixelAction* pAct = static_cast<MetaPixelAction*>(pAction);
            ImplActionBounds( aBound,
                              tools::Rectangle( OutputDevice::LogicToLogic( pAct->GetPoint(), aMapVDev->GetMapMode(), GetPrefMapMode() ),
                                       aMapVDev->PixelToLogic( Size( 1, 1 ), GetPrefMapMode() ) ),
                             aClipStack, pUseHairline );
        }
        break;

        case MetaActionType::POINT:
        {
            MetaPointAction* pAct = static_cast<MetaPointAction*>(pAction);
            ImplActionBounds( aBound,
                              tools::Rectangle( OutputDevice::LogicToLogic( pAct->GetPoint(), aMapVDev->GetMapMode(), GetPrefMapMode() ),
                                       aMapVDev->PixelToLogic( Size( 1, 1 ), GetPrefMapMode() ) ),
                             aClipStack, pUseHairline );
        }
        break;

        case MetaActionType::LINE:
        {
            MetaLineAction* pAct = static_cast<MetaLineAction*>(pAction);
            Point aP1( pAct->GetStartPoint() ), aP2( pAct->GetEndPoint() );
            tools::Rectangle aRect( aP1, aP2 );
            aRect.Justify();

            if(pUseHairline)
            {
                const LineInfo& rLineInfo = pAct->GetLineInfo();

                if(0 != rLineInfo.GetWidth())
                    pUseHairline = nullptr;
            }

            ImplActionBounds( aBound, OutputDevice::LogicToLogic( aRect, aMapVDev->GetMapMode(), GetPrefMapMode() ), aClipStack, pUseHairline );
        }
        break;

        case MetaActionType::RECT:
        {
            MetaRectAction* pAct = static_cast<MetaRectAction*>(pAction);
            ImplActionBounds( aBound, OutputDevice::LogicToLogic( pAct->GetRect(), aMapVDev->GetMapMode(), GetPrefMapMode() ), aClipStack, pUseHairline );
        }
        break;

        case MetaActionType::ROUNDRECT:
        {
            MetaRoundRectAction*    pAct = static_cast<MetaRoundRectAction*>(pAction);
            ImplActionBounds( aBound, OutputDevice::LogicToLogic( pAct->GetRect(), aMapVDev->GetMapMode(), GetPrefMapMode() ), aClipStack, pUseHairline );
        }
        break;

        case MetaActionType::ELLIPSE:
        {
            MetaEllipseAction*      pAct = static_cast<MetaEllipseAction*>(pAction);
            ImplActionBounds( aBound, OutputDevice::LogicToLogic( pAct->GetRect(), aMapVDev->GetMapMode(), GetPrefMapMode() ), aClipStack, pUseHairline );
        }
        break;

        case MetaActionType::ARC:
        {
            MetaArcAction*  pAct = static_cast<MetaArcAction*>(pAction);
            // FIXME: this is imprecise
            // e.g. for small arcs the whole rectangle is WAY too large
            ImplActionBounds( aBound, OutputDevice::LogicToLogic( pAct->GetRect(), aMapVDev->GetMapMode(), GetPrefMapMode() ), aClipStack, pUseHairline );
        }
        break;

        case MetaActionType::PIE:
        {
            MetaPieAction*  pAct = static_cast<MetaPieAction*>(pAction);
            // FIXME: this is imprecise
            // e.g. for small arcs the whole rectangle is WAY too large
            ImplActionBounds( aBound, OutputDevice::LogicToLogic( pAct->GetRect(), aMapVDev->GetMapMode(), GetPrefMapMode() ), aClipStack, pUseHairline );
        }
        break;

        case MetaActionType::CHORD:
        {
            MetaChordAction*    pAct = static_cast<MetaChordAction*>(pAction);
            // FIXME: this is imprecise
            // e.g. for small arcs the whole rectangle is WAY too large
            ImplActionBounds( aBound, OutputDevice::LogicToLogic( pAct->GetRect(), aMapVDev->GetMapMode(), GetPrefMapMode() ), aClipStack, pUseHairline );
        }
        break;

        case MetaActionType::POLYLINE:
        {
            MetaPolyLineAction* pAct = static_cast<MetaPolyLineAction*>(pAction);
            tools::Rectangle aRect( pAct->GetPolygon().GetBoundRect() );

            if(pUseHairline)
            {
                const LineInfo& rLineInfo = pAct->GetLineInfo();

                if(0 != rLineInfo.GetWidth())
                    pUseHairline = nullptr;
            }

            ImplActionBounds( aBound, OutputDevice::LogicToLogic( aRect, aMapVDev->GetMapMode(), GetPrefMapMode() ), aClipStack, pUseHairline );
        }
        break;

        case MetaActionType::POLYGON:
        {
            MetaPolygonAction* pAct = static_cast<MetaPolygonAction*>(pAction);
            tools::Rectangle aRect( pAct->GetPolygon().GetBoundRect() );
            ImplActionBounds( aBound, OutputDevice::LogicToLogic( aRect, aMapVDev->GetMapMode(), GetPrefMapMode() ), aClipStack, pUseHairline );
        }
        break;

        case MetaActionType::POLYPOLYGON:
        {
            MetaPolyPolygonAction* pAct = static_cast<MetaPolyPolygonAction*>(pAction);
            tools::Rectangle aRect( pAct->GetPolyPolygon().GetBoundRect() );
            ImplActionBounds( aBound, OutputDevice::LogicToLogic( aRect, aMapVDev->GetMapMode(), GetPrefMapMode() ), aClipStack, pUseHairline );
        }
        break;

        case MetaActionType::TEXT:
        {
            MetaTextAction* pAct = static_cast<MetaTextAction*>(pAction);
            tools::Rectangle aRect;
            // hdu said base = index
            aMapVDev->GetTextBoundRect( aRect, pAct->GetText(), pAct->GetIndex(), pAct->GetIndex(), pAct->GetLen() );
            Point aPt( pAct->GetPoint() );
            aRect.Move( aPt.X(), aPt.Y() );
            ImplActionBounds( aBound, OutputDevice::LogicToLogic( aRect, aMapVDev->GetMapMode(), GetPrefMapMode() ), aClipStack, nullptr );
        }
        break;

        case MetaActionType::TEXTARRAY:
        {
            MetaTextArrayAction* pAct = static_cast<MetaTextArrayAction*>(pAction);
            tools::Rectangle aRect;
            // hdu said base = index
            aMapVDev->GetTextBoundRect( aRect, pAct->GetText(), pAct->GetIndex(), pAct->GetIndex(), pAct->GetLen(),
                                       0, pAct->GetDXArray() );
            Point aPt( pAct->GetPoint() );
            aRect.Move( aPt.X(), aPt.Y() );
            ImplActionBounds( aBound, OutputDevice::LogicToLogic( aRect, aMapVDev->GetMapMode(), GetPrefMapMode() ), aClipStack, nullptr );
        }
        break;

        case MetaActionType::STRETCHTEXT:
        {
            MetaStretchTextAction* pAct = static_cast<MetaStretchTextAction*>(pAction);
            tools::Rectangle aRect;
            // hdu said base = index
            aMapVDev->GetTextBoundRect( aRect, pAct->GetText(), pAct->GetIndex(), pAct->GetIndex(), pAct->GetLen(),
                                       pAct->GetWidth() );
            Point aPt( pAct->GetPoint() );
            aRect.Move( aPt.X(), aPt.Y() );
            ImplActionBounds( aBound, OutputDevice::LogicToLogic( aRect, aMapVDev->GetMapMode(), GetPrefMapMode() ), aClipStack, nullptr );
        }
        break;

        case MetaActionType::TEXTLINE:
        {
            MetaTextLineAction* pAct = static_cast<MetaTextLineAction*>(pAction);
            // measure a test string to get ascend and descent right
            static constexpr OUStringLiteral pStr = u"\u00c4g";
            OUString aStr( pStr );

            tools::Rectangle aRect;
            aMapVDev->GetTextBoundRect( aRect, aStr, 0, 0, aStr.getLength() );
            Point aPt( pAct->GetStartPoint() );
            aRect.Move( aPt.X(), aPt.Y() );
            aRect.SetRight( aRect.Left() + pAct->GetWidth() );
            ImplActionBounds( aBound, OutputDevice::LogicToLogic( aRect, aMapVDev->GetMapMode(), GetPrefMapMode() ), aClipStack, nullptr );
        }
        break;

        case MetaActionType::BMPSCALE:
        {
            MetaBmpScaleAction* pAct = static_cast<MetaBmpScaleAction*>(pAction);
            tools::Rectangle aRect( pAct->GetPoint(), pAct->GetSize() );
            ImplActionBounds( aBound, OutputDevice::LogicToLogic( aRect, aMapVDev->GetMapMode(), GetPrefMapMode() ), aClipStack, nullptr );
        }
        break;

        case MetaActionType::BMPSCALEPART:
        {
            MetaBmpScalePartAction* pAct = static_cast<MetaBmpScalePartAction*>(pAction);
            tools::Rectangle aRect( pAct->GetDestPoint(), pAct->GetDestSize() );
            ImplActionBounds( aBound, OutputDevice::LogicToLogic( aRect, aMapVDev->GetMapMode(), GetPrefMapMode() ), aClipStack, nullptr );
        }
        break;

        case MetaActionType::BMPEXSCALE:
        {
            MetaBmpExScaleAction*   pAct = static_cast<MetaBmpExScaleAction*>(pAction);
            tools::Rectangle aRect( pAct->GetPoint(), pAct->GetSize() );
            ImplActionBounds( aBound, OutputDevice::LogicToLogic( aRect, aMapVDev->GetMapMode(), GetPrefMapMode() ), aClipStack, nullptr );
        }
        break;

        case MetaActionType::BMPEXSCALEPART:
        {
            MetaBmpExScalePartAction*   pAct = static_cast<MetaBmpExScalePartAction*>(pAction);
            tools::Rectangle aRect( pAct->GetDestPoint(), pAct->GetDestSize() );
            ImplActionBounds( aBound, OutputDevice::LogicToLogic( aRect, aMapVDev->GetMapMode(), GetPrefMapMode() ), aClipStack, nullptr );
        }
        break;

        case MetaActionType::GRADIENT:
        {
            MetaGradientAction* pAct = static_cast<MetaGradientAction*>(pAction);
            tools::Rectangle aRect( pAct->GetRect() );
            ImplActionBounds( aBound, OutputDevice::LogicToLogic( aRect, aMapVDev->GetMapMode(), GetPrefMapMode() ), aClipStack, nullptr );
        }
        break;

        case MetaActionType::GRADIENTEX:
        {
            MetaGradientExAction* pAct = static_cast<MetaGradientExAction*>(pAction);
            tools::Rectangle aRect( pAct->GetPolyPolygon().GetBoundRect() );
            ImplActionBounds( aBound, OutputDevice::LogicToLogic( aRect, aMapVDev->GetMapMode(), GetPrefMapMode() ), aClipStack, nullptr );
        }
        break;

        case MetaActionType::COMMENT:
        {
            // nothing to do
        };
        break;

        case MetaActionType::HATCH:
        {
            MetaHatchAction*    pAct = static_cast<MetaHatchAction*>(pAction);
            tools::Rectangle aRect( pAct->GetPolyPolygon().GetBoundRect() );
            ImplActionBounds( aBound, OutputDevice::LogicToLogic( aRect, aMapVDev->GetMapMode(), GetPrefMapMode() ), aClipStack, nullptr );
        }
        break;

        case MetaActionType::Transparent:
        {
            MetaTransparentAction* pAct = static_cast<MetaTransparentAction*>(pAction);
            tools::Rectangle aRect( pAct->GetPolyPolygon().GetBoundRect() );
            ImplActionBounds( aBound, OutputDevice::LogicToLogic( aRect, aMapVDev->GetMapMode(), GetPrefMapMode() ), aClipStack, nullptr );
        }
        break;

        case MetaActionType::FLOATTRANSPARENT:
        {
            MetaFloatTransparentAction* pAct = static_cast<MetaFloatTransparentAction*>(pAction);
            // MetaFloatTransparentAction is defined limiting its content Metafile
            // to its geometry definition(Point, Size), so use these directly
            const tools::Rectangle aRect( pAct->GetPoint(), pAct->GetSize() );
            ImplActionBounds( aBound, OutputDevice::LogicToLogic( aRect, aMapVDev->GetMapMode(), GetPrefMapMode() ), aClipStack, nullptr );
        }
        break;

        case MetaActionType::EPS:
        {
            MetaEPSAction*  pAct = static_cast<MetaEPSAction*>(pAction);
            tools::Rectangle aRect( pAct->GetPoint(), pAct->GetSize() );
            ImplActionBounds( aBound, OutputDevice::LogicToLogic( aRect, aMapVDev->GetMapMode(), GetPrefMapMode() ), aClipStack, nullptr );
        }
        break;

        case MetaActionType::CLIPREGION:
        {
            MetaClipRegionAction* pAct = static_cast<MetaClipRegionAction*>(pAction);
            if( pAct->IsClipping() )
                aClipStack.back() = OutputDevice::LogicToLogic( pAct->GetRegion().GetBoundRect(), aMapVDev->GetMapMode(), GetPrefMapMode() );
            else
                aClipStack.back() = tools::Rectangle();
        }
        break;

        case MetaActionType::ISECTRECTCLIPREGION:
        {
            MetaISectRectClipRegionAction* pAct = static_cast<MetaISectRectClipRegionAction*>(pAction);
            tools::Rectangle aRect( OutputDevice::LogicToLogic( pAct->GetRect(), aMapVDev->GetMapMode(), GetPrefMapMode() ) );
            if( aClipStack.back().IsEmpty() )
                aClipStack.back() = aRect;
            else
                aClipStack.back().Intersection( aRect );
        }
        break;

        case MetaActionType::ISECTREGIONCLIPREGION:
        {
            MetaISectRegionClipRegionAction*    pAct = static_cast<MetaISectRegionClipRegionAction*>(pAction);
            tools::Rectangle aRect( OutputDevice::LogicToLogic( pAct->GetRegion().GetBoundRect(), aMapVDev->GetMapMode(), GetPrefMapMode() ) );
            if( aClipStack.back().IsEmpty() )
                aClipStack.back() = aRect;
            else
                aClipStack.back().Intersection( aRect );
        }
        break;

        case MetaActionType::BMP:
        {
            MetaBmpAction* pAct = static_cast<MetaBmpAction*>(pAction);
            tools::Rectangle aRect( pAct->GetPoint(), aMapVDev->PixelToLogic( pAct->GetBitmap().GetSizePixel() ) );
            ImplActionBounds( aBound, OutputDevice::LogicToLogic( aRect, aMapVDev->GetMapMode(), GetPrefMapMode() ), aClipStack, nullptr );
        }
        break;

        case MetaActionType::BMPEX:
        {
            MetaBmpExAction* pAct = static_cast<MetaBmpExAction*>(pAction);
            tools::Rectangle aRect( pAct->GetPoint(), aMapVDev->PixelToLogic( pAct->GetBitmapEx().GetSizePixel() ) );
            ImplActionBounds( aBound, OutputDevice::LogicToLogic( aRect, aMapVDev->GetMapMode(), GetPrefMapMode() ), aClipStack, nullptr );
        }
        break;

        case MetaActionType::MASK:
        {
            MetaMaskAction* pAct = static_cast<MetaMaskAction*>(pAction);
            tools::Rectangle aRect( pAct->GetPoint(), aMapVDev->PixelToLogic( pAct->GetBitmap().GetSizePixel() ) );
            ImplActionBounds( aBound, OutputDevice::LogicToLogic( aRect, aMapVDev->GetMapMode(), GetPrefMapMode() ), aClipStack, nullptr );
        }
        break;

        case MetaActionType::MASKSCALE:
        {
            MetaMaskScalePartAction* pAct = static_cast<MetaMaskScalePartAction*>(pAction);
            tools::Rectangle aRect( pAct->GetDestPoint(), pAct->GetDestSize() );
            ImplActionBounds( aBound, OutputDevice::LogicToLogic( aRect, aMapVDev->GetMapMode(), GetPrefMapMode() ), aClipStack, nullptr );
        }
        break;

        case MetaActionType::MASKSCALEPART:
        {
            MetaMaskScalePartAction* pAct = static_cast<MetaMaskScalePartAction*>(pAction);
            tools::Rectangle aRect( pAct->GetDestPoint(), pAct->GetDestSize() );
            ImplActionBounds( aBound, OutputDevice::LogicToLogic( aRect, aMapVDev->GetMapMode(), GetPrefMapMode() ), aClipStack, nullptr );
        }
        break;

        case MetaActionType::WALLPAPER:
        {
            MetaWallpaperAction* pAct = static_cast<MetaWallpaperAction*>(pAction);
            tools::Rectangle aRect( pAct->GetRect() );
            ImplActionBounds( aBound, OutputDevice::LogicToLogic( aRect, aMapVDev->GetMapMode(), GetPrefMapMode() ), aClipStack, nullptr );
        }
        break;

        case MetaActionType::TEXTRECT:
        {
            MetaTextRectAction* pAct = static_cast<MetaTextRectAction*>(pAction);
            tools::Rectangle aRect( pAct->GetRect() );
            ImplActionBounds( aBound, OutputDevice::LogicToLogic( aRect, aMapVDev->GetMapMode(), GetPrefMapMode() ), aClipStack, nullptr );
        }
        break;

        case MetaActionType::MOVECLIPREGION:
        {
            MetaMoveClipRegionAction* pAct = static_cast<MetaMoveClipRegionAction*>(pAction);
            if( ! aClipStack.back().IsEmpty() )
            {
                Size aDelta( pAct->GetHorzMove(), pAct->GetVertMove() );
                aDelta = OutputDevice::LogicToLogic( aDelta, aMapVDev->GetMapMode(), GetPrefMapMode() );
                aClipStack.back().Move( aDelta.Width(), aDelta.Width() );
            }
        }
        break;

        default:
            {
                pAction->Execute( aMapVDev.get() );

                if( nActionType == MetaActionType::PUSH )
                {
                    MetaPushAction* pAct = static_cast<MetaPushAction*>(pAction);
                    aPushFlagStack.push_back( pAct->GetFlags() );
                    if( aPushFlagStack.back() & vcl::PushFlags::CLIPREGION )
                    {
                        tools::Rectangle aRect( aClipStack.back() );
                        aClipStack.push_back( aRect );
                    }
                }
                else if( nActionType == MetaActionType::POP )
                {
                    // sanity check
                    if( ! aPushFlagStack.empty() )
                    {
                        if( aPushFlagStack.back() & vcl::PushFlags::CLIPREGION )
                        {
                            if( aClipStack.size() > 1 )
                                aClipStack.pop_back();
                        }
                        aPushFlagStack.pop_back();
                    }
                }
            }
            break;
        }
    }
    return aBound;
}

Color GDIMetaFile::ImplColAdjustFnc( const Color& rColor, const void* pColParam )
{
    return Color( ColorAlpha, rColor.GetAlpha(),
                  static_cast<const ImplColAdjustParam*>(pColParam)->pMapR[ rColor.GetRed() ],
                  static_cast<const ImplColAdjustParam*>(pColParam)->pMapG[ rColor.GetGreen() ],
                  static_cast<const ImplColAdjustParam*>(pColParam)->pMapB[ rColor.GetBlue() ] );

}

BitmapEx GDIMetaFile::ImplBmpAdjustFnc( const BitmapEx& rBmpEx, const void* pBmpParam )
{
    const ImplBmpAdjustParam*   p = static_cast<const ImplBmpAdjustParam*>(pBmpParam);
    BitmapEx                    aRet( rBmpEx );

    aRet.Adjust( p->nLuminancePercent, p->nContrastPercent,
                 p->nChannelRPercent, p->nChannelGPercent, p->nChannelBPercent,
                 p->fGamma, p->bInvert );

    return aRet;
}

Color GDIMetaFile::ImplColConvertFnc( const Color& rColor, const void* pColParam )
{
    sal_uInt8 cLum = rColor.GetLuminance();

    if( MtfConversion::N1BitThreshold == static_cast<const ImplColConvertParam*>(pColParam)->eConversion )
        cLum = ( cLum < 128 ) ? 0 : 255;

    return Color( ColorAlpha, rColor.GetAlpha(), cLum, cLum, cLum );
}

BitmapEx GDIMetaFile::ImplBmpConvertFnc( const BitmapEx& rBmpEx, const void* pBmpParam )
{
    BitmapEx aRet( rBmpEx );

    aRet.Convert( static_cast<const ImplBmpConvertParam*>(pBmpParam)->eConversion );

    return aRet;
}

Color GDIMetaFile::ImplColMonoFnc( const Color&, const void* pColParam )
{
    return static_cast<const ImplColMonoParam*>(pColParam)->aColor;
}

BitmapEx GDIMetaFile::ImplBmpMonoFnc( const BitmapEx& rBmpEx, const void* pBmpParam )
{
    BitmapPalette aPal( 3 );
    aPal[ 0 ] = COL_BLACK;
    aPal[ 1 ] = COL_WHITE;
    aPal[ 2 ] = static_cast<const ImplBmpMonoParam*>(pBmpParam)->aColor;

    Bitmap aBmp(rBmpEx.GetSizePixel(), vcl::PixelFormat::N8_BPP, &aPal);
    aBmp.Erase( static_cast<const ImplBmpMonoParam*>(pBmpParam)->aColor );

    if( rBmpEx.IsAlpha() )
        return BitmapEx( aBmp, rBmpEx.GetAlpha() );
    else
        return BitmapEx( aBmp );
}

Color GDIMetaFile::ImplColReplaceFnc( const Color& rColor, const void* pColParam )
{
    const sal_uLong nR = rColor.GetRed(), nG = rColor.GetGreen(), nB = rColor.GetBlue();

    for( sal_uLong i = 0; i < static_cast<const ImplColReplaceParam*>(pColParam)->nCount; i++ )
    {
        if( ( static_cast<const ImplColReplaceParam*>(pColParam)->pMinR[ i ] <= nR ) &&
            ( static_cast<const ImplColReplaceParam*>(pColParam)->pMaxR[ i ] >= nR ) &&
            ( static_cast<const ImplColReplaceParam*>(pColParam)->pMinG[ i ] <= nG ) &&
            ( static_cast<const ImplColReplaceParam*>(pColParam)->pMaxG[ i ] >= nG ) &&
            ( static_cast<const ImplColReplaceParam*>(pColParam)->pMinB[ i ] <= nB ) &&
            ( static_cast<const ImplColReplaceParam*>(pColParam)->pMaxB[ i ] >= nB ) )
        {
            return static_cast<const ImplColReplaceParam*>(pColParam)->pDstCols[ i ];
        }
    }

    return rColor;
}

BitmapEx GDIMetaFile::ImplBmpReplaceFnc( const BitmapEx& rBmpEx, const void* pBmpParam )
{
    const ImplBmpReplaceParam*  p = static_cast<const ImplBmpReplaceParam*>(pBmpParam);
    BitmapEx                    aRet( rBmpEx );

    aRet.Replace( p->pSrcCols, p->pDstCols, p->nCount );

    return aRet;
}

void GDIMetaFile::ImplExchangeColors( ColorExchangeFnc pFncCol, const void* pColParam,
                                      BmpExchangeFnc pFncBmp, const void* pBmpParam )
{
    GDIMetaFile aMtf;

    aMtf.m_aPrefSize = m_aPrefSize;
    aMtf.m_aPrefMapMode = m_aPrefMapMode;
    aMtf.m_bUseCanvas = m_bUseCanvas;

    for( MetaAction* pAction = FirstAction(); pAction; pAction = NextAction() )
    {
        const MetaActionType nType = pAction->GetType();

        switch( nType )
        {
            case MetaActionType::PIXEL:
            {
                MetaPixelAction* pAct = static_cast<MetaPixelAction*>(pAction);
                aMtf.push_back( new MetaPixelAction( pAct->GetPoint(), pFncCol( pAct->GetColor(), pColParam ) ) );
            }
            break;

            case MetaActionType::LINECOLOR:
            {
                MetaLineColorAction* pAct = static_cast<MetaLineColorAction*>(pAction);

                if( pAct->IsSetting() )
                    pAct = new MetaLineColorAction( pFncCol( pAct->GetColor(), pColParam ), true );

                aMtf.push_back( pAct );
            }
            break;

            case MetaActionType::FILLCOLOR:
            {
                MetaFillColorAction* pAct = static_cast<MetaFillColorAction*>(pAction);

                if( pAct->IsSetting() )
                    pAct = new MetaFillColorAction( pFncCol( pAct->GetColor(), pColParam ), true );

                aMtf.push_back( pAct );
            }
            break;

            case MetaActionType::TEXTCOLOR:
            {
                MetaTextColorAction* pAct = static_cast<MetaTextColorAction*>(pAction);
                aMtf.push_back( new MetaTextColorAction( pFncCol( pAct->GetColor(), pColParam ) ) );
            }
            break;

            case MetaActionType::TEXTFILLCOLOR:
            {
                MetaTextFillColorAction* pAct = static_cast<MetaTextFillColorAction*>(pAction);

                if( pAct->IsSetting() )
                    pAct = new MetaTextFillColorAction( pFncCol( pAct->GetColor(), pColParam ), true );

                aMtf.push_back( pAct );
            }
            break;

            case MetaActionType::TEXTLINECOLOR:
            {
                MetaTextLineColorAction* pAct = static_cast<MetaTextLineColorAction*>(pAction);

                if( pAct->IsSetting() )
                    pAct = new MetaTextLineColorAction( pFncCol( pAct->GetColor(), pColParam ), true );

                aMtf.push_back( pAct );
            }
            break;

            case MetaActionType::OVERLINECOLOR:
            {
                MetaOverlineColorAction* pAct = static_cast<MetaOverlineColorAction*>(pAction);

                if( pAct->IsSetting() )
                    pAct = new MetaOverlineColorAction( pFncCol( pAct->GetColor(), pColParam ), true );

                aMtf.push_back( pAct );
            }
            break;

            case MetaActionType::FONT:
            {
                MetaFontAction* pAct = static_cast<MetaFontAction*>(pAction);
                vcl::Font       aFont( pAct->GetFont() );

                aFont.SetColor( pFncCol( aFont.GetColor(), pColParam ) );
                aFont.SetFillColor( pFncCol( aFont.GetFillColor(), pColParam ) );
                aMtf.push_back( new MetaFontAction( aFont ) );
            }
            break;

            case MetaActionType::WALLPAPER:
            {
                MetaWallpaperAction*    pAct = static_cast<MetaWallpaperAction*>(pAction);
                Wallpaper               aWall( pAct->GetWallpaper() );
                const tools::Rectangle&        rRect = pAct->GetRect();

                aWall.SetColor( pFncCol( aWall.GetColor(), pColParam ) );

                if( aWall.IsBitmap() )
                    aWall.SetBitmap( pFncBmp( aWall.GetBitmap(), pBmpParam ) );

                if( aWall.IsGradient() )
                {
                    Gradient aGradient( aWall.GetGradient() );

                    aGradient.SetStartColor( pFncCol( aGradient.GetStartColor(), pColParam ) );
                    aGradient.SetEndColor( pFncCol( aGradient.GetEndColor(), pColParam ) );
                    aWall.SetGradient( aGradient );
                }

                aMtf.push_back( new MetaWallpaperAction( rRect, aWall ) );
            }
            break;

            case MetaActionType::BMP:
            case MetaActionType::BMPEX:
            case MetaActionType::MASK:
            {
                OSL_FAIL( "Don't use bitmap actions of this type in metafiles!" );
            }
            break;

            case MetaActionType::BMPSCALE:
            {
                MetaBmpScaleAction* pAct = static_cast<MetaBmpScaleAction*>(pAction);
                aMtf.push_back( new MetaBmpScaleAction( pAct->GetPoint(), pAct->GetSize(),
                                    pFncBmp( BitmapEx(pAct->GetBitmap()), pBmpParam ).GetBitmap() ) );
            }
            break;

            case MetaActionType::BMPSCALEPART:
            {
                MetaBmpScalePartAction* pAct = static_cast<MetaBmpScalePartAction*>(pAction);
                aMtf.push_back( new MetaBmpScalePartAction( pAct->GetDestPoint(), pAct->GetDestSize(),
                                                    pAct->GetSrcPoint(), pAct->GetSrcSize(),
                                                    pFncBmp( BitmapEx(pAct->GetBitmap()), pBmpParam ).GetBitmap() )
                                                );
            }
            break;

            case MetaActionType::BMPEXSCALE:
            {
                MetaBmpExScaleAction* pAct = static_cast<MetaBmpExScaleAction*>(pAction);
                aMtf.push_back( new MetaBmpExScaleAction( pAct->GetPoint(), pAct->GetSize(),
                                                          pFncBmp( pAct->GetBitmapEx(), pBmpParam ) )
                                                        );
            }
            break;

            case MetaActionType::BMPEXSCALEPART:
            {
                MetaBmpExScalePartAction* pAct = static_cast<MetaBmpExScalePartAction*>(pAction);
                aMtf.push_back( new MetaBmpExScalePartAction( pAct->GetDestPoint(), pAct->GetDestSize(),
                                                              pAct->GetSrcPoint(), pAct->GetSrcSize(),
                                                              pFncBmp( pAct->GetBitmapEx(), pBmpParam ) )
                                                            );
            }
            break;

            case MetaActionType::MASKSCALE:
            {
                MetaMaskScaleAction* pAct = static_cast<MetaMaskScaleAction*>(pAction);
                aMtf.push_back( new MetaMaskScaleAction( pAct->GetPoint(), pAct->GetSize(),
                                                         pAct->GetBitmap(),
                                                         pFncCol( pAct->GetColor(), pColParam ) )
                                                       );
            }
            break;

            case MetaActionType::MASKSCALEPART:
            {
                MetaMaskScalePartAction* pAct = static_cast<MetaMaskScalePartAction*>(pAction);
                aMtf.push_back( new MetaMaskScalePartAction( pAct->GetDestPoint(), pAct->GetDestSize(),
                                                             pAct->GetSrcPoint(), pAct->GetSrcSize(),
                                                             pAct->GetBitmap(),
                                                             pFncCol( pAct->GetColor(), pColParam ) )
                                                           );
            }
            break;

            case MetaActionType::GRADIENT:
            {
                MetaGradientAction* pAct = static_cast<MetaGradientAction*>(pAction);
                Gradient            aGradient( pAct->GetGradient() );

                aGradient.SetStartColor( pFncCol( aGradient.GetStartColor(), pColParam ) );
                aGradient.SetEndColor( pFncCol( aGradient.GetEndColor(), pColParam ) );
                aMtf.push_back( new MetaGradientAction( pAct->GetRect(), aGradient ) );
            }
            break;

            case MetaActionType::GRADIENTEX:
            {
                MetaGradientExAction* pAct = static_cast<MetaGradientExAction*>(pAction);
                Gradient              aGradient( pAct->GetGradient() );

                aGradient.SetStartColor( pFncCol( aGradient.GetStartColor(), pColParam ) );
                aGradient.SetEndColor( pFncCol( aGradient.GetEndColor(), pColParam ) );
                aMtf.push_back( new MetaGradientExAction( pAct->GetPolyPolygon(), aGradient ) );
            }
            break;

            case MetaActionType::HATCH:
            {
                MetaHatchAction*    pAct = static_cast<MetaHatchAction*>(pAction);
                Hatch               aHatch( pAct->GetHatch() );

                aHatch.SetColor( pFncCol( aHatch.GetColor(), pColParam ) );
                aMtf.push_back( new MetaHatchAction( pAct->GetPolyPolygon(), aHatch ) );
            }
            break;

            case MetaActionType::FLOATTRANSPARENT:
            {
                MetaFloatTransparentAction* pAct = static_cast<MetaFloatTransparentAction*>(pAction);
                GDIMetaFile                 aTransMtf( pAct->GetGDIMetaFile() );

                aTransMtf.ImplExchangeColors( pFncCol, pColParam, pFncBmp, pBmpParam );
                aMtf.push_back( new MetaFloatTransparentAction( aTransMtf,
                                                                pAct->GetPoint(), pAct->GetSize(),
                                                                pAct->GetGradient() )
                                                              );
            }
            break;

            case MetaActionType::EPS:
            {
                MetaEPSAction*  pAct = static_cast<MetaEPSAction*>(pAction);
                GDIMetaFile     aSubst( pAct->GetSubstitute() );

                aSubst.ImplExchangeColors( pFncCol, pColParam, pFncBmp, pBmpParam );
                aMtf.push_back( new MetaEPSAction( pAct->GetPoint(), pAct->GetSize(),
                                                   pAct->GetLink(), aSubst )
                                                 );
            }
            break;

            default:
            {
                aMtf.push_back( pAction );
            }
            break;
        }
    }

    *this = aMtf;
}

void GDIMetaFile::Adjust( short nLuminancePercent, short nContrastPercent,
                          short nChannelRPercent, short nChannelGPercent,
                          short nChannelBPercent, double fGamma, bool bInvert, bool msoBrightness )
{
    // nothing to do? => return quickly
    if( !(nLuminancePercent || nContrastPercent ||
          nChannelRPercent || nChannelGPercent || nChannelBPercent ||
          ( fGamma != 1.0 ) || bInvert) )
        return;

    double              fM, fROff, fGOff, fBOff, fOff;
    ImplColAdjustParam  aColParam;
    ImplBmpAdjustParam  aBmpParam;

    aColParam.pMapR.reset(new sal_uInt8[ 256 ]);
    aColParam.pMapG.reset(new sal_uInt8[ 256 ]);
    aColParam.pMapB.reset(new sal_uInt8[ 256 ]);

    // calculate slope
    if( nContrastPercent >= 0 )
        fM = 128.0 / ( 128.0 - 1.27 * MinMax( nContrastPercent, 0, 100 ) );
    else
        fM = ( 128.0 + 1.27 * MinMax( nContrastPercent, -100, 0 ) ) / 128.0;

    if(!msoBrightness)
        // total offset = luminance offset + contrast offset
        fOff = MinMax( nLuminancePercent, -100, 100 ) * 2.55 + 128.0 - fM * 128.0;
    else
        fOff = MinMax( nLuminancePercent, -100, 100 ) * 2.55;

    // channel offset = channel offset  + total offset
    fROff = nChannelRPercent * 2.55 + fOff;
    fGOff = nChannelGPercent * 2.55 + fOff;
    fBOff = nChannelBPercent * 2.55 + fOff;

    // calculate gamma value
    fGamma = ( fGamma <= 0.0 || fGamma > 10.0 ) ? 1.0 : ( 1.0 / fGamma );
    const bool bGamma = ( fGamma != 1.0 );

    // create mapping table
    for( tools::Long nX = 0; nX < 256; nX++ )
    {
        if(!msoBrightness)
        {
            aColParam.pMapR[ nX ] = static_cast<sal_uInt8>(MinMax( FRound( nX * fM + fROff ), 0, 255 ));
            aColParam.pMapG[ nX ] = static_cast<sal_uInt8>(MinMax( FRound( nX * fM + fGOff ), 0, 255 ));
            aColParam.pMapB[ nX ] = static_cast<sal_uInt8>(MinMax( FRound( nX * fM + fBOff ), 0, 255 ));
        }
        else
        {
            aColParam.pMapR[ nX ] = static_cast<sal_uInt8>(MinMax( FRound( (nX+fROff/2-128) * fM + 128 + fROff/2 ), 0, 255 ));
            aColParam.pMapG[ nX ] = static_cast<sal_uInt8>(MinMax( FRound( (nX+fGOff/2-128) * fM + 128 + fGOff/2 ), 0, 255 ));
            aColParam.pMapB[ nX ] = static_cast<sal_uInt8>(MinMax( FRound( (nX+fBOff/2-128) * fM + 128 + fBOff/2 ), 0, 255 ));
        }
        if( bGamma )
        {
            aColParam.pMapR[ nX ] = GAMMA( aColParam.pMapR[ nX ], fGamma );
            aColParam.pMapG[ nX ] = GAMMA( aColParam.pMapG[ nX ], fGamma );
            aColParam.pMapB[ nX ] = GAMMA( aColParam.pMapB[ nX ], fGamma );
        }

        if( bInvert )
        {
            aColParam.pMapR[ nX ] = ~aColParam.pMapR[ nX ];
            aColParam.pMapG[ nX ] = ~aColParam.pMapG[ nX ];
            aColParam.pMapB[ nX ] = ~aColParam.pMapB[ nX ];
        }
    }

    aBmpParam.nLuminancePercent = nLuminancePercent;
    aBmpParam.nContrastPercent = nContrastPercent;
    aBmpParam.nChannelRPercent = nChannelRPercent;
    aBmpParam.nChannelGPercent = nChannelGPercent;
    aBmpParam.nChannelBPercent = nChannelBPercent;
    aBmpParam.fGamma = fGamma;
    aBmpParam.bInvert = bInvert;

    // do color adjustment
    ImplExchangeColors( ImplColAdjustFnc, &aColParam, ImplBmpAdjustFnc, &aBmpParam );
}

void GDIMetaFile::Convert( MtfConversion eConversion )
{
    ImplColConvertParam aColParam;
    ImplBmpConvertParam aBmpParam;

    aColParam.eConversion = eConversion;
    aBmpParam.eConversion = ( MtfConversion::N1BitThreshold == eConversion ) ? BmpConversion::N1BitThreshold : BmpConversion::N8BitGreys;

    ImplExchangeColors( ImplColConvertFnc, &aColParam, ImplBmpConvertFnc, &aBmpParam );
}

void GDIMetaFile::ReplaceColors( const Color* pSearchColors, const Color* pReplaceColors, sal_uLong nColorCount )
{
    ImplColReplaceParam aColParam;
    ImplBmpReplaceParam aBmpParam;

    aColParam.pMinR.reset(new sal_uLong[ nColorCount ]);
    aColParam.pMaxR.reset(new sal_uLong[ nColorCount ]);
    aColParam.pMinG.reset(new sal_uLong[ nColorCount ]);
    aColParam.pMaxG.reset(new sal_uLong[ nColorCount ]);
    aColParam.pMinB.reset(new sal_uLong[ nColorCount ]);
    aColParam.pMaxB.reset(new sal_uLong[ nColorCount ]);

    for( sal_uLong i = 0; i < nColorCount; i++ )
    {
        tools::Long        nVal;

        nVal = pSearchColors[ i ].GetRed();
        aColParam.pMinR[ i ] = static_cast<sal_uLong>(std::max( nVal, tools::Long(0) ));
        aColParam.pMaxR[ i ] = static_cast<sal_uLong>(std::min( nVal, tools::Long(255) ));

        nVal = pSearchColors[ i ].GetGreen();
        aColParam.pMinG[ i ] = static_cast<sal_uLong>(std::max( nVal, tools::Long(0) ));
        aColParam.pMaxG[ i ] = static_cast<sal_uLong>(std::min( nVal, tools::Long(255) ));

        nVal = pSearchColors[ i ].GetBlue();
        aColParam.pMinB[ i ] = static_cast<sal_uLong>(std::max( nVal, tools::Long(0) ));
        aColParam.pMaxB[ i ] = static_cast<sal_uLong>(std::min( nVal, tools::Long(255) ));
    }

    aColParam.pDstCols = pReplaceColors;
    aColParam.nCount = nColorCount;

    aBmpParam.pSrcCols = pSearchColors;
    aBmpParam.pDstCols = pReplaceColors;
    aBmpParam.nCount = nColorCount;

    ImplExchangeColors( ImplColReplaceFnc, &aColParam, ImplBmpReplaceFnc, &aBmpParam );
};

GDIMetaFile GDIMetaFile::GetMonochromeMtf( const Color& rColor ) const
{
    GDIMetaFile aRet( *this );

    ImplColMonoParam    aColParam;
    ImplBmpMonoParam    aBmpParam;

    aColParam.aColor = rColor;
    aBmpParam.aColor = rColor;

    aRet.ImplExchangeColors( ImplColMonoFnc, &aColParam, ImplBmpMonoFnc, &aBmpParam );

    return aRet;
}

sal_uLong GDIMetaFile::GetSizeBytes() const
{
    sal_uLong nSizeBytes = 0;

    for( size_t i = 0, nObjCount = GetActionSize(); i < nObjCount; ++i )
    {
        MetaAction* pAction = GetAction( i );

        // default action size is set to 32 (=> not the exact value)
        nSizeBytes += 32;

        // add sizes for large action content
        switch( pAction->GetType() )
        {
            case MetaActionType::BMP:            nSizeBytes += static_cast<MetaBmpAction*>( pAction )->GetBitmap().GetSizeBytes(); break;
            case MetaActionType::BMPSCALE:       nSizeBytes += static_cast<MetaBmpScaleAction*>( pAction )->GetBitmap().GetSizeBytes(); break;
            case MetaActionType::BMPSCALEPART:   nSizeBytes += static_cast<MetaBmpScalePartAction*>( pAction )->GetBitmap().GetSizeBytes(); break;

            case MetaActionType::BMPEX:          nSizeBytes += static_cast<MetaBmpExAction*>( pAction )->GetBitmapEx().GetSizeBytes(); break;
            case MetaActionType::BMPEXSCALE:     nSizeBytes += static_cast<MetaBmpExScaleAction*>( pAction )->GetBitmapEx().GetSizeBytes(); break;
            case MetaActionType::BMPEXSCALEPART: nSizeBytes += static_cast<MetaBmpExScalePartAction*>( pAction )->GetBitmapEx().GetSizeBytes(); break;

            case MetaActionType::MASK:           nSizeBytes += static_cast<MetaMaskAction*>( pAction )->GetBitmap().GetSizeBytes(); break;
            case MetaActionType::MASKSCALE:      nSizeBytes += static_cast<MetaMaskScaleAction*>( pAction )->GetBitmap().GetSizeBytes(); break;
            case MetaActionType::MASKSCALEPART:  nSizeBytes += static_cast<MetaMaskScalePartAction*>( pAction )->GetBitmap().GetSizeBytes(); break;

            case MetaActionType::POLYLINE:       nSizeBytes += static_cast<MetaPolyLineAction*>( pAction )->GetPolygon().GetSize() * sizeof( Point ); break;
            case MetaActionType::POLYGON:        nSizeBytes += static_cast<MetaPolygonAction*>( pAction )->GetPolygon().GetSize() * sizeof( Point ); break;
            case MetaActionType::POLYPOLYGON:
            {
                const tools::PolyPolygon& rPolyPoly = static_cast<MetaPolyPolygonAction*>( pAction )->GetPolyPolygon();

                for( sal_uInt16 n = 0; n < rPolyPoly.Count(); ++n )
                    nSizeBytes += ( rPolyPoly[ n ].GetSize() * sizeof( Point ) );
            }
            break;

            case MetaActionType::TEXT:        nSizeBytes += static_cast<MetaTextAction*>( pAction )->GetText().getLength() * sizeof( sal_Unicode ); break;
            case MetaActionType::STRETCHTEXT: nSizeBytes += static_cast<MetaStretchTextAction*>( pAction )->GetText().getLength() * sizeof( sal_Unicode ); break;
            case MetaActionType::TEXTRECT:    nSizeBytes += static_cast<MetaTextRectAction*>( pAction )->GetText().getLength() * sizeof( sal_Unicode ); break;
            case MetaActionType::TEXTARRAY:
            {
                MetaTextArrayAction* pTextArrayAction = static_cast<MetaTextArrayAction*>(pAction);

                nSizeBytes += ( pTextArrayAction->GetText().getLength() * sizeof( sal_Unicode ) );

                if( !pTextArrayAction->GetDXArray().empty() )
                    nSizeBytes += ( pTextArrayAction->GetLen() << 2 );
            }
            break;
            default: break;
        }
    }

    return nSizeBytes;
}

bool GDIMetaFile::CreateThumbnail(BitmapEx& rBitmapEx, BmpConversion eColorConversion, BmpScaleFlag nScaleFlag) const
{
    // initialization seems to be complicated but is used to avoid rounding errors
    ScopedVclPtrInstance< VirtualDevice > aVDev;
    // set Enable to tease the rendering down the code paths which use B2DPolygon and
    // avoid integer overflows on scaling tools::Polygon, e.g. moz1545040-1.svg
    // note: this is similar to DocumentToGraphicRenderer::renderToGraphic
    aVDev->SetAntialiasing(AntialiasingFlags::Enable | aVDev->GetAntialiasing());
    const Point     aNullPt;
    const Point     aTLPix( aVDev->LogicToPixel( aNullPt, GetPrefMapMode() ) );
    const Point     aBRPix( aVDev->LogicToPixel( Point( GetPrefSize().Width() - 1, GetPrefSize().Height() - 1 ), GetPrefMapMode() ) );
    Size            aDrawSize( aVDev->LogicToPixel( GetPrefSize(), GetPrefMapMode() ) );
    Size            aSizePix( std::abs( aBRPix.X() - aTLPix.X() ) + 1, std::abs( aBRPix.Y() - aTLPix.Y() ) + 1 );
    sal_uInt32      nMaximumExtent = 512;

    if (!rBitmapEx.IsEmpty())
        rBitmapEx.SetEmpty();

    // determine size that has the same aspect ratio as image size and
    // fits into the rectangle determined by nMaximumExtent
    if ( aSizePix.Width() && aSizePix.Height()
      && ( sal::static_int_cast< tools::ULong >(aSizePix.Width()) >
               nMaximumExtent ||
           sal::static_int_cast< tools::ULong >(aSizePix.Height()) >
               nMaximumExtent ) )
    {
        const Size  aOldSizePix( aSizePix );
        double      fWH = static_cast< double >( aSizePix.Width() ) / aSizePix.Height();

        if ( fWH <= 1.0 )
        {
            aSizePix.setWidth( FRound( nMaximumExtent * fWH ) );
            aSizePix.setHeight( nMaximumExtent );
        }
        else
        {
            aSizePix.setWidth( nMaximumExtent );
            aSizePix.setHeight( FRound(  nMaximumExtent / fWH ) );
        }

        aDrawSize.setWidth( FRound( ( static_cast< double >( aDrawSize.Width() ) * aSizePix.Width() ) / aOldSizePix.Width() ) );
        aDrawSize.setHeight( FRound( ( static_cast< double >( aDrawSize.Height() ) * aSizePix.Height() ) / aOldSizePix.Height() ) );
    }

    // draw image(s) into VDev and get resulting image
    // do it 4x larger to be able to scale it down & get beautiful antialias
    Size aAntialiasSize(aSizePix.Width() * 4, aSizePix.Height() * 4);
    if (aVDev->SetOutputSizePixel(aAntialiasSize))
    {
        // antialias: provide 4x larger size, and then scale down the result
        Size aAntialias(aDrawSize.Width() * 4, aDrawSize.Height() * 4);

        // draw metafile into VDev
        const_cast<GDIMetaFile *>(this)->WindStart();
        const_cast<GDIMetaFile *>(this)->Play(*aVDev, Point(), aAntialias);

        // get paint bitmap
        BitmapEx aBitmap( aVDev->GetBitmapEx( aNullPt, aVDev->GetOutputSizePixel() ) );

        // scale down the image to the desired size - use the input scaler for the scaling operation
        aBitmap.Scale(aDrawSize, nScaleFlag);

        // convert to desired bitmap color format
        Size aSize(aBitmap.GetSizePixel());
        if (aSize.Width() && aSize.Height())
            aBitmap.Convert(eColorConversion);

        rBitmapEx = aBitmap;
    }

    return !rBitmapEx.IsEmpty();
}

void GDIMetaFile::UseCanvas( bool _bUseCanvas )
{
    m_bUseCanvas = _bUseCanvas;
}

void GDIMetaFile::dumpAsXml(const char* pFileName) const
{
    SvFileStream aStream(pFileName ? OUString::fromUtf8(pFileName) : OUString("file:///tmp/metafile.xml"),
            StreamMode::STD_READWRITE | StreamMode::TRUNC);
    assert(aStream.good());
    MetafileXmlDump aDumper;
    aDumper.dump(*this, aStream);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
