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

#include <hintids.hxx>
#include <svl/urihelper.hxx>
#include <osl/thread.h>
#include <unotools/pathoptions.hxx>
#include <tools/stream.hxx>
#include <sfx2/docfile.hxx>
#include <svl/itemiter.hxx>
#include <editeng/brushitem.hxx>

#include <tools/resid.hxx>
#include <fmtornt.hxx>
#include <swtypes.hxx>
#include <wrtsh.hxx>
#include <uinums.hxx>
#include <poolfmt.hxx>
#include <charfmt.hxx>
#include <frmatr.hxx>

#include <unomid.h>

using namespace ::com::sun::star;

#define CHAPTER_FILENAME "chapter.cfg"

/*
 Description:   Saving a rule
 Parameter:     rCopy -- the rule to save
                    nIdx -- position, where the rule is to be saved.
                        An old rule at that position will be overwritten.
*/

SwBaseNumRules::SwBaseNumRules( const OUString& rFileName )
    :
    sFileName( rFileName ),
    bModified( false )
{
    Init();
}

SwBaseNumRules::~SwBaseNumRules()
{
    if( bModified )
    {
        SvtPathOptions aPathOpt;
        OUString sNm( aPathOpt.GetUserConfigPath() + "/" + sFileName );
        INetURLObject aTempObj(sNm);
        sNm = aTempObj.GetFull();
        SfxMedium aStrm( sNm, STREAM_WRITE | STREAM_TRUNC |
                                        STREAM_SHARE_DENYALL );
        Store( *aStrm.GetOutStream() );
    }

    for( sal_uInt16 i = 0; i < nMaxRules; ++i )
        delete pNumRules[i];
}

void  SwBaseNumRules::Init()
{
    for(sal_uInt16 i = 0; i < nMaxRules; ++i )
        pNumRules[i] = 0;

    OUString sNm( sFileName );
    SvtPathOptions aOpt;
    if( aOpt.SearchFile( sNm, SvtPathOptions::PATH_USERCONFIG ))
    {
        SfxMedium aStrm( sNm, STREAM_STD_READ );
        Load( *aStrm.GetInStream() );
    }
}

void SwBaseNumRules::ApplyNumRules(const SwNumRulesWithName &rCopy, sal_uInt16 nIdx)
{
    OSL_ENSURE(nIdx < nMaxRules, "Array der NumRules ueberindiziert.");
    if( !pNumRules[nIdx] )
        pNumRules[nIdx] = new SwNumRulesWithName( rCopy );
    else
        *pNumRules[nIdx] = rCopy;
}

bool SwBaseNumRules::Store(SvStream &rStream)
{
    // Write, what positions are occupied by a rule
    // Then write each of the rules
    for(sal_uInt16 i = 0; i < nMaxRules; ++i)
    {
        if(pNumRules[i])
        {
            rStream.WriteUChar( (unsigned char) sal_True );
            pNumRules[i]->Store( rStream );
        }
        else
            rStream.WriteUChar( (unsigned char) sal_False );
    }
    return true;
}

int SwBaseNumRules::Load(SvStream &rStream)
{
    int         rc = 0;

    unsigned char bRule = sal_False;
    for(sal_uInt16 i = 0; i < nMaxRules; ++i)
    {
        rStream.ReadUChar( bRule );
        if(bRule)
            pNumRules[i] = new SwNumRulesWithName( rStream );
    }

    return rc;
}

SwChapterNumRules::SwChapterNumRules() :
    SwBaseNumRules(OUString(CHAPTER_FILENAME))
{
}

 SwChapterNumRules::~SwChapterNumRules()
{
}

void SwChapterNumRules::ApplyNumRules(const SwNumRulesWithName &rCopy, sal_uInt16 nIdx)
{
    bModified = true;
    SwBaseNumRules::ApplyNumRules(rCopy, nIdx);
}

SwNumRulesWithName::SwNumRulesWithName( const SwNumRule &rCopy,
                                        const OUString &rName )
    : maName(rName)
{
    for( sal_uInt16 n = 0; n < MAXLEVEL; ++n )
    {
        const SwNumFmt* pFmt = rCopy.GetNumFmt( n );
        if( pFmt )
            aFmts[ n ] = new _SwNumFmtGlobal( *pFmt );
        else
            aFmts[ n ] = 0;
    }
}

SwNumRulesWithName::SwNumRulesWithName( const SwNumRulesWithName& rCopy )
{
    memset( aFmts, 0, sizeof( aFmts ));
    *this = rCopy;
}

SwNumRulesWithName::~SwNumRulesWithName()
{
    for( int n = 0; n < MAXLEVEL; ++n )
        delete aFmts[ n ];
}

const SwNumRulesWithName& SwNumRulesWithName::operator=(const SwNumRulesWithName &rCopy)
{
    if( this != &rCopy )
    {
        maName = rCopy.maName;
        for( int n = 0; n < MAXLEVEL; ++n )
        {
            delete aFmts[ n ];

            _SwNumFmtGlobal* pFmt = rCopy.aFmts[ n ];
            if( pFmt )
                aFmts[ n ] = new _SwNumFmtGlobal( *pFmt );
            else
                aFmts[ n ] = 0;
        }
    }
    return *this;
}

SwNumRulesWithName::SwNumRulesWithName( SvStream &rStream )
{
    rtl_TextEncoding eEncoding = osl_getThreadTextEncoding();
    maName = rStream.ReadUniOrByteString(eEncoding);

    char c;
    for(sal_uInt16 n = 0; n < MAXLEVEL; ++n )
    {
        rStream.ReadChar( c );

        if( c )
            aFmts[ n ] = new _SwNumFmtGlobal( rStream );
        else
            aFmts[ n ] = 0;
    }
}

void SwNumRulesWithName::MakeNumRule( SwWrtShell& rSh, SwNumRule& rChg ) const
{
    // #i89178#
    rChg = SwNumRule( maName, numfunc::GetDefaultPositionAndSpaceMode() );
    rChg.SetAutoRule( false );
    _SwNumFmtGlobal* pFmt;
    for( sal_uInt16 n = 0; n < MAXLEVEL; ++n )
        if( 0 != ( pFmt = aFmts[ n ] ) )
        {
            SwNumFmt aNew;
            pFmt->ChgNumFmt( rSh, aNew );
            rChg.Set( n, aNew );
        }
}

void SwNumRulesWithName::Store( SvStream &rStream )
{
    rtl_TextEncoding eEncoding = osl_getThreadTextEncoding();
    rStream.WriteUniOrByteString(maName, eEncoding);

    for( sal_uInt16 n = 0; n < MAXLEVEL; ++n )
    {
        _SwNumFmtGlobal* pFmt = aFmts[ n ];
        if( pFmt )
        {
            rStream.WriteChar( (char)1 );
            pFmt->Store( rStream );
        }
        else
            rStream.WriteChar( (char)0 );
    }
}

SwNumRulesWithName::_SwNumFmtGlobal::_SwNumFmtGlobal( const SwNumFmt& rFmt )
    : aFmt( rFmt ), nCharPoolId( USHRT_MAX )
{
    // relative gaps?????

    SwCharFmt* pFmt = rFmt.GetCharFmt();
    if( pFmt )
    {
        sCharFmtName = pFmt->GetName();
        nCharPoolId = pFmt->GetPoolFmtId();
        if( pFmt->GetAttrSet().Count() )
        {
            SfxItemIter aIter( pFmt->GetAttrSet() );
            const SfxPoolItem *pCurr = aIter.GetCurItem();
            while( true )
            {
                aItems.push_back( pCurr->Clone() );
                if( aIter.IsAtEnd() )
                    break;
                pCurr = aIter.NextItem();
            }
        }

        aFmt.SetCharFmt( 0 );
    }
}

SwNumRulesWithName::_SwNumFmtGlobal::_SwNumFmtGlobal( const _SwNumFmtGlobal& rFmt )
    :
    aFmt( rFmt.aFmt ),
    sCharFmtName( rFmt.sCharFmtName ),
    nCharPoolId( rFmt.nCharPoolId )
{
    for( sal_uInt16 n = rFmt.aItems.size(); n; )
        aItems.push_back( rFmt.aItems[ --n ].Clone() );
}

SwNumRulesWithName::_SwNumFmtGlobal::_SwNumFmtGlobal( SvStream& rStream )
    : nCharPoolId( USHRT_MAX )
{
    rtl_TextEncoding eEncoding = osl_getThreadTextEncoding();
    {
        sal_uInt16 nUS;
        sal_Char cChar;
        bool bFlag;
        OUString sStr;

        rStream.ReadUInt16( nUS );           aFmt.SetNumberingType((sal_Int16)nUS );
        rStream.ReadChar( cChar );           aFmt.SetBulletChar( cChar );

        rStream.ReadCharAsBool( bFlag );     aFmt.SetIncludeUpperLevels( bFlag );

        sal_Int32 nL;
        rStream.ReadChar( cChar );       aFmt.SetStart( (sal_uInt16)cChar );

        sStr = rStream.ReadUniOrByteString(eEncoding);
        aFmt.SetPrefix( sStr );
        sStr = rStream.ReadUniOrByteString(eEncoding);
        aFmt.SetSuffix( sStr );
        rStream.ReadUInt16( nUS );        aFmt.SetNumAdjust( SvxAdjust( nUS ) );
        rStream.ReadInt32( nL );          aFmt.SetLSpace( lNumIndent );
        rStream.ReadInt32( nL );          aFmt.SetFirstLineOffset( (short)nL );

        sal_uInt16  nFamily;
        sal_uInt16  nCharSet;
        short   nWidth;
        short   nHeight;
        sal_uInt16  nPitch;
        OUString aName;

        aName = rStream.ReadUniOrByteString(eEncoding);
        rStream.ReadUInt16( nFamily ).ReadUInt16( nCharSet ).ReadInt16( nWidth ).ReadInt16( nHeight ).ReadUInt16( nPitch );

        if( !aName.isEmpty() )
        {
            vcl::Font aFont( static_cast<FontFamily>(nFamily), Size( nWidth, nHeight ) );
            aFont.SetName( aName );
            aFont.SetCharSet( (rtl_TextEncoding)nCharSet );
            aFont.SetPitch( (FontPitch)nPitch );

            aFmt.SetBulletFont( &aFont );
        }
        else
            nCharSet = RTL_TEXTENCODING_SYMBOL;

        sal_Char cEncoded(aFmt.GetBulletChar());
        aFmt.SetBulletChar(OUString(&cEncoded, 1, nCharSet).toChar());
    }

    sal_uInt16 nItemCount;
    rStream.ReadUInt16( nCharPoolId );
    sCharFmtName = rStream.ReadUniOrByteString(eEncoding);
    rStream.ReadUInt16( nItemCount );

    while( nItemCount-- )
    {
        sal_uInt16 nWhich;
        rStream.ReadUInt16( nWhich );
        aItems.push_back( GetDfltAttr( nWhich )->Create( rStream ) );
    }

    if( SVX_NUM_BITMAP == aFmt.GetNumberingType() )
    {
        sal_uInt8 cF;
        sal_Int32 nWidth(0), nHeight(0);

        rStream.ReadInt32( nWidth ).ReadInt32( nHeight );

        Size aSz(nWidth, nHeight);

        rStream.ReadUChar( cF );
        if( cF )
        {
            SvxBrushItem* pBrush = 0;
            SwFmtVertOrient* pVOrient = 0;
            sal_uInt16 nVer;

            if( cF & 1 )
            {
                rStream.ReadUInt16( nVer );
                pBrush = (SvxBrushItem*)GetDfltAttr( RES_BACKGROUND )
                                        ->Create( rStream );
            }

            if( cF & 2 )
            {
                rStream.ReadUInt16( nVer );
                pVOrient = (SwFmtVertOrient*)GetDfltAttr( RES_VERT_ORIENT )
                                        ->Create( rStream );
            }
            sal_Int16 eOrient = text::VertOrientation::NONE;
            if(pVOrient)
                eOrient = (sal_Int16)pVOrient->GetVertOrient();
            aFmt.SetGraphicBrush( pBrush, &aSz, pVOrient ? &eOrient : 0 );
        }
    }
}

SwNumRulesWithName::_SwNumFmtGlobal::~_SwNumFmtGlobal()
{
}

void SwNumRulesWithName::_SwNumFmtGlobal::Store( SvStream& rStream )
{
    rtl_TextEncoding eEncoding = osl_getThreadTextEncoding();
    {
        OUString aName;
        sal_uInt16 nFamily = FAMILY_DONTKNOW, nCharSet = 0, nPitch = 0;
        short  nWidth = 0, nHeight = 0;

        const vcl::Font* pFnt = aFmt.GetBulletFont();
        if( pFnt )
        {
            aName = pFnt->GetName();
            nFamily = (sal_uInt16)pFnt->GetFamily();
            nCharSet = (sal_uInt16)pFnt->GetCharSet();
            nWidth = (short)pFnt->GetSize().Width();
            nHeight = (short)pFnt->GetSize().Height();
            nPitch = (sal_uInt16)pFnt->GetPitch();
        }

        rStream.WriteUInt16( sal_uInt16(aFmt.GetNumberingType()) )
               .WriteUInt16( aFmt.GetBulletChar() )
               .WriteUChar( aFmt.GetIncludeUpperLevels() > 0 )
               .WriteUInt16( aFmt.GetStart() );
        rStream.WriteUniOrByteString( aFmt.GetPrefix(), eEncoding );
        rStream.WriteUniOrByteString( aFmt.GetSuffix(), eEncoding );
        rStream.WriteUInt16( sal_uInt16( aFmt.GetNumAdjust() ) )
               .WriteInt16( aFmt.GetAbsLSpace() )
               .WriteInt16( aFmt.GetFirstLineOffset() )
               .WriteInt16( aFmt.GetCharTextDistance() )
               .WriteInt16( aFmt.GetLSpace() )
               .WriteUChar( sal_False );//aFmt.IsRelLSpace();
        rStream.WriteUniOrByteString( aName, eEncoding );
        rStream.WriteUInt16( nFamily )
               .WriteUInt16( nCharSet )
               .WriteInt16( nWidth )
               .WriteInt16( nHeight )
               .WriteUInt16( nPitch );
    }
    rStream.WriteUInt16( nCharPoolId );
    rStream.WriteUniOrByteString( sCharFmtName, eEncoding );
    rStream.WriteUInt16( static_cast<sal_uInt16>(aItems.size()) );

    for( sal_uInt16 n = aItems.size(); n; )
    {
        SfxPoolItem* pItem = &aItems[ --n ];
        rStream.WriteUInt16( pItem->Which() );
        pItem->Store( rStream );
    }

    // Extensions for 40A

    if( SVX_NUM_BITMAP == aFmt.GetNumberingType() )
    {
        rStream.WriteInt32( (sal_Int32)aFmt.GetGraphicSize().Width() )
               .WriteInt32( (sal_Int32)aFmt.GetGraphicSize().Height() );
        sal_uInt8 cFlg = ( 0 != aFmt.GetBrush() ? 1 : 0 ) +
                    ( 0 != aFmt.GetGraphicOrientation() ? 2 : 0 );
        rStream.WriteUChar( cFlg );

        if( aFmt.GetBrush() )
        {
            aFmt.GetBrush()->Store( rStream );
        }
        if( aFmt.GetGraphicOrientation() )
        {
            aFmt.GetGraphicOrientation()->Store( rStream );
        }
    }
}

void SwNumRulesWithName::_SwNumFmtGlobal::ChgNumFmt( SwWrtShell& rSh,
                            SwNumFmt& rNew ) const
{
    SwCharFmt* pFmt = 0;
    if( !sCharFmtName.isEmpty() )
    {
        // at first, look for the name
        sal_uInt16 nArrLen = rSh.GetCharFmtCount();
        for( sal_uInt16 i = 1; i < nArrLen; ++i )
        {
            pFmt = &rSh.GetCharFmt( i );
            if (pFmt->GetName()==sCharFmtName)
                // exists, so leave attributes as they are!
                break;
            pFmt = 0;
        }

        if( !pFmt )
        {
            if( IsPoolUserFmt( nCharPoolId ) )
            {
                pFmt = rSh.MakeCharFmt( sCharFmtName );
                pFmt->SetAuto( false );
            }
            else
                pFmt = rSh.GetCharFmtFromPool( nCharPoolId );

            if( !pFmt->GetDepends() )       // set attributes
                for( sal_uInt16 n = aItems.size(); n; )
                    pFmt->SetFmtAttr( aItems[ --n ] );
        }
    }
    ((SwNumFmt&)aFmt).SetCharFmt( pFmt );
    rNew = aFmt;
    if( pFmt )
        ((SwNumFmt&)aFmt).SetCharFmt( 0 );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
