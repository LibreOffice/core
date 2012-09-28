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
 *  Font manager related classes header file
 *  LwpFontTableEntry, LwpFontTable, LwpFontNameEntry,
 * LwpFontAttrEntry, LwpFontNameManager,LwpFontAttrManager,
 * LwpFontManager
 ************************************************************************/
/*************************************************************************
 * Change History
 Jan 2005           Created
 ************************************************************************/

#ifndef _LWPFONT_HXX
#define _LWPFONT_HXX

#include "lwpheader.hxx"
#include "lwpobjstrm.hxx"
#include "lwpatomholder.hxx"
#include "lwpcolor.hxx"
#include "lwpbasetype.hxx"
#include "lwpobj.hxx"
#include "xfilter/xffont.hxx"
#include "xfilter/xftextstyle.hxx"

class LwpFontTableEntry
{
public:
    LwpFontTableEntry(){}
    ~LwpFontTableEntry(){}
public:
    void Read(LwpObjectStream *pStrm);
    OUString GetFaceName();
private:
    LwpAtomHolder m_WindowsFaceName;    //font face name under windows
    LwpAtomHolder m_FaceName;           //font face name
    void RegisterFontDecl();
     //Not useful now, so skip
    //LwpPanoseNumber m_PanoseNumber;
};

class LwpFontTable
{
public:
    LwpFontTable();
    ~LwpFontTable();
public:
    void Read(LwpObjectStream *pStrm);
    OUString GetFaceName(sal_uInt16 index); //index: start from 1
//  void RegisterFontDecls();
private:
    sal_uInt16 m_nCount;
    LwpFontTableEntry* m_pFontEntries;
};


class LwpFontNameEntry
{
public:
    LwpFontNameEntry(){}
    ~LwpFontNameEntry(){}
public:
    void Read(LwpObjectStream *pStrm);
    inline sal_uInt16 GetFaceID(){return m_nFaceName;}
    inline sal_uInt16 GetAltFaceID(){return m_nAltFaceName;}
    void Override(XFFont* pFont);
    inline sal_Bool IsFaceNameOverridden();
    inline sal_Bool IsAltFaceNameOverridden();
private:
    //Data of CFontDescriptionOverrideBase
    sal_uInt8 m_nOverrideBits;
    sal_uInt8 m_nApplyBits;
    sal_uInt32 m_nPointSize;
    sal_uInt16 m_nOverstrike;
    sal_uInt16 m_nTightness;
    LwpColor m_Color;
    LwpColor m_BackColor;
    //Data of LwpFontNameEntry
    sal_uInt16 m_nFaceName; //CFontTableID
    sal_uInt16 m_nAltFaceName; //CFontTableID
    enum
    {
        POINTSIZE       = 0x01,
        COLOR           = 0x02,
        OVERSTRIKE      = 0x04,
        TIGHTNESS       = 0x08,
        FACENAME        = 0x10,
        BKCOLOR         = 0x20,
        ALTFACENAME     = 0x40,
        ALL_BITS        = (POINTSIZE | COLOR | OVERSTRIKE
                            | ALTFACENAME
                            | TIGHTNESS | FACENAME | BKCOLOR)
    };

    inline sal_Bool IsPointSizeOverridden();
    inline sal_Bool IsColorOverridden();
    inline sal_Bool IsBackgroundColorOverridden();
//TODO
//    inline sal_Bool IsTightnessOverridden();
//    inline sal_Bool IsAnythingOverridden();
};

class LwpFontNameManager
{
public:
    LwpFontNameManager();
    ~LwpFontNameManager();
    OUString GetNameByIndex(sal_uInt16 index);
private:
    sal_uInt16 m_nCount;
    LwpFontNameEntry* m_pFontNames;
    LwpFontTable m_FontTbl;

public:
    void Read(LwpObjectStream *pStrm);
    void    Override(sal_uInt16 index, XFFont* pFont);
};

class LwpFontAttrEntry
{
public:
    LwpFontAttrEntry(){}
    ~LwpFontAttrEntry(){}
public:
    void Read(LwpObjectStream *pStrm);
    void Override(XFFont*pFont);
private:
    sal_uInt16 m_nAttrBits;
    sal_uInt16 m_nAttrOverrideBits;
    sal_uInt16 m_nAttrApplyBits;
    sal_uInt8 m_nAttrOverrideBits2;
    sal_uInt8 m_nAttrApplyBits2;
    sal_uInt8 m_nCase;
    sal_uInt8 m_nUnder;
    enum
    {
        BOLD            = 0x0001,
        ITALIC          = 0x0002,
        STRIKETHRU      = 0x0004,

        SUPERSCRIPT = 0x0100,
        SUBSCRIPT       = 0x0200,

        SMALLCAPS       = 0x0400,

        ALL_ATTRS       = BOLD | ITALIC | STRIKETHRU
                        | SUPERSCRIPT | SUBSCRIPT
                        | SMALLCAPS,

        CASE_DONTCARE   = 0,
        CASE_NORMAL = 1,
        CASE_UPPER      = 2,
        CASE_LOWER  = 3,
        CASE_INITCAPS   = 4,
        CASE_STYLE      = 7,

        UNDER_DONTCARE  = 0,
        UNDER_OFF           = 1,
        UNDER_SINGLE        = 2,
        UNDER_DOUBLE        = 3,
        UNDER_WORD_SINGLE   = 4,
        UNDER_WORD_DOUBLE   = 5,
        UNDER_STYLE         = 7,

        CASE    = 0x01,
        UNDER   = 0x02,
        ALL_ATTRS2          = CASE | UNDER
    };
    sal_Bool Is(sal_uInt16 Attr);
    sal_Bool IsBoldOverridden();
    sal_Bool IsItalicOverridden();
    sal_Bool IsStrikeThruOverridden();
    sal_Bool IsSmallCapsOverridden();
    sal_Bool IsSuperOverridden();
    sal_Bool IsSubOverridden();
    sal_Bool IsUnderlineOverridden();
    sal_Bool IsCaseOverridden();

};

class LwpFontAttrManager
{
public:
    LwpFontAttrManager():m_pFontAttrs(NULL){}
    ~LwpFontAttrManager();
public:
    void Read(LwpObjectStream *pStrm);
    void Override(sal_uInt16 index, XFFont*pFont);
private:
    sal_uInt16 m_nCount;
    LwpFontAttrEntry* m_pFontAttrs;
};

class LwpFontManager
{
public:
    LwpFontManager(){}
    ~LwpFontManager(){}
private:
    LwpFontNameManager m_FNMgr;
    LwpFontAttrManager m_AttrMgr;

public:
    void Read(LwpObjectStream *pStrm);
    XFFont* CreateOverrideFont(sal_uInt32 fontID, sal_uInt32 overID);
    XFFont*  CreateFont(sal_uInt32 fontID);
    OUString GetNameByID(sal_uInt32 fontID);

private:
    void Override(sal_uInt32 fontID, XFFont* pFont);
    inline sal_uInt16 GetFontNameIndex(sal_uInt32 fontID);
    inline sal_uInt16 GetFontAttrIndex(sal_uInt32 fontID);
};

sal_uInt16 LwpFontManager::GetFontNameIndex(sal_uInt32 fontID)
{
    return static_cast<sal_uInt16>((fontID>>16)&0xFFFF);
}

sal_uInt16 LwpFontManager::GetFontAttrIndex(sal_uInt32 fontID)
{
    return static_cast<sal_uInt16>(fontID);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
