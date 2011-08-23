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

#ifndef _LWPDIVINFO_HXX
#define _LWPDIVINFO_HXX

#include "lwpobj.hxx"
#include "lwpobjid.hxx"
#include "lwpatomholder.hxx"
#include "lwpcolor.hxx"
#include "lwpheader.hxx"
#include "lwpdoc.hxx"

#define STR_OleDivisionClassName	"OLEDivision"


class LwpDivInfo : public LwpObject
{
public:
    LwpDivInfo(LwpObjectHeader& objHdr, LwpSvStream* pStrm);
    ~LwpDivInfo();
    LwpObjectID* GetInitialLayoutID(){ return &m_InitialLayoutID;}
    LwpObjectID* GetFillerPageTextID(){ return &m_FillerPageTextID;}
    // add by  ,03/14/2004
    rtl::OUString GetDivName() { return m_Name.str(); }
    // end add

    rtl::OUString GetClassName() { return m_ClassName.str(); }
    inline sal_Bool HasContents();
    inline sal_Bool IsOleDivision();
    inline sal_Bool IsScrollable();
    inline sal_Bool IsGotoable();
    inline LwpDocument* GetDivision();
    void GetNumberOfPages(sal_uInt16& nPageno);
    sal_uInt16 GetMaxNumberOfPages();
    LwpAtomHolder* GetExternalName(){return &m_ExternalName;};
protected:
    void Read();
private:
    LwpObjectID m_ParentID;
    LwpAtomHolder m_Name;
    LwpObjectID m_LayoutID;
    sal_uInt16 m_nFlags;
    LwpAtomHolder m_ExternalName;
    LwpAtomHolder m_ExternalType;
    LwpAtomHolder m_ClassName;
    LwpObjectID m_InitialLayoutID;
    //pCDLVOListHeadHolder cPresInfo;

    sal_uInt16 m_nPageNoStyle;
    LwpColor m_TabColor;
    LwpObjectID m_FillerPageTextID;

    enum
    {
        DI_SCROLLABLE = 0x01,
        DI_HASCONTENTS = 0x02,
        DI_EXPANDED = 0x04,
        DI_SHOWSTAB = 0x08,

        DI_MUDPUPPY = 0x10,
        DI_NOTDELETABLE = 0x20,
        DI_BEGINGROUP = 0x40,
        DI_GROUP = 0x80,

        DI_ENDGROUP = 0x100,
        DI_SUPPRESS_FILLER_PAGE_HEADERS = 0x200,
        DI_EXTERNALFILE = 0x400,
        DI_FIXED = 0x800,

        DI_KNOWIFANYOLEDDELINKS = 0x1000,
        DI_ANYOLEDDELINKS = 0x2000
    };

    void SkipFront();

};

inline sal_Bool LwpDivInfo::HasContents()
{
    return (m_nFlags & DI_HASCONTENTS) ? sal_True : sal_False;
}

inline sal_Bool LwpDivInfo::IsOleDivision()
{
    OUString strClassName = GetClassName();
    if (strClassName.equals(A2OUSTR(STR_OleDivisionClassName)))
        return sal_True;
    return sal_False;
}

inline sal_Bool LwpDivInfo::IsScrollable()
{
    return (m_nFlags & DI_SCROLLABLE) ? sal_True : sal_False;
}

inline sal_Bool LwpDivInfo::IsGotoable()
{
    return HasContents() && (IsScrollable() || IsOleDivision());
}

inline LwpDocument* LwpDivInfo::GetDivision()
{
    return static_cast<LwpDocument*>(m_ParentID.obj());
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
