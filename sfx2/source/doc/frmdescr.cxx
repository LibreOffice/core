/*************************************************************************
 *
 *  $RCSfile: frmdescr.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:52:32 $
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

#include <sot/object.hxx>

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#ifndef _SV_SPLITWIN_HXX //autogen
#include <vcl/splitwin.hxx>
#endif
#ifndef _SFXITEMSET_HXX //autogen
#include <svtools/itemset.hxx>
#endif
#pragma hdrstop

#include "frmdescr.hxx"
#include "app.hxx"

DBG_NAME(SfxFrameSetDescriptor);
DBG_NAME(SfxFrameDescriptor);

#define VERSION (USHORT) 3

struct SfxFrameSetDescriptor_Impl
{
    Wallpaper*  pWallpaper;
    Bitmap*     pBitmap;
    BOOL        bNetscapeCompat;
};

struct SfxFrameDescriptor_Impl
{
    Wallpaper*  pWallpaper;
    SfxItemSet* pArgs;
    BOOL        bEditable;

    SfxFrameDescriptor_Impl() : pWallpaper( NULL ), pArgs( NULL ), bEditable( TRUE ) {}
    ~SfxFrameDescriptor_Impl()
    {
        delete pWallpaper;
        delete pArgs;
    }
};

SfxFrameSetDescriptor::SfxFrameSetDescriptor(SfxFrameDescriptor *pFrame) :
    pParentFrame( pFrame ),
    nFrameSpacing( SPACING_NOT_SET ),
    bIsRoot( pParentFrame ? (pParentFrame->pParentFrameSet == 0) : TRUE ),
    bRowSet( FALSE ),
    nMaxId( 0 ),
    nHasBorder( BORDER_YES )
{
    DBG_CTOR(SfxFrameSetDescriptor, 0);

    pImp = new SfxFrameSetDescriptor_Impl;
    pImp->pBitmap = NULL;
    pImp->pWallpaper = NULL;
    pImp->bNetscapeCompat = TRUE;
    if ( pParentFrame )
        pParentFrame->pFrameSet = this;
    if ( bIsRoot )
        nHasBorder |= BORDER_SET;
}

SfxFrameSetDescriptor::~SfxFrameSetDescriptor()
{
    DBG_DTOR(SfxFrameSetDescriptor, 0);

    for ( USHORT n=0; n<aFrames.Count(); n++ )
    {
        SfxFrameDescriptor *pFrame = aFrames[n];
        pFrame->pParentFrameSet = NULL;
        delete pFrame;
    }
    if ( pParentFrame )
        pParentFrame->pFrameSet = NULL;
    delete pImp;
}

SfxFrameDescriptor* SfxFrameSetDescriptor::SearchFrame( USHORT nId )
{
    for ( USHORT n=0; n<aFrames.Count(); n++ )
    {
        SfxFrameDescriptor *pFrame = aFrames[n];
        if ( pFrame->nItemId == nId )
            return pFrame;
        if ( pFrame->GetFrameSet() )
        {
            pFrame = pFrame->GetFrameSet()->SearchFrame( nId );
            if ( pFrame )
                return pFrame;
        }
    }

    return NULL;
}

SfxFrameDescriptor* SfxFrameSetDescriptor::SearchFrame( const String& rName )
{
    for ( USHORT n=0; n<aFrames.Count(); n++ )
    {
        SfxFrameDescriptor *pFrame = aFrames[n];
        if ( pFrame->aName == rName )
            return pFrame;
        if ( pFrame->GetFrameSet() )
        {
            pFrame = pFrame->GetFrameSet()->SearchFrame( rName );
            if ( pFrame )
                return pFrame;
        }
    }

    return NULL;
}

void SfxFrameSetDescriptor::InsertFrame
(
    SfxFrameDescriptor* pFrame,
    USHORT nPos
)
{
    // fucking SvPtrArrs ...
    if ( nPos == 0xFFFF )
        nPos = aFrames.Count();

    aFrames.Insert( pFrame, nPos );
    pFrame->pParentFrameSet = this;
}

void SfxFrameSetDescriptor::RemoveFrame( SfxFrameDescriptor* pFrame )
{
    USHORT nPos = aFrames.GetPos( pFrame );
    DBG_ASSERT( nPos != 0xFFFF, "Unbekannter Frame!" );
    aFrames.Remove( nPos );
    pFrame->pParentFrameSet = 0L;
}

BOOL SfxFrameSetDescriptor::HasFrameBorder() const
{
    if ( pImp->bNetscapeCompat && !GetFrameSpacing() )
        return FALSE;

    if ( ( nHasBorder & BORDER_SET ) || bIsRoot )
        return (nHasBorder & BORDER_YES) != 0;
    else
        return pParentFrame->HasFrameBorder();
}

//------------------------------------------------------------------------

void SfxFrameSetDescriptor::SetNetscapeCompatibility( BOOL bCompat )
// Frame-Spacing Verhalten wenn kein Border
{
    pImp->bNetscapeCompat = bCompat;
}

//------------------------------------------------------------------------

BOOL SfxFrameSetDescriptor::GetNetscapeCompatibility() const
// Frame-Spacing Verhalten wenn kein Border
{
    return pImp->bNetscapeCompat;
}

//------------------------------------------------------------------------

long SfxFrameSetDescriptor::GetFrameSpacing() const
{
    if ( nFrameSpacing == SPACING_NOT_SET && !bIsRoot &&
            pParentFrame && pParentFrame->pParentFrameSet )
    {
        return pParentFrame->pParentFrameSet->GetFrameSpacing();
    }
    else
        return nFrameSpacing;
}

BOOL SfxFrameSetDescriptor::Store( SvStream& rStream ) const
{
    // Die "0" ist wg. Kompatibilit"at zu alten Versionen n"otig. Diese
    // glauben dann, ein leeres Frameset zu lesen und st"urzen dann
    // wenigstens nicht ab.
    long lLength = 20;
    rStream << VERSION << lLength << (USHORT) 0 << (USHORT) 0;
    rStream << nHasBorder
            << nFrameSpacing
            << (USHORT) bRowSet
            << aFrames.Count();

    for ( USHORT n=0; n<aFrames.Count(); n++ )
    {
        SfxFrameDescriptor* pFrame = aFrames[n];
        pFrame->Store( rStream );
    }
    return TRUE;
}

BOOL SfxFrameSetDescriptor::Load( SvStream& rStream )
{
    long lPos = rStream.Tell();             // aktuelle Position merken
    USHORT nCount=0, nHorizontal;
    rStream >> nHasBorder
            >> nFrameSpacing
            >> nHorizontal
            >> nCount;

    if ( nCount == 0 )
    {
        // Das mu\s ein neues Format sein, da ein Frame normalerweise immer
        // dabei ist
        USHORT nVersion = nHasBorder;
        rStream.Seek( lPos );
        return Load( rStream, nVersion );
    }

    bRowSet = (BOOL) nHorizontal;
    for ( USHORT n=0; n<nCount; n++ )
    {
        SfxFrameDescriptor *pFrame = new SfxFrameDescriptor( this );
        pFrame->Load( rStream, 2 );
    }
    return TRUE;
}

BOOL SfxFrameSetDescriptor::Load( SvStream& rStream, USHORT nVersion )
{
    long lLength = 10;                      // in Version 2
    long lPos = rStream.Tell();             // aktuelle Position merken

    USHORT nCount=0, nHorizontal, nDummy1, nDummy2;
    if ( nVersion > 2 )
        // In der final Version
        rStream >> nVersion >> lLength >> nDummy1 >> nDummy2;

    rStream >> nHasBorder
            >> nFrameSpacing
            >> nHorizontal
            >> nCount;

    rStream.Seek( lPos + lLength );

    bRowSet = (BOOL) nHorizontal;
    for ( USHORT n=0; n<nCount; n++ )
    {
        SfxFrameDescriptor *pFrame = new SfxFrameDescriptor( this );
        pFrame->Load( rStream, nVersion );
    }
    return TRUE;
}

void SfxFrameSetDescriptor::SetWallpaper( const Wallpaper& rWallpaper )
{
    DELETEZ( pImp->pWallpaper );

    if ( rWallpaper.GetStyle() != WALLPAPER_NULL )
        pImp->pWallpaper = new Wallpaper( rWallpaper );
}

const Wallpaper* SfxFrameSetDescriptor::GetWallpaper() const
{
    return pImp->pWallpaper;
}

SfxFrameSetDescriptor* SfxFrameSetDescriptor::Clone(
    SfxFrameDescriptor *pFrame, BOOL bWithIds ) const
{
    SfxFrameSetDescriptor *pSet = new SfxFrameSetDescriptor( pFrame );

    for ( USHORT n=0; n<aFrames.Count(); n++ )
        SfxFrameDescriptor* pFrame = aFrames[n]->Clone( pSet, bWithIds );

    pSet->aDocumentTitle = aDocumentTitle;
    pSet->nFrameSpacing = nFrameSpacing;
    pSet->nHasBorder = nHasBorder;
    pSet->nMaxId = nMaxId;
    pSet->bIsRoot = bIsRoot;
    pSet->bRowSet = bRowSet;
    if ( pImp->pWallpaper )
        pSet->pImp->pWallpaper = new Wallpaper( *pImp->pWallpaper );
    pSet->pImp->bNetscapeCompat = pImp->bNetscapeCompat;

    return pSet;
}

BOOL SfxFrameSetDescriptor::CheckContent() const
{
    BOOL bRet=FALSE;
    for ( USHORT n=0; n<aFrames.Count(); n++ )
        if ( bRet = aFrames[n]->CheckContent() )
            break;
    return bRet;
}

BOOL SfxFrameSetDescriptor::CompareOriginal(
    SfxFrameSetDescriptor& rDescr ) const
{
    if( aFrames.Count() != rDescr.aFrames.Count() )
        return FALSE;
    else
        for( USHORT nPos = aFrames.Count(); nPos--; )
            if( !aFrames[ nPos ]->CompareOriginal(
                *rDescr.aFrames[ nPos ] ) )
                return FALSE;
    return TRUE;
}


void SfxFrameSetDescriptor::UnifyContent( BOOL bTakeActual )
{
    for ( USHORT n=0; n<aFrames.Count(); n++ )
        aFrames[n]->UnifyContent( bTakeActual );
}

void SfxFrameSetDescriptor::CutRootSet()
{
    for ( USHORT n=0; n<aFrames.Count(); n++ )
    {
        SfxFrameDescriptor* pFrame = aFrames[n];
        SfxFrameSetDescriptor *pSet = pFrame->GetFrameSet();
        if ( pSet )
        {
            if ( pSet->bIsRoot )
            {
                delete pSet;
                pFrame->pFrameSet = NULL;
            }
            else
                pSet->CutRootSet();
        }
    }
}

USHORT SfxFrameSetDescriptor::MakeItemId()
{
    if ( pParentFrame && pParentFrame->pParentFrameSet )
        return pParentFrame->pParentFrameSet->MakeItemId();
    else
        return ++nMaxId;
}

SfxFrameDescriptor::SfxFrameDescriptor( SfxFrameSetDescriptor *pParSet ) :
    aMargin( -1, -1 ),
    nWidth( 0L ),
    nItemId( 0 ),
    pFrameSet( 0L ),
    pParentFrameSet( pParSet ),
    eScroll( ScrollingAuto ),
    eSizeSelector( SIZE_ABS ),
    nHasBorder( BORDER_YES ),
    bResizeHorizontal( TRUE ),
    bResizeVertical( TRUE ),
    bHasUI( TRUE ),
    bReadOnly( FALSE )
{
    DBG_CTOR(SfxFrameDescriptor, 0);

    pImp = new SfxFrameDescriptor_Impl;
    if ( pParentFrameSet )
        pParentFrameSet->InsertFrame( this );
}

SfxFrameDescriptor::~SfxFrameDescriptor()
{
    DBG_DTOR(SfxFrameDescriptor, 0);

    if ( pFrameSet )
        delete pFrameSet;
    if ( pParentFrameSet )
        pParentFrameSet->RemoveFrame( this );
    delete pImp;
}

SfxItemSet* SfxFrameDescriptor::GetArgs()
{
    if( !pImp->pArgs )
        pImp->pArgs = new SfxAllItemSet( SFX_APP()->GetPool() );
    return pImp->pArgs;
}

void SfxFrameDescriptor::SetURL( const String& rURL )
{
    aURL = rURL;
    SetActualURL( aURL );
}

void SfxFrameDescriptor::SetURL( const INetURLObject& rURL )
{
    aURL = rURL.GetMainURL();
    SetActualURL( aURL );
}

void SfxFrameDescriptor::SetActualURL( const String& rURL )
{
    aActualURL = rURL;
    if ( pImp->pArgs )
        pImp->pArgs->ClearItem();
}

void SfxFrameDescriptor::SetActualURL( const INetURLObject& rURL )
{
    SetActualURL( rURL.GetMainURL() );
}

void SfxFrameDescriptor::SetEditable( BOOL bSet )
{
    pImp->bEditable = bSet;
}

BOOL SfxFrameDescriptor::IsEditable() const
{
    return pImp->bEditable;
}

BOOL SfxFrameDescriptor::CompareOriginal( SfxFrameDescriptor& rDescr ) const
{
    if( aURL != rDescr.aURL )
        return FALSE;
    else
        // Zwei Descriptoren sind kompatibel, wenn einer keinen SetDescriptor
        // und der andere einen RootDescriptor hat,
        // wenn beide SetDescriptoren haben
        // und diese kompatibel sind oder wenn beide keine
        // SetDescriptoren haben.
        return
            !pFrameSet &&
            ( !rDescr.pFrameSet || rDescr.pFrameSet->IsRootFrameSet() ) ||
            !rDescr.pFrameSet && pFrameSet->IsRootFrameSet() ||
            pFrameSet && rDescr.pFrameSet && pFrameSet->CompareOriginal(
                *rDescr.pFrameSet );
}

BOOL SfxFrameDescriptor::CheckContent() const
{
    BOOL bRet = !( aURL == aActualURL );
    if ( !bRet && pFrameSet )
        bRet = pFrameSet->CheckContent();
    return bRet;
}

void SfxFrameDescriptor::UnifyContent( BOOL bTakeActual )
{
    if ( bTakeActual )
        aURL = aActualURL;
    else
        aActualURL = aURL;
    if ( pFrameSet )
        pFrameSet->UnifyContent( bTakeActual );
}

BOOL SfxFrameDescriptor::Store( SvStream& rStream ) const
{
    long lPos = rStream.Tell();
    long lLength = 0L;
    rStream << lLength << VERSION;

    USHORT nFlags1 = 0;
    USHORT nFlags2 = 0;
    if ( bResizeHorizontal )
        nFlags1 |= 0x01;
    if ( bResizeVertical )
        nFlags2 |= 0x01;
    if ( !bHasUI )              // anders herum, damit kompatibel
        nFlags1 |= 0x02;
    if ( bReadOnly )
        nFlags1 |= 0x04;

    rStream.WriteByteString( INetURLObject::AbsToRel( aURL.GetMainURL() ), RTL_TEXTENCODING_UTF8 );
    rStream.WriteByteString( aName, RTL_TEXTENCODING_UTF8 );
    rStream << aMargin
            << nWidth
            << (USHORT) eSizeSelector
            << (USHORT) eScroll
            << (USHORT) nFlags1
            << (USHORT) nFlags2
            << (USHORT) (pFrameSet != 0)
            << nHasBorder;

    long lActPos = rStream.Tell();
    lLength = lActPos - lPos;
    rStream.Seek( lPos );
    rStream << lLength;
    rStream.Seek( lActPos );

    if ( pFrameSet )
        pFrameSet->Store(rStream);

    return TRUE;
}

BOOL SfxFrameDescriptor::Load( SvStream& rStream, USHORT nVersion )
{
    String aURLName;
    USHORT nFlags1, nFlags2, nScroll, nSet, nSelector;

    long lLength = 0L;
    long lPos = rStream.Tell();     // aktuelle Position merken

    if ( nVersion > 2 )
        rStream >> lLength >> nVersion;

    rStream.ReadByteString( aURLName, RTL_TEXTENCODING_UTF8 );
    rStream.ReadByteString( aName, RTL_TEXTENCODING_UTF8 );
    rStream >> aMargin
            >> nWidth
            >> nSelector
            >> nScroll
            >> nFlags1
            >> nFlags2
            >> nSet
            >> nHasBorder;

    if ( nVersion > 2 )
        rStream.Seek( lPos + lLength );

    bResizeHorizontal = ( nFlags1 & 0x01 ) != 0;
    bResizeVertical = ( nFlags2 & 0x01 ) != 0;
    bHasUI = ( nFlags1 & 0x02 ) == 0;
    bReadOnly = ( nFlags1 & 0x04 ) != 0;
    eSizeSelector = (SizeSelector) nSelector;
    aURL = INetURLObject::RelToAbs( aURLName );
    eScroll = (ScrollingMode) nScroll;
    aActualURL = aURL;

    if ( nSet )
    {
        pFrameSet = new SfxFrameSetDescriptor( this );
        pFrameSet->Load( rStream, nVersion );
    }

    return TRUE;
}

SfxFrameDescriptor* SfxFrameDescriptor::Clone(
    SfxFrameSetDescriptor *pSet, BOOL bWithIds ) const
{
    SfxFrameDescriptor *pFrame =
        new SfxFrameDescriptor( pSet );

    pFrame->aURL = aURL;
    pFrame->aActualURL = aActualURL;
    pFrame->aName = aName;
    pFrame->aMargin = aMargin;
    pFrame->nWidth = nWidth;
    pFrame->eSizeSelector = eSizeSelector;
    pFrame->eScroll = eScroll;
    pFrame->bResizeHorizontal = bResizeHorizontal;
    pFrame->bResizeVertical = bResizeVertical;
    pFrame->nHasBorder = nHasBorder;
    pFrame->bHasUI = bHasUI;
    pFrame->SetReadOnly( IsReadOnly() );
    pFrame->SetEditable( IsEditable() );
    if ( pImp->pWallpaper )
        pFrame->pImp->pWallpaper = new Wallpaper( *pImp->pWallpaper );
    if( pImp->pArgs )
    {
        // Aktuell ist im Clone von SfxAllItemSets noch ein Bug...
        pFrame->pImp->pArgs = new SfxAllItemSet( SFX_APP()->GetPool() );
        pFrame->pImp->pArgs->Put(*pImp->pArgs);
    }

    if ( bWithIds )
        pFrame->nItemId = nItemId;
    else
        pFrame->nItemId = 0;

    if ( pFrameSet )
        pFrame->pFrameSet = pFrameSet->Clone( pFrame, bWithIds );

    return pFrame;
}


SfxFrameDescriptor* SfxFrameDescriptor::Split( BOOL bHorizontal, BOOL bParent )
{
    DBG_ASSERT( pParentFrameSet, "Frames ohne Set koennen nicht gesplittet werden!" );

    // Die Root holen, um neue Ids erzeugen zu k"onnen
    SfxFrameSetDescriptor *pRoot = pParentFrameSet;
    while ( pRoot->pParentFrame && pRoot->pParentFrame->pParentFrameSet )
        pRoot = pRoot->pParentFrame->pParentFrameSet;

    // Der Frame, hinter dem ein neuer eingef"ugt werden soll
    SfxFrameDescriptor *pPrev = this;

    // Dieser Descriptor soll gesplittet werden ...
    SfxFrameDescriptor *pSplit = this;
    SfxFrameSetDescriptor *pSplitSet = pParentFrameSet;

    if ( bParent )
    {
        // Ein FrameSet suchen, das die gleiche Orientierung hat wie
        // die "ubergebene
        pSplit = pSplitSet->bIsRoot ? NULL : pSplitSet->pParentFrame;
        while ( pSplit && pSplit->pParentFrameSet )
        {
            pSplitSet = pSplit->pParentFrameSet;
            if ( pSplitSet->bRowSet == bHorizontal )
                break;
            pSplit = pSplitSet->pParentFrame;
        }

        // pSplit ist NULL oder TopLevel, wenn das Set das RootSet ist
        pPrev = pSplit;
    }
    else
    {
        // Wenn das aktuelle FrameSet eine andere Orientierung hat, der
        // zu splittende Frame aber der einzige ist, wird einfach die
        // Orientierung des Sets umgedreht( k"urzt den Baum)
        if ( pSplitSet->GetFrameCount() == 1 )
            pSplitSet->bRowSet = bHorizontal;
    }

    if ( pSplitSet->bRowSet != bHorizontal )
    {
        BOOL bCreated = FALSE;
        if ( !pSplit || !pSplit->pParentFrameSet )
        {
            // RootSet splitten: das Alignment wechselt
            pSplitSet->bRowSet = bHorizontal;
            bHorizontal = !bHorizontal;
            pSplit = new SfxFrameDescriptor( pSplitSet );
            bCreated = TRUE;
        }

        if ( bParent )
        {
            // "Unter pSplit wird ein FrameSet eingef"ugt
            SfxFrameSetDescriptor *pSet = new SfxFrameSetDescriptor( NULL );
            pSet->bIsRoot = FALSE;
            pSet->bRowSet = bHorizontal;

            // Die Frames unterhalb des alten Sets umh"angen
            USHORT nCount = pSplitSet->aFrames.Count();
            USHORT nPos = 0;
            for ( USHORT n=0; n<nCount; n++ )
            {
                SfxFrameDescriptor *pCur = pSplitSet->aFrames[0];
                if ( pCur != pSplit )
                {
                    pSplitSet->aFrames.Remove(nPos);
                    pSet->InsertFrame( pCur );
                    pCur->pParentFrameSet = pSet;
                }
                else
                    nPos++;
            }

            pPrev = pSplit;

            if ( bCreated )
            {
                USHORT nID = pRoot->MakeItemId();
                pSplit->SetItemId( nID );

                // Neuer Frame erh"alt ganzen Platz
                pPrev->nWidth = 100;
                pPrev->eSizeSelector = SIZE_PERCENT;

                // Verkettung herstellen
                pSplit->pFrameSet = pSet;
                pSet->pParentFrame = pSplit;
            }
        }
        else
        {
            // Den gesplitteten Frame kopieren und moven
            USHORT nPos = pSplitSet->aFrames.GetPos( pPrev );
            pSplit = pPrev->Clone( pSplitSet );
            pSplitSet->RemoveFrame( pSplit );
            pSplitSet->InsertFrame( pSplit, nPos );

            // Unter dem neuen Frame wird ein FrameSet eingef"ugt, das den
            // alten Frame enth"alt, der anschlie\send geteilt wird
            SfxFrameSetDescriptor *pSet = new SfxFrameSetDescriptor( pSplit );
            pSet->bIsRoot = FALSE;
            pSet->bRowSet = bHorizontal;

            // Attribute des neu eingef"ugten Frames zur"ucksetzen und eine
            // Id erzeugen
            pSplit->aName.Erase();
            pSplit->aMargin = Size( -1, -1 );
            pSplit->aURL.SetURL( "" );
            pSplit->aActualURL.SetURL( "" );
            pSplit->SetItemId( pRoot->MakeItemId() );
            bCreated = TRUE;

            // Alter Frame erh"alt ganzen Platz im neu eingef"ugten Frameset
            pPrev->nWidth = 100;
            pPrev->eSizeSelector = SIZE_PERCENT;

            // Alten Frame umh"angen
            pSplitSet->RemoveFrame( pPrev );
            pSet->InsertFrame( pPrev );
        }
    }

    if ( !pPrev || !pPrev->pParentFrameSet )
        return NULL;

    // Hinter pPrev einen neuen Frame einf"ugen
    SfxFrameDescriptor *pFrame = new SfxFrameDescriptor( NULL );
    pSplitSet = pPrev->pParentFrameSet;
    pSplitSet->InsertFrame( pFrame, pSplitSet->aFrames.GetPos( pPrev ) + 1 );
    pFrame->pParentFrameSet = pSplitSet;

    // Size und ID setzen
    USHORT n = pRoot->MakeItemId();
    pFrame->SetItemId( n );

    // Platz zwischen beiden Frames aufteilen
    long nSize = pPrev->nWidth / 2L;
    pPrev->nWidth = nSize ? nSize : 1L;
    pFrame->nWidth = pPrev->nWidth;
    pFrame->eSizeSelector = pPrev->eSizeSelector;

    return pFrame;
}

BOOL SfxFrameDescriptor::CanSplit( BOOL bHorizontal, BOOL bParent ) const
{
    DBG_ASSERT( pParentFrameSet, "Frames ohne Set koennen nicht gesplittet werden!" );

    // Die Root holen, um neue Ids erzeugen zu k"onnen
    const SfxFrameSetDescriptor *pRoot = pParentFrameSet;
    while ( pRoot->pParentFrame && pRoot->pParentFrame->pParentFrameSet )
        pRoot = pRoot->pParentFrame->pParentFrameSet;

    // Der Frame, hinter dem ein neuer eingef"ugt werden soll
    const SfxFrameDescriptor *pPrev = this;

    // Dieser Descriptor soll gesplittet werden ...
    const SfxFrameDescriptor *pSplit = this;
    const SfxFrameSetDescriptor *pSplitSet = pParentFrameSet;
    BOOL bRowSet = pSplitSet->bRowSet;

    if ( bParent )
    {
        // Ein FrameSet suchen, das die gleiche Orientierung hat wie
        // die "ubergebene
        pSplit = pSplitSet->bIsRoot ? NULL : pSplitSet->pParentFrame;
        while ( pSplit && pSplit->pParentFrameSet )
        {
            pSplitSet = pSplit->pParentFrameSet;
            if ( pSplitSet->bRowSet == bHorizontal )
                break;
            pSplit = pSplitSet->pParentFrame;
        }

        // pSplit ist jetzt NULL oder TopLevel, wenn das Set das RootSet ist
        pPrev = pSplit;
        bRowSet = pSplitSet->bRowSet;
    }
    else
    {
        // Wenn das aktuelle FrameSet eine andere Orientierung hat, der
        // zu splittende Frame aber der einzige ist, wird einfach die
        // Orientierung des Sets umgedreht( k"urzt den Baum)
        if ( pSplitSet->GetFrameCount() == 1 )
            bRowSet = bHorizontal;
    }

    if ( bRowSet == bHorizontal && ( !pPrev || !pPrev->pParentFrameSet ) )
        return FALSE;
    return TRUE;
}

SfxFrameDescriptor* SfxFrameDescriptor::Next() const
{
    if ( !pParentFrameSet )
        return NULL;

    USHORT nPos = pParentFrameSet->aFrames.GetPos( (SfxFrameDescriptor*) this );
    if ( nPos < pParentFrameSet->aFrames.Count() - 1 )
        return pParentFrameSet->aFrames[nPos+1];
    else
        return NULL;
}

SfxFrameDescriptor* SfxFrameDescriptor::Prev() const
{
    if ( !pParentFrameSet )
        return NULL;

    USHORT nPos = pParentFrameSet->aFrames.GetPos( (SfxFrameDescriptor*) this );
    if ( nPos > 0 )
        return pParentFrameSet->aFrames[nPos-1];
    else
        return NULL;
}

USHORT SfxFrameDescriptor::GetParentSetId() const
{
    if ( !pParentFrameSet )
        return USHRT_MAX;

    SfxFrameDescriptor *pFrame = pParentFrameSet->pParentFrame;
    if ( pFrame )
        return pFrame->nItemId;
    else
        return 0;
}

USHORT SfxFrameDescriptor::GetWinBits() const
{
    USHORT nBits = 0;
    if ( eSizeSelector == SIZE_REL )
        nBits |= SWIB_RELATIVESIZE;
    if ( eSizeSelector == SIZE_PERCENT )
        nBits |= SWIB_PERCENTSIZE;
    if ( !IsResizable() )
        nBits |= SWIB_FIXED;
    if ( pFrameSet && pFrameSet->IsColSet() )
        nBits |= SWIB_COLSET;
    if ( !nWidth )
        nBits |= SWIB_INVISIBLE;
    return nBits;
}

BOOL SfxFrameDescriptor::HasFrameBorder() const
{
    if ( pParentFrameSet )
    {
        if ( pParentFrameSet->pImp->bNetscapeCompat && !pParentFrameSet->GetFrameSpacing() )
            return FALSE;
        else if ( ( nHasBorder & BORDER_SET ) )
            return (nHasBorder & BORDER_YES) != 0;
        else
            return pParentFrameSet->HasFrameBorder();
    }
    else
        return (nHasBorder & BORDER_YES) != 0;
}

long SfxFrameDescriptor::GetSize() const
{
#if !defined ( VCL ) && ( defined (WIN) || defined (WNT) )
    long nSize = nWidth;
    if ( nSize && eSizeSelector == SIZE_ABS && System::GetGUIVersion() < 400 && HasFrameBorder() )
        // SV zeichnet dann den Border um das Fenster herum
        nSize += 1L;
    return nSize;
#endif

    return nWidth;
}

void SfxFrameDescriptor::TakeProperties( const SfxFrameProperties& rProp )
{
    aURL = aActualURL = rProp.aURL;
    aName = rProp.aName;
    aMargin.Width() = rProp.lMarginWidth;
    aMargin.Height() = rProp.lMarginHeight;
    nWidth = rProp.lSize;
    eScroll = rProp.eScroll;
    eSizeSelector = rProp.eSizeSelector;
    nHasBorder = rProp.bHasBorder ? BORDER_YES : BORDER_NO;
    if ( rProp.bBorderSet )
        nHasBorder |= BORDER_SET;
    bResizeHorizontal = bResizeVertical = rProp.bResizable;
    if ( pParentFrameSet )
    {
        SfxFrameDescriptor *pParent = pParentFrameSet->pParentFrame;
        if ( pParent )
        {
            pParent->nWidth = rProp.lSetSize;
            pParent->eSizeSelector = rProp.eSetSizeSelector;
            pParent->bResizeHorizontal = pParent->bResizeVertical = rProp.bSetResizable;
        }

        pParentFrameSet->nFrameSpacing = rProp.lFrameSpacing;
    }
}

void SfxFrameDescriptor::SetWallpaper( const Wallpaper& rWallpaper )
{
    DELETEZ( pImp->pWallpaper );

    if ( rWallpaper.GetStyle() != WALLPAPER_NULL )
        pImp->pWallpaper = new Wallpaper( rWallpaper );
}

const Wallpaper* SfxFrameDescriptor::GetWallpaper() const
{
    return pImp->pWallpaper;
}

USHORT SfxFrameDescriptor::GetItemPos() const
{
    if ( pParentFrameSet )
    {
        USHORT nRet = pParentFrameSet->aFrames.GetPos((SfxFrameDescriptor*)this);
        for ( USHORT n=0; n<nRet; n++ )
            if ( !pParentFrameSet->GetFrame(n)->nWidth )
                nRet--;
        return nRet;
    }

    return USHRT_MAX;
}


SfxFrameProperties::SfxFrameProperties( const SfxFrameDescriptor *pD )
    : aURL( pD->GetURL().GetMainURL() )
    , aName( pD->GetName() )
    , lMarginWidth( pD->GetMargin().Width() )
    , lMarginHeight( pD->GetMargin().Height() )
    , lSize( pD->GetWidth() )
    , lSetSize( SIZE_NOT_SET )
    , lFrameSpacing( SPACING_NOT_SET )
    , lInheritedFrameSpacing( SPACING_NOT_SET )
    , eScroll( pD->GetScrollingMode() )
    , eSizeSelector( pD->GetSizeSelector() )
    , eSetSizeSelector( SIZE_REL )
    , bHasBorder( pD->HasFrameBorder() )
    , bHasBorderInherited( FALSE )
    , bBorderSet( pD->IsFrameBorderSet() )
    , bResizable( pD->IsResizable() )
    , bIsInColSet( FALSE )
    , bSetResizable( FALSE )
    , bIsRootSet( FALSE )
    , pFrame( pD->Clone() )
{
    SfxFrameSetDescriptor *pSet = pD->GetParent();
    if ( pSet )
    {
        bIsRootSet = ( pSet->IsRootFrameSet() );
        lFrameSpacing = pSet->GetFrameSpacing();
        lSetSize = bIsRootSet ? SIZE_NOT_SET : pSet->GetParentFrame()->GetWidth();
        eSetSizeSelector = bIsRootSet ? SIZE_ABS : pSet->GetParentFrame()->GetSizeSelector();
        bSetResizable = bIsRootSet ? FALSE : pSet->GetParentFrame()->IsResizable();
        bIsInColSet = pSet->IsColSet();
        bHasBorderInherited = pSet->HasFrameBorder();
        lInheritedFrameSpacing = bIsRootSet ? SPACING_NOT_SET : pSet->GetParentFrame()->GetParent()->GetFrameSpacing();
    }
    else
        bBorderSet = TRUE;
}

SfxFrameProperties& SfxFrameProperties::operator =(
    const SfxFrameProperties &rProp )
{
    aURL = rProp.aURL;
    aName = rProp.aName;
    lMarginWidth = rProp.lMarginWidth;
    lMarginHeight = rProp.lMarginHeight;
    lSize = rProp.lSize;
    lSetSize = rProp.lSetSize;
    lFrameSpacing = rProp.lFrameSpacing;
    lInheritedFrameSpacing = rProp.lInheritedFrameSpacing;
    eScroll = rProp.eScroll;
    eSizeSelector = rProp.eSizeSelector;
    eSetSizeSelector = rProp.eSetSizeSelector;
    bHasBorder = rProp.bHasBorder;
    bBorderSet = rProp.bBorderSet;
    bResizable = rProp.bResizable;
    bSetResizable = rProp.bSetResizable;
    bIsRootSet = rProp.bIsRootSet;
    bIsInColSet = rProp.bIsInColSet;
    bHasBorderInherited = rProp.bHasBorderInherited;
    pFrame = rProp.pFrame->Clone();
    return *this;
}

int SfxFrameProperties::operator ==( const SfxFrameProperties& rProp ) const
{
    return aURL == rProp.aURL && aName == rProp.aName && lMarginWidth == rProp.lMarginWidth && lMarginHeight == rProp.lMarginHeight &&
            lSize == rProp.lSize && eScroll == rProp.eScroll && eSizeSelector == rProp.eSizeSelector &&
            lSetSize == rProp.lSetSize && lFrameSpacing == rProp.lFrameSpacing && eSetSizeSelector == rProp.eSetSizeSelector &&
            bHasBorder == rProp.bHasBorder && bBorderSet == rProp.bBorderSet &&
            bResizable == rProp.bResizable && bSetResizable == rProp.bSetResizable;
}

TYPEINIT1(SfxFrameDescriptorItem, SfxPoolItem);

SfxFrameDescriptorItem::~SfxFrameDescriptorItem()
{}

int SfxFrameDescriptorItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    return aProperties == ((SfxFrameDescriptorItem&)rAttr).aProperties;
}

// -----------------------------------------------------------------------

SfxPoolItem* SfxFrameDescriptorItem::Clone( SfxItemPool* ) const
{
    return new SfxFrameDescriptorItem( *this );
}

//------------------------------------------------------------------------

SfxItemPresentation SfxFrameDescriptorItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText,
    const International *
)   const
{
    rText.Erase();
    return SFX_ITEM_PRESENTATION_NONE;
}


