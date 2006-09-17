/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: swafopt.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 04:53:29 $
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

#ifndef _SV_KEYCODES_HXX //autogen
#include <vcl/keycodes.hxx>
#endif
#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#include "swafopt.hxx"

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

SvxSwAutoFmtFlags::SvxSwAutoFmtFlags()
    : aBulletFont( String::CreateFromAscii(
                        RTL_CONSTASCII_STRINGPARAM( "StarSymbol" )),
                    Size( 0, 14 ) )
{
    bReplaceQuote =
    bAutoCorrect =
    bCptlSttSntnc =
    bCptlSttWrd =
    bChkFontAttr =
    bChgUserColl =
    bChgEnumNum =
    bChgFracionSymbol =
    bChgOrdinalNumber =
    bChgToEnEmDash =
    bChgWeightUnderl =
    bSetINetAttr =
    bAFmtDelSpacesAtSttEnd =
    bAFmtDelSpacesBetweenLines =
    bAFmtByInpDelSpacesAtSttEnd =
    bAFmtByInpDelSpacesBetweenLines =
    bDummy = TRUE;

    bReplaceStyles =
    bDelEmptyNode =
    bWithRedlining =
    bAutoCmpltEndless =
    bAutoCmpltAppendBlanc =
    bAutoCmpltShowAsTip = FALSE;

    bSetBorder =
    bCreateTable =
    bSetNumRule =
    bAFmtByInput =
    bRightMargin =
    bAutoCompleteWords =
    bAutoCmpltCollectWords =
    bAutoCmpltKeepList = TRUE;

    bDummy5 = bDummy6 = bDummy7 = bDummy8 =
         FALSE;

    nRightMargin = 50;      // dflt. 50 %
    nAutoCmpltExpandKey = KEY_RETURN;

    aBulletFont.SetCharSet( RTL_TEXTENCODING_SYMBOL );
    aBulletFont.SetFamily( FAMILY_DONTKNOW );
    aBulletFont.SetPitch( PITCH_DONTKNOW );
    aBulletFont.SetWeight( WEIGHT_DONTKNOW );
    aBulletFont.SetTransparent( TRUE );

    cBullet = 0x2022;
    cByInputBullet = cBullet;
    aByInputBulletFont = aBulletFont;

    nAutoCmpltWordLen = 10;
    nAutoCmpltListLen = 500;
    pAutoCmpltList = 0;
}


SvxSwAutoFmtFlags& SvxSwAutoFmtFlags::operator=( const SvxSwAutoFmtFlags& rAFFlags )
{
    bAutoCorrect = rAFFlags.bAutoCorrect;
    bReplaceQuote = rAFFlags.bReplaceQuote;
    bCptlSttSntnc = rAFFlags.bCptlSttSntnc;
    bCptlSttWrd = rAFFlags.bCptlSttWrd;
    bChkFontAttr = rAFFlags.bChkFontAttr;

    bChgUserColl = rAFFlags.bChgUserColl;
    bChgEnumNum = rAFFlags.bChgEnumNum;
    bDelEmptyNode = rAFFlags.bDelEmptyNode;
    bSetNumRule = rAFFlags.bSetNumRule;
    bAFmtByInput = rAFFlags.bAFmtByInput;

    bChgFracionSymbol = rAFFlags.bChgFracionSymbol;
    bChgOrdinalNumber = rAFFlags.bChgOrdinalNumber;
    bChgToEnEmDash = rAFFlags.bChgToEnEmDash;
    bChgWeightUnderl = rAFFlags.bChgWeightUnderl;
    bSetINetAttr = rAFFlags.bSetINetAttr;
    bSetBorder = rAFFlags.bSetBorder;
    bCreateTable = rAFFlags.bCreateTable;
    bReplaceStyles = rAFFlags.bReplaceStyles;
    bAFmtDelSpacesAtSttEnd = rAFFlags.bAFmtDelSpacesAtSttEnd;
    bAFmtDelSpacesBetweenLines = rAFFlags.bAFmtDelSpacesBetweenLines;
    bAFmtByInpDelSpacesAtSttEnd = rAFFlags.bAFmtByInpDelSpacesAtSttEnd;
    bAFmtByInpDelSpacesBetweenLines = rAFFlags.bAFmtByInpDelSpacesBetweenLines;

    bDummy = rAFFlags.bDummy;

    bDummy5 = rAFFlags.bDummy5;
    bDummy6 = rAFFlags.bDummy6;
    bDummy7 = rAFFlags.bDummy7;
    bDummy8 = rAFFlags.bDummy8;

    bWithRedlining = rAFFlags.bWithRedlining;

    bRightMargin = rAFFlags.bRightMargin;
    nRightMargin = rAFFlags.nRightMargin;

    cBullet = rAFFlags.cBullet;
    aBulletFont = rAFFlags.aBulletFont;

    cByInputBullet = rAFFlags.cByInputBullet;
    aByInputBulletFont = rAFFlags.aByInputBulletFont;

    bAutoCompleteWords = rAFFlags.bAutoCompleteWords;
    bAutoCmpltCollectWords = rAFFlags.bAutoCmpltCollectWords;
    bAutoCmpltKeepList = rAFFlags.bAutoCmpltKeepList;
    bAutoCmpltEndless = rAFFlags.bAutoCmpltEndless;
    bAutoCmpltAppendBlanc = rAFFlags.bAutoCmpltAppendBlanc;
    bAutoCmpltShowAsTip = rAFFlags.bAutoCmpltShowAsTip;
    pAutoCmpltList = rAFFlags.pAutoCmpltList;
    nAutoCmpltExpandKey = rAFFlags.nAutoCmpltExpandKey;

    nAutoCmpltWordLen = rAFFlags.nAutoCmpltWordLen;
    nAutoCmpltListLen = rAFFlags.nAutoCmpltListLen;

    return *this;
}

