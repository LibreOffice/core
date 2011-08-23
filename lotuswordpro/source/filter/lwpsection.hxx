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
/*************************************************************************
 * Change History
 Jan 2005			Created
 ************************************************************************/

#ifndef _LWPSECTION_HXX
#define _LWPSECTION_HXX
#include "lwppagelayout.hxx"

/**
 * @brief		COrderedObject class in .lwp file
 *
 */
class LwpDLNFVList;
class LwpOrderedObject : public LwpDLNFVList
{
public:
    LwpOrderedObject(LwpObjectHeader &objHdr, LwpSvStream* pStrm);
    virtual ~LwpOrderedObject(){};
    void Read();
    LwpObjectID* GetPosition(){ return &m_Para;};
    LwpObjectID* GetListList(){ return &m_ListList;};
protected:
    LwpObjectID m_ListList;//lwplistlist object,
    LwpObjectID m_Para;
};

/**
 * @brief		VO_SECTION object in .lwp file
 *
 */
class LwpColor;
class LwpAtomHolder;
class LwpSection : public LwpOrderedObject
{
public:
    LwpSection(LwpObjectHeader &objHdr, LwpSvStream* pStrm);
    ~LwpSection();
    inline sal_uInt16 GetFlags();
    inline LwpPageLayout* GetPageLayout();
    inline LwpColor* GetColor();
    inline LwpAtomHolder* GetAtomHolder();
    void Parse(IXFStream* pOutputStream);
    OUString GetSectionName(){return m_AtomHolder.str();}

protected:
    void Read();
private:
    sal_uInt16 m_Flags;
    LwpObjectID m_PageLayout;
    LwpColor m_Color;
    LwpAtomHolder m_AtomHolder;

};

/**
 * @descr Get flage value
 *
 */
inline sal_uInt16 LwpSection::GetFlags()
{
    return m_Flags;
}

/**
 * @descr Get page layout pointer
 *
 */
inline LwpPageLayout* LwpSection::GetPageLayout()
{
    return static_cast<LwpPageLayout*>(m_PageLayout.obj());
}

/**
 * @descr get section tab color
 *
 */
inline LwpColor* LwpSection::GetColor()
{
    return &m_Color;
}

/**
 * @descr get section atom holder pointer
 *
 */
inline LwpAtomHolder* LwpSection::GetAtomHolder()
{
    return &m_AtomHolder;
}


class LwpIndexSection : public LwpSection
{
public:
    LwpIndexSection(LwpObjectHeader &objHdr, LwpSvStream* pStrm);
    ~LwpIndexSection();
    sal_Bool IsFormatRunin();
    sal_Bool IsFormatSeparator();
protected:
    void Read();
private:
    enum{
        SEPARATORS = 0x01,
        RUN_IN = 0X02,
        };
    LwpAtomHolder m_TextMarker;
    LwpAtomHolder m_ParentName;
    LwpAtomHolder m_DivisionName;
    LwpAtomHolder m_SectionName;
    sal_uInt16 m_nForm;
    sal_uInt16 m_nFlags;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
