/*************************************************************************
 *
 *  $RCSfile: swafopt.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:14 $
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

#pragma hdrstop

#ifndef _SV_KEYCODES_HXX //autogen
#include <vcl/keycodes.hxx>
#endif
#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#include "swafopt.hxx"

#if defined UNX

#if defined GCC
extern const sal_Char __FAR_DATA sBulletFntName[];
const sal_Char __FAR_DATA sBulletFntName[] = "starbats";
#else
extern const sal_Char __FAR_DATA sBulletFntName[] = "starbats";
#endif

#else
extern const sal_Char __FAR_DATA sBulletFntName[] = "StarBats";
#endif

// !! JP - 17.04.00: which value becomes this Bulletchar
//                   in the Unicode Version?
const sal_Char cBulletChar  = '\x95';   // character for Aufzaehlungen

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

SvxSwAutoFmtFlags::SvxSwAutoFmtFlags()
    : aBulletFont( String( sBulletFntName, RTL_TEXTENCODING_MS_1252 ),
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
    bAutoCmpltCollectWords = TRUE;

    bDummy4 = bDummy5 = bDummy6 = bDummy7 = bDummy8 =
         FALSE;

    nRightMargin = 50;      // dflt. 50 %
    nAutoCmpltExpandKey = KEY_RETURN;

    aBulletFont.SetCharSet( RTL_TEXTENCODING_SYMBOL );
    aBulletFont.SetFamily( FAMILY_DONTKNOW );
    aBulletFont.SetPitch( PITCH_DONTKNOW );
    aBulletFont.SetWeight( WEIGHT_DONTKNOW );
    aBulletFont.SetTransparent( TRUE );

    cBullet = ByteString::ConvertToUnicode( cBulletChar,
                                            RTL_TEXTENCODING_SYMBOL );
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

    bDummy4 = rAFFlags.bDummy4;
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
    bAutoCmpltEndless = rAFFlags.bAutoCmpltEndless;
    bAutoCmpltAppendBlanc = rAFFlags.bAutoCmpltAppendBlanc;
    bAutoCmpltShowAsTip = rAFFlags.bAutoCmpltShowAsTip;
    pAutoCmpltList = rAFFlags.pAutoCmpltList;
    nAutoCmpltExpandKey = rAFFlags.nAutoCmpltExpandKey;

    nAutoCmpltWordLen = rAFFlags.nAutoCmpltWordLen;
    nAutoCmpltListLen = rAFFlags.nAutoCmpltListLen;

    return *this;
}

