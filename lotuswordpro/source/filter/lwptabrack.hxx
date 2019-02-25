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
 * Tabrack for LwpTabPiece object.
 ************************************************************************/
#ifndef INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPTABRACK_HXX
#define INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPTABRACK_HXX

#include <lwpobj.hxx>

class   LwpObjectStream;
class   LwpTab;
class   LwpTabRack;

class LwpTab
{
public:
    LwpTab();

    enum TabType
    {
        TT_LEFT = 1,
        TT_CENTER,
        TT_RIGHT,
        TT_NUMERIC
    };

    enum LeaderType
    {
        TL_NONE = 0,
        TL_HYPHEN,
        TL_DOT,
        TL_LINE
    };

    enum RelativeType
    {
        TR_LEFT = 1,
        TR_RIGHT,
        TR_CENTER
    };
public:
    void    Read(LwpObjectStream *pStrm);
    inline sal_uInt32 GetPosition();
    inline TabType GetTabType();
    inline LeaderType GetLeaderType();
    inline sal_uInt16 GetAlignChar();

private:
    sal_uInt32      m_nX;
    sal_uInt8       m_nType;
    sal_uInt8       m_nLeader;
    sal_uInt8       m_nRelativeType;
    sal_uInt16      m_nAlignChar;       //be careful
};

inline sal_uInt32 LwpTab::GetPosition()
{
    return m_nX;
}

inline LwpTab::TabType LwpTab::GetTabType()
{
    return static_cast<LwpTab::TabType>(m_nType);
}

inline LwpTab::LeaderType LwpTab::GetLeaderType()
{
    return static_cast<LwpTab::LeaderType>(m_nLeader);
}

inline sal_uInt16 LwpTab::GetAlignChar()
{
    return m_nAlignChar;
}

class   LwpTabRack : public LwpObject
{
public:
    LwpTabRack(LwpObjectHeader objHdr, LwpSvStream* pStrm);
public:
    void Read() override;
    sal_uInt16 GetNumTabs();
    LwpTab* Lookup(sal_uInt16 nIndex);
    LwpTabRack* GetNext();
private:
    virtual ~LwpTabRack() override {}

    enum{
        MaxTabs = 15
    };
    sal_uInt16      m_nNumTabs;
    LwpTab          m_aTabs[MaxTabs];
    LwpObjectID     m_NextID;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
