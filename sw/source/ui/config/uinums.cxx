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
#include "precompiled_sw.hxx"




#include <hintids.hxx>
#include <svl/urihelper.hxx>
#include <unotools/pathoptions.hxx>
#include <tools/stream.hxx>
#ifndef _SFX_INIMGR_HXX
#endif
#include <sfx2/docfile.hxx>
#include <svl/itemiter.hxx>
#include <editeng/brshitem.hxx>


#include <tools/resid.hxx>
#include <fmtornt.hxx>
#include <swtypes.hxx>      // Leerstring
#include <wrtsh.hxx>
#include <uinums.hxx>
#include <poolfmt.hxx>
#include <charfmt.hxx>
#include <frmatr.hxx>

#include <unomid.h>

using namespace ::com::sun::star;


#define VERSION_30B     ((sal_uInt16)250)
#define VERSION_31B     ((sal_uInt16)326)
#define VERSION_40A     ((sal_uInt16)364)
#define VERSION_50A     ((sal_uInt16)373)
#define VERSION_53A     ((sal_uInt16)596)
#define ACT_NUM_VERSION VERSION_53A

#define NUMRULE_FILENAME "numrule.cfg"
#define CHAPTER_FILENAME "chapter.cfg"

/*------------------------------------------------------------------------
 Beschreibung:  Ops. zum Laden / Speichern
------------------------------------------------------------------------*/


SV_IMPL_PTRARR( _SwNumFmtsAttrs, SfxPoolItem* )


// SwNumRulesWithName ----------------------------------------------------
// PUBLIC METHODES -------------------------------------------------------
/*------------------------------------------------------------------------
 Beschreibung:  Speichern einer Regel
 Parameter:     rCopy -- die zu speichernde Regel
                nIdx -- Position, an der die Regel zu speichern ist.
                        Eine alte Regel an dieser Position wird ueberschrieben.
------------------------------------------------------------------------*/

SwBaseNumRules::SwBaseNumRules( const String& rFileName )
    :
    sFileName( rFileName ),
    nVersion(0),
    bModified( sal_False )
{
    Init();
}

/*-----------------26.06.97 08.30-------------------

--------------------------------------------------*/
SwBaseNumRules::~SwBaseNumRules()
{
    if( bModified )
    {
        SvtPathOptions aPathOpt;
        String sNm( aPathOpt.GetUserConfigPath() );
        sNm += INET_PATH_TOKEN;
        sNm += sFileName;
        INetURLObject aTempObj(sNm);
        sNm = aTempObj.GetFull();
        SfxMedium aStrm( sNm, STREAM_WRITE | STREAM_TRUNC |
                                        STREAM_SHARE_DENYALL, sal_True );
        Store( *aStrm.GetOutStream() );
    }

    for( sal_uInt16 i = 0; i < nMaxRules; ++i )
        delete pNumRules[i];
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/
void  SwBaseNumRules::Init()
{
    for(sal_uInt16 i = 0; i < nMaxRules; ++i )
        pNumRules[i] = 0;

    String sNm( sFileName );
    SvtPathOptions aOpt;
    if( aOpt.SearchFile( sNm, SvtPathOptions::PATH_USERCONFIG ))
    {
        SfxMedium aStrm( sNm, STREAM_STD_READ, sal_True );
        Load( *aStrm.GetInStream() );
    }
}

/*-----------------26.06.97 08.30-------------------

--------------------------------------------------*/

void SwBaseNumRules::ApplyNumRules(const SwNumRulesWithName &rCopy, sal_uInt16 nIdx)
{
    ASSERT(nIdx < nMaxRules, Array der NumRules ueberindiziert.);
    if( !pNumRules[nIdx] )
        pNumRules[nIdx] = new SwNumRulesWithName( rCopy );
    else
        *pNumRules[nIdx] = rCopy;
}

// PROTECTED METHODES ----------------------------------------------------
/*------------------------------------------------------------------------
 Beschreibung:  Speichern
------------------------------------------------------------------------*/

sal_Bool /**/ SwBaseNumRules::Store(SvStream &rStream)
{
    rStream << ACT_NUM_VERSION;
        // Schreiben, welche Positionen durch eine Regel belegt sind
        // Anschliessend Schreiben der einzelnen Rules
    for(sal_uInt16 i = 0; i < nMaxRules; ++i)
    {
        if(pNumRules[i])
        {
            rStream << (unsigned char) sal_True;
            pNumRules[i]->Store( rStream );
        }
        else
            rStream << (unsigned char) sal_False;
    }
    return sal_True;
}



/*------------------------------------------------------------------------
 Beschreibung:  Speichern / Laden
------------------------------------------------------------------------*/


int SwBaseNumRules::Load(SvStream &rStream)
{
    int         rc = 0;

    rStream >> nVersion;

    // wegen eines kleinen aber schweren Fehlers schreibt die PreFinal die
    // gleiche VERSION_40A wie das SP2 #55402#
    if(VERSION_40A == nVersion)
    {
        DBG_ERROR("Version 364 ist nicht eindeutig #55402#");
    }
    else if( VERSION_30B == nVersion || VERSION_31B == nVersion ||
             ACT_NUM_VERSION >= nVersion )
    {
        unsigned char bRule = sal_False;
        for(sal_uInt16 i = 0; i < nMaxRules; ++i)
        {
            rStream >> bRule;
            if(bRule)
                pNumRules[i] = new SwNumRulesWithName( rStream, nVersion );
        }
    }
    else
    {
        rc = 1;
    }

    return rc;
}

/*-----------------26.06.97 08.34-------------------

--------------------------------------------------*/

/*------------------------------------------------------------------------*/


SwChapterNumRules::SwChapterNumRules() :
    SwBaseNumRules(C2S(CHAPTER_FILENAME))
{
}

/*------------------------------------------------------------------------*/

 SwChapterNumRules::~SwChapterNumRules()
{
}

/*-----------------26.06.97 08.23-------------------

--------------------------------------------------*/
void SwChapterNumRules::ApplyNumRules(const SwNumRulesWithName &rCopy, sal_uInt16 nIdx)
{
    bModified = sal_True;
    SwBaseNumRules::ApplyNumRules(rCopy, nIdx);
}

/*------------------------------------------------------------------------*/

SwNumRulesWithName::SwNumRulesWithName( const SwNumRule &rCopy,
                                        const String &rName )
    : aName(rName)
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

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/
SwNumRulesWithName::SwNumRulesWithName( const SwNumRulesWithName& rCopy )
{
    memset( aFmts, 0, sizeof( aFmts ));
    *this = rCopy;
}


/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/
SwNumRulesWithName::~SwNumRulesWithName()
{
    for( int n = 0; n < MAXLEVEL; ++n )
        delete aFmts[ n ];
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/
const SwNumRulesWithName& SwNumRulesWithName::operator=(const SwNumRulesWithName &rCopy)
{
    if( this != &rCopy )
    {
        aName = rCopy.aName;
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

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/
SwNumRulesWithName::SwNumRulesWithName( SvStream &rStream, sal_uInt16 nVersion )
{
    CharSet eEncoding = gsl_getSystemTextEncoding();
    rStream.ReadByteString(aName, eEncoding);

    char c;
    for(sal_uInt16 n = 0; n < MAXLEVEL; ++n )
    {
        if( VERSION_30B == nVersion )
            c = 1;
        // wegen eines kleinen aber schweren Fehlers schreibt die PreFinal die
        // gleiche VERSION_40A wie das SP2 #55402#
        else if(nVersion < VERSION_40A && n > 5)
//      else if(nVersion < VERSION_50A && n > 5)
            c = 0;
        else
            rStream >> c;

        if( c )
            aFmts[ n ] = new _SwNumFmtGlobal( rStream, nVersion );
        else
            aFmts[ n ] = 0;
    }
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

void SwNumRulesWithName::MakeNumRule( SwWrtShell& rSh, SwNumRule& rChg ) const
{
    // --> OD 2008-02-11 #newlistlevelattrs#
    // --> OD 2008-06-06 #i89178#
    rChg = SwNumRule( aName, numfunc::GetDefaultPositionAndSpaceMode() );
    // <--
    rChg.SetAutoRule( sal_False );
    _SwNumFmtGlobal* pFmt;
    for( sal_uInt16 n = 0; n < MAXLEVEL; ++n )
        if( 0 != ( pFmt = aFmts[ n ] ) )
        {
            SwNumFmt aNew;
            pFmt->ChgNumFmt( rSh, aNew );
            rChg.Set( n, aNew );
        }
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/
void SwNumRulesWithName::Store( SvStream &rStream )
{
    CharSet eEncoding = gsl_getSystemTextEncoding();
    rStream.WriteByteString(aName, eEncoding);

    for( sal_uInt16 n = 0; n < MAXLEVEL; ++n )
    {
        _SwNumFmtGlobal* pFmt = aFmts[ n ];
        if( pFmt )
        {
            rStream << (char)1;
            pFmt->Store( rStream );
        }
        else
            rStream << (char)0;
    }
}
/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/


SwNumRulesWithName::_SwNumFmtGlobal::_SwNumFmtGlobal( const SwNumFmt& rFmt )
    : aFmt( rFmt ), nCharPoolId( USHRT_MAX )
{
    // relative Abstaende ?????

    SwCharFmt* pFmt = rFmt.GetCharFmt();
    if( pFmt )
    {
        sCharFmtName = pFmt->GetName();
        nCharPoolId = pFmt->GetPoolFmtId();
        if( pFmt->GetAttrSet().Count() )
        {
            SfxItemIter aIter( pFmt->GetAttrSet() );
            const SfxPoolItem *pCurr = aIter.GetCurItem();
            while( sal_True )
            {
                aItems.Insert( pCurr->Clone(), aItems.Count() );
                if( aIter.IsAtEnd() )
                    break;
                pCurr = aIter.NextItem();
            }
        }

        aFmt.SetCharFmt( 0 );
    }
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

SwNumRulesWithName::_SwNumFmtGlobal::_SwNumFmtGlobal( const _SwNumFmtGlobal& rFmt )
    :
    aFmt( rFmt.aFmt ),
    sCharFmtName( rFmt.sCharFmtName ),
    nCharPoolId( rFmt.nCharPoolId )
{
    for( sal_uInt16 n = rFmt.aItems.Count(); n; )
        aItems.Insert( rFmt.aItems[ --n ]->Clone(), aItems.Count() );
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

SwNumRulesWithName::_SwNumFmtGlobal::_SwNumFmtGlobal( SvStream& rStream,
                                                        sal_uInt16 nVersion )
    : nCharPoolId( USHRT_MAX )
{
    CharSet eEncoding = gsl_getSystemTextEncoding();
    {
        sal_uInt16 nUS;
        sal_Char cChar;
        short nShort;
        sal_Bool bFlag;
        String sStr;

        rStream >> nUS;             aFmt.SetNumberingType((sal_Int16)nUS );
        if( VERSION_53A > nVersion )
        {
            rStream >> cChar;       aFmt.SetBulletChar( cChar );
        }
        else
        {
            rStream >> nUS;         aFmt.SetBulletChar( nUS );
        }

        rStream >> bFlag;           aFmt.SetIncludeUpperLevels( bFlag );

        if( VERSION_30B == nVersion )
        {
            long nL;
            rStream >> cChar;       aFmt.SetStart( (sal_uInt16)cChar );

            rStream.ReadByteString(sStr, eEncoding);
            aFmt.SetPrefix( sStr );
            rStream.ReadByteString(sStr, eEncoding);
            aFmt.SetSuffix( sStr );
            rStream >> nUS;         aFmt.SetNumAdjust( SvxAdjust( nUS ) );
            rStream >> nL;          aFmt.SetLSpace( lNumIndent );
            rStream >> nL;          aFmt.SetFirstLineOffset( (short)nL );
        }
        else                // alter StartWert war ein Byte
        {
            rStream >> nUS;         aFmt.SetStart( nUS );
            rStream.ReadByteString(sStr, eEncoding);
            aFmt.SetPrefix( sStr );
            rStream.ReadByteString(sStr, eEncoding);
            aFmt.SetSuffix( sStr );
            rStream >> nUS;         aFmt.SetNumAdjust( SvxAdjust( nUS ) );
            rStream >> nUS;         aFmt.SetAbsLSpace( nUS );
            rStream >> nShort;      aFmt.SetFirstLineOffset( nShort );
            rStream >> nUS;         aFmt.SetCharTextDistance( nUS );
            rStream >> nShort;      aFmt.SetLSpace( nShort );
            rStream >> bFlag;
        }

        sal_uInt16  nFamily;
        sal_uInt16  nCharSet;
        short   nWidth;
        short   nHeight;
        sal_uInt16  nPitch;
        String aName;

        rStream.ReadByteString(aName, eEncoding);
        rStream >> nFamily >> nCharSet >> nWidth >> nHeight >> nPitch;

        if( aName.Len() )
        {
            Font aFont( nFamily, Size( nWidth, nHeight ) );
            aFont.SetName( aName );
            aFont.SetCharSet( (CharSet)nCharSet );
            aFont.SetPitch( (FontPitch)nPitch );

            aFmt.SetBulletFont( &aFont );
        }
        else
            nCharSet = RTL_TEXTENCODING_SYMBOL;

        if( VERSION_53A > nVersion )
            aFmt.SetBulletChar( ByteString::ConvertToUnicode(
                                            sal_Char(aFmt.GetBulletChar()), nCharSet ));
    }

    if( VERSION_30B != nVersion )
    {
        sal_uInt16 nItemCount;
        rStream >> nCharPoolId;
        rStream.ReadByteString(sCharFmtName, eEncoding);
        rStream >> nItemCount;

        while( nItemCount-- )
        {
            sal_uInt16 nWhich, nVers;
            rStream >> nWhich >> nVers;
            aItems.Insert( GetDfltAttr( nWhich )->Create( rStream, nVers ),
                            aItems.Count() );
        }
    }

    if( VERSION_40A == nVersion && SVX_NUM_BITMAP == aFmt.GetNumberingType() )
    {
        sal_uInt8 cF;
        Size aSz;

        rStream >> aSz.Width() >> aSz.Height();

        rStream >> cF;
        if( cF )
        {
            SvxBrushItem* pBrush = 0;
            SwFmtVertOrient* pVOrient = 0;
            sal_uInt16 nVer;

            if( cF & 1 )
            {
                rStream >> nVer;
                pBrush = (SvxBrushItem*)GetDfltAttr( RES_BACKGROUND )
                                        ->Create( rStream, nVer );
            }

            if( cF & 2 )
            {
                rStream >> nVer;
                pVOrient = (SwFmtVertOrient*)GetDfltAttr( RES_VERT_ORIENT )
                                        ->Create( rStream, nVer );
            }
            sal_Int16 eOrient = text::VertOrientation::NONE;
            if(pVOrient)
                eOrient = (sal_Int16)pVOrient->GetVertOrient();
            aFmt.SetGraphicBrush( pBrush, &aSz, pVOrient ? &eOrient : 0 );
        }
    }
}


/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

SwNumRulesWithName::_SwNumFmtGlobal::~_SwNumFmtGlobal()
{
}
/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/


void SwNumRulesWithName::_SwNumFmtGlobal::Store( SvStream& rStream )
{
    CharSet eEncoding = gsl_getSystemTextEncoding();
    {
        String aName;
        sal_uInt16 nFamily = FAMILY_DONTKNOW, nCharSet = 0, nPitch = 0;
        short  nWidth = 0, nHeight = 0;

        const Font* pFnt = aFmt.GetBulletFont();
        if( pFnt )
        {
            aName = pFnt->GetName();
            nFamily = (sal_uInt16)pFnt->GetFamily();
            nCharSet = (sal_uInt16)pFnt->GetCharSet();
            nWidth = (short)pFnt->GetSize().Width();
            nHeight = (short)pFnt->GetSize().Height();
            nPitch = (sal_uInt16)pFnt->GetPitch();
        }

        rStream << sal_uInt16(aFmt.GetNumberingType())
                << aFmt.GetBulletChar()
                << (aFmt.GetIncludeUpperLevels() > 0)
                << aFmt.GetStart();
        rStream.WriteByteString( aFmt.GetPrefix(), eEncoding );
        rStream.WriteByteString( aFmt.GetSuffix(), eEncoding );
        rStream << sal_uInt16( aFmt.GetNumAdjust() )
                << aFmt.GetAbsLSpace()
                << aFmt.GetFirstLineOffset()
                << aFmt.GetCharTextDistance()
                << aFmt.GetLSpace()
                << sal_False;//aFmt.IsRelLSpace();
        rStream.WriteByteString( aName, eEncoding );
        rStream << nFamily
                << nCharSet
                << nWidth
                << nHeight
                << nPitch;
    }
    rStream << nCharPoolId;
    rStream.WriteByteString( sCharFmtName, eEncoding );
    rStream << aItems.Count();

    for( sal_uInt16 n = aItems.Count(); n; )
    {
        SfxPoolItem* pItem = aItems[ --n ];
        sal_uInt16 nIVers = pItem->GetVersion( SOFFICE_FILEFORMAT_50 );
        ASSERT( nIVers != USHRT_MAX,
                "Was'n das: Item-Version USHRT_MAX in der aktuellen Version" );
        rStream << pItem->Which()
                << nIVers;
        pItem->Store( rStream, nIVers );
    }

    // Erweiterungen fuer Version 40A

    if( SVX_NUM_BITMAP == aFmt.GetNumberingType() )
    {
        rStream << (sal_Int32)aFmt.GetGraphicSize().Width()
                << (sal_Int32)aFmt.GetGraphicSize().Height();
        sal_uInt8 cFlg = ( 0 != aFmt.GetBrush() ? 1 : 0 ) +
                    ( 0 != aFmt.GetGraphicOrientation() ? 2 : 0 );
        rStream << cFlg;

        if( aFmt.GetBrush() )
        {
            sal_uInt16 nVersion = aFmt.GetBrush()->GetVersion( SOFFICE_FILEFORMAT_50 );
            rStream << nVersion;
            aFmt.GetBrush()->Store( rStream, nVersion );
        }
        if( aFmt.GetGraphicOrientation() )
        {
            sal_uInt16 nVersion = aFmt.GetGraphicOrientation()->GetVersion( SOFFICE_FILEFORMAT_50 );
            rStream << nVersion;
            aFmt.GetGraphicOrientation()->Store( rStream, nVersion );
        }
    }
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

void SwNumRulesWithName::_SwNumFmtGlobal::ChgNumFmt( SwWrtShell& rSh,
                            SwNumFmt& rNew ) const
{
    SwCharFmt* pFmt = 0;
    if( sCharFmtName.Len() )
    {
        // suche erstmal ueber den Namen
        sal_uInt16 nArrLen = rSh.GetCharFmtCount();
        for( sal_uInt16 i = 1; i < nArrLen; ++i )
        {
            pFmt = &rSh.GetCharFmt( i );
            if( COMPARE_EQUAL == pFmt->GetName().CompareTo( sCharFmtName ))
                // ist vorhanden, also belasse die Attribute wie sie sind!
                break;
            pFmt = 0;
        }

        if( !pFmt )
        {
            if( IsPoolUserFmt( nCharPoolId ) )
            {
                pFmt = rSh.MakeCharFmt( sCharFmtName );
                pFmt->SetAuto( sal_False );
            }
            else
                pFmt = rSh.GetCharFmtFromPool( nCharPoolId );

            if( !pFmt->GetDepends() )       // Attribute setzen
                for( sal_uInt16 n = aItems.Count(); n; )
                    pFmt->SetFmtAttr( *aItems[ --n ] );
        }
    }
    ((SwNumFmt&)aFmt).SetCharFmt( pFmt );
    rNew = aFmt;
    if( pFmt )
        ((SwNumFmt&)aFmt).SetCharFmt( 0 );
}

