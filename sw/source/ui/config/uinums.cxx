/*************************************************************************
 *
 *  $RCSfile: uinums.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: os $ $Date: 2001-02-23 12:45:29 $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop


#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#ifndef _SFX_INIMGR_HXX
#endif
#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif
#ifndef _SFXITEMITER_HXX //autogen
#include <svtools/itemiter.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif


#ifndef _TOOLS_RESID_HXX
#include <tools/resid.hxx>
#endif
#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>      // Leerstring
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _UINUMS_HXX
#include <uinums.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _CHARFMT_HXX
#include <charfmt.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif

#define VERSION_30B     ((USHORT)250)
#define VERSION_31B     ((USHORT)326)
#define VERSION_40A     ((USHORT)364)
#define VERSION_50A     ((USHORT)373)
#define VERSION_53A     ((USHORT)596)
#define ACT_NUM_VERSION VERSION_53A

#define NUMRULE_FILENAME "numrule.cfg"
#define CHAPTER_FILENAME "chapter.cfg"
#define C2S(cChar) String::CreateFromAscii(cChar)
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
    : nVersion(0),
    sFileName( rFileName ),
    bModified( FALSE )
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
        String sNm( URIHelper::SmartRelToAbs( aPathOpt.GetUserConfigPath() ));
        sNm += INET_PATH_TOKEN;
        sNm += sFileName;
        INetURLObject aTempObj(sNm);
        sNm = aTempObj.GetFull();
        SfxMedium aStrm( sNm, STREAM_WRITE | STREAM_TRUNC |
                                        STREAM_SHARE_DENYALL, TRUE );
        Store( *aStrm.GetOutStream() );
    }

    for( USHORT i = 0; i < nMaxRules; ++i )
        delete pNumRules[i];
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/
void  SwBaseNumRules::Init()
{
    for(USHORT i = 0; i < nMaxRules; ++i )
        pNumRules[i] = 0;

    String sNm( sFileName );
    SvtPathOptions aOpt;
    if( aOpt.SearchFile( sNm, SvtPathOptions::PATH_USERCONFIG ))
    {
        SfxMedium aStrm( sNm, STREAM_STD_READ, TRUE );
        Load( *aStrm.GetInStream() );
    }
}

/*-----------------26.06.97 08.30-------------------

--------------------------------------------------*/

void SwBaseNumRules::ApplyNumRules(const SwNumRulesWithName &rCopy, USHORT nIdx)
{
    ASSERT(nIdx < nMaxRules, Array der NumRules ueberindiziert.);
    if( !pNumRules[nIdx] )
        pNumRules[nIdx] = new SwNumRulesWithName( rCopy );
    else
        *pNumRules[nIdx] = rCopy;
}
/*------------------------------------------------------------------------
 Beschreibung:  Zugriff auf eine Regel ueber ihren Namen
 Return:        Pointer auf die Regel oder Nullptr, wenn nicht vorhanden
------------------------------------------------------------------------*/


const SwNumRulesWithName *SwBaseNumRules::GetRules(const String &rName) const
{
    for(USHORT i = 0; i < nMaxRules; ++i)
    {
        SwNumRulesWithName *pRule = pNumRules[i];
        if( pRule && pRule->GetName() == rName)
            return pRule;
    }
    return 0;
}
// PROTECTED METHODES ----------------------------------------------------
/*------------------------------------------------------------------------
 Beschreibung:  Speichern
------------------------------------------------------------------------*/

BOOL /**/ SwBaseNumRules::Store(SvStream &rStream)
{
    rStream << ACT_NUM_VERSION;
        // Schreiben, welche Positionen durch eine Regel belegt sind
        // Anschliessend Schreiben der einzelnen Rules
    for(USHORT i = 0; i < nMaxRules; ++i)
    {
        if(pNumRules[i])
        {
            rStream << (unsigned char) TRUE;
            pNumRules[i]->Store( rStream );
        }
        else
            rStream << (unsigned char) FALSE;
    }
    return TRUE;
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
        DBG_ERROR("Version 364 ist nicht eindeutig #55402#")
    }
    else if( VERSION_30B == nVersion || VERSION_31B == nVersion ||
             ACT_NUM_VERSION >= nVersion )
    {
        unsigned char bRule = FALSE;
        for(USHORT i = 0; i < nMaxRules; ++i)
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
void SwChapterNumRules::ApplyNumRules(const SwNumRulesWithName &rCopy, USHORT nIdx)
{
    bModified = TRUE;
    SwBaseNumRules::ApplyNumRules(rCopy, nIdx);
}

/*------------------------------------------------------------------------*/

SwNumRulesWithName::SwNumRulesWithName(const SwNumRule &rCopy,
                                        const String &rName)
    : aName(rName)
{
    for( int n = 0; n < MAXLEVEL; ++n )
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
SwNumRulesWithName::SwNumRulesWithName( SvStream &rStream, USHORT nVersion )
{
    CharSet eEncoding = gsl_getSystemTextEncoding();
    rStream.ReadByteString(aName, eEncoding);

    char c;
    for(USHORT n = 0; n < MAXLEVEL; ++n )
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
    rChg = SwNumRule( aName );
    rChg.SetAutoRule( FALSE );
    _SwNumFmtGlobal* pFmt;
    for( USHORT n = 0; n < MAXLEVEL; ++n )
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

    for( USHORT n = 0; n < MAXLEVEL; ++n )
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
            while( TRUE )
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
    : aFmt( rFmt.aFmt ), nCharPoolId( rFmt.nCharPoolId ),
    sCharFmtName( rFmt.sCharFmtName )
{
    for( USHORT n = rFmt.aItems.Count(); n; )
        aItems.Insert( rFmt.aItems[ --n ]->Clone(), aItems.Count() );
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

SwNumRulesWithName::_SwNumFmtGlobal::_SwNumFmtGlobal( SvStream& rStream,
                                                        USHORT nVersion )
    : nCharPoolId( USHRT_MAX )
{
    CharSet eEncoding = gsl_getSystemTextEncoding();
    {
        USHORT nUS;
        sal_Char cChar;
        short nShort;
        BOOL bFlag;
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
            rStream >> cChar;       aFmt.SetStart( (USHORT)cChar );

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

        USHORT  nFamily;
        USHORT  nCharSet;
        short   nWidth;
        short   nHeight;
        USHORT  nPitch;
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
                                            aFmt.GetBulletChar(), nCharSet ));
    }

    if( VERSION_30B != nVersion )
    {
        USHORT nItemCount;
        rStream >> nCharPoolId;
        rStream.ReadByteString(sCharFmtName, eEncoding);
        rStream >> nItemCount;

        while( nItemCount-- )
        {
            USHORT nWhich, nVers;
            rStream >> nWhich >> nVers;
            aItems.Insert( GetDfltAttr( nWhich )->Create( rStream, nVers ),
                            aItems.Count() );
        }
    }

    if( VERSION_40A == nVersion && SVX_NUM_BITMAP == aFmt.GetNumberingType() )
    {
        BYTE cF;
        Size aSz;

        rStream >> aSz.Width() >> aSz.Height();

        rStream >> cF;
        if( cF )
        {
            SvxBrushItem* pBrush = 0;
            SwFmtVertOrient* pVOrient = 0;
            USHORT nVer;

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
            SvxFrameVertOrient eOrient = SVX_VERT_NONE;
            if(pVOrient)
                eOrient = (SvxFrameVertOrient)pVOrient->GetVertOrient();
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
        USHORT nFamily = FAMILY_DONTKNOW, nCharSet = 0, nPitch = 0;
        short  nWidth = 0, nHeight = 0;

        const Font* pFnt = aFmt.GetBulletFont();
        if( pFnt )
        {
            aName = pFnt->GetName();
            nFamily = (USHORT)pFnt->GetFamily();
            nCharSet = (USHORT)pFnt->GetCharSet();
            nWidth = (short)pFnt->GetSize().Width();
            nHeight = (short)pFnt->GetSize().Height();
            nPitch = (USHORT)pFnt->GetPitch();
        }

        rStream << USHORT(aFmt.GetNumberingType())
                << aFmt.GetBulletChar()
                << (aFmt.GetIncludeUpperLevels() > 0)
                << aFmt.GetStart();
        rStream.WriteByteString( aFmt.GetPrefix(), eEncoding );
        rStream.WriteByteString( aFmt.GetSuffix(), eEncoding );
        rStream << USHORT( aFmt.GetNumAdjust() )
                << aFmt.GetAbsLSpace()
                << aFmt.GetFirstLineOffset()
                << aFmt.GetCharTextDistance()
                << aFmt.GetLSpace()
                << FALSE;//aFmt.IsRelLSpace();
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

    for( USHORT n = aItems.Count(); n; )
    {
        SfxPoolItem* pItem = aItems[ --n ];
        USHORT nIVers = pItem->GetVersion( SOFFICE_FILEFORMAT_50 );
        ASSERT( nIVers != USHRT_MAX,
                "Was'n das: Item-Version USHRT_MAX in der aktuellen Version" );
        rStream << pItem->Which()
                << nIVers;
        pItem->Store( rStream, nIVers );
    }

    // Erweiterungen fuer Version 40A

    if( SVX_NUM_BITMAP == aFmt.GetNumberingType() )
    {
        rStream << (INT32)aFmt.GetGraphicSize().Width()
                << (INT32)aFmt.GetGraphicSize().Height();
        BYTE cFlg = ( 0 != aFmt.GetBrush() ? 1 : 0 ) +
                    ( 0 != aFmt.GetGraphicOrientation() ? 2 : 0 );
        rStream << cFlg;

        if( aFmt.GetBrush() )
        {
            USHORT nVersion = aFmt.GetBrush()->GetVersion( SOFFICE_FILEFORMAT_50 );
            rStream << nVersion;
            aFmt.GetBrush()->Store( rStream, nVersion );
        }
        if( aFmt.GetGraphicOrientation() )
        {
            USHORT nVersion = aFmt.GetGraphicOrientation()->GetVersion( SOFFICE_FILEFORMAT_50 );
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
        USHORT nArrLen = rSh.GetCharFmtCount();
        for( USHORT i = 1; i < nArrLen; ++i )
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
                pFmt->SetAuto( FALSE );
            }
            else
                pFmt = rSh.GetCharFmtFromPool( nCharPoolId );

            if( !pFmt->GetDepends() )       // Attribute setzen
                for( USHORT n = aItems.Count(); n; )
                    pFmt->SetAttr( *aItems[ --n ] );
        }
    }
    ((SwNumFmt&)aFmt).SetCharFmt( pFmt );
    rNew = aFmt;
    if( pFmt )
        ((SwNumFmt&)aFmt).SetCharFmt( 0 );
}

