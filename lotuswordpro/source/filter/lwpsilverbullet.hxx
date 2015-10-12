/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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
 *  The Initial Developer of the Original Code is: IBM Corporation
 *
 *  Copyright: 2008 by IBM Corporation
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
/*************************************************************************
 * @file
 *  For LWP filter architecture prototype
 ************************************************************************/

#ifndef INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPSILVERBULLET_HXX
#define INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPSILVERBULLET_HXX

#include "lwpheader.hxx"
#include "lwpobj.hxx"
#include "lwpdlvlist.hxx"
#include "lwpobjid.hxx"
#include "lwpatomholder.hxx"
#include "unicode/utypes.h"
const sal_uInt16 MAXNUMBERPOSITIONS = 10; //max number of positions
const sal_uInt16 NUMCHAR_none = 0x00;   //none of numberchar
const sal_uInt16 NUMCHAR_1 = 0x01;      //index for numberchar "1"
const sal_uInt16 NUMCHAR_A = 0x02;      //index for numberchar "A"
const sal_uInt16 NUMCHAR_a = 0x03;      //index for numberchar "a"
const sal_uInt16 NUMCHAR_I = 0x04;      //index for numberchar "I"
const sal_uInt16 NUMCHAR_i = 0x05;      //index for numberchar "i"
const sal_uInt16 NUMCHAR_other= 0x06;   //index for numberchar "other"
const sal_uInt16 NUMCHAR_01 = 0x0B;     //index for numberchar "01"

const sal_uInt16 NUMCHAR_Chinese1 = 0x09;       //index for numberchar "Ò¼"
const sal_uInt16 NUMCHAR_Chinese2 = 0x08;       //index for numberchar "Ò»"
const sal_uInt16 NUMCHAR_Chinese3= 0x0A;    //index for numberchar "¼×"
const sal_uInt16 NUMCHAR_Chinese4 = 0x13;       //index for numberchar "01" in chinese version

class LwpAtomHolder;
class LwpPara;
class LwpFribParaNumber;

class LwpSilverBullet : public LwpDLNFVList
{
public:
    LwpSilverBullet(LwpObjectHeader& objHdr, LwpSvStream* pStrm);

    virtual ~LwpSilverBullet();

    virtual void RegisterStyle() override;

    void Read() override;

    bool IsBulletOrdered();

    OUString GetBulletFontName();

    inline OUString GetBulletStyleName() const;

    OUString GetBulletChar();

    static OUString GetPrefix() { return OUString(); }

    static OUString GetSuffix() { return OUString(); }

    ;

    ;

    bool HasName();

    static OUString GetNumCharByStyleID(LwpFribParaNumber* pParaNumber);

    ;
    inline bool IsLesserLevel(sal_uInt16 nPos);
    ;

    LwpPara* GetBulletPara();

    sal_uInt16 GetDisplayLevel(sal_uInt8 nPos);

    OUString GetAdditionalName(sal_uInt8 nPos);

    OUString GetDivisionName();

    OUString GetSectionName();

private:
    sal_uInt16      m_nFlags;
    LwpObjectID     m_aStory;
    sal_uInt8       m_pResetPositionFlags[MAXNUMBERPOSITIONS];
    sal_uInt32      m_nUseCount;
    LwpAtomHolder*  m_pAtomHolder;

    LwpPara* m_pBulletPara;
    OUString m_strStyleName;
    sal_uInt16 m_pHideLevels[10];

private:
    enum    // For m_pResetPositionFlags
    {
        LESSERLEVEL     = 0x01,
        LESSERSPECIFIC  = 0x02,
        NEWDIVISION     = 0x04,
        NEWSECTION      = 0x08,
        CUMULATIVE      = 0x10
    };
};
inline OUString LwpSilverBullet::GetBulletStyleName() const
{
    return m_strStyleName;
}
inline bool LwpSilverBullet::IsLesserLevel(sal_uInt16 nPos)
{
    return ((m_pResetPositionFlags[nPos] & LESSERLEVEL) != 0);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
