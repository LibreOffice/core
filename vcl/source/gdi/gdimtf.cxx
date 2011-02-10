/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_vcl.hxx"
#include <rtl/crc.h>
#include <tools/stream.hxx>
#include <tools/vcompat.hxx>
#include <vcl/metaact.hxx>
#include <vcl/salbtype.hxx>
#include <vcl/outdev.hxx>
#include <vcl/window.hxx>
#include <vcl/cvtsvm.hxx>
#include <vcl/virdev.hxx>
#include <vcl/salbmp.hxx>
#include <vcl/svapp.hxx>
#include <vcl/svdata.hxx>
#include <vcl/salinst.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/graphictools.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/unohelp.hxx>

#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/MtfRenderer.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/awt/XGraphics.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/graphic/XGraphicRenderer.hpp>

using namespace com::sun::star;

// -----------
// - Defines -
// -----------

#define GAMMA( _def_cVal, _def_InvGamma )   ((BYTE)MinMax(FRound(pow( _def_cVal/255.0,_def_InvGamma)*255.0),0L,255L))

// --------------------------
// - Color exchange structs -
// --------------------------

struct ImplColAdjustParam
{
    BYTE*   pMapR;
    BYTE*   pMapG;
    BYTE*   pMapB;
};

struct ImplBmpAdjustParam
{
    short   nLuminancePercent;
    short   nContrastPercent;
    short   nChannelRPercent;
    short   nChannelGPercent;
    short   nChannelBPercent;
    double  fGamma;
    BOOL    bInvert;
};

// -----------------------------------------------------------------------------

struct ImplColConvertParam
{
    MtfConversion   eConversion;
};

struct ImplBmpConvertParam
{
    BmpConversion   eConversion;
};

// -----------------------------------------------------------------------------

struct ImplColMonoParam
{
    Color aColor;
};

struct ImplBmpMonoParam
{
    Color aColor;
};

// -----------------------------------------------------------------------------

struct ImplColReplaceParam
{
    ULONG*          pMinR;
    ULONG*          pMaxR;
    ULONG*          pMinG;
    ULONG*          pMaxG;
    ULONG*          pMinB;
    ULONG*          pMaxB;
    const Color*    pDstCols;
    ULONG           nCount;
};

struct ImplBmpReplaceParam
{
    const Color*    pSrcCols;
    const Color*    pDstCols;
    ULONG           nCount;
    const ULONG*    pTols;
};


// ---------
// - Label -
// ---------

struct ImpLabel
{
    String  aLabelName;
    ULONG   nActionPos;

            ImpLabel( const String& rLabelName, ULONG _nActionPos ) :
                aLabelName( rLabelName ),
                nActionPos( _nActionPos ) {}
};

// -------------
// - LabelList -
// -------------

class ImpLabelList : private List
{
public:

                ImpLabelList() : List( 8, 4, 4 ) {}
                ImpLabelList( const ImpLabelList& rList );
                ~ImpLabelList();

    void        ImplInsert( ImpLabel* p ) { Insert( p, LIST_APPEND ); }
    ImpLabel*   ImplRemove( ULONG nPos ) { return (ImpLabel*) Remove( nPos ); }
    void        ImplReplace( ImpLabel* p ) { Replace( (void*)p ); }
    ImpLabel*   ImplFirst() { return (ImpLabel*) First(); }
    ImpLabel*   ImplNext() { return (ImpLabel*) Next(); }
    ImpLabel*   ImplGetLabel( ULONG nPos ) const { return (ImpLabel*) GetObject( nPos ); }
    ULONG       ImplGetLabelPos( const String& rLabelName );
    ULONG       ImplCount() const { return Count(); }
};

// ------------------------------------------------------------------------

ImpLabelList::ImpLabelList( const ImpLabelList& rList ) :
        List( rList )
{
    for( ImpLabel* pLabel = ImplFirst(); pLabel; pLabel = ImplNext() )
        ImplReplace( new ImpLabel( *pLabel ) );
}

// ------------------------------------------------------------------------

ImpLabelList::~ImpLabelList()
{
    for( ImpLabel* pLabel = ImplFirst(); pLabel; pLabel = ImplNext() )
        delete pLabel;
}

// ------------------------------------------------------------------------

ULONG ImpLabelList::ImplGetLabelPos( const String& rLabelName )
{
    ULONG nLabelPos = METAFILE_LABEL_NOTFOUND;

    for( ImpLabel* pLabel = ImplFirst(); pLabel; pLabel = ImplNext() )
    {
        if ( rLabelName == pLabel->aLabelName )
        {
            nLabelPos = GetCurPos();
            break;
        }
    }

    return nLabelPos;
}

// ---------------
// - GDIMetaFile -
// ---------------

GDIMetaFile::GDIMetaFile() :
    List        ( 0x3EFF, 64, 64 ),
    aPrefSize   ( 1, 1 ),
    pPrev       ( NULL ),
    pNext       ( NULL ),
    pOutDev     ( NULL ),
    pLabelList  ( NULL ),
    bPause      ( FALSE ),
    bRecord     ( FALSE ),
    bUseCanvas  ( FALSE )
{
}

// ------------------------------------------------------------------------

GDIMetaFile::GDIMetaFile( const GDIMetaFile& rMtf ) :
    List            ( rMtf ),
    aPrefMapMode    ( rMtf.aPrefMapMode ),
    aPrefSize       ( rMtf.aPrefSize ),
    aHookHdlLink    ( rMtf.aHookHdlLink ),
    pPrev           ( rMtf.pPrev ),
    pNext           ( rMtf.pNext ),
    pOutDev         ( NULL ),
    bPause          ( FALSE ),
    bRecord         ( FALSE ),
    bUseCanvas      ( rMtf.bUseCanvas )
{
    // RefCount der MetaActions erhoehen
    for( void* pAct = First(); pAct; pAct = Next() )
        ( (MetaAction*) pAct )->Duplicate();

    if( rMtf.pLabelList )
        pLabelList = new ImpLabelList( *rMtf.pLabelList );
    else
        pLabelList = NULL;

    if( rMtf.bRecord )
    {
        Record( rMtf.pOutDev );

        if ( rMtf.bPause )
            Pause( TRUE );
    }
}

// ------------------------------------------------------------------------

GDIMetaFile::~GDIMetaFile()
{
    Clear();
}

// ------------------------------------------------------------------------

GDIMetaFile& GDIMetaFile::operator=( const GDIMetaFile& rMtf )
{
    if( this != &rMtf )
    {
        Clear();

        List::operator=( rMtf );

        // RefCount der MetaActions erhoehen
        for( void* pAct = First(); pAct; pAct = Next() )
            ( (MetaAction*) pAct )->Duplicate();

        if( rMtf.pLabelList )
            pLabelList = new ImpLabelList( *rMtf.pLabelList );
        else
           pLabelList = NULL;

        aPrefMapMode = rMtf.aPrefMapMode;
        aPrefSize = rMtf.aPrefSize;
        aHookHdlLink = rMtf.aHookHdlLink;
        pPrev = rMtf.pPrev;
        pNext = rMtf.pNext;
        pOutDev = NULL;
        bPause = FALSE;
        bRecord = FALSE;
        bUseCanvas = rMtf.bUseCanvas;

        if( rMtf.bRecord )
        {
            Record( rMtf.pOutDev );

            if( rMtf.bPause )
                Pause( TRUE );
        }
    }

    return *this;
}

// ------------------------------------------------------------------------

BOOL GDIMetaFile::operator==( const GDIMetaFile& rMtf ) const
{
    const ULONG nObjCount = Count();
    BOOL        bRet = FALSE;

    if( this == &rMtf )
        bRet = TRUE;
    else if( rMtf.GetActionCount() == nObjCount &&
             rMtf.GetPrefSize() == aPrefSize &&
             rMtf.GetPrefMapMode() == aPrefMapMode )
    {
        bRet = TRUE;

        for( ULONG n = 0UL; n < nObjCount; n++ )
        {
            if( GetObject( n ) != rMtf.GetObject( n ) )
            {
                bRet = FALSE;
                break;
            }
        }
    }

    return bRet;
}

// ------------------------------------------------------------------------

sal_Bool GDIMetaFile::IsEqual( const GDIMetaFile& rMtf ) const
{
    const ULONG nObjCount = Count();
    BOOL        bRet = FALSE;

    if( this == &rMtf )
        bRet = TRUE;
    else if( rMtf.GetActionCount() == nObjCount &&
             rMtf.GetPrefSize() == aPrefSize &&
             rMtf.GetPrefMapMode() == aPrefMapMode )
    {
        bRet = TRUE;

        for( ULONG n = 0UL; n < nObjCount; n++ )
        {
            if(!((MetaAction*)GetObject( n ))->IsEqual(*((MetaAction*)rMtf.GetObject( n ))))
            {
                bRet = FALSE;
                break;
            }
        }
    }

    return bRet;
}

// ------------------------------------------------------------------------

void GDIMetaFile::Clear()
{
    if( bRecord )
        Stop();

    for( void* pAct = First(); pAct; pAct = Next() )
        ( (MetaAction*) pAct )->Delete();

    List::Clear();

    delete pLabelList;
    pLabelList = NULL;
}

// ------------------------------------------------------------------------

void GDIMetaFile::Linker( OutputDevice* pOut, BOOL bLink )
{
    if( bLink )
    {
        pNext = NULL;
        pPrev = pOut->GetConnectMetaFile();
        pOut->SetConnectMetaFile( this );

        if( pPrev )
            pPrev->pNext = this;
    }
    else
    {
        if( pNext )
        {
            pNext->pPrev = pPrev;

            if( pPrev )
                pPrev->pNext = pNext;
        }
        else
        {
            if( pPrev )
                pPrev->pNext = NULL;

            pOut->SetConnectMetaFile( pPrev );
        }

        pPrev = NULL;
        pNext = NULL;
    }
}

// ------------------------------------------------------------------------

long GDIMetaFile::Hook()
{
    return aHookHdlLink.Call( this );
}

// ------------------------------------------------------------------------

void GDIMetaFile::Record( OutputDevice* pOut )
{
    if( bRecord )
        Stop();

    Last();
    pOutDev = pOut;
    bRecord = TRUE;
    Linker( pOut, TRUE );
}

// ------------------------------------------------------------------------

void GDIMetaFile::Play( GDIMetaFile& rMtf, ULONG nPos )
{
    if ( !bRecord && !rMtf.bRecord )
    {
        MetaAction* pAction = GetCurAction();
        const ULONG nObjCount = Count();

        rMtf.UseCanvas( rMtf.GetUseCanvas() | bUseCanvas );

        if( nPos > nObjCount )
            nPos = nObjCount;

        for( ULONG nCurPos = GetCurPos(); nCurPos < nPos; nCurPos++ )
        {
            if( !Hook() )
            {
                pAction->Duplicate();
                rMtf.AddAction( pAction );
            }

            pAction = (MetaAction*) Next();
        }
    }
}

// ------------------------------------------------------------------------

void GDIMetaFile::Play( OutputDevice* pOut, ULONG nPos )
{
    if( !bRecord )
    {
        MetaAction* pAction = GetCurAction();
        const ULONG nObjCount = Count();
        ULONG       i  = 0, nSyncCount = ( pOut->GetOutDevType() == OUTDEV_WINDOW ) ? 0x000000ff : 0xffffffff;

        if( nPos > nObjCount )
            nPos = nObjCount;

        // #i23407# Set backwards-compatible text language and layout mode
        // This is necessary, since old metafiles don't even know of these
        // recent add-ons. Newer metafiles must of course explicitely set
        // those states.
        pOut->Push( PUSH_TEXTLAYOUTMODE|PUSH_TEXTLANGUAGE );
        pOut->SetLayoutMode( 0 );
        pOut->SetDigitLanguage( 0 );

        for( ULONG nCurPos = GetCurPos(); nCurPos < nPos; nCurPos++ )
        {
            if( !Hook() )
            {
                MetaCommentAction* pCommentAct = static_cast<MetaCommentAction*>(pAction);
                if( pAction->GetType() == META_COMMENT_ACTION &&
                    pCommentAct->GetComment().Equals("DELEGATE_PLUGGABLE_RENDERER") )
                {
                    ImplDelegate2PluggableRenderer(pCommentAct, pOut);
                }
                else
                {
                    pAction->Execute( pOut );
                }

                // flush output from time to time
                if( i++ > nSyncCount )
                    ( (Window*) pOut )->Flush(), i = 0;
            }

            pAction = (MetaAction*) Next();
        }

        pOut->Pop();
    }
}

// ------------------------------------------------------------------------

bool GDIMetaFile::ImplPlayWithRenderer( OutputDevice* pOut, const Point& rPos, Size rDestSize )
{
    const Window* win = dynamic_cast <Window*> ( pOut );

    if (!win)
        win = Application::GetActiveTopWindow();
    if (!win)
        win = Application::GetFirstTopLevelWindow();

    if (!win)
        return false;

    try {
        const uno::Reference<rendering::XCanvas>& xCanvas = win->GetCanvas ();
        Size aSize (rDestSize.Width () + 1, rDestSize.Height () + 1);
        const uno::Reference<rendering::XBitmap>& xBitmap = xCanvas->getDevice ()->createCompatibleAlphaBitmap (vcl::unotools::integerSize2DFromSize( aSize));
        uno::Reference< lang::XMultiServiceFactory > xFactory = vcl::unohelper::GetMultiServiceFactory();
        if( xFactory.is() && xBitmap.is () ) {
            uno::Reference< rendering::XMtfRenderer > xMtfRenderer;
            uno::Sequence< uno::Any > args (1);
            uno::Reference< rendering::XBitmapCanvas > xBitmapCanvas( xBitmap, uno::UNO_QUERY );
            if( xBitmapCanvas.is() ) {
                args[0] = uno::Any( xBitmapCanvas );
                xMtfRenderer.set( xFactory->createInstanceWithArguments( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.rendering.MtfRenderer")),
                                                                         args ), uno::UNO_QUERY );

                if( xMtfRenderer.is() ) {
                    xBitmapCanvas->clear();
                    uno::Reference< beans::XFastPropertySet > xMtfFastPropertySet( xMtfRenderer, uno::UNO_QUERY );
                    if( xMtfFastPropertySet.is() )
                        // set this metafile to the renderer to
                        // speedup things (instead of copying data to
                        // sequence of bytes passed to renderer)
                        xMtfFastPropertySet->setFastPropertyValue( 0, uno::Any( reinterpret_cast<sal_Int64>( this ) ) );

                    xMtfRenderer->draw( rDestSize.Width(), rDestSize.Height() );

                    uno::Reference< beans::XFastPropertySet > xFastPropertySet( xBitmapCanvas, uno::UNO_QUERY );
                    if( xFastPropertySet.get() ) {
                        // 0 means get BitmapEx
                        uno::Any aAny = xFastPropertySet->getFastPropertyValue( 0 );
                        BitmapEx* pBitmapEx = (BitmapEx*) *reinterpret_cast<const sal_Int64*>(aAny.getValue());
                        if( pBitmapEx ) {
                            pOut->DrawBitmapEx( rPos, *pBitmapEx );
                            delete pBitmapEx;
                            return true;
                        }
                    }

                    SalBitmap* pSalBmp = ImplGetSVData()->mpDefInst->CreateSalBitmap();
                    SalBitmap* pSalMask = ImplGetSVData()->mpDefInst->CreateSalBitmap();

                    if( pSalBmp->Create( xBitmapCanvas, aSize ) && pSalMask->Create( xBitmapCanvas, aSize, true ) ) {
                        Bitmap aBitmap( pSalBmp );
                        Bitmap aMask( pSalMask );
                        AlphaMask aAlphaMask( aMask );
                        BitmapEx aBitmapEx( aBitmap, aAlphaMask );
                        pOut->DrawBitmapEx( rPos, aBitmapEx );
                        return true;
                    }

                    delete pSalBmp;
                    delete pSalMask;
                }
            }
        }
    } catch( uno::RuntimeException& ) {
        throw; // runtime errors are fatal
    } catch( uno::Exception& ) {
        // ignore errors, no way of reporting them here
    }

    return false;
}

// ------------------------------------------------------------------------

void GDIMetaFile::ImplDelegate2PluggableRenderer( const MetaCommentAction* pAct, OutputDevice* pOut )
{
    OSL_ASSERT( pAct->GetComment().Equals("DELEGATE_PLUGGABLE_RENDERER") );

    // read payload - string of service name, followed by raw render input
    const BYTE* pData = pAct->GetData();
    const BYTE* const pEndData = pData + pAct->GetDataSize();
    if( !pData )
        return;

    ::rtl::OUStringBuffer aBuffer;
    while( pData<pEndData && *pData )
        aBuffer.append(static_cast<sal_Unicode>(*pData++));
    const ::rtl::OUString aRendererServiceName=aBuffer.makeStringAndClear();
    ++pData;

    while( pData<pEndData && *pData )
        aBuffer.append(static_cast<sal_Unicode>(*pData++));
    const ::rtl::OUString aGraphicServiceName=aBuffer.makeStringAndClear();
    ++pData;

    uno::Reference< lang::XMultiServiceFactory > xFactory = vcl::unohelper::GetMultiServiceFactory();
    if( pData<pEndData && xFactory.is() )
    {
        try
        {
            // instantiate render service
            uno::Sequence<uno::Any> aRendererArgs(1);
            aRendererArgs[0] = makeAny(uno::Reference<awt::XGraphics>(pOut->CreateUnoGraphics()));
            uno::Reference<graphic::XGraphicRenderer> xRenderer(
                xFactory->createInstanceWithArguments(
                    aRendererServiceName,
                    aRendererArgs),
                uno::UNO_QUERY );

            // instantiate graphic service
            uno::Reference<graphic::XGraphic> xGraphic(
                xFactory->createInstance(
                    aGraphicServiceName),
                uno::UNO_QUERY );

            uno::Reference<lang::XInitialization> xInit(
                xGraphic, uno::UNO_QUERY);

            if(xGraphic.is() && xRenderer.is() && xInit.is())
            {
                // delay intialization of XGraphic, to only expose
                // XGraphic-generating services to arbitrary binary data
                uno::Sequence< sal_Int8 > aSeq(
                    (sal_Int8*)&pData, pEndData-pData );
                uno::Sequence<uno::Any> aGraphicsArgs(1);
                aGraphicsArgs[0] = makeAny(aSeq);
                xInit->initialize(aGraphicsArgs);

                xRenderer->render(xGraphic);
            }
        }
        catch( uno::RuntimeException& )
        {
            // runtime errors are fatal
            throw;
        }
        catch( uno::Exception& )
        {
            // ignore errors, no way of reporting them here
        }
    }
}

// ------------------------------------------------------------------------

void GDIMetaFile::Play( OutputDevice* pOut, const Point& rPos,
                        const Size& rSize, ULONG nPos )
{
    Region  aDrawClipRegion;
    MapMode aDrawMap( GetPrefMapMode() );
    Size    aDestSize( pOut->LogicToPixel( rSize ) );

    if( aDestSize.Width() && aDestSize.Height() )
    {
        GDIMetaFile*    pMtf = pOut->GetConnectMetaFile();

        if( bUseCanvas && !pMtf && ImplPlayWithRenderer( pOut, rPos, aDestSize ) )
            return;

        Size aTmpPrefSize( pOut->LogicToPixel( GetPrefSize(), aDrawMap ) );

        if( !aTmpPrefSize.Width() )
            aTmpPrefSize.Width() = aDestSize.Width();

        if( !aTmpPrefSize.Height() )
            aTmpPrefSize.Height() = aDestSize.Height();

        Fraction aScaleX( aDestSize.Width(), aTmpPrefSize.Width() );
        Fraction aScaleY( aDestSize.Height(), aTmpPrefSize.Height() );

        aScaleX *= aDrawMap.GetScaleX(); aDrawMap.SetScaleX( aScaleX );
        aScaleY *= aDrawMap.GetScaleY(); aDrawMap.SetScaleY( aScaleY );

        // #i47260# Convert logical output position to offset within
        // the metafile's mapmode. Therefore, disable pixel offset on
        // outdev, it's inverse mnOutOffLogicX/Y is calculated for a
        // different mapmode (the one currently set on pOut, that is)
        // - thus, aDrawMap's origin would generally be wrong. And
        // even _if_ aDrawMap is similar to pOutDev's current mapmode,
        // it's _still_ undesirable to have pixel offset unequal zero,
        // because one would still get round-off errors (the
        // round-trip error for LogicToPixel( PixelToLogic() ) was the
        // reason for having pixel offset in the first place).
        const Size& rOldOffset( pOut->GetPixelOffset() );
        const Size  aEmptySize;
        pOut->SetPixelOffset( aEmptySize );
        aDrawMap.SetOrigin( pOut->PixelToLogic( pOut->LogicToPixel( rPos ), aDrawMap ) );
        pOut->SetPixelOffset( rOldOffset );

        pOut->Push();

        if ( pMtf && pMtf->IsRecord() && ( pOut->GetOutDevType() != OUTDEV_PRINTER ) )
            pOut->SetRelativeMapMode( aDrawMap );
        else
            pOut->SetMapMode( aDrawMap );

        // #i23407# Set backwards-compatible text language and layout mode
        // This is necessary, since old metafiles don't even know of these
        // recent add-ons. Newer metafiles must of course explicitely set
        // those states.
        pOut->SetLayoutMode( 0 );
        pOut->SetDigitLanguage( 0 );

        Play( pOut, nPos );

        pOut->Pop();
    }
}

// ------------------------------------------------------------------------

void GDIMetaFile::Pause( BOOL _bPause )
{
    if( bRecord )
    {
        if( _bPause )
        {
            if( !bPause )
                Linker( pOutDev, FALSE );
        }
        else
        {
            if( bPause )
                Linker( pOutDev, TRUE );
        }

        bPause = _bPause;
    }
}

// ------------------------------------------------------------------------

void GDIMetaFile::Stop()
{
    if( bRecord )
    {
        bRecord = FALSE;

        if( !bPause )
            Linker( pOutDev, FALSE );
        else
            bPause = FALSE;
    }
}

// ------------------------------------------------------------------------

void GDIMetaFile::WindStart()
{
    if( !bRecord )
        First();
}

// ------------------------------------------------------------------------

void GDIMetaFile::WindEnd()
{
    if( !bRecord )
        Last();
}

// ------------------------------------------------------------------------

void GDIMetaFile::Wind( ULONG nActionPos )
{
    if( !bRecord )
        Seek( nActionPos );
}

// ------------------------------------------------------------------------

void GDIMetaFile::WindPrev()
{
    if( !bRecord )
        Prev();
}

// ------------------------------------------------------------------------

void GDIMetaFile::WindNext()
{
    if( !bRecord )
        Next();
}

// ------------------------------------------------------------------------

void GDIMetaFile::AddAction( MetaAction* pAction )
{
    Insert( pAction, LIST_APPEND );

    if( pPrev )
    {
        pAction->Duplicate();
        pPrev->AddAction( pAction );
    }
}

// ------------------------------------------------------------------------

void GDIMetaFile::AddAction( MetaAction* pAction, ULONG nPos )
{
    Insert( pAction, nPos );

    if( pPrev )
    {
        pAction->Duplicate();
        pPrev->AddAction( pAction, nPos );
    }
}

// ------------------------------------------------------------------------

// @since #110496#
void GDIMetaFile::RemoveAction( ULONG nPos )
{
    Remove( nPos );

    if( pPrev )
        pPrev->RemoveAction( nPos );
}

// ------------------------------------------------------------------------

MetaAction* GDIMetaFile::CopyAction( ULONG nPos ) const
{
    return ( (MetaAction*) GetObject( nPos ) )->Clone();
}

// ------------------------------------------------------------------------

ULONG GDIMetaFile::GetActionPos( const String& rLabel )
{
    ImpLabel* pLabel = NULL;

    if( pLabelList )
        pLabel = pLabelList->ImplGetLabel( pLabelList->ImplGetLabelPos( rLabel ) );
    else
        pLabel = NULL;

    return( pLabel ? pLabel->nActionPos : METAFILE_LABEL_NOTFOUND );
}

// ------------------------------------------------------------------------

BOOL GDIMetaFile::InsertLabel( const String& rLabel, ULONG nActionPos )
{
    BOOL bRet = FALSE;

    if( !pLabelList )
        pLabelList = new ImpLabelList;

    if( pLabelList->ImplGetLabelPos( rLabel ) == METAFILE_LABEL_NOTFOUND )
    {
        pLabelList->ImplInsert( new ImpLabel( rLabel, nActionPos ) );
        bRet = TRUE;
    }

    return bRet;
}

// ------------------------------------------------------------------------

void GDIMetaFile::RemoveLabel( const String& rLabel )
{
    if( pLabelList )
    {
        const ULONG nLabelPos = pLabelList->ImplGetLabelPos( rLabel );

        if( nLabelPos != METAFILE_LABEL_NOTFOUND )
            delete pLabelList->ImplRemove( nLabelPos );
    }
}

// ------------------------------------------------------------------------

void GDIMetaFile::RenameLabel( const String& rLabel, const String& rNewLabel )
{
    if( pLabelList )
    {
        const ULONG nLabelPos = pLabelList->ImplGetLabelPos( rLabel );

        if ( nLabelPos != METAFILE_LABEL_NOTFOUND )
            pLabelList->ImplGetLabel( nLabelPos )->aLabelName = rNewLabel;
    }
}

// ------------------------------------------------------------------------

ULONG GDIMetaFile::GetLabelCount() const
{
    return( pLabelList ? pLabelList->ImplCount() : 0UL );
}

// ------------------------------------------------------------------------

String GDIMetaFile::GetLabel( ULONG nLabel )
{
    String aString;

    if( pLabelList )
    {
        const ImpLabel* pLabel = pLabelList->ImplGetLabel( nLabel );

        if( pLabel )
            aString = pLabel->aLabelName;
    }

    return aString;
}

// ------------------------------------------------------------------------

BOOL GDIMetaFile::SaveStatus()
{
    if ( bRecord )
    {
        if ( bPause )
            Linker( pOutDev, TRUE );

        AddAction( new MetaLineColorAction( pOutDev->GetLineColor(),
                                            pOutDev->IsLineColor() ) );
        AddAction( new MetaFillColorAction( pOutDev->GetFillColor(),
                                            pOutDev->IsFillColor() ) );
        AddAction( new MetaFontAction( pOutDev->GetFont() ) );
        AddAction( new MetaTextColorAction( pOutDev->GetTextColor() ) );
        AddAction( new MetaTextFillColorAction( pOutDev->GetTextFillColor(),
                                                pOutDev->IsTextFillColor() ) );
        AddAction( new MetaTextLineColorAction( pOutDev->GetTextLineColor(),
                                                pOutDev->IsTextLineColor() ) );
        AddAction( new MetaOverlineColorAction( pOutDev->GetOverlineColor(),
                                                pOutDev->IsOverlineColor() ) );
        AddAction( new MetaTextAlignAction( pOutDev->GetTextAlign() ) );
        AddAction( new MetaRasterOpAction( pOutDev->GetRasterOp() ) );
        AddAction( new MetaMapModeAction( pOutDev->GetMapMode() ) );
        AddAction( new MetaClipRegionAction( pOutDev->GetClipRegion(),
                                             pOutDev->IsClipRegion() ) );

        if ( bPause )
            Linker( pOutDev, FALSE );

        return TRUE;
    }
    else
        return FALSE;
}

// ------------------------------------------------------------------------

BOOL GDIMetaFile::Mirror( ULONG nMirrorFlags )
{
    const Size  aOldPrefSize( GetPrefSize() );
    long        nMoveX, nMoveY;
    double      fScaleX, fScaleY;
    BOOL        bRet;

    if( nMirrorFlags & MTF_MIRROR_HORZ )
        nMoveX = SAL_ABS( aOldPrefSize.Width() ) - 1, fScaleX = -1.0;
    else
        nMoveX = 0, fScaleX = 1.0;

    if( nMirrorFlags & MTF_MIRROR_VERT )
        nMoveY = SAL_ABS( aOldPrefSize.Height() ) - 1, fScaleY = -1.0;
    else
        nMoveY = 0, fScaleY = 1.0;

    if( ( fScaleX != 1.0 ) || ( fScaleY != 1.0 ) )
    {
        Scale( fScaleX, fScaleY );
        Move( nMoveX, nMoveY );
        SetPrefSize( aOldPrefSize );
        bRet = TRUE;
    }
    else
        bRet = FALSE;

    return bRet;
}

// ------------------------------------------------------------------------

void GDIMetaFile::Move( long nX, long nY )
{
    const Size      aBaseOffset( nX, nY );
    Size            aOffset( aBaseOffset );
    VirtualDevice   aMapVDev;

    aMapVDev.EnableOutput( FALSE );
    aMapVDev.SetMapMode( GetPrefMapMode() );

    for( MetaAction* pAct = (MetaAction*) First(); pAct; pAct = (MetaAction*) Next() )
    {
        const long  nType = pAct->GetType();
        MetaAction* pModAct;

        if( pAct->GetRefCount() > 1 )
        {
            Replace( pModAct = pAct->Clone(), GetCurPos() );
            pAct->Delete();
        }
        else
            pModAct = pAct;

        if( ( META_MAPMODE_ACTION == nType ) ||
            ( META_PUSH_ACTION == nType ) ||
            ( META_POP_ACTION == nType ) )
        {
            pModAct->Execute( &aMapVDev );
            aOffset = aMapVDev.LogicToLogic( aBaseOffset, GetPrefMapMode(), aMapVDev.GetMapMode() );
        }

        pModAct->Move( aOffset.Width(), aOffset.Height() );
    }
}

void GDIMetaFile::Move( long nX, long nY, long nDPIX, long nDPIY )
{
    const Size      aBaseOffset( nX, nY );
    Size            aOffset( aBaseOffset );
    VirtualDevice   aMapVDev;

    aMapVDev.EnableOutput( FALSE );
    aMapVDev.SetReferenceDevice( nDPIX, nDPIY );
    aMapVDev.SetMapMode( GetPrefMapMode() );

    for( MetaAction* pAct = (MetaAction*) First(); pAct; pAct = (MetaAction*) Next() )
    {
        const long  nType = pAct->GetType();
        MetaAction* pModAct;

        if( pAct->GetRefCount() > 1 )
        {
            Replace( pModAct = pAct->Clone(), GetCurPos() );
            pAct->Delete();
        }
        else
            pModAct = pAct;

        if( ( META_MAPMODE_ACTION == nType ) ||
            ( META_PUSH_ACTION == nType ) ||
            ( META_POP_ACTION == nType ) )
        {
            pModAct->Execute( &aMapVDev );
            if( aMapVDev.GetMapMode().GetMapUnit() == MAP_PIXEL )
            {
                aOffset = aMapVDev.LogicToPixel( aBaseOffset, GetPrefMapMode() );
                MapMode aMap( aMapVDev.GetMapMode() );
                aOffset.Width() = static_cast<long>(aOffset.Width() * (double)aMap.GetScaleX());
                aOffset.Height() = static_cast<long>(aOffset.Height() * (double)aMap.GetScaleY());
            }
            else
                aOffset = aMapVDev.LogicToLogic( aBaseOffset, GetPrefMapMode(), aMapVDev.GetMapMode() );
        }

        pModAct->Move( aOffset.Width(), aOffset.Height() );
    }
}

// ------------------------------------------------------------------------

void GDIMetaFile::Scale( double fScaleX, double fScaleY )
{
    for( MetaAction* pAct = (MetaAction*) First(); pAct; pAct = (MetaAction*) Next() )
    {
        MetaAction* pModAct;

        if( pAct->GetRefCount() > 1 )
        {
            Replace( pModAct = pAct->Clone(), GetCurPos() );
            pAct->Delete();
        }
        else
            pModAct = pAct;

        pModAct->Scale( fScaleX, fScaleY );
    }

    aPrefSize.Width() = FRound( aPrefSize.Width() * fScaleX );
    aPrefSize.Height() = FRound( aPrefSize.Height() * fScaleY );
}

// ------------------------------------------------------------------------

void GDIMetaFile::Scale( const Fraction& rScaleX, const Fraction& rScaleY )
{
    Scale( (double) rScaleX, (double) rScaleY );
}

// ------------------------------------------------------------------------

void GDIMetaFile::Clip( const Rectangle& i_rClipRect )
{
    Rectangle aCurRect( i_rClipRect );
    VirtualDevice   aMapVDev;

    aMapVDev.EnableOutput( FALSE );
    aMapVDev.SetMapMode( GetPrefMapMode() );

    for( MetaAction* pAct = (MetaAction*) First(); pAct; pAct = (MetaAction*) Next() )
    {
        const long  nType = pAct->GetType();

        if( ( META_MAPMODE_ACTION == nType ) ||
            ( META_PUSH_ACTION == nType ) ||
            ( META_POP_ACTION == nType ) )
        {
            pAct->Execute( &aMapVDev );
            aCurRect = aMapVDev.LogicToLogic( i_rClipRect, GetPrefMapMode(), aMapVDev.GetMapMode() );
        }
        else if( nType == META_CLIPREGION_ACTION )
        {
            MetaClipRegionAction* pOldAct = (MetaClipRegionAction*)pAct;
            Region aNewReg( aCurRect );
            if( pOldAct->IsClipping() )
                aNewReg.Intersect( pOldAct->GetRegion() );
            MetaClipRegionAction* pNewAct = new MetaClipRegionAction( aNewReg, TRUE );
            Replace( pNewAct, GetCurPos() );
            pOldAct->Delete();
        }
    }
}

// ------------------------------------------------------------------------

Point GDIMetaFile::ImplGetRotatedPoint( const Point& rPt, const Point& rRotatePt,
                                        const Size& rOffset, double fSin, double fCos )
{
    const long nX = rPt.X() - rRotatePt.X();
    const long nY = rPt.Y() - rRotatePt.Y();

    return Point( FRound( fCos * nX + fSin * nY ) + rRotatePt.X() + rOffset.Width(),
                  -FRound( fSin * nX - fCos * nY ) + rRotatePt.Y() + rOffset.Height() );
}

// ------------------------------------------------------------------------

Polygon GDIMetaFile::ImplGetRotatedPolygon( const Polygon& rPoly, const Point& rRotatePt,
                                            const Size& rOffset, double fSin, double fCos )
{
    Polygon aRet( rPoly );

    aRet.Rotate( rRotatePt, fSin, fCos );
    aRet.Move( rOffset.Width(), rOffset.Height() );

    return aRet;
}

// ------------------------------------------------------------------------

PolyPolygon GDIMetaFile::ImplGetRotatedPolyPolygon( const PolyPolygon& rPolyPoly, const Point& rRotatePt,
                                                    const Size& rOffset, double fSin, double fCos )
{
    PolyPolygon aRet( rPolyPoly );

    aRet.Rotate( rRotatePt, fSin, fCos );
    aRet.Move( rOffset.Width(), rOffset.Height() );

    return aRet;
}

// ------------------------------------------------------------------------

void GDIMetaFile::ImplAddGradientEx( GDIMetaFile&         rMtf,
                                     const OutputDevice&  rMapDev,
                                     const PolyPolygon&   rPolyPoly,
                                     const Gradient&      rGrad     )
{
    // #105055# Generate comment, GradientEx and Gradient actions
    // (within DrawGradient)
    VirtualDevice aVDev( rMapDev, 0 );
    aVDev.EnableOutput( FALSE );
    GDIMetaFile aGradMtf;

    aGradMtf.Record( &aVDev );
    aVDev.DrawGradient( rPolyPoly, rGrad );
    aGradMtf.Stop();

    int i, nAct( aGradMtf.GetActionCount() );
    for( i=0; i<nAct; ++i )
    {
        MetaAction* pMetaAct = aGradMtf.GetAction(i);
        pMetaAct->Duplicate();
        rMtf.AddAction( pMetaAct );
    }
}

// ------------------------------------------------------------------------

void GDIMetaFile::Rotate( long nAngle10 )
{
    nAngle10 %= 3600L;
    nAngle10 = ( nAngle10 < 0L ) ? ( 3599L + nAngle10 ) : nAngle10;

    if( nAngle10 )
    {
        GDIMetaFile     aMtf;
        VirtualDevice   aMapVDev;
        const double    fAngle = F_PI1800 * nAngle10;
        const double    fSin = sin( fAngle );
        const double    fCos = cos( fAngle );
        Rectangle       aRect=Rectangle( Point(), GetPrefSize() );
        Polygon         aPoly( aRect );

        aPoly.Rotate( Point(), fSin, fCos );

        aMapVDev.EnableOutput( FALSE );
        aMapVDev.SetMapMode( GetPrefMapMode() );

        const Rectangle aNewBound( aPoly.GetBoundRect() );

        const Point aOrigin( GetPrefMapMode().GetOrigin().X(), GetPrefMapMode().GetOrigin().Y() );
        const Size  aOffset( -aNewBound.Left(), -aNewBound.Top() );

        Point     aRotAnchor( aOrigin );
        Size      aRotOffset( aOffset );

        for( MetaAction* pAction = (MetaAction*) First(); pAction; pAction = (MetaAction*) Next() )
        {
            const USHORT nActionType = pAction->GetType();

            switch( nActionType )
            {
                case( META_PIXEL_ACTION ):
                {
                    MetaPixelAction* pAct = (MetaPixelAction*) pAction;
                    aMtf.AddAction( new MetaPixelAction( ImplGetRotatedPoint( pAct->GetPoint(), aRotAnchor, aRotOffset, fSin, fCos ),
                                                                              pAct->GetColor() ) );
                }
                break;

                case( META_POINT_ACTION ):
                {
                    MetaPointAction* pAct = (MetaPointAction*) pAction;
                    aMtf.AddAction( new MetaPointAction( ImplGetRotatedPoint( pAct->GetPoint(), aRotAnchor, aRotOffset, fSin, fCos ) ) );
                }
                break;

                case( META_LINE_ACTION ):
                {
                    MetaLineAction* pAct = (MetaLineAction*) pAction;
                    aMtf.AddAction( new MetaLineAction( ImplGetRotatedPoint( pAct->GetStartPoint(), aRotAnchor, aRotOffset, fSin, fCos ),
                                                        ImplGetRotatedPoint( pAct->GetEndPoint(), aRotAnchor, aRotOffset, fSin, fCos ),
                                                        pAct->GetLineInfo() ) );
                }
                break;

                case( META_RECT_ACTION ):
                {
                    MetaRectAction* pAct = (MetaRectAction*) pAction;
                    aMtf.AddAction( new MetaPolygonAction( ImplGetRotatedPolygon( pAct->GetRect(), aRotAnchor, aRotOffset, fSin, fCos ) ) );
                }
                break;

                case( META_ROUNDRECT_ACTION ):
                {
                    MetaRoundRectAction*    pAct = (MetaRoundRectAction*) pAction;
                    const Polygon           aRoundRectPoly( pAct->GetRect(), pAct->GetHorzRound(), pAct->GetVertRound() );

                    aMtf.AddAction( new MetaPolygonAction( ImplGetRotatedPolygon( aRoundRectPoly, aRotAnchor, aRotOffset, fSin, fCos ) ) );
                }
                break;

                case( META_ELLIPSE_ACTION ):
                {
                    MetaEllipseAction*      pAct = (MetaEllipseAction*) pAction;
                    const Polygon           aEllipsePoly( pAct->GetRect().Center(), pAct->GetRect().GetWidth() >> 1, pAct->GetRect().GetHeight() >> 1 );

                    aMtf.AddAction( new MetaPolygonAction( ImplGetRotatedPolygon( aEllipsePoly, aRotAnchor, aRotOffset, fSin, fCos ) ) );
                }
                break;

                case( META_ARC_ACTION ):
                {
                    MetaArcAction*  pAct = (MetaArcAction*) pAction;
                    const Polygon   aArcPoly( pAct->GetRect(), pAct->GetStartPoint(), pAct->GetEndPoint(), POLY_ARC );

                    aMtf.AddAction( new MetaPolygonAction( ImplGetRotatedPolygon( aArcPoly, aRotAnchor, aRotOffset, fSin, fCos ) ) );
                }
                break;

                case( META_PIE_ACTION ):
                {
                    MetaPieAction*  pAct = (MetaPieAction*) pAction;
                    const Polygon   aPiePoly( pAct->GetRect(), pAct->GetStartPoint(), pAct->GetEndPoint(), POLY_PIE );

                    aMtf.AddAction( new MetaPolygonAction( ImplGetRotatedPolygon( aPiePoly, aRotAnchor, aRotOffset, fSin, fCos ) ) );
                }
                break;

                case( META_CHORD_ACTION ):
                {
                    MetaChordAction*    pAct = (MetaChordAction*) pAction;
                    const Polygon       aChordPoly( pAct->GetRect(), pAct->GetStartPoint(), pAct->GetEndPoint(), POLY_CHORD );

                    aMtf.AddAction( new MetaPolygonAction( ImplGetRotatedPolygon( aChordPoly, aRotAnchor, aRotOffset, fSin, fCos ) ) );
                }
                break;

                case( META_POLYLINE_ACTION ):
                {
                    MetaPolyLineAction* pAct = (MetaPolyLineAction*) pAction;
                    aMtf.AddAction( new MetaPolyLineAction( ImplGetRotatedPolygon( pAct->GetPolygon(), aRotAnchor, aRotOffset, fSin, fCos ), pAct->GetLineInfo() ) );
                }
                break;

                case( META_POLYGON_ACTION ):
                {
                    MetaPolygonAction* pAct = (MetaPolygonAction*) pAction;
                    aMtf.AddAction( new MetaPolygonAction( ImplGetRotatedPolygon( pAct->GetPolygon(), aRotAnchor, aRotOffset, fSin, fCos ) ) );
                }
                break;

                case( META_POLYPOLYGON_ACTION ):
                {
                    MetaPolyPolygonAction* pAct = (MetaPolyPolygonAction*) pAction;
                    aMtf.AddAction( new MetaPolyPolygonAction( ImplGetRotatedPolyPolygon( pAct->GetPolyPolygon(), aRotAnchor, aRotOffset, fSin, fCos ) ) );
                }
                break;

                case( META_TEXT_ACTION ):
                {
                    MetaTextAction* pAct = (MetaTextAction*) pAction;
                    aMtf.AddAction( new MetaTextAction( ImplGetRotatedPoint( pAct->GetPoint(), aRotAnchor, aRotOffset, fSin, fCos ),
                                                                             pAct->GetText(), pAct->GetIndex(), pAct->GetLen() ) );
                }
                break;

                case( META_TEXTARRAY_ACTION ):
                {
                    MetaTextArrayAction* pAct = (MetaTextArrayAction*) pAction;
                    aMtf.AddAction( new MetaTextArrayAction( ImplGetRotatedPoint( pAct->GetPoint(), aRotAnchor, aRotOffset, fSin, fCos ),
                                                                                  pAct->GetText(), pAct->GetDXArray(), pAct->GetIndex(), pAct->GetLen() ) );
                }
                break;

                case( META_STRETCHTEXT_ACTION ):
                {
                    MetaStretchTextAction* pAct = (MetaStretchTextAction*) pAction;
                    aMtf.AddAction( new MetaStretchTextAction( ImplGetRotatedPoint( pAct->GetPoint(), aRotAnchor, aRotOffset, fSin, fCos ),
                                                                                    pAct->GetWidth(), pAct->GetText(), pAct->GetIndex(), pAct->GetLen() ) );
                }
                break;

                case( META_TEXTLINE_ACTION ):
                {
                    MetaTextLineAction* pAct = (MetaTextLineAction*) pAction;
                    aMtf.AddAction( new MetaTextLineAction( ImplGetRotatedPoint( pAct->GetStartPoint(), aRotAnchor, aRotOffset, fSin, fCos ),
                                                                                 pAct->GetWidth(), pAct->GetStrikeout(), pAct->GetUnderline(), pAct->GetOverline() ) );
                }
                break;

                case( META_BMPSCALE_ACTION ):
                {
                    MetaBmpScaleAction* pAct = (MetaBmpScaleAction*) pAction;
                    Polygon             aBmpPoly( ImplGetRotatedPolygon( Rectangle( pAct->GetPoint(), pAct->GetSize() ), aRotAnchor, aRotOffset, fSin, fCos ) );
                    Rectangle           aBmpRect( aBmpPoly.GetBoundRect() );
                    BitmapEx            aBmpEx( pAct->GetBitmap() );

                    aBmpEx.Rotate( nAngle10, Color( COL_TRANSPARENT ) );
                    aMtf.AddAction( new MetaBmpExScaleAction( aBmpRect.TopLeft(), aBmpRect.GetSize(),
                                                              aBmpEx ) );
                }
                break;

                case( META_BMPSCALEPART_ACTION ):
                {
                    MetaBmpScalePartAction* pAct = (MetaBmpScalePartAction*) pAction;
                    Polygon                 aBmpPoly( ImplGetRotatedPolygon( Rectangle( pAct->GetDestPoint(), pAct->GetDestSize() ), aRotAnchor, aRotOffset, fSin, fCos ) );
                    Rectangle               aBmpRect( aBmpPoly.GetBoundRect() );
                    BitmapEx                aBmpEx( pAct->GetBitmap() );

                    aBmpEx.Crop( Rectangle( pAct->GetSrcPoint(), pAct->GetSrcSize() ) );
                    aBmpEx.Rotate( nAngle10, Color( COL_TRANSPARENT ) );

                    aMtf.AddAction( new MetaBmpExScaleAction( aBmpRect.TopLeft(), aBmpRect.GetSize(), aBmpEx ) );
                }
                break;

                case( META_BMPEXSCALE_ACTION ):
                {
                    MetaBmpExScaleAction*   pAct = (MetaBmpExScaleAction*) pAction;
                    Polygon                 aBmpPoly( ImplGetRotatedPolygon( Rectangle( pAct->GetPoint(), pAct->GetSize() ), aRotAnchor, aRotOffset, fSin, fCos ) );
                    Rectangle               aBmpRect( aBmpPoly.GetBoundRect() );
                    BitmapEx                aBmpEx( pAct->GetBitmapEx() );

                    aBmpEx.Rotate( nAngle10, Color( COL_TRANSPARENT ) );

                    aMtf.AddAction( new MetaBmpExScaleAction( aBmpRect.TopLeft(), aBmpRect.GetSize(), aBmpEx ) );
                }
                break;

                case( META_BMPEXSCALEPART_ACTION ):
                {
                    MetaBmpExScalePartAction*   pAct = (MetaBmpExScalePartAction*) pAction;
                    Polygon                     aBmpPoly( ImplGetRotatedPolygon( Rectangle( pAct->GetDestPoint(), pAct->GetDestSize() ), aRotAnchor, aRotOffset, fSin, fCos ) );
                    Rectangle                   aBmpRect( aBmpPoly.GetBoundRect() );
                    BitmapEx                    aBmpEx( pAct->GetBitmapEx() );

                    aBmpEx.Crop( Rectangle( pAct->GetSrcPoint(), pAct->GetSrcSize() ) );
                    aBmpEx.Rotate( nAngle10, Color( COL_TRANSPARENT ) );

                    aMtf.AddAction( new MetaBmpExScaleAction( aBmpRect.TopLeft(), aBmpRect.GetSize(), aBmpEx ) );
                }
                break;

                case( META_GRADIENT_ACTION ):
                {
                    MetaGradientAction* pAct = (MetaGradientAction*) pAction;

                    ImplAddGradientEx( aMtf, aMapVDev,
                                       ImplGetRotatedPolygon( pAct->GetRect(), aRotAnchor, aRotOffset, fSin, fCos ),
                                       pAct->GetGradient() );
                }
                break;

                case( META_GRADIENTEX_ACTION ):
                {
                    MetaGradientExAction* pAct = (MetaGradientExAction*) pAction;
                    aMtf.AddAction( new MetaGradientExAction( ImplGetRotatedPolyPolygon( pAct->GetPolyPolygon(), aRotAnchor, aRotOffset, fSin, fCos ),
                                                              pAct->GetGradient() ) );
                }
                break;

                // #105055# Handle gradientex comment block correctly
                case( META_COMMENT_ACTION ):
                {
                    MetaCommentAction* pCommentAct = (MetaCommentAction*) pAction;
                    if( pCommentAct->GetComment().Equals( "XGRAD_SEQ_BEGIN" ) )
                    {
                        int nBeginComments( 1 );
                        pAction = (MetaAction*) Next();

                        // skip everything, except gradientex action
                        while( pAction )
                        {
                            const USHORT nType = pAction->GetType();

                            if( META_GRADIENTEX_ACTION == nType )
                            {
                                // Add rotated gradientex
                                MetaGradientExAction* pAct = (MetaGradientExAction*) pAction;
                                ImplAddGradientEx( aMtf, aMapVDev,
                                                   ImplGetRotatedPolyPolygon( pAct->GetPolyPolygon(), aRotAnchor, aRotOffset, fSin, fCos ),
                                                   pAct->GetGradient() );
                            }
                            else if( META_COMMENT_ACTION == nType)
                            {
                                MetaCommentAction* pAct = (MetaCommentAction*) pAction;
                                if( pAct->GetComment().Equals( "XGRAD_SEQ_END" ) )
                                {
                                    // handle nested blocks
                                    --nBeginComments;

                                    // gradientex comment block: end reached, done.
                                    if( !nBeginComments )
                                        break;
                                }
                                else if( pAct->GetComment().Equals( "XGRAD_SEQ_BEGIN" ) )
                                {
                                    // handle nested blocks
                                    ++nBeginComments;
                                }

                            }

                            pAction = (MetaAction*) Next();
                        }
                    }
                    else
                    {
                        sal_Bool bPathStroke = pCommentAct->GetComment().Equals( "XPATHSTROKE_SEQ_BEGIN" );
                        if ( bPathStroke || pCommentAct->GetComment().Equals( "XPATHFILL_SEQ_BEGIN" ) )
                        {
                            if ( pCommentAct->GetDataSize() )
                            {
                                SvMemoryStream aMemStm( (void*)pCommentAct->GetData(), pCommentAct->GetDataSize(), STREAM_READ );
                                SvMemoryStream aDest;
                                if ( bPathStroke )
                                {
                                    SvtGraphicStroke aStroke;
                                    aMemStm >> aStroke;
                                    Polygon aPath;
                                    aStroke.getPath( aPath );
                                    aStroke.setPath( ImplGetRotatedPolygon( aPath, aRotAnchor, aRotOffset, fSin, fCos ) );
                                    aDest << aStroke;
                                    aMtf.AddAction( new MetaCommentAction( "XPATHSTROKE_SEQ_BEGIN", 0,
                                                        static_cast<const BYTE*>( aDest.GetData()), aDest.Tell() ) );
                                }
                                else
                                {
                                    SvtGraphicFill aFill;
                                    aMemStm >> aFill;
                                    PolyPolygon aPath;
                                    aFill.getPath( aPath );
                                    aFill.setPath( ImplGetRotatedPolyPolygon( aPath, aRotAnchor, aRotOffset, fSin, fCos ) );
                                    aDest << aFill;
                                    aMtf.AddAction( new MetaCommentAction( "XPATHFILL_SEQ_BEGIN", 0,
                                                        static_cast<const BYTE*>( aDest.GetData()), aDest.Tell() ) );
                                }
                            }
                        }
                        else if ( pCommentAct->GetComment().Equals( "XPATHSTROKE_SEQ_END" )
                               || pCommentAct->GetComment().Equals( "XPATHFILL_SEQ_END" ) )
                        {
                            pAction->Execute( &aMapVDev );
                            pAction->Duplicate();
                            aMtf.AddAction( pAction );
                        }
                    }
                }
                break;

                case( META_HATCH_ACTION ):
                {
                    MetaHatchAction*    pAct = (MetaHatchAction*) pAction;
                    Hatch               aHatch( pAct->GetHatch() );

                    aHatch.SetAngle( aHatch.GetAngle() + (USHORT) nAngle10 );
                    aMtf.AddAction( new MetaHatchAction( ImplGetRotatedPolyPolygon( pAct->GetPolyPolygon(), aRotAnchor, aRotOffset, fSin, fCos ),
                                                                                    aHatch ) );
                }
                break;

                case( META_TRANSPARENT_ACTION ):
                {
                    MetaTransparentAction* pAct = (MetaTransparentAction*) pAction;
                    aMtf.AddAction( new MetaTransparentAction( ImplGetRotatedPolyPolygon( pAct->GetPolyPolygon(), aRotAnchor, aRotOffset, fSin, fCos ),
                                                                                          pAct->GetTransparence() ) );
                }
                break;

                case( META_FLOATTRANSPARENT_ACTION ):
                {
                    MetaFloatTransparentAction* pAct = (MetaFloatTransparentAction*) pAction;
                    GDIMetaFile                 aTransMtf( pAct->GetGDIMetaFile() );
                    Polygon                     aMtfPoly( ImplGetRotatedPolygon( Rectangle( pAct->GetPoint(), pAct->GetSize() ), aRotAnchor, aRotOffset, fSin, fCos ) );
                    Rectangle                   aMtfRect( aMtfPoly.GetBoundRect() );

                    aTransMtf.Rotate( nAngle10 );
                    aMtf.AddAction( new MetaFloatTransparentAction( aTransMtf, aMtfRect.TopLeft(), aMtfRect.GetSize(),
                                                                    pAct->GetGradient() ) );
                }
                break;

                case( META_EPS_ACTION ):
                {
                    MetaEPSAction*  pAct = (MetaEPSAction*) pAction;
                    GDIMetaFile     aEPSMtf( pAct->GetSubstitute() );
                    Polygon         aEPSPoly( ImplGetRotatedPolygon( Rectangle( pAct->GetPoint(), pAct->GetSize() ), aRotAnchor, aRotOffset, fSin, fCos ) );
                    Rectangle       aEPSRect( aEPSPoly.GetBoundRect() );

                    aEPSMtf.Rotate( nAngle10 );
                    aMtf.AddAction( new MetaEPSAction( aEPSRect.TopLeft(), aEPSRect.GetSize(),
                                                       pAct->GetLink(), aEPSMtf ) );
                }
                break;

                case( META_CLIPREGION_ACTION ):
                {
                    MetaClipRegionAction* pAct = (MetaClipRegionAction*) pAction;

                    if( pAct->IsClipping() && pAct->GetRegion().HasPolyPolygon() )
                        aMtf.AddAction( new MetaClipRegionAction( Region( ImplGetRotatedPolyPolygon( pAct->GetRegion().GetPolyPolygon(), aRotAnchor, aRotOffset, fSin, fCos ) ), TRUE ) );
                    else
                    {
                        pAction->Duplicate();
                        aMtf.AddAction( pAction );
                    }
                }
                break;

                case( META_ISECTRECTCLIPREGION_ACTION ):
                {
                    MetaISectRectClipRegionAction*  pAct = (MetaISectRectClipRegionAction*) pAction;
                    aMtf.AddAction( new MetaISectRegionClipRegionAction( ImplGetRotatedPolygon( pAct->GetRect(), aRotAnchor, aRotOffset, fSin, fCos ) ) );
                }
                break;

                case( META_ISECTREGIONCLIPREGION_ACTION ):
                {
                    MetaISectRegionClipRegionAction*    pAct = (MetaISectRegionClipRegionAction*) pAction;
                    const Region&                       rRegion = pAct->GetRegion();

                    if( rRegion.HasPolyPolygon() )
                        aMtf.AddAction( new MetaISectRegionClipRegionAction( Region( ImplGetRotatedPolyPolygon( rRegion.GetPolyPolygon(), aRotAnchor, aRotOffset, fSin, fCos ) ) ) );
                    else
                    {
                        pAction->Duplicate();
                        aMtf.AddAction( pAction );
                    }
                }
                break;

                case( META_REFPOINT_ACTION ):
                {
                    MetaRefPointAction* pAct = (MetaRefPointAction*) pAction;
                    aMtf.AddAction( new MetaRefPointAction( ImplGetRotatedPoint( pAct->GetRefPoint(), aRotAnchor, aRotOffset, fSin, fCos ), pAct->IsSetting() ) );
                }
                break;

                case( META_FONT_ACTION ):
                {
                    MetaFontAction* pAct = (MetaFontAction*) pAction;
                    Font            aFont( pAct->GetFont() );

                    aFont.SetOrientation( aFont.GetOrientation() + (USHORT) nAngle10 );
                    aMtf.AddAction( new MetaFontAction( aFont ) );
                }
                break;

                case( META_BMP_ACTION ):
                case( META_BMPEX_ACTION ):
                case( META_MASK_ACTION ):
                case( META_MASKSCALE_ACTION ):
                case( META_MASKSCALEPART_ACTION ):
                case( META_WALLPAPER_ACTION ):
                case( META_TEXTRECT_ACTION ):
                case( META_MOVECLIPREGION_ACTION ):
                {
                    DBG_ERROR( "GDIMetaFile::Rotate(): unsupported action" );
                }
                break;

                default:
                {
                    pAction->Execute( &aMapVDev );
                    pAction->Duplicate();
                    aMtf.AddAction( pAction );

                    // update rotation point and offset, if necessary
                    if( ( META_MAPMODE_ACTION == nActionType ) ||
                        ( META_PUSH_ACTION == nActionType ) ||
                        ( META_POP_ACTION == nActionType ) )
                    {
                        aRotAnchor = aMapVDev.LogicToLogic( aOrigin, aPrefMapMode, aMapVDev.GetMapMode() );
                        aRotOffset = aMapVDev.LogicToLogic( aOffset, aPrefMapMode, aMapVDev.GetMapMode() );
                    }
                }
                break;
            }
        }

        aMtf.aPrefMapMode = aPrefMapMode;
        aMtf.aPrefSize = aNewBound.GetSize();

        *this = aMtf;
    }
}

// ------------------------------------------------------------------------

static void ImplActionBounds( Rectangle& o_rOutBounds,
                              const Rectangle& i_rInBounds,
                              const std::vector<Rectangle>& i_rClipStack )
{
    Rectangle aBounds( i_rInBounds );
    if( ! i_rInBounds.IsEmpty() && ! i_rClipStack.empty() && ! i_rClipStack.back().IsEmpty() )
        aBounds.Intersection( i_rClipStack.back() );
    if( ! aBounds.IsEmpty() )
    {
        if( ! o_rOutBounds.IsEmpty() )
            o_rOutBounds.Union( aBounds );
        else
            o_rOutBounds = aBounds;
    }
}

Rectangle GDIMetaFile::GetBoundRect( OutputDevice& i_rReference )
{
    GDIMetaFile     aMtf;
    VirtualDevice   aMapVDev( i_rReference );

    aMapVDev.EnableOutput( FALSE );
    aMapVDev.SetMapMode( GetPrefMapMode() );

    std::vector<Rectangle> aClipStack( 1, Rectangle() );
    std::vector<USHORT> aPushFlagStack;

    Rectangle aBound;

    for( MetaAction* pAction = (MetaAction*) First(); pAction; pAction = (MetaAction*) Next() )
    {
        const USHORT nActionType = pAction->GetType();

        switch( nActionType )
        {
        case( META_PIXEL_ACTION ):
            {
                MetaPixelAction* pAct = (MetaPixelAction*) pAction;
                ImplActionBounds( aBound,
                                 Rectangle( aMapVDev.LogicToLogic( pAct->GetPoint(), aMapVDev.GetMapMode(), GetPrefMapMode() ),
                                           aMapVDev.PixelToLogic( Size( 1, 1 ), GetPrefMapMode() ) ),
                                 aClipStack );
            }
            break;

        case( META_POINT_ACTION ):
            {
                MetaPointAction* pAct = (MetaPointAction*) pAction;
                ImplActionBounds( aBound,
                                 Rectangle( aMapVDev.LogicToLogic( pAct->GetPoint(), aMapVDev.GetMapMode(), GetPrefMapMode() ),
                                           aMapVDev.PixelToLogic( Size( 1, 1 ), GetPrefMapMode() ) ),
                                 aClipStack );
            }
            break;

        case( META_LINE_ACTION ):
            {
                MetaLineAction* pAct = (MetaLineAction*) pAction;
                Point aP1( pAct->GetStartPoint() ), aP2( pAct->GetEndPoint() );
                Rectangle aRect( aP1, aP2 );
                aRect.Justify();
                ImplActionBounds( aBound, aMapVDev.LogicToLogic( aRect, aMapVDev.GetMapMode(), GetPrefMapMode() ), aClipStack );
            }
            break;

        case( META_RECT_ACTION ):
            {
                MetaRectAction* pAct = (MetaRectAction*) pAction;
                ImplActionBounds( aBound, aMapVDev.LogicToLogic( pAct->GetRect(), aMapVDev.GetMapMode(), GetPrefMapMode() ), aClipStack );
            }
            break;

        case( META_ROUNDRECT_ACTION ):
            {
                MetaRoundRectAction*    pAct = (MetaRoundRectAction*) pAction;
                ImplActionBounds( aBound, aMapVDev.LogicToLogic( pAct->GetRect(), aMapVDev.GetMapMode(), GetPrefMapMode() ), aClipStack );
            }
            break;

        case( META_ELLIPSE_ACTION ):
            {
                MetaEllipseAction*      pAct = (MetaEllipseAction*) pAction;
                ImplActionBounds( aBound, aMapVDev.LogicToLogic( pAct->GetRect(), aMapVDev.GetMapMode(), GetPrefMapMode() ), aClipStack );
            }
            break;

        case( META_ARC_ACTION ):
            {
                MetaArcAction*  pAct = (MetaArcAction*) pAction;
                // FIXME: this is imprecise
                // e.g. for small arcs the whole rectangle is WAY too large
                ImplActionBounds( aBound, aMapVDev.LogicToLogic( pAct->GetRect(), aMapVDev.GetMapMode(), GetPrefMapMode() ), aClipStack );
            }
            break;

        case( META_PIE_ACTION ):
            {
                MetaPieAction*  pAct = (MetaPieAction*) pAction;
                // FIXME: this is imprecise
                // e.g. for small arcs the whole rectangle is WAY too large
                ImplActionBounds( aBound, aMapVDev.LogicToLogic( pAct->GetRect(), aMapVDev.GetMapMode(), GetPrefMapMode() ), aClipStack );
            }
            break;

        case( META_CHORD_ACTION ):
            {
                MetaChordAction*    pAct = (MetaChordAction*) pAction;
                // FIXME: this is imprecise
                // e.g. for small arcs the whole rectangle is WAY too large
                ImplActionBounds( aBound, aMapVDev.LogicToLogic( pAct->GetRect(), aMapVDev.GetMapMode(), GetPrefMapMode() ), aClipStack );
            }
            break;

        case( META_POLYLINE_ACTION ):
            {
                MetaPolyLineAction* pAct = (MetaPolyLineAction*) pAction;
                Rectangle aRect( pAct->GetPolygon().GetBoundRect() );
                ImplActionBounds( aBound, aMapVDev.LogicToLogic( aRect, aMapVDev.GetMapMode(), GetPrefMapMode() ), aClipStack );
            }
            break;

        case( META_POLYGON_ACTION ):
            {
                MetaPolygonAction* pAct = (MetaPolygonAction*) pAction;
                Rectangle aRect( pAct->GetPolygon().GetBoundRect() );
                ImplActionBounds( aBound, aMapVDev.LogicToLogic( aRect, aMapVDev.GetMapMode(), GetPrefMapMode() ), aClipStack );
            }
            break;

        case( META_POLYPOLYGON_ACTION ):
            {
                MetaPolyPolygonAction* pAct = (MetaPolyPolygonAction*) pAction;
                Rectangle aRect( pAct->GetPolyPolygon().GetBoundRect() );
                ImplActionBounds( aBound, aMapVDev.LogicToLogic( aRect, aMapVDev.GetMapMode(), GetPrefMapMode() ), aClipStack );
            }
            break;

        case( META_TEXT_ACTION ):
            {
                MetaTextAction* pAct = (MetaTextAction*) pAction;
                Rectangle aRect;
                // hdu said base = index
                aMapVDev.GetTextBoundRect( aRect, pAct->GetText(), pAct->GetIndex(), pAct->GetIndex(), pAct->GetLen() );
                Point aPt( pAct->GetPoint() );
                aRect.Move( aPt.X(), aPt.Y() );
                ImplActionBounds( aBound, aMapVDev.LogicToLogic( aRect, aMapVDev.GetMapMode(), GetPrefMapMode() ), aClipStack );
            }
            break;

        case( META_TEXTARRAY_ACTION ):
            {
                MetaTextArrayAction* pAct = (MetaTextArrayAction*) pAction;
                Rectangle aRect;
                // hdu said base = index
                aMapVDev.GetTextBoundRect( aRect, pAct->GetText(), pAct->GetIndex(), pAct->GetIndex(), pAct->GetLen(),
                                           0, pAct->GetDXArray() );
                Point aPt( pAct->GetPoint() );
                aRect.Move( aPt.X(), aPt.Y() );
                ImplActionBounds( aBound, aMapVDev.LogicToLogic( aRect, aMapVDev.GetMapMode(), GetPrefMapMode() ), aClipStack );
            }
            break;

        case( META_STRETCHTEXT_ACTION ):
            {
                MetaStretchTextAction* pAct = (MetaStretchTextAction*) pAction;
                Rectangle aRect;
                // hdu said base = index
                aMapVDev.GetTextBoundRect( aRect, pAct->GetText(), pAct->GetIndex(), pAct->GetIndex(), pAct->GetLen(),
                                           pAct->GetWidth(), NULL );
                Point aPt( pAct->GetPoint() );
                aRect.Move( aPt.X(), aPt.Y() );
                ImplActionBounds( aBound, aMapVDev.LogicToLogic( aRect, aMapVDev.GetMapMode(), GetPrefMapMode() ), aClipStack );
            }
            break;

        case( META_TEXTLINE_ACTION ):
            {
                MetaTextLineAction* pAct = (MetaTextLineAction*) pAction;
                // measure a test string to get ascend and descent right
                static const sal_Unicode pStr[] = { 0xc4, 0x67, 0 };
                String aStr( pStr );

                Rectangle aRect;
                aMapVDev.GetTextBoundRect( aRect, aStr, 0, 0, aStr.Len(), 0, NULL );
                Point aPt( pAct->GetStartPoint() );
                aRect.Move( aPt.X(), aPt.Y() );
                aRect.Right() = aRect.Left() + pAct->GetWidth();
                ImplActionBounds( aBound, aMapVDev.LogicToLogic( aRect, aMapVDev.GetMapMode(), GetPrefMapMode() ), aClipStack );
            }
            break;

        case( META_BMPSCALE_ACTION ):
            {
                MetaBmpScaleAction* pAct = (MetaBmpScaleAction*) pAction;
                Rectangle aRect( pAct->GetPoint(), pAct->GetSize() );
                ImplActionBounds( aBound, aMapVDev.LogicToLogic( aRect, aMapVDev.GetMapMode(), GetPrefMapMode() ), aClipStack );
            }
            break;

        case( META_BMPSCALEPART_ACTION ):
            {
                MetaBmpScalePartAction* pAct = (MetaBmpScalePartAction*) pAction;
                Rectangle aRect( pAct->GetDestPoint(), pAct->GetDestSize() );
                ImplActionBounds( aBound, aMapVDev.LogicToLogic( aRect, aMapVDev.GetMapMode(), GetPrefMapMode() ), aClipStack );
            }
            break;

        case( META_BMPEXSCALE_ACTION ):
            {
                MetaBmpExScaleAction*   pAct = (MetaBmpExScaleAction*) pAction;
                Rectangle aRect( pAct->GetPoint(), pAct->GetSize() );
                ImplActionBounds( aBound, aMapVDev.LogicToLogic( aRect, aMapVDev.GetMapMode(), GetPrefMapMode() ), aClipStack );
            }
            break;

        case( META_BMPEXSCALEPART_ACTION ):
            {
                MetaBmpExScalePartAction*   pAct = (MetaBmpExScalePartAction*) pAction;
                Rectangle aRect( pAct->GetDestPoint(), pAct->GetDestSize() );
                ImplActionBounds( aBound, aMapVDev.LogicToLogic( aRect, aMapVDev.GetMapMode(), GetPrefMapMode() ), aClipStack );
            }
            break;

        case( META_GRADIENT_ACTION ):
            {
                MetaGradientAction* pAct = (MetaGradientAction*) pAction;
                Rectangle aRect( pAct->GetRect() );
                ImplActionBounds( aBound, aMapVDev.LogicToLogic( aRect, aMapVDev.GetMapMode(), GetPrefMapMode() ), aClipStack );
            }
            break;

        case( META_GRADIENTEX_ACTION ):
            {
                MetaGradientExAction* pAct = (MetaGradientExAction*) pAction;
                Rectangle aRect( pAct->GetPolyPolygon().GetBoundRect() );
                ImplActionBounds( aBound, aMapVDev.LogicToLogic( aRect, aMapVDev.GetMapMode(), GetPrefMapMode() ), aClipStack );
            }
            break;

        case( META_COMMENT_ACTION ):
            {
                // nothing to do
            };
            break;

        case( META_HATCH_ACTION ):
            {
                MetaHatchAction*    pAct = (MetaHatchAction*) pAction;
                Rectangle aRect( pAct->GetPolyPolygon().GetBoundRect() );
                ImplActionBounds( aBound, aMapVDev.LogicToLogic( aRect, aMapVDev.GetMapMode(), GetPrefMapMode() ), aClipStack );
            }
            break;

        case( META_TRANSPARENT_ACTION ):
            {
                MetaTransparentAction* pAct = (MetaTransparentAction*) pAction;
                Rectangle aRect( pAct->GetPolyPolygon().GetBoundRect() );
                ImplActionBounds( aBound, aMapVDev.LogicToLogic( aRect, aMapVDev.GetMapMode(), GetPrefMapMode() ), aClipStack );
            }
            break;

        case( META_FLOATTRANSPARENT_ACTION ):
            {
                MetaFloatTransparentAction* pAct = (MetaFloatTransparentAction*) pAction;
                GDIMetaFile                 aTransMtf( pAct->GetGDIMetaFile() );
                // get the bound rect of the contained metafile
                Rectangle aRect( aTransMtf.GetBoundRect( i_rReference ) );
                // scale the rect now on the assumption that the correct top left of the metafile
                // (not its bounds !) is (0,0)
                Size aPSize( aTransMtf.GetPrefSize() );
                aPSize = aMapVDev.LogicToLogic( aPSize, aTransMtf.GetPrefMapMode(), aMapVDev.GetMapMode() );
                Size aActSize( pAct->GetSize() );
                double fX = double(aActSize.Width())/double(aPSize.Width());
                double fY = double(aActSize.Height())/double(aPSize.Height());
                aRect.Left()   = long(double(aRect.Left())*fX);
                aRect.Right()  = long(double(aRect.Right())*fX);
                aRect.Top()    = long(double(aRect.Top())*fY);
                aRect.Bottom() = long(double(aRect.Bottom())*fY);

                // transform the rect to current VDev state
                aRect = aMapVDev.LogicToLogic( aRect, aTransMtf.GetPrefMapMode(), aMapVDev.GetMapMode() );

                ImplActionBounds( aBound, aRect, aClipStack );
            }
            break;

        case( META_EPS_ACTION ):
            {
                MetaEPSAction*  pAct = (MetaEPSAction*) pAction;
                Rectangle aRect( pAct->GetPoint(), pAct->GetSize() );
                ImplActionBounds( aBound, aMapVDev.LogicToLogic( aRect, aMapVDev.GetMapMode(), GetPrefMapMode() ), aClipStack );
            }
            break;

        case( META_CLIPREGION_ACTION ):
            {
                MetaClipRegionAction* pAct = (MetaClipRegionAction*) pAction;
                if( pAct->IsClipping() )
                    aClipStack.back() = aMapVDev.LogicToLogic( pAct->GetRegion().GetBoundRect(), aMapVDev.GetMapMode(), GetPrefMapMode() );
                else
                    aClipStack.back() = Rectangle();
            }
            break;

        case( META_ISECTRECTCLIPREGION_ACTION ):
            {
                MetaISectRectClipRegionAction* pAct = (MetaISectRectClipRegionAction*) pAction;
                Rectangle aRect( aMapVDev.LogicToLogic( pAct->GetRect(), aMapVDev.GetMapMode(), GetPrefMapMode() ) );
                if( aClipStack.back().IsEmpty() )
                    aClipStack.back() = aRect;
                else
                    aClipStack.back().Intersection( aRect );
            }
            break;

        case( META_ISECTREGIONCLIPREGION_ACTION ):
            {
                MetaISectRegionClipRegionAction*    pAct = (MetaISectRegionClipRegionAction*) pAction;
                Rectangle aRect( aMapVDev.LogicToLogic( pAct->GetRegion().GetBoundRect(), aMapVDev.GetMapMode(), GetPrefMapMode() ) );
                if( aClipStack.back().IsEmpty() )
                    aClipStack.back() = aRect;
                else
                    aClipStack.back().Intersection( aRect );
            }
            break;

        case( META_BMP_ACTION ):
            {
                MetaBmpAction* pAct = (MetaBmpAction*) pAction;
                Rectangle aRect( pAct->GetPoint(), aMapVDev.PixelToLogic( pAct->GetBitmap().GetSizePixel() ) );
                ImplActionBounds( aBound, aMapVDev.LogicToLogic( aRect, aMapVDev.GetMapMode(), GetPrefMapMode() ), aClipStack );
            }
            break;

        case( META_BMPEX_ACTION ):
            {
                MetaBmpExAction* pAct = (MetaBmpExAction*) pAction;
                Rectangle aRect( pAct->GetPoint(), aMapVDev.PixelToLogic( pAct->GetBitmapEx().GetSizePixel() ) );
                ImplActionBounds( aBound, aMapVDev.LogicToLogic( aRect, aMapVDev.GetMapMode(), GetPrefMapMode() ), aClipStack );
            }
            break;

        case( META_MASK_ACTION ):
            {
                MetaMaskAction* pAct = (MetaMaskAction*) pAction;
                Rectangle aRect( pAct->GetPoint(), aMapVDev.PixelToLogic( pAct->GetBitmap().GetSizePixel() ) );
                ImplActionBounds( aBound, aMapVDev.LogicToLogic( aRect, aMapVDev.GetMapMode(), GetPrefMapMode() ), aClipStack );
            }
            break;

        case( META_MASKSCALE_ACTION ):
            {
                MetaMaskScalePartAction* pAct = (MetaMaskScalePartAction*) pAction;
                Rectangle aRect( pAct->GetDestPoint(), pAct->GetDestSize() );
                ImplActionBounds( aBound, aMapVDev.LogicToLogic( aRect, aMapVDev.GetMapMode(), GetPrefMapMode() ), aClipStack );
            }
            break;

        case( META_MASKSCALEPART_ACTION ):
            {
                MetaMaskScalePartAction* pAct = (MetaMaskScalePartAction*) pAction;
                Rectangle aRect( pAct->GetDestPoint(), pAct->GetDestSize() );
                ImplActionBounds( aBound, aMapVDev.LogicToLogic( aRect, aMapVDev.GetMapMode(), GetPrefMapMode() ), aClipStack );
            }
            break;

        case( META_WALLPAPER_ACTION ):
            {
                MetaWallpaperAction* pAct = (MetaWallpaperAction*) pAction;
                Rectangle aRect( pAct->GetRect() );
                ImplActionBounds( aBound, aMapVDev.LogicToLogic( aRect, aMapVDev.GetMapMode(), GetPrefMapMode() ), aClipStack );
            }
            break;

        case( META_TEXTRECT_ACTION ):
            {
                MetaTextRectAction* pAct = (MetaTextRectAction*) pAction;
                Rectangle aRect( pAct->GetRect() );
                ImplActionBounds( aBound, aMapVDev.LogicToLogic( aRect, aMapVDev.GetMapMode(), GetPrefMapMode() ), aClipStack );
            }
            break;

        case( META_MOVECLIPREGION_ACTION ):
            {
                MetaMoveClipRegionAction* pAct = (MetaMoveClipRegionAction*) pAction;
                if( ! aClipStack.back().IsEmpty() )
                {
                    Size aDelta( pAct->GetHorzMove(), pAct->GetVertMove() );
                    aDelta = aMapVDev.LogicToLogic( aDelta, aMapVDev.GetMapMode(), GetPrefMapMode() );
                    aClipStack.back().Move( aDelta.Width(), aDelta.Width() );
                }
            }
            break;

        default:
            {
                pAction->Execute( &aMapVDev );

                if( nActionType == META_PUSH_ACTION )
                {
                    MetaPushAction* pAct = (MetaPushAction*) pAction;
                    aPushFlagStack.push_back( pAct->GetFlags() );
                    if( (aPushFlagStack.back() & PUSH_CLIPREGION) != 0 )
                    {
                        Rectangle aRect( aClipStack.back() );
                        aClipStack.push_back( aRect );
                    }
                }
                else if( nActionType == META_POP_ACTION )
                {
                    // sanity check
                    if( ! aPushFlagStack.empty() )
                    {
                        if( (aPushFlagStack.back() & PUSH_CLIPREGION) != 0 )
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

// ------------------------------------------------------------------------

Color GDIMetaFile::ImplColAdjustFnc( const Color& rColor, const void* pColParam )
{
    return Color( rColor.GetTransparency(),
                  ( (const ImplColAdjustParam*) pColParam )->pMapR[ rColor.GetRed() ],
                  ( (const ImplColAdjustParam*) pColParam )->pMapG[ rColor.GetGreen() ],
                  ( (const ImplColAdjustParam*) pColParam )->pMapB[ rColor.GetBlue() ] );

}

// ------------------------------------------------------------------------

BitmapEx GDIMetaFile::ImplBmpAdjustFnc( const BitmapEx& rBmpEx, const void* pBmpParam )
{
    const ImplBmpAdjustParam*   p = (const ImplBmpAdjustParam*) pBmpParam;
    BitmapEx                    aRet( rBmpEx );

    aRet.Adjust( p->nLuminancePercent, p->nContrastPercent,
                 p->nChannelRPercent, p->nChannelGPercent, p->nChannelBPercent,
                 p->fGamma, p->bInvert );

    return aRet;
}

// ------------------------------------------------------------------------

Color GDIMetaFile::ImplColConvertFnc( const Color& rColor, const void* pColParam )
{
    BYTE cLum = rColor.GetLuminance();

    if( MTF_CONVERSION_1BIT_THRESHOLD == ( (const ImplColConvertParam*) pColParam )->eConversion )
        cLum = ( cLum < 128 ) ? 0 : 255;

    return Color( rColor.GetTransparency(), cLum, cLum, cLum );
}

// ------------------------------------------------------------------------

BitmapEx GDIMetaFile::ImplBmpConvertFnc( const BitmapEx& rBmpEx, const void* pBmpParam )
{
    BitmapEx aRet( rBmpEx );

    aRet.Convert( ( (const ImplBmpConvertParam*) pBmpParam )->eConversion );

    return aRet;
}

// ------------------------------------------------------------------------

Color GDIMetaFile::ImplColMonoFnc( const Color&, const void* pColParam )
{
    return( ( (const ImplColMonoParam*) pColParam )->aColor );
}

// ------------------------------------------------------------------------

BitmapEx GDIMetaFile::ImplBmpMonoFnc( const BitmapEx& rBmpEx, const void* pBmpParam )
{
    BitmapPalette aPal( 3 );

    aPal[ 0 ] = Color( COL_BLACK );
    aPal[ 1 ] = Color( COL_WHITE );
    aPal[ 2 ] = ( (const ImplBmpMonoParam*) pBmpParam )->aColor;

    Bitmap aBmp( rBmpEx.GetSizePixel(), 4, &aPal );
    aBmp.Erase( ( (const ImplBmpMonoParam*) pBmpParam )->aColor );

    if( rBmpEx.IsAlpha() )
        return BitmapEx( aBmp, rBmpEx.GetAlpha() );
    else if( rBmpEx.IsTransparent() )
        return BitmapEx( aBmp, rBmpEx.GetMask() );
    else
        return aBmp;
}

// ------------------------------------------------------------------------

Color GDIMetaFile::ImplColReplaceFnc( const Color& rColor, const void* pColParam )
{
    const ULONG nR = rColor.GetRed(), nG = rColor.GetGreen(), nB = rColor.GetBlue();

    for( ULONG i = 0; i < ( (const ImplColReplaceParam*) pColParam )->nCount; i++ )
    {
        if( ( ( (const ImplColReplaceParam*) pColParam )->pMinR[ i ] <= nR ) &&
            ( ( (const ImplColReplaceParam*) pColParam )->pMaxR[ i ] >= nR ) &&
            ( ( (const ImplColReplaceParam*) pColParam )->pMinG[ i ] <= nG ) &&
            ( ( (const ImplColReplaceParam*) pColParam )->pMaxG[ i ] >= nG ) &&
            ( ( (const ImplColReplaceParam*) pColParam )->pMinB[ i ] <= nB ) &&
            ( ( (const ImplColReplaceParam*) pColParam )->pMaxB[ i ] >= nB ) )
        {
            return( ( (const ImplColReplaceParam*) pColParam )->pDstCols[ i ] );
        }
    }

    return rColor;
}

// ------------------------------------------------------------------------

BitmapEx GDIMetaFile::ImplBmpReplaceFnc( const BitmapEx& rBmpEx, const void* pBmpParam )
{
    const ImplBmpReplaceParam*  p = (const ImplBmpReplaceParam*) pBmpParam;
    BitmapEx                    aRet( rBmpEx );

    aRet.Replace( p->pSrcCols, p->pDstCols, p->nCount, p->pTols );

    return aRet;
}

// ------------------------------------------------------------------------

void GDIMetaFile::ImplExchangeColors( ColorExchangeFnc pFncCol, const void* pColParam,
                                      BmpExchangeFnc pFncBmp, const void* pBmpParam )
{
    GDIMetaFile aMtf;

    aMtf.aPrefSize = aPrefSize;
    aMtf.aPrefMapMode = aPrefMapMode;

    for( MetaAction* pAction = (MetaAction*) First(); pAction; pAction = (MetaAction*) Next() )
    {
        const USHORT nType = pAction->GetType();

        switch( nType )
        {
            case( META_PIXEL_ACTION ):
            {
                MetaPixelAction* pAct = (MetaPixelAction*) pAction;
                aMtf.Insert( new MetaPixelAction( pAct->GetPoint(), pFncCol( pAct->GetColor(), pColParam ) ), LIST_APPEND );
            }
            break;

            case( META_LINECOLOR_ACTION ):
            {
                MetaLineColorAction* pAct = (MetaLineColorAction*) pAction;

                if( !pAct->IsSetting() )
                    pAct->Duplicate();
                else
                    pAct = new MetaLineColorAction( pFncCol( pAct->GetColor(), pColParam ), TRUE );

                aMtf.Insert( pAct, LIST_APPEND );
            }
            break;

            case( META_FILLCOLOR_ACTION ):
            {
                MetaFillColorAction* pAct = (MetaFillColorAction*) pAction;

                if( !pAct->IsSetting() )
                    pAct->Duplicate();
                else
                    pAct = new MetaFillColorAction( pFncCol( pAct->GetColor(), pColParam ), TRUE );

                aMtf.Insert( pAct, LIST_APPEND );
            }
            break;

            case( META_TEXTCOLOR_ACTION ):
            {
                MetaTextColorAction* pAct = (MetaTextColorAction*) pAction;
                aMtf.Insert( new MetaTextColorAction( pFncCol( pAct->GetColor(), pColParam ) ), LIST_APPEND );
            }
            break;

            case( META_TEXTFILLCOLOR_ACTION ):
            {
                MetaTextFillColorAction* pAct = (MetaTextFillColorAction*) pAction;

                if( !pAct->IsSetting() )
                    pAct->Duplicate();
                else
                    pAct = new MetaTextFillColorAction( pFncCol( pAct->GetColor(), pColParam ), TRUE );

                aMtf.Insert( pAct, LIST_APPEND );
            }
            break;

            case( META_TEXTLINECOLOR_ACTION ):
            {
                MetaTextLineColorAction* pAct = (MetaTextLineColorAction*) pAction;

                if( !pAct->IsSetting() )
                    pAct->Duplicate();
                else
                    pAct = new MetaTextLineColorAction( pFncCol( pAct->GetColor(), pColParam ), TRUE );

                aMtf.Insert( pAct, LIST_APPEND );
            }
            break;

            case( META_OVERLINECOLOR_ACTION ):
            {
                MetaOverlineColorAction* pAct = (MetaOverlineColorAction*) pAction;

                if( !pAct->IsSetting() )
                    pAct->Duplicate();
                else
                    pAct = new MetaOverlineColorAction( pFncCol( pAct->GetColor(), pColParam ), TRUE );

                aMtf.Insert( pAct, LIST_APPEND );
            }
            break;

            case( META_FONT_ACTION ):
            {
                MetaFontAction* pAct = (MetaFontAction*) pAction;
                Font            aFont( pAct->GetFont() );

                aFont.SetColor( pFncCol( aFont.GetColor(), pColParam ) );
                aFont.SetFillColor( pFncCol( aFont.GetFillColor(), pColParam ) );
                aMtf.Insert( new MetaFontAction( aFont ), LIST_APPEND );
            }
            break;

            case( META_WALLPAPER_ACTION ):
            {
                MetaWallpaperAction*    pAct = (MetaWallpaperAction*) pAction;
                Wallpaper               aWall( pAct->GetWallpaper() );
                const Rectangle&        rRect = pAct->GetRect();

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

                aMtf.Insert( new MetaWallpaperAction( rRect, aWall ), LIST_APPEND );
            }
            break;

            case( META_BMP_ACTION ):
            case( META_BMPEX_ACTION ):
            case( META_MASK_ACTION ):
            {
                DBG_ERROR( "Don't use bitmap actions of this type in metafiles!" );
            }
            break;

            case( META_BMPSCALE_ACTION ):
            {
                MetaBmpScaleAction* pAct = (MetaBmpScaleAction*) pAction;
                aMtf.Insert( new MetaBmpScaleAction( pAct->GetPoint(), pAct->GetSize(),
                                                     pFncBmp( pAct->GetBitmap(), pBmpParam ).GetBitmap() ),
                                                     LIST_APPEND );
            }
            break;

            case( META_BMPSCALEPART_ACTION ):
            {
                MetaBmpScalePartAction* pAct = (MetaBmpScalePartAction*) pAction;
                aMtf.Insert( new MetaBmpScalePartAction( pAct->GetDestPoint(), pAct->GetDestSize(),
                                                         pAct->GetSrcPoint(), pAct->GetSrcSize(),
                                                         pFncBmp( pAct->GetBitmap(), pBmpParam ).GetBitmap() ),
                                                         LIST_APPEND );
            }
            break;

            case( META_BMPEXSCALE_ACTION ):
            {
                MetaBmpExScaleAction* pAct = (MetaBmpExScaleAction*) pAction;
                aMtf.Insert( new MetaBmpExScaleAction( pAct->GetPoint(), pAct->GetSize(),
                                                       pFncBmp( pAct->GetBitmapEx(), pBmpParam ) ),
                                                       LIST_APPEND );
            }
            break;

            case( META_BMPEXSCALEPART_ACTION ):
            {
                MetaBmpExScalePartAction* pAct = (MetaBmpExScalePartAction*) pAction;
                aMtf.Insert( new MetaBmpExScalePartAction( pAct->GetDestPoint(), pAct->GetDestSize(),
                                                           pAct->GetSrcPoint(), pAct->GetSrcSize(),
                                                           pFncBmp( pAct->GetBitmapEx(), pBmpParam ) ),
                                                           LIST_APPEND );
            }
            break;

            case( META_MASKSCALE_ACTION ):
            {
                MetaMaskScaleAction* pAct = (MetaMaskScaleAction*) pAction;
                aMtf.Insert( new MetaMaskScaleAction( pAct->GetPoint(), pAct->GetSize(),
                                                      pAct->GetBitmap(),
                                                      pFncCol( pAct->GetColor(), pColParam ) ),
                                                      LIST_APPEND );
            }
            break;

            case( META_MASKSCALEPART_ACTION ):
            {
                MetaMaskScalePartAction* pAct = (MetaMaskScalePartAction*) pAction;
                aMtf.Insert( new MetaMaskScalePartAction( pAct->GetDestPoint(), pAct->GetDestSize(),
                                                          pAct->GetSrcPoint(), pAct->GetSrcSize(),
                                                          pAct->GetBitmap(),
                                                          pFncCol( pAct->GetColor(), pColParam ) ),
                                                          LIST_APPEND );
            }
            break;

            case( META_GRADIENT_ACTION ):
            {
                MetaGradientAction* pAct = (MetaGradientAction*) pAction;
                Gradient            aGradient( pAct->GetGradient() );

                aGradient.SetStartColor( pFncCol( aGradient.GetStartColor(), pColParam ) );
                aGradient.SetEndColor( pFncCol( aGradient.GetEndColor(), pColParam ) );
                aMtf.Insert( new MetaGradientAction( pAct->GetRect(), aGradient ), LIST_APPEND );
            }
            break;

            case( META_GRADIENTEX_ACTION ):
            {
                MetaGradientExAction* pAct = (MetaGradientExAction*) pAction;
                Gradient              aGradient( pAct->GetGradient() );

                aGradient.SetStartColor( pFncCol( aGradient.GetStartColor(), pColParam ) );
                aGradient.SetEndColor( pFncCol( aGradient.GetEndColor(), pColParam ) );
                aMtf.Insert( new MetaGradientExAction( pAct->GetPolyPolygon(), aGradient ), LIST_APPEND );
            }
            break;

            case( META_HATCH_ACTION ):
            {
                MetaHatchAction*    pAct = (MetaHatchAction*) pAction;
                Hatch               aHatch( pAct->GetHatch() );

                aHatch.SetColor( pFncCol( aHatch.GetColor(), pColParam ) );
                aMtf.Insert( new MetaHatchAction( pAct->GetPolyPolygon(), aHatch ), LIST_APPEND );
            }
            break;

            case( META_FLOATTRANSPARENT_ACTION ):
            {
                MetaFloatTransparentAction* pAct = (MetaFloatTransparentAction*) pAction;
                GDIMetaFile                 aTransMtf( pAct->GetGDIMetaFile() );

                aTransMtf.ImplExchangeColors( pFncCol, pColParam, pFncBmp, pBmpParam );
                aMtf.Insert( new MetaFloatTransparentAction( aTransMtf,
                                                             pAct->GetPoint(), pAct->GetSize(),
                                                             pAct->GetGradient() ),
                                                             LIST_APPEND );
            }
            break;

            case( META_EPS_ACTION ):
            {
                MetaEPSAction*  pAct = (MetaEPSAction*) pAction;
                GDIMetaFile     aSubst( pAct->GetSubstitute() );

                aSubst.ImplExchangeColors( pFncCol, pColParam, pFncBmp, pBmpParam );
                aMtf.Insert( new MetaEPSAction( pAct->GetPoint(), pAct->GetSize(),
                                                pAct->GetLink(), aSubst ),
                                                LIST_APPEND );
            }
            break;

            default:
            {
                pAction->Duplicate();
                aMtf.Insert( pAction, LIST_APPEND );
            }
            break;
        }
    }

    *this = aMtf;
}

// ------------------------------------------------------------------------

void GDIMetaFile::Adjust( short nLuminancePercent, short nContrastPercent,
                          short nChannelRPercent, short nChannelGPercent,
                          short nChannelBPercent, double fGamma, BOOL bInvert )
{
    // nothing to do? => return quickly
    if( nLuminancePercent || nContrastPercent ||
        nChannelRPercent || nChannelGPercent || nChannelBPercent ||
        ( fGamma != 1.0 ) || bInvert )
    {
        double              fM, fROff, fGOff, fBOff, fOff;
        ImplColAdjustParam  aColParam;
        ImplBmpAdjustParam  aBmpParam;

        aColParam.pMapR = new BYTE[ 256 ];
        aColParam.pMapG = new BYTE[ 256 ];
        aColParam.pMapB = new BYTE[ 256 ];

        // calculate slope
        if( nContrastPercent >= 0 )
            fM = 128.0 / ( 128.0 - 1.27 * MinMax( nContrastPercent, 0L, 100L ) );
        else
            fM = ( 128.0 + 1.27 * MinMax( nContrastPercent, -100L, 0L ) ) / 128.0;

        // total offset = luminance offset + contrast offset
        fOff = MinMax( nLuminancePercent, -100L, 100L ) * 2.55 + 128.0 - fM * 128.0;

        // channel offset = channel offset  + total offset
        fROff = nChannelRPercent * 2.55 + fOff;
        fGOff = nChannelGPercent * 2.55 + fOff;
        fBOff = nChannelBPercent * 2.55 + fOff;

        // calculate gamma value
        fGamma = ( fGamma <= 0.0 || fGamma > 10.0 ) ? 1.0 : ( 1.0 / fGamma );
        const BOOL bGamma = ( fGamma != 1.0 );

        // create mapping table
        for( long nX = 0L; nX < 256L; nX++ )
        {
            aColParam.pMapR[ nX ] = (BYTE) MinMax( FRound( nX * fM + fROff ), 0L, 255L );
            aColParam.pMapG[ nX ] = (BYTE) MinMax( FRound( nX * fM + fGOff ), 0L, 255L );
            aColParam.pMapB[ nX ] = (BYTE) MinMax( FRound( nX * fM + fBOff ), 0L, 255L );

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

        delete[] aColParam.pMapR;
        delete[] aColParam.pMapG;
        delete[] aColParam.pMapB;
    }
}

// ------------------------------------------------------------------------

void GDIMetaFile::Convert( MtfConversion eConversion )
{
    // nothing to do? => return quickly
    if( eConversion != MTF_CONVERSION_NONE )
    {
        ImplColConvertParam aColParam;
        ImplBmpConvertParam aBmpParam;

        aColParam.eConversion = eConversion;
        aBmpParam.eConversion = ( MTF_CONVERSION_1BIT_THRESHOLD == eConversion ) ? BMP_CONVERSION_1BIT_THRESHOLD : BMP_CONVERSION_8BIT_GREYS;

        ImplExchangeColors( ImplColConvertFnc, &aColParam, ImplBmpConvertFnc, &aBmpParam );
    }
}

// ------------------------------------------------------------------------

void GDIMetaFile::ReplaceColors( const Color& rSearchColor, const Color& rReplaceColor, ULONG nTol )
{
    ReplaceColors( &rSearchColor, &rReplaceColor, 1, &nTol );
}

// ------------------------------------------------------------------------

void GDIMetaFile::ReplaceColors( const Color* pSearchColors, const Color* pReplaceColors, ULONG nColorCount, ULONG* pTols )
{
    ImplColReplaceParam aColParam;
    ImplBmpReplaceParam aBmpParam;

    aColParam.pMinR = new ULONG[ nColorCount ];
    aColParam.pMaxR = new ULONG[ nColorCount ];
    aColParam.pMinG = new ULONG[ nColorCount ];
    aColParam.pMaxG = new ULONG[ nColorCount ];
    aColParam.pMinB = new ULONG[ nColorCount ];
    aColParam.pMaxB = new ULONG[ nColorCount ];

    for( ULONG i = 0; i < nColorCount; i++ )
    {
        const long  nTol = pTols ? ( pTols[ i ] * 255 ) / 100 : 0;
        long        nVal;

        nVal = pSearchColors[ i ].GetRed();
        aColParam.pMinR[ i ] = (ULONG) Max( nVal - nTol, 0L );
        aColParam.pMaxR[ i ] = (ULONG) Min( nVal + nTol, 255L );

        nVal = pSearchColors[ i ].GetGreen();
        aColParam.pMinG[ i ] = (ULONG) Max( nVal - nTol, 0L );
        aColParam.pMaxG[ i ] = (ULONG) Min( nVal + nTol, 255L );

        nVal = pSearchColors[ i ].GetBlue();
        aColParam.pMinB[ i ] = (ULONG) Max( nVal - nTol, 0L );
        aColParam.pMaxB[ i ] = (ULONG) Min( nVal + nTol, 255L );
    }

    aColParam.pDstCols = pReplaceColors;
    aColParam.nCount = nColorCount;

    aBmpParam.pSrcCols = pSearchColors;
    aBmpParam.pDstCols = pReplaceColors;
    aBmpParam.nCount = nColorCount;
    aBmpParam.pTols = pTols;

    ImplExchangeColors( ImplColReplaceFnc, &aColParam, ImplBmpReplaceFnc, &aBmpParam );

    delete[] aColParam.pMinR;
    delete[] aColParam.pMaxR;
    delete[] aColParam.pMinG;
    delete[] aColParam.pMaxG;
    delete[] aColParam.pMinB;
    delete[] aColParam.pMaxB;
};

// ------------------------------------------------------------------------

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

// ------------------------------------------------------------------------

ULONG GDIMetaFile::GetChecksum() const
{
    GDIMetaFile         aMtf;
    SvMemoryStream      aMemStm( 65535, 65535 );
    ImplMetaWriteData   aWriteData; aWriteData.meActualCharSet = aMemStm.GetStreamCharSet();
    SVBT16              aBT16;
    SVBT32              aBT32;
    ULONG               nCrc = 0;

    for( ULONG i = 0, nObjCount = GetActionCount(); i < nObjCount; i++ )
    {
        MetaAction* pAction = GetAction( i );

        switch( pAction->GetType() )
        {
            case( META_BMP_ACTION ):
            {
                MetaBmpAction* pAct = (MetaBmpAction*) pAction;

                ShortToSVBT16( pAct->GetType(), aBT16 );
                nCrc = rtl_crc32( nCrc, aBT16, 2 );

                UInt32ToSVBT32( pAct->GetBitmap().GetChecksum(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                UInt32ToSVBT32( pAct->GetPoint().X(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                UInt32ToSVBT32( pAct->GetPoint().Y(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );
            }
            break;

            case( META_BMPSCALE_ACTION ):
            {
                MetaBmpScaleAction* pAct = (MetaBmpScaleAction*) pAction;

                ShortToSVBT16( pAct->GetType(), aBT16 );
                nCrc = rtl_crc32( nCrc, aBT16, 2 );

                UInt32ToSVBT32( pAct->GetBitmap().GetChecksum(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                UInt32ToSVBT32( pAct->GetPoint().X(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                UInt32ToSVBT32( pAct->GetPoint().Y(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                UInt32ToSVBT32( pAct->GetSize().Width(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                UInt32ToSVBT32( pAct->GetSize().Height(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );
            }
            break;

            case( META_BMPSCALEPART_ACTION ):
            {
                MetaBmpScalePartAction* pAct = (MetaBmpScalePartAction*) pAction;

                ShortToSVBT16( pAct->GetType(), aBT16 );
                nCrc = rtl_crc32( nCrc, aBT16, 2 );

                UInt32ToSVBT32( pAct->GetBitmap().GetChecksum(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                UInt32ToSVBT32( pAct->GetDestPoint().X(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                UInt32ToSVBT32( pAct->GetDestPoint().Y(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                UInt32ToSVBT32( pAct->GetDestSize().Width(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                UInt32ToSVBT32( pAct->GetDestSize().Height(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                UInt32ToSVBT32( pAct->GetSrcPoint().X(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                UInt32ToSVBT32( pAct->GetSrcPoint().Y(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                UInt32ToSVBT32( pAct->GetSrcSize().Width(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                UInt32ToSVBT32( pAct->GetSrcSize().Height(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );
            }
            break;

            case( META_BMPEX_ACTION ):
            {
                MetaBmpExAction* pAct = (MetaBmpExAction*) pAction;

                ShortToSVBT16( pAct->GetType(), aBT16 );
                nCrc = rtl_crc32( nCrc, aBT16, 2 );

                UInt32ToSVBT32( pAct->GetBitmapEx().GetChecksum(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                UInt32ToSVBT32( pAct->GetPoint().X(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                UInt32ToSVBT32( pAct->GetPoint().Y(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );
            }
            break;

            case( META_BMPEXSCALE_ACTION ):
            {
                MetaBmpExScaleAction* pAct = (MetaBmpExScaleAction*) pAction;

                ShortToSVBT16( pAct->GetType(), aBT16 );
                nCrc = rtl_crc32( nCrc, aBT16, 2 );

                UInt32ToSVBT32( pAct->GetBitmapEx().GetChecksum(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                UInt32ToSVBT32( pAct->GetPoint().X(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                UInt32ToSVBT32( pAct->GetPoint().Y(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                UInt32ToSVBT32( pAct->GetSize().Width(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                UInt32ToSVBT32( pAct->GetSize().Height(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );
            }
            break;

            case( META_BMPEXSCALEPART_ACTION ):
            {
                MetaBmpExScalePartAction* pAct = (MetaBmpExScalePartAction*) pAction;

                ShortToSVBT16( pAct->GetType(), aBT16 );
                nCrc = rtl_crc32( nCrc, aBT16, 2 );

                UInt32ToSVBT32( pAct->GetBitmapEx().GetChecksum(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                UInt32ToSVBT32( pAct->GetDestPoint().X(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                UInt32ToSVBT32( pAct->GetDestPoint().Y(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                UInt32ToSVBT32( pAct->GetDestSize().Width(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                UInt32ToSVBT32( pAct->GetDestSize().Height(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                UInt32ToSVBT32( pAct->GetSrcPoint().X(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                UInt32ToSVBT32( pAct->GetSrcPoint().Y(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                UInt32ToSVBT32( pAct->GetSrcSize().Width(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                UInt32ToSVBT32( pAct->GetSrcSize().Height(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );
            }
            break;

            case( META_MASK_ACTION ):
            {
                MetaMaskAction* pAct = (MetaMaskAction*) pAction;

                ShortToSVBT16( pAct->GetType(), aBT16 );
                nCrc = rtl_crc32( nCrc, aBT16, 2 );

                UInt32ToSVBT32( pAct->GetBitmap().GetChecksum(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                UInt32ToSVBT32( pAct->GetColor().GetColor(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                UInt32ToSVBT32( pAct->GetPoint().X(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                UInt32ToSVBT32( pAct->GetPoint().Y(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );
            }
            break;

            case( META_MASKSCALE_ACTION ):
            {
                MetaMaskScaleAction* pAct = (MetaMaskScaleAction*) pAction;

                ShortToSVBT16( pAct->GetType(), aBT16 );
                nCrc = rtl_crc32( nCrc, aBT16, 2 );

                UInt32ToSVBT32( pAct->GetBitmap().GetChecksum(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                UInt32ToSVBT32( pAct->GetColor().GetColor(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                UInt32ToSVBT32( pAct->GetPoint().X(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                UInt32ToSVBT32( pAct->GetPoint().Y(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                UInt32ToSVBT32( pAct->GetSize().Width(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                UInt32ToSVBT32( pAct->GetSize().Height(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );
            }
            break;

            case( META_MASKSCALEPART_ACTION ):
            {
                MetaMaskScalePartAction* pAct = (MetaMaskScalePartAction*) pAction;

                ShortToSVBT16( pAct->GetType(), aBT16 );
                nCrc = rtl_crc32( nCrc, aBT16, 2 );

                UInt32ToSVBT32( pAct->GetBitmap().GetChecksum(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                UInt32ToSVBT32( pAct->GetColor().GetColor(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                UInt32ToSVBT32( pAct->GetDestPoint().X(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                UInt32ToSVBT32( pAct->GetDestPoint().Y(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                UInt32ToSVBT32( pAct->GetDestSize().Width(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                UInt32ToSVBT32( pAct->GetDestSize().Height(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                UInt32ToSVBT32( pAct->GetSrcPoint().X(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                UInt32ToSVBT32( pAct->GetSrcPoint().Y(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                UInt32ToSVBT32( pAct->GetSrcSize().Width(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                UInt32ToSVBT32( pAct->GetSrcSize().Height(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );
            }
            break;

            case META_EPS_ACTION :
            {
                MetaEPSAction* pAct = (MetaEPSAction*) pAction;
                nCrc = rtl_crc32( nCrc, pAct->GetLink().GetData(), pAct->GetLink().GetDataSize() );
            }
            break;

            default:
            {
                pAction->Write( aMemStm, &aWriteData );
                nCrc = rtl_crc32( nCrc, aMemStm.GetData(), aMemStm.Tell() );
                aMemStm.Seek( 0 );
            }
            break;
        }
    }

    return nCrc;
}

// ------------------------------------------------------------------------

ULONG GDIMetaFile::GetSizeBytes() const
{
    ULONG nSizeBytes = 0;

    for( ULONG i = 0, nObjCount = GetActionCount(); i < nObjCount; ++i )
    {
        MetaAction* pAction = GetAction( i );

        // default action size is set to 32 (=> not the exact value)
        nSizeBytes += 32;

        // add sizes for large action content
        switch( pAction->GetType() )
        {
            case( META_BMP_ACTION ): nSizeBytes += ( (MetaBmpAction*) pAction )->GetBitmap().GetSizeBytes(); break;
            case( META_BMPSCALE_ACTION ): nSizeBytes += ( (MetaBmpScaleAction*) pAction )->GetBitmap().GetSizeBytes(); break;
            case( META_BMPSCALEPART_ACTION ): nSizeBytes += ( (MetaBmpScalePartAction*) pAction )->GetBitmap().GetSizeBytes(); break;

            case( META_BMPEX_ACTION ): nSizeBytes += ( (MetaBmpExAction*) pAction )->GetBitmapEx().GetSizeBytes(); break;
            case( META_BMPEXSCALE_ACTION ): nSizeBytes += ( (MetaBmpExScaleAction*) pAction )->GetBitmapEx().GetSizeBytes(); break;
            case( META_BMPEXSCALEPART_ACTION ): nSizeBytes += ( (MetaBmpExScalePartAction*) pAction )->GetBitmapEx().GetSizeBytes(); break;

            case( META_MASK_ACTION ): nSizeBytes += ( (MetaMaskAction*) pAction )->GetBitmap().GetSizeBytes(); break;
            case( META_MASKSCALE_ACTION ): nSizeBytes += ( (MetaMaskScaleAction*) pAction )->GetBitmap().GetSizeBytes(); break;
            case( META_MASKSCALEPART_ACTION ): nSizeBytes += ( (MetaMaskScalePartAction*) pAction )->GetBitmap().GetSizeBytes(); break;

            case( META_POLYLINE_ACTION ): nSizeBytes += ( ( (MetaPolyLineAction*) pAction )->GetPolygon().GetSize() * sizeof( Point ) ); break;
            case( META_POLYGON_ACTION ): nSizeBytes += ( ( (MetaPolygonAction*) pAction )->GetPolygon().GetSize() * sizeof( Point ) ); break;
            case( META_POLYPOLYGON_ACTION ):
            {
                const PolyPolygon& rPolyPoly = ( (MetaPolyPolygonAction*) pAction )->GetPolyPolygon();

                for( USHORT n = 0; n < rPolyPoly.Count(); ++n )
                    nSizeBytes += ( rPolyPoly[ n ].GetSize() * sizeof( Point ) );
            }
            break;

            case( META_TEXT_ACTION ): nSizeBytes += ( ( (MetaTextAction*) pAction )->GetText().Len() * sizeof( sal_Unicode ) ); break;
            case( META_STRETCHTEXT_ACTION ): nSizeBytes += ( ( (MetaStretchTextAction*) pAction )->GetText().Len() * sizeof( sal_Unicode ) ); break;
            case( META_TEXTRECT_ACTION ): nSizeBytes += ( ( (MetaTextRectAction*) pAction )->GetText().Len() * sizeof( sal_Unicode ) ); break;
            case( META_TEXTARRAY_ACTION ):
            {
                MetaTextArrayAction* pTextArrayAction = (MetaTextArrayAction*) pAction;

                nSizeBytes += ( pTextArrayAction->GetText().Len() * sizeof( sal_Unicode ) );

                if( pTextArrayAction->GetDXArray() )
                    nSizeBytes += ( pTextArrayAction->GetLen() << 2 );
            }
            break;

            default:
            break;
        }
    }

    return( nSizeBytes );
}

// ------------------------------------------------------------------------

SvStream& operator>>( SvStream& rIStm, GDIMetaFile& rGDIMetaFile )
{
    if( !rIStm.GetError() )
    {
        char    aId[ 7 ];
        ULONG   nStmPos = rIStm.Tell();
        USHORT  nOldFormat = rIStm.GetNumberFormatInt();

        rIStm.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );

        aId[ 0 ] = 0;
        aId[ 6 ] = 0;
        rIStm.Read( aId, 6 );

        if ( !strcmp( aId, "VCLMTF" ) )
        {
            // new format
            VersionCompat*  pCompat;
            MetaAction*     pAction;
            UINT32          nStmCompressMode = 0;
            UINT32          nCount = 0;

            pCompat = new VersionCompat( rIStm, STREAM_READ );

            rIStm >> nStmCompressMode;
            rIStm >> rGDIMetaFile.aPrefMapMode;
            rIStm >> rGDIMetaFile.aPrefSize;
            rIStm >> nCount;

            delete pCompat;

            ImplMetaReadData aReadData;
            aReadData.meActualCharSet = rIStm.GetStreamCharSet();

            for( UINT32 nAction = 0UL; ( nAction < nCount ) && !rIStm.IsEof(); nAction++ )
            {
                pAction = MetaAction::ReadMetaAction( rIStm, &aReadData );

                if( pAction )
                    rGDIMetaFile.AddAction( pAction );
            }
        }
        else
        {
            // to avoid possible compiler optimizations => new/delete
            rIStm.Seek( nStmPos );
            delete( new SVMConverter( rIStm, rGDIMetaFile, CONVERT_FROM_SVM1 ) );
        }

        // check for errors
        if( rIStm.GetError() )
        {
            rGDIMetaFile.Clear();
            rIStm.Seek( nStmPos );
        }

        rIStm.SetNumberFormatInt( nOldFormat );
    }

    return rIStm;
}

// ------------------------------------------------------------------------

SvStream& operator<<( SvStream& rOStm, const GDIMetaFile& rGDIMetaFile )
{
    if( !rOStm.GetError() )
    {
        if( rOStm.GetVersion() >= SOFFICE_FILEFORMAT_50 )
            ((GDIMetaFile&) rGDIMetaFile ).Write( rOStm );
        else
            delete( new SVMConverter( rOStm, (GDIMetaFile&) rGDIMetaFile, CONVERT_TO_SVM1 ) );
    }

    return rOStm;
}

// ------------------------------------------------------------------------

SvStream& GDIMetaFile::Read( SvStream& rIStm )
{
    Clear();
    rIStm >> *this;

    return rIStm;
}

// ------------------------------------------------------------------------

SvStream& GDIMetaFile::Write( SvStream& rOStm )
{
    VersionCompat*  pCompat;
    const UINT32    nStmCompressMode = rOStm.GetCompressMode();
    USHORT          nOldFormat = rOStm.GetNumberFormatInt();

    rOStm.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    rOStm.Write( "VCLMTF", 6 );

    pCompat = new VersionCompat( rOStm, STREAM_WRITE, 1 );

    rOStm << nStmCompressMode;
    rOStm << aPrefMapMode;
    rOStm << aPrefSize;
    rOStm << (UINT32) GetActionCount();

    delete pCompat;

    ImplMetaWriteData aWriteData;
    aWriteData.meActualCharSet = rOStm.GetStreamCharSet();

    MetaAction* pAct = (MetaAction*)First();
    while ( pAct )
    {
        pAct->Write( rOStm, &aWriteData );
        pAct = (MetaAction*)Next();
    }

    rOStm.SetNumberFormatInt( nOldFormat );

    return rOStm;
}

// ------------------------------------------------------------------------

BOOL GDIMetaFile::CreateThumbnail( sal_uInt32 nMaximumExtent,
                                    BitmapEx& rBmpEx,
                                    const BitmapEx* pOverlay,
                                    const Rectangle* pOverlayRect ) const
{
    // the implementation is provided by KA

    // initialization seems to be complicated but is used to avoid rounding errors
    VirtualDevice   aVDev;
    const Point     aNullPt;
    const Point     aTLPix( aVDev.LogicToPixel( aNullPt, GetPrefMapMode() ) );
    const Point     aBRPix( aVDev.LogicToPixel( Point( GetPrefSize().Width() - 1, GetPrefSize().Height() - 1 ), GetPrefMapMode() ) );
    Size            aDrawSize( aVDev.LogicToPixel( GetPrefSize(), GetPrefMapMode() ) );
    Size            aSizePix( labs( aBRPix.X() - aTLPix.X() ) + 1, labs( aBRPix.Y() - aTLPix.Y() ) + 1 );
    Point           aPosPix;

    if ( !rBmpEx.IsEmpty() )
        rBmpEx.SetEmpty();

    // determine size that has the same aspect ratio as image size and
    // fits into the rectangle determined by nMaximumExtent
    if ( aSizePix.Width() && aSizePix.Height()
      && ( sal::static_int_cast< unsigned long >(aSizePix.Width()) >
               nMaximumExtent ||
           sal::static_int_cast< unsigned long >(aSizePix.Height()) >
               nMaximumExtent ) )
    {
        const Size  aOldSizePix( aSizePix );
        double      fWH = static_cast< double >( aSizePix.Width() ) / aSizePix.Height();

        if ( fWH <= 1.0 )
        {
            aSizePix.Width() = FRound( nMaximumExtent * fWH );
            aSizePix.Height() = nMaximumExtent;
        }
        else
        {
            aSizePix.Width() = nMaximumExtent;
            aSizePix.Height() = FRound(  nMaximumExtent / fWH );
        }

        aDrawSize.Width() = FRound( ( static_cast< double >( aDrawSize.Width() ) * aSizePix.Width() ) / aOldSizePix.Width() );
        aDrawSize.Height() = FRound( ( static_cast< double >( aDrawSize.Height() ) * aSizePix.Height() ) / aOldSizePix.Height() );
    }

    Size        aFullSize;
    Point       aBackPosPix;
    Rectangle   aOverlayRect;

    // calculate addigtional positions and sizes if an overlay image is used
    if (  pOverlay )
    {
        aFullSize = Size( nMaximumExtent, nMaximumExtent );
        aOverlayRect = Rectangle( aNullPt, aFullSize  );

        aOverlayRect.Intersection( pOverlayRect ? *pOverlayRect : Rectangle( aNullPt, pOverlay->GetSizePixel() ) );

        if ( !aOverlayRect.IsEmpty() )
            aBackPosPix = Point( ( nMaximumExtent - aSizePix.Width() ) >> 1, ( nMaximumExtent - aSizePix.Height() ) >> 1 );
        else
            pOverlay = NULL;
    }
    else
    {
        aFullSize = aSizePix;
        pOverlay = NULL;
    }

    // draw image(s) into VDev and get resulting image
    if ( aVDev.SetOutputSizePixel( aFullSize ) )
    {
        // draw metafile into VDev
        const_cast<GDIMetaFile *>(this)->WindStart();
        const_cast<GDIMetaFile *>(this)->Play( &aVDev, aBackPosPix, aDrawSize );

        // draw overlay if neccessary
        if ( pOverlay )
            aVDev.DrawBitmapEx( aOverlayRect.TopLeft(), aOverlayRect.GetSize(), *pOverlay );

        // get paint bitmap
        Bitmap aBmp( aVDev.GetBitmap( aNullPt, aVDev.GetOutputSizePixel() ) );

        // assure that we have a true color image
        if ( aBmp.GetBitCount() != 24 )
            aBmp.Convert( BMP_CONVERSION_24BIT );

        // create resulting mask bitmap with metafile output set to black
        GDIMetaFile aMonchromeMtf( GetMonochromeMtf( COL_BLACK ) );
        aVDev.DrawWallpaper( Rectangle( aNullPt, aSizePix ), Wallpaper( Color( COL_WHITE ) ) );
        aMonchromeMtf.WindStart();
        aMonchromeMtf.Play( &aVDev, aBackPosPix, aDrawSize );

        // watch for overlay mask
        if ( pOverlay  )
        {
            Bitmap aOverlayMergeBmp( aVDev.GetBitmap( aOverlayRect.TopLeft(), aOverlayRect.GetSize() ) );

            // create ANDed resulting mask at overlay area
            if ( pOverlay->IsTransparent() )
                aVDev.DrawBitmap( aOverlayRect.TopLeft(), aOverlayRect.GetSize(), pOverlay->GetMask() );
            else
            {
                aVDev.SetLineColor( COL_BLACK );
                aVDev.SetFillColor( COL_BLACK );
                aVDev.DrawRect( aOverlayRect);
            }

            aOverlayMergeBmp.CombineSimple( aVDev.GetBitmap( aOverlayRect.TopLeft(), aOverlayRect.GetSize() ), BMP_COMBINE_AND );
            aVDev.DrawBitmap( aOverlayRect.TopLeft(), aOverlayRect.GetSize(), aOverlayMergeBmp );
        }

        rBmpEx = BitmapEx( aBmp, aVDev.GetBitmap( aNullPt, aVDev.GetOutputSizePixel() ) );
    }

    return !rBmpEx.IsEmpty();
}

void GDIMetaFile::UseCanvas( BOOL _bUseCanvas )
{
    bUseCanvas = _bUseCanvas;
}

// ------------------------------------------------------------------------

MetaCommentAction* makePluggableRendererAction( const rtl::OUString& rRendererServiceName,
                                                const rtl::OUString& rGraphicServiceName,
                                                const void* _pData,
                                                sal_uInt32 nDataSize )
{
    const BYTE* pData=(BYTE*)_pData;

    // data gets copied twice, unfortunately
    rtl::OString aRendererServiceName(
        rRendererServiceName.getStr(),
        rRendererServiceName.getLength(),
        RTL_TEXTENCODING_ASCII_US);
    rtl::OString aGraphicServiceName(
        rGraphicServiceName.getStr(),
        rGraphicServiceName.getLength(),
        RTL_TEXTENCODING_ASCII_US);

    std::vector<sal_uInt8> aMem(
        aRendererServiceName.getLength()+
        aGraphicServiceName.getLength()+2+nDataSize);
    sal_uInt8* pMem=&aMem[0];

    std::copy(aRendererServiceName.getStr(),
              aRendererServiceName.getStr()+aRendererServiceName.getLength()+1,
              pMem);
    pMem+=aRendererServiceName.getLength()+1;
    std::copy(aGraphicServiceName.getStr(),
              aGraphicServiceName.getStr()+aGraphicServiceName.getLength()+1,
              pMem);
    pMem+=aGraphicServiceName.getLength()+1;

    std::copy(pData,pData+nDataSize,
              pMem);

    return new MetaCommentAction(
        "DELEGATE_PLUGGABLE_RENDERER",
        0,
        &aMem[0],
        aMem.size());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
