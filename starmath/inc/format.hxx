/*************************************************************************
 *
 *  $RCSfile: format.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:57:24 $
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
#ifndef FORMAT_HXX
#define FORMAT_HXX


#ifndef _SFXSMPLHINT_HXX //autogen
#include <svtools/smplhint.hxx>
#endif
#ifndef _SFXBRDCST_HXX //autogen
#include <svtools/brdcst.hxx>
#endif

#ifndef UTILITY_HXX
#include "utility.hxx"
#endif


#define SM_FMT_VERSION_51   ((BYTE) 0x01)
#define SM_FMT_VERSION_NOW  SM_FMT_VERSION_51

#ifndef MAC
#define FNTNAME_TIMES   "Times New Roman"
#else
#define FNTNAME_TIMES   "Times"
#endif
#define FNTNAME_HELV    "Helvetica"
#define FNTNAME_COUR    "Courier"
#define FNTNAME_MATH    "StarMath"


// symbolic names used as array indices
#define SIZ_BEGIN       0
#define SIZ_TEXT        0
#define SIZ_INDEX       1
#define SIZ_FUNCTION    2
#define SIZ_OPERATOR    3
#define SIZ_LIMITS      4
#define SIZ_END         4

// symbolic names used as array indices
#define FNT_BEGIN       0
#define FNT_VARIABLE    0
#define FNT_FUNCTION    1
#define FNT_NUMBER      2
#define FNT_TEXT        3
#define FNT_SERIF       4
#define FNT_SANS        5
#define FNT_FIXED       6
#define FNT_MATH        7
#define FNT_END         7

// symbolic names used as array indices
#define DIS_BEGIN                0
#define DIS_HORIZONTAL           0
#define DIS_VERTICAL             1
#define DIS_ROOT                 2
#define DIS_SUPERSCRIPT          3
#define DIS_SUBSCRIPT            4
#define DIS_NUMERATOR            5
#define DIS_DENOMINATOR          6
#define DIS_FRACTION             7
#define DIS_STROKEWIDTH          8
#define DIS_UPPERLIMIT           9
#define DIS_LOWERLIMIT          10
#define DIS_BRACKETSIZE         11
#define DIS_BRACKETSPACE        12
#define DIS_MATRIXROW           13
#define DIS_MATRIXCOL           14
#define DIS_ORNAMENTSIZE        15
#define DIS_ORNAMENTSPACE       16
#define DIS_OPERATORSIZE        17
#define DIS_OPERATORSPACE       18
#define DIS_LEFTSPACE           19
#define DIS_RIGHTSPACE          20
#define DIS_TOPSPACE            21
#define DIS_BOTTOMSPACE         22
#define DIS_NORMALBRACKETSIZE   23
#define DIS_END                 23


// to be broadcastet on format changes:
#define HINT_FORMATCHANGED  10003

enum SmHorAlign { AlignLeft, AlignCenter, AlignRight };

class SmFormat : public SfxBroadcaster
{
    SmFace      vFont[FNT_END + 1];
    Size        aBaseSize;
    long        nVersion;
    USHORT      vSize[SIZ_END + 1];
    USHORT      vDist[DIS_END + 1];
    SmHorAlign  eHorAlign;
    BOOL        bIsTextmode,
                bScaleNormalBrackets;

public:
    SmFormat();
    SmFormat(const SmFormat &rFormat) { *this = rFormat; }

    const Size &    GetBaseSize() const             { return aBaseSize; }
    void            SetBaseSize(const Size &rSize)  { aBaseSize = rSize; }

    const SmFace &  GetFont(USHORT nIdent) const { return vFont[nIdent]; }
    SmFace &        Font   (USHORT nIdent)       { return vFont[nIdent]; }
    void            SetFont(USHORT nIdent, const SmFace &rFont) { vFont[nIdent] = rFont; }

    USHORT          GetRelSize(USHORT nIdent) const         { return vSize[nIdent]; }
    void            SetRelSize(USHORT nIdent, USHORT nVal)  { vSize[nIdent] = nVal;}

    USHORT          GetDistance(USHORT nIdent) const            { return vDist[nIdent]; }
    void            SetDistance(USHORT nIdent, USHORT nVal) { vDist[nIdent] = nVal; }

    SmHorAlign      GetHorAlign() const             { return eHorAlign; }
    void            SetHorAlign(SmHorAlign eAlign)  { eHorAlign = eAlign; }

    BOOL            IsTextmode() const     { return bIsTextmode; }
    void            SetTextmode(BOOL bVal) { bIsTextmode = bVal; }

    BOOL            IsScaleNormalBrackets() const     { return bScaleNormalBrackets; }
    void            SetScaleNormalBrackets(BOOL bVal) { bScaleNormalBrackets = bVal; }

    long            GetVersion() const { return nVersion; }

    //! at time (5.1) use only the lower byte!!!
    void            SetVersion(long nVer) { nVersion = nVer; }

    SmFormat &      operator = (const SmFormat &rFormat);

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

#endif

