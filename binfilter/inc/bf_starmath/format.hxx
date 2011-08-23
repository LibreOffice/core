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
#ifndef FORMAT_HXX
#define FORMAT_HXX


#ifndef _SFXSMPLHINT_HXX //autogen
#include <bf_svtools/smplhint.hxx>
#endif
#ifndef _SFXBRDCST_HXX //autogen
#include <bf_svtools/brdcst.hxx>
#endif

#ifndef UTILITY_HXX
#include "utility.hxx"
#endif
#ifndef TYPES_HXX
#include <types.hxx>
#endif
namespace binfilter {


#define SM_FMT_VERSION_51	((BYTE) 0x01)
#define SM_FMT_VERSION_NOW	SM_FMT_VERSION_51

#define FNTNAME_TIMES	"Times"
#define FNTNAME_HELV	"Helvetica"
#define FNTNAME_COUR	"Courier"
#define FNTNAME_MATH    FONTNAME_MATH


// symbolic names used as array indices
#define SIZ_BEGIN		0
#define SIZ_TEXT		0
#define SIZ_INDEX		1
#define SIZ_FUNCTION	2
#define SIZ_OPERATOR	3
#define SIZ_LIMITS		4
#define SIZ_END			4

// symbolic names used as array indices
#define FNT_BEGIN		0
#define FNT_VARIABLE	0
#define FNT_FUNCTION	1
#define FNT_NUMBER		2
#define FNT_TEXT		3
#define FNT_SERIF		4
#define FNT_SANS		5
#define FNT_FIXED		6
#define FNT_MATH		7
#define FNT_END			7

// symbolic names used as array indices
#define DIS_BEGIN				 0
#define DIS_HORIZONTAL			 0
#define DIS_VERTICAL			 1
#define DIS_ROOT				 2
#define DIS_SUPERSCRIPT 		 3
#define DIS_SUBSCRIPT			 4
#define DIS_NUMERATOR			 5
#define DIS_DENOMINATOR 		 6
#define DIS_FRACTION			 7
#define DIS_STROKEWIDTH 		 8
#define DIS_UPPERLIMIT			 9
#define DIS_LOWERLIMIT			10
#define DIS_BRACKETSIZE 		11
#define DIS_BRACKETSPACE		12
#define DIS_MATRIXROW			13
#define DIS_MATRIXCOL			14
#define DIS_ORNAMENTSIZE		15
#define DIS_ORNAMENTSPACE		16
#define DIS_OPERATORSIZE		17
#define DIS_OPERATORSPACE		18
#define DIS_LEFTSPACE			19
#define DIS_RIGHTSPACE			20
#define DIS_TOPSPACE			21
#define DIS_BOTTOMSPACE			22
#define DIS_NORMALBRACKETSIZE	23
#define DIS_END					23


// to be broadcastet on format changes:
#define HINT_FORMATCHANGED	10003

enum SmHorAlign	{ AlignLeft, AlignCenter, AlignRight };

String GetDefaultFontName( LanguageType nLang, USHORT nIdent );

class SmFormat : public SfxBroadcaster
{
    SmFace		vFont[FNT_END + 1];
    BOOL        bDefaultFont[FNT_END + 1];
    Size		aBaseSize;
    long		nVersion;
    USHORT		vSize[SIZ_END + 1];
    USHORT		vDist[DIS_END + 1];
    SmHorAlign	eHorAlign;
    BOOL		bIsTextmode,
                bScaleNormalBrackets;

public:
    SmFormat();
    SmFormat(const SmFormat &rFormat) { *this = rFormat; }

    const Size & 	GetBaseSize() const 			{ return aBaseSize; }
    void			SetBaseSize(const Size &rSize)	{ aBaseSize = rSize; }

    const SmFace &  GetFont(USHORT nIdent) const { return vFont[nIdent]; }
    void            SetFont(USHORT nIdent, const SmFace &rFont, BOOL bDefault = FALSE);
    void            SetFontSize(USHORT nIdent, const Size &rSize)   { vFont[nIdent].SetSize( rSize ); }

    void            SetDefaultFont(USHORT nIdent, BOOL bVal)    { bDefaultFont[nIdent] = bVal; }
    BOOL            IsDefaultFont(USHORT nIdent) const   { return bDefaultFont[nIdent]; }

    USHORT			GetRelSize(USHORT nIdent) const			{ return vSize[nIdent]; }
    void			SetRelSize(USHORT nIdent, USHORT nVal)	{ vSize[nIdent] = nVal;}

    USHORT			GetDistance(USHORT nIdent) const			{ return vDist[nIdent]; }
    void			SetDistance(USHORT nIdent, USHORT nVal)	{ vDist[nIdent] = nVal; }

    SmHorAlign		GetHorAlign() const				{ return eHorAlign; }
    void			SetHorAlign(SmHorAlign eAlign)	{ eHorAlign = eAlign; }

    BOOL			IsTextmode() const     { return bIsTextmode; }
    void			SetTextmode(BOOL bVal) { bIsTextmode = bVal; }

    BOOL			IsScaleNormalBrackets() const     { return bScaleNormalBrackets; }
    void			SetScaleNormalBrackets(BOOL bVal) { bScaleNormalBrackets = bVal; }

    long			GetVersion() const { return nVersion; }

    //! at time (5.1) use only the lower byte!!!
    void			SetVersion(long nVer) { nVersion = nVer; }

    SmFormat & 		operator = (const SmFormat &rFormat);

     BOOL            operator == (const SmFormat &rFormat) const;
     inline BOOL     operator != (const SmFormat &rFormat) const;

    void RequestApplyChanges() const
    {
        ((SmFormat *) this)->Broadcast(SfxSimpleHint(HINT_FORMATCHANGED));
    }

    // functions for compatibility with older versions
    void ReadSM20Format(SvStream &rStream);
    void From300To304a();

    friend SvStream & operator << (SvStream &rStream, const SmFormat &rFormat);
    friend SvStream & operator >> (SvStream &rStream, SmFormat &rFormat);
};
    
inline BOOL    SmFormat::operator != (const SmFormat &rFormat) const
{
    return !(*this == rFormat);
}

} //namespace binfilter
#endif

