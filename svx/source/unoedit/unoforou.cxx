/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unoforou.cxx,v $
 *
 *  $Revision: 1.32 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 10:13:49 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <algorithm>

#ifndef _SFXSTYLE_HXX
#include <svtools/style.hxx>
#endif
#ifndef _COM_SUN_STAR_I18N_WORDTYPE_HPP_
#include <com/sun/star/i18n/WordType.hpp>
#endif

#include <svtools/itemset.hxx>
#include <svx/editeng.hxx>
#include <svx/editdata.hxx>
#include <svx/outliner.hxx>
#include <unoedhlp.hxx>
#include <svx/svdobj.hxx>

#ifndef _SFXPOOLITEM_HXX
#include <svtools/poolitem.hxx>
#endif

#ifndef _SV_WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif

#ifndef _EEITEM_HXX //autogen
#include <svx/eeitem.hxx>
#endif

#include <svx/unoforou.hxx>
#include <svx/unofored.hxx>

using namespace ::com::sun::star;

//------------------------------------------------------------------------

SvxOutlinerForwarder::SvxOutlinerForwarder( Outliner& rOutl, SdrObject* pSdrObj /* = 0 */ ) :
    rOutliner( rOutl ),
    pSdrObject( pSdrObj ),
    mpAttribsCache( NULL ),
    mpParaAttribsCache( NULL ),
    mnParaAttribsCache( 0 )
{
}

SvxOutlinerForwarder::~SvxOutlinerForwarder()
{
    flushCache();
}

USHORT SvxOutlinerForwarder::GetParagraphCount() const
{
    return (USHORT)rOutliner.GetParagraphCount();
}

USHORT SvxOutlinerForwarder::GetTextLen( USHORT nParagraph ) const
{
    return rOutliner.GetEditEngine().GetTextLen( nParagraph );
}

String SvxOutlinerForwarder::GetText( const ESelection& rSel ) const
{
    //! GetText(ESelection) sollte es wohl auch mal am Outliner geben
    //  solange den Hack fuer die EditEngine uebernehmen:
    EditEngine* pEditEngine = (EditEngine*)&rOutliner.GetEditEngine();
    return pEditEngine->GetText( rSel, LINEEND_LF );
}

static SfxItemSet ImplOutlinerForwarderGetAttribs( const ESelection& rSel, BOOL bOnlyHardAttrib, EditEngine& rEditEngine )
{
    if( rSel.nStartPara == rSel.nEndPara )
    {
        sal_uInt8 nFlags = 0;

        switch( bOnlyHardAttrib )
        {
        case EditEngineAttribs_All:
            nFlags = GETATTRIBS_ALL;
            break;
        case EditEngineAttribs_HardAndPara:
            nFlags = GETATTRIBS_PARAATTRIBS|GETATTRIBS_CHARATTRIBS;
            break;
        case EditEngineAttribs_OnlyHard:
            nFlags = GETATTRIBS_CHARATTRIBS;
            break;
        default:
            DBG_ERROR("unknown flags for SvxOutlinerForwarder::GetAttribs");
        }
        return rEditEngine.GetAttribs( rSel.nStartPara, rSel.nStartPos, rSel.nEndPos, nFlags );
    }
    else
    {
        return rEditEngine.GetAttribs( rSel, bOnlyHardAttrib );
    }
}

SfxItemSet SvxOutlinerForwarder::GetAttribs( const ESelection& rSel, BOOL bOnlyHardAttrib ) const
{
    if( mpAttribsCache && ( 0 == bOnlyHardAttrib ) )
    {
        // have we the correct set in cache?
        if( ((SvxOutlinerForwarder*)this)->maAttribCacheSelection.IsEqual(rSel) )
        {
            // yes! just return the cache
            return *mpAttribsCache;
        }
        else
        {
            // no, we need delete the old cache
            delete mpAttribsCache;
            mpAttribsCache = NULL;
        }
    }

    //! gibt's das nicht am Outliner ???
    //! und warum ist GetAttribs an der EditEngine nicht const?
    EditEngine& rEditEngine = (EditEngine&)rOutliner.GetEditEngine();

    SfxItemSet aSet( ImplOutlinerForwarderGetAttribs( rSel, bOnlyHardAttrib, rEditEngine ) );

    if( 0 == bOnlyHardAttrib )
    {
        mpAttribsCache = new SfxItemSet( aSet );
        maAttribCacheSelection = rSel;
    }

    SfxStyleSheet* pStyle = rEditEngine.GetStyleSheet( rSel.nStartPara );
    if( pStyle )
        aSet.SetParent( &(pStyle->GetItemSet() ) );

    return aSet;
}

SfxItemSet SvxOutlinerForwarder::GetParaAttribs( USHORT nPara ) const
{
    if( mpParaAttribsCache )
    {
        // have we the correct set in cache?
        if( nPara == mnParaAttribsCache )
        {
            // yes! just return the cache
            return *mpParaAttribsCache;
        }
        else
        {
            // no, we need delete the old cache
            delete mpParaAttribsCache;
            mpParaAttribsCache = NULL;
        }
    }

    mpParaAttribsCache = new SfxItemSet( rOutliner.GetParaAttribs( nPara ) );
    mnParaAttribsCache = nPara;

    EditEngine& rEditEngine = (EditEngine&)rOutliner.GetEditEngine();

    SfxStyleSheet* pStyle = rEditEngine.GetStyleSheet( nPara );
    if( pStyle )
        mpParaAttribsCache->SetParent( &(pStyle->GetItemSet() ) );

    return *mpParaAttribsCache;
}

void SvxOutlinerForwarder::SetParaAttribs( USHORT nPara, const SfxItemSet& rSet )
{
    flushCache();

    const SfxItemSet* pOldParent = rSet.GetParent();
    if( pOldParent )
        ((SfxItemSet*)&rSet)->SetParent( NULL );

    rOutliner.SetParaAttribs( nPara, rSet, true );

    if( pOldParent )
        ((SfxItemSet*)&rSet)->SetParent( pOldParent );
}

SfxItemPool* SvxOutlinerForwarder::GetPool() const
{
    return rOutliner.GetEmptyItemSet().GetPool();
}

void SvxOutlinerForwarder::GetPortions( USHORT nPara, SvUShorts& rList ) const
{
    ((EditEngine&)rOutliner.GetEditEngine()).GetPortions( nPara, rList );
}

void SvxOutlinerForwarder::QuickInsertText( const String& rText, const ESelection& rSel )
{
    flushCache();
    if( rText.Len() == 0 )
    {
        rOutliner.QuickDelete( rSel );
    }
    else
    {
        rOutliner.QuickInsertText( rText, rSel );
    }
}

void SvxOutlinerForwarder::QuickInsertLineBreak( const ESelection& rSel )
{
    flushCache();
    rOutliner.QuickInsertLineBreak( rSel );
}

void SvxOutlinerForwarder::QuickInsertField( const SvxFieldItem& rFld, const ESelection& rSel )
{
    flushCache();
    rOutliner.QuickInsertField( rFld, rSel );
}

void SvxOutlinerForwarder::QuickSetAttribs( const SfxItemSet& rSet, const ESelection& rSel )
{
    flushCache();
    rOutliner.QuickSetAttribs( rSet, rSel );
}

XubString SvxOutlinerForwarder::CalcFieldValue( const SvxFieldItem& rField, USHORT nPara, USHORT nPos, Color*& rpTxtColor, Color*& rpFldColor )
{
    return rOutliner.CalcFieldValue( rField, nPara, nPos, rpTxtColor, rpFldColor );
}

BOOL SvxOutlinerForwarder::IsValid() const
{
    // cannot reliably query outliner state
    // while in the middle of an update
    return rOutliner.GetUpdateMode();
}

extern USHORT GetSvxEditEngineItemState( EditEngine& rEditEngine, const ESelection& rSel, USHORT nWhich );

USHORT SvxOutlinerForwarder::GetItemState( const ESelection& rSel, USHORT nWhich ) const
{
    return GetSvxEditEngineItemState( (EditEngine&)rOutliner.GetEditEngine(), rSel, nWhich );
}

USHORT SvxOutlinerForwarder::GetItemState( USHORT nPara, USHORT nWhich ) const
{
    const SfxItemSet& rSet = rOutliner.GetParaAttribs( nPara );
    return rSet.GetItemState( nWhich );
}


void SvxOutlinerForwarder::flushCache()
{
    if( mpAttribsCache )
    {
        delete mpAttribsCache;
        mpAttribsCache = NULL;
    }

    if( mpParaAttribsCache )
    {
        delete mpParaAttribsCache;
        mpParaAttribsCache = NULL;
    }
}

LanguageType SvxOutlinerForwarder::GetLanguage( USHORT nPara, USHORT nIndex ) const
{
    return rOutliner.GetLanguage(nPara, nIndex);
}

USHORT SvxOutlinerForwarder::GetFieldCount( USHORT nPara ) const
{
    return rOutliner.GetEditEngine().GetFieldCount(nPara);
}

EFieldInfo SvxOutlinerForwarder::GetFieldInfo( USHORT nPara, USHORT nField ) const
{
    return rOutliner.GetEditEngine().GetFieldInfo( nPara, nField );
}

EBulletInfo SvxOutlinerForwarder::GetBulletInfo( USHORT nPara ) const
{
    return rOutliner.GetBulletInfo( nPara );
}

Rectangle SvxOutlinerForwarder::GetCharBounds( USHORT nPara, USHORT nIndex ) const
{
    // #101701#
    // EditEngine's 'internal' methods like GetCharacterBounds()
    // don't rotate for vertical text.
    Size aSize( rOutliner.CalcTextSize() );
    ::std::swap( aSize.Width(), aSize.Height() );
    bool bIsVertical( rOutliner.IsVertical() == TRUE );

    // #108900# Handle virtual position one-past-the end of the string
    if( nIndex >= GetTextLen(nPara) )
    {
        Rectangle aLast;

        if( nIndex )
        {
            // use last character, if possible
            aLast = rOutliner.GetEditEngine().GetCharacterBounds( EPosition(nPara, nIndex-1) );

            // move at end of this last character, make one pixel wide
            aLast.Move( aLast.Right() - aLast.Left(), 0 );
            aLast.SetSize( Size(1, aLast.GetHeight()) );

            // take care for CTL
            aLast = SvxEditSourceHelper::EEToUserSpace( aLast, aSize, bIsVertical );
        }
        else
        {
            // #109864# Bounds must lie within the paragraph
            aLast = GetParaBounds( nPara );

            // #109151# Don't use paragraph height, but line height
            // instead. aLast is already CTL-correct
            if( bIsVertical)
                aLast.SetSize( Size( rOutliner.GetLineHeight(nPara,0), 1 ) );
            else
                aLast.SetSize( Size( 1, rOutliner.GetLineHeight(nPara,0) ) );
        }

        return aLast;
    }
    else
    {
        return SvxEditSourceHelper::EEToUserSpace( rOutliner.GetEditEngine().GetCharacterBounds( EPosition(nPara, nIndex) ),
                                                   aSize, bIsVertical );
    }
}

Rectangle SvxOutlinerForwarder::GetParaBounds( USHORT nPara ) const
{
    Point aPnt = rOutliner.GetDocPosTopLeft( nPara );
    Size aSize = rOutliner.CalcTextSize();

    if( rOutliner.IsVertical() )
    {
        // #101701#
        // Hargl. Outliner's 'external' methods return the rotated
        // dimensions, 'internal' methods like GetTextHeight( n )
        // don't rotate.
        ULONG nWidth = rOutliner.GetTextHeight( nPara );

        return Rectangle( aSize.Width() - aPnt.Y() - nWidth, 0, aSize.Width() - aPnt.Y(), aSize.Height() );
    }
    else
    {
        ULONG nHeight = rOutliner.GetTextHeight( nPara );

        return Rectangle( 0, aPnt.Y(), aSize.Width(), aPnt.Y() + nHeight );
    }
}

MapMode SvxOutlinerForwarder::GetMapMode() const
{
    return rOutliner.GetRefMapMode();
}

OutputDevice* SvxOutlinerForwarder::GetRefDevice() const
{
    return rOutliner.GetRefDevice();
}

sal_Bool SvxOutlinerForwarder::GetIndexAtPoint( const Point& rPos, USHORT& nPara, USHORT& nIndex ) const
{
    // #101701#
    Size aSize( rOutliner.CalcTextSize() );
    ::std::swap( aSize.Width(), aSize.Height() );
    Point aEEPos( SvxEditSourceHelper::UserSpaceToEE( rPos,
                                                      aSize,
                                                      rOutliner.IsVertical() == TRUE ));

    EPosition aDocPos = rOutliner.GetEditEngine().FindDocPosition( aEEPos );

    nPara = aDocPos.nPara;
    nIndex = aDocPos.nIndex;

    return sal_True;
}

sal_Bool SvxOutlinerForwarder::GetWordIndices( USHORT nPara, USHORT nIndex, USHORT& nStart, USHORT& nEnd ) const
{
    ESelection aRes = rOutliner.GetEditEngine().GetWord( ESelection(nPara, nIndex, nPara, nIndex), com::sun::star::i18n::WordType::DICTIONARY_WORD );

    if( aRes.nStartPara == nPara &&
        aRes.nStartPara == aRes.nEndPara )
    {
        nStart = aRes.nStartPos;
        nEnd = aRes.nEndPos;

        return sal_True;
    }

    return sal_False;
}

sal_Bool SvxOutlinerForwarder::GetAttributeRun( USHORT& nStartIndex, USHORT& nEndIndex, USHORT nPara, USHORT nIndex ) const
{
    return SvxEditSourceHelper::GetAttributeRun( nStartIndex, nEndIndex, rOutliner.GetEditEngine(), nPara, nIndex );
}

USHORT SvxOutlinerForwarder::GetLineCount( USHORT nPara ) const
{
    return static_cast < USHORT >( rOutliner.GetLineCount(nPara) );
}

USHORT SvxOutlinerForwarder::GetLineLen( USHORT nPara, USHORT nLine ) const
{
    return rOutliner.GetLineLen(nPara, nLine);
}

sal_Bool SvxOutlinerForwarder::QuickFormatDoc( BOOL )
{
    rOutliner.QuickFormatDoc();

    return sal_True;
}

sal_Bool SvxOutlinerForwarder::Delete( const ESelection& rSelection )
{
    flushCache();
    rOutliner.QuickDelete( rSelection );
    rOutliner.QuickFormatDoc();

    return sal_True;
}

sal_Bool SvxOutlinerForwarder::InsertText( const String& rStr, const ESelection& rSelection )
{
    flushCache();
    rOutliner.QuickInsertText( rStr, rSelection );
    rOutliner.QuickFormatDoc();

    return sal_True;
}

USHORT SvxOutlinerForwarder::GetDepth( USHORT nPara ) const
{
    DBG_ASSERT( nPara < GetParagraphCount(), "SvxOutlinerForwarder::GetDepth: Invalid paragraph index");

    Paragraph* pPara = rOutliner.GetParagraph( nPara );

    USHORT nLevel(0);

    if( pPara )
    {
        nLevel = rOutliner.GetDepth( nPara );

        if(pSdrObject != NULL)
        {
            if((pSdrObject->GetObjInventor() == SdrInventor) &&
               (pSdrObject->GetObjIdentifier() == OBJ_OUTLINETEXT))
                --nLevel;
        }
    }

    return nLevel;
}

sal_Bool SvxOutlinerForwarder::SetDepth( USHORT nPara, USHORT nNewDepth )
{
    DBG_ASSERT( nPara < GetParagraphCount(), "SvxOutlinerForwarder::SetDepth: Invalid paragraph index");

    const sal_Bool bOutlinerText = pSdrObject && (pSdrObject->GetObjInventor() == SdrInventor) && (pSdrObject->GetObjIdentifier() == OBJ_OUTLINETEXT);

    if(bOutlinerText)
        ++nNewDepth;

    if( nNewDepth <= 9 )
    {
        Paragraph* pPara = rOutliner.GetParagraph( nPara );
        if( pPara )
        {
            rOutliner.SetDepth( pPara, nNewDepth );
            if( bOutlinerText )
                rOutliner.SetLevelDependendStyleSheet( nPara );

            return sal_True;
        }
    }

    return sal_False;
}

const SfxItemSet * SvxOutlinerForwarder::GetEmptyItemSetPtr()
{
    EditEngine& rEditEngine = const_cast< EditEngine& >( rOutliner.GetEditEngine() );
    return &rEditEngine.GetEmptyItemSet();
}

void SvxOutlinerForwarder::AppendParagraph()
{
    EditEngine& rEditEngine = const_cast< EditEngine& >( rOutliner.GetEditEngine() );
    rEditEngine.InsertParagraph( rEditEngine.GetParagraphCount(), String::EmptyString() );
}

xub_StrLen SvxOutlinerForwarder::AppendTextPortion( USHORT nPara, const String &rText, const SfxItemSet & /*rSet*/ )
{
    xub_StrLen nLen = 0;

    EditEngine& rEditEngine = const_cast< EditEngine& >( rOutliner.GetEditEngine() );
    USHORT nParaCount = rEditEngine.GetParagraphCount();
    DBG_ASSERT( nPara < nParaCount, "paragraph index out of bounds" );
    if (/*0 <= nPara && */nPara < nParaCount)
    {
        nLen = rEditEngine.GetTextLen( nPara );
        rEditEngine.QuickInsertText( rText, ESelection( nPara, nLen, nPara, nLen ) );
    }

    return nLen;
}


//------------------------------------------------------------------------
