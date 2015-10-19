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
#include "lwpglobalmgr.hxx"
#include "lwpfont.hxx"
#include "xfilter/xfstylemanager.hxx"
#include "xfilter/xffontfactory.hxx"

void LwpFontAttrEntry::Read(LwpObjectStream *pStrm)
{
    m_nAttrBits = pStrm->QuickReaduInt16();
    m_nAttrOverrideBits = pStrm->QuickReaduInt16();
    m_nAttrApplyBits = pStrm->QuickReaduInt16();
    m_nAttrOverrideBits2 = pStrm->QuickReaduInt8();
    m_nAttrApplyBits2 = pStrm->QuickReaduInt8();
    m_nCase = pStrm->QuickReaduInt8();
    m_nUnder = pStrm->QuickReaduInt8();
    pStrm->SkipExtra();
}

#include "xfilter/xfdefs.hxx"
void LwpFontAttrEntry::Override( rtl::Reference<XFFont> const & pFont )
{
    if (IsBoldOverridden())
        pFont->SetBold(Is(BOLD));

    if (IsItalicOverridden())
        pFont->SetItalic(Is(ITALIC));

    if (IsStrikeThruOverridden())
    {
        if(Is(STRIKETHRU))
        {
            pFont->SetCrossout(enumXFCrossoutSignel);
        }
        else
        {
            pFont->SetCrossout(enumXFCrossoutNone);
        }
    }

    if (IsSuperOverridden())
    {
        if(Is(SUPERSCRIPT))
            pFont->SetPosition();
    }

    if (IsSubOverridden())
    {
        if(Is(SUBSCRIPT))
            pFont->SetPosition(false);
    }

    if (IsUnderlineOverridden())
    {
        switch(m_nUnder)
        {
            case UNDER_SINGLE:
                pFont->SetUnderline(enumXFUnderlineSingle);
                break;
            case UNDER_DOUBLE:
                pFont->SetUnderline(enumXFUnderlineDouble);
                break;
            case UNDER_WORD_SINGLE:
                pFont->SetUnderline(enumXFUnderlineSingle, true);
                break;
            case UNDER_WORD_DOUBLE:
                pFont->SetUnderline(enumXFUnderlineSingle, true);
                break;
            case UNDER_DONTCARE:    //fall through
            case UNDER_OFF:         //fall through
            case UNDER_STYLE:       //fall through
            default:
                break;
                //do nothing;
        }
    }

    if (IsCaseOverridden())
    {
        switch(m_nCase)
        {
        case CASE_UPPER:
            pFont->SetTransform(enumXFTransformUpper);
            break;
        case CASE_LOWER:
            pFont->SetTransform(enumXFTransformLower);
            break;
        case CASE_NORMAL:
            pFont->SetTransform(enumXFTransformNone);
            break;
        case CASE_INITCAPS:
            pFont->SetTransform(enumXFTransformCapitalize);
            break;
        case CASE_STYLE:        //fall through
        case CASE_DONTCARE: //fall through
        default:
            //do nothing
            ;
        }
    }

    //Please note that, put the SmallCaps setting after the case setting,
    //for SmallCaps has higher priority than LowerCase but low
    if (IsSmallCapsOverridden())
    {
        if( pFont->GetTransform()!=enumXFTransformUpper )   //SmallCaps should not override upper case
        {
            if(Is(SMALLCAPS))
                pFont->SetTransform(enumXFTransformSmallCaps);
        }
    }

    // TODO: tightness
    //if (IsTightnessOverridden())
    //  pFont->SetTightness(cTightness);*/
}

bool LwpFontAttrEntry::Is(sal_uInt16 Attr)
{
    return (0 != (m_nAttrBits & Attr));
}

bool LwpFontAttrEntry::IsBoldOverridden()
{
    return (0 != (m_nAttrOverrideBits & BOLD));
}

bool LwpFontAttrEntry::IsItalicOverridden()
{
    return (0 != (m_nAttrOverrideBits & ITALIC));
}
bool LwpFontAttrEntry::IsStrikeThruOverridden()
{
    return (0 != (m_nAttrOverrideBits & STRIKETHRU));
}
bool LwpFontAttrEntry::IsSmallCapsOverridden()
{
    return (0 != (m_nAttrOverrideBits & SMALLCAPS));
}
bool LwpFontAttrEntry::IsSuperOverridden()
{
    return (0 != (m_nAttrOverrideBits & SUPERSCRIPT));

}
bool LwpFontAttrEntry::IsSubOverridden()
{
    return (0 != (m_nAttrOverrideBits & SUBSCRIPT));

}

bool LwpFontAttrEntry::IsUnderlineOverridden()
{
    return (0 != (m_nAttrOverrideBits2 & UNDER));
}
bool LwpFontAttrEntry::IsCaseOverridden()
{
    return (0 != (m_nAttrOverrideBits2 & CASE));
}

void LwpFontTableEntry::Read(LwpObjectStream *pStrm)
{
    m_WindowsFaceName.Read(pStrm);

    // use the m_WindowsFaceName to set the m_FaceName temporarily
    m_FaceName = m_WindowsFaceName;

    //Skip the panoseNumber
    //m_PanoseNumber.Read(pStrm);
    LwpPanoseNumber thePanoseToSkip;
    thePanoseToSkip.Read(pStrm);

    pStrm->SkipExtra();

    RegisterFontDecl();
}

OUString LwpFontTableEntry::GetFaceName()
{
    return (m_WindowsFaceName.str());
}

void LwpFontTableEntry::RegisterFontDecl()
{
    if(m_FaceName.str().isEmpty()) return;
    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    XFFontDecl aFontDecl1( m_FaceName.str(), m_FaceName.str(), false );
    pXFStyleManager->AddFontDecl(aFontDecl1);
}

LwpFontTable::LwpFontTable()
    : m_nCount(0)
    , m_pFontEntries(NULL)
{}

void LwpFontTable::Read(LwpObjectStream *pStrm)
{
    m_pFontEntries = NULL;
    m_nCount = pStrm->QuickReaduInt16();
    if(m_nCount>0)
    {
        m_pFontEntries = new LwpFontTableEntry[m_nCount];
        for(sal_uInt16 i=0; i<m_nCount; i++)
        {
            m_pFontEntries[i].Read(pStrm);
        }
    }
    pStrm->SkipExtra();
}

OUString LwpFontTable::GetFaceName(sal_uInt16 index) //index: start from 1
{
    assert(index <= m_nCount && index > 0);
    return (index <= m_nCount && index > 0) ? m_pFontEntries[index-1].GetFaceName() : OUString();
}

LwpFontTable::~LwpFontTable()
{
    if(m_pFontEntries)
    {
        delete [] m_pFontEntries;
        m_pFontEntries = NULL;
    }
}

void LwpFontNameEntry::Read(LwpObjectStream *pStrm)
{
    //Read CFontDescriptionOverrideBase
    m_nOverrideBits = pStrm->QuickReaduInt8();
    m_nApplyBits = pStrm->QuickReaduInt8();
    m_nPointSize = pStrm->QuickReaduInt32();
    m_nOverstrike = pStrm->QuickReaduInt16();
    m_nTightness = pStrm->QuickReaduInt16();
    m_Color.Read(pStrm);
    m_BackColor.Read(pStrm);
    pStrm->SkipExtra();

    //Read data of LwpFontNameEntry
    m_nFaceName = pStrm->QuickReaduInt16();
    m_nAltFaceName = pStrm->QuickReaduInt16();
    pStrm->SkipExtra();
}
#include "xfilter/xfcolor.hxx"
void LwpFontNameEntry::Override(rtl::Reference<XFFont> const & pFont)
{
    if (IsPointSizeOverridden())
        pFont->SetFontSize(static_cast<sal_uInt8>(m_nPointSize/65536L));

    if (IsColorOverridden() && m_Color.IsValidColor())
    {
        XFColor aColor(m_Color.To24Color());
        pFont->SetColor(aColor);
    }

    if (IsBackgroundColorOverridden() )
    {
        if (m_BackColor.IsValidColor())
        {
            XFColor aColor(m_BackColor.To24Color());
            pFont->SetBackColor( aColor );
        }
        else if (m_BackColor.IsTransparent())
        {
            pFont->SetBackColorTransparent();
        }
    }

    // TODO: tightness
    //if (IsTightnessOverridden())
    //  pFont->SetTightness(cTightness);
}

bool LwpFontNameEntry::IsFaceNameOverridden()
{
    return (0 != (m_nOverrideBits & FACENAME));
}

bool LwpFontNameEntry::IsAltFaceNameOverridden()
{
    return (0 != (m_nOverrideBits & ALTFACENAME));
}

bool LwpFontNameEntry::IsPointSizeOverridden()
{
    return (0 != (m_nOverrideBits & POINTSIZE));
}

bool LwpFontNameEntry::IsColorOverridden()
{
    return (0 != (m_nOverrideBits & COLOR));
}

bool LwpFontNameEntry::IsBackgroundColorOverridden()
{
    return (0 != (m_nOverrideBits & BKCOLOR));
}

//TODO
//sal_Bool LwpFontNameEntry::IsTightnessOverridden()
//{
//    return (0 != (m_nOverrideBits & TIGHTNESS));
//}

//sal_Bool LwpFontNameEntry::IsAnythingOverridden()
//{
//    return (0 != (m_nOverrideBits & ALL_BITS));
//}

LwpFontNameManager::LwpFontNameManager()
    : m_nCount(0)
    , m_pFontNames(NULL)
{}

LwpFontNameManager::~LwpFontNameManager()
{
    if(m_pFontNames)
    {
        delete [] m_pFontNames;
        m_pFontNames = NULL;
    }
}

void LwpFontNameManager::Read(LwpObjectStream *pStrm)
{
    m_nCount = pStrm->QuickReaduInt16();
    if(m_nCount>0)
    {
        m_pFontNames = new LwpFontNameEntry[m_nCount];
        for(sal_uInt16 i=0; i<m_nCount; i++)
        {
            m_pFontNames[i].Read(pStrm);
        }
    }
    m_FontTbl.Read(pStrm);
    pStrm->SkipExtra();
}

void    LwpFontNameManager::Override(sal_uInt16 index, rtl::Reference<XFFont> const & pFont)
    //index: start from 1
{
    if (index > m_nCount || index < 1)
        return ;

    m_pFontNames[index-1].Override(pFont);
    if(m_pFontNames[index-1].IsFaceNameOverridden())
        pFont->SetFontName(m_FontTbl.GetFaceName(m_pFontNames[index-1].GetFaceID()));
    if(m_pFontNames[index-1].IsAltFaceNameOverridden())
        pFont->SetFontNameAsia(m_FontTbl.GetFaceName(m_pFontNames[index-1].GetAltFaceID()));
}
OUString LwpFontNameManager::GetNameByIndex(sal_uInt16 index)
{
    sal_uInt16 nameindex = m_pFontNames[index-1].GetFaceID();
    return (m_FontTbl.GetFaceName(nameindex));
}

void LwpFontAttrManager::Read(LwpObjectStream *pStrm)
{
    m_nCount = pStrm->QuickReaduInt16();
    m_pFontAttrs = new LwpFontAttrEntry[m_nCount];

    for(sal_uInt16 i=0; i<m_nCount; i++)
    {
        m_pFontAttrs[i].Read(pStrm);
    }
    pStrm->SkipExtra();
}

void    LwpFontAttrManager::Override(sal_uInt16 index, rtl::Reference<XFFont> const & pFont)
    //index: start from 1
{
    if (index > m_nCount || index < 1)
        return ;

    m_pFontAttrs[index-1].Override(pFont);
}

LwpFontAttrManager::~LwpFontAttrManager()
{
    if(m_pFontAttrs)
        delete []m_pFontAttrs;
}

void LwpFontManager::Read(LwpObjectStream *pStrm)
{
    m_FNMgr.Read(pStrm);
    m_AttrMgr.Read(pStrm);
    pStrm->SkipExtra();

}

#include "xfilter/xftextstyle.hxx"

/*
VO_PARASTYLE call this method to add its style to XFStyleManager based on the fontID
1. Construct the text style based on the fontID
2. Add the style to XFStyleManager, and return the <office:styles> style name
3. Add it to LwpParaStyleMap.
Note: A temporary method for only font support phase. The next AddStyle should be used later.
// To be replaced by LwpStyleManager::AddStyle() and the following CreateFont()
*/
/*void LwpFontManager::AddStyle(LwpObjectID styleObjID, sal_uInt32 fontID, OUString styleName)
{
    XFTextStyle* pStyle = new XFTextStyle();        //to be deleted by XFStyleManager
    AddStyle(styleObjID, fontID, styleName, pStyle);
}*/

/*
VO_PARASTYLE/VO_CHARACTERSTYLE call this method to add its style to XFStyleManager based on the fontID
1. Construct the text style based on the fontID
2. Add the style to XFStyleManager, and return the <office:styles> style name
3. Add it to LwpParaStyleMap.
Prerequisite: pStyle has been created and the paragraph properties has been set to it.
//To be replaced by LwpStyleManager::AddStyle() and the following CreateFont()
*/
/*void LwpFontManager::AddStyle(LwpObjectID styleObjID, sal_uInt32 fontID, OUString styleName, XFTextStyle* pStyle)
{
    assert(pStyle);
    XFFont* pFont = CreateFont(fontID);
    pStyle->SetFont(pFont);
    pStyle->SetStyleName(styleName);
    XFStyleManager::AddStyle(pStyle);
    m_StyleList.insert(LwpParaStyleMap::value_type(styleObjID, styleName));
}*/

/*
Create XFFont based on the fotID
*/
rtl::Reference<XFFont> LwpFontManager::CreateFont(sal_uInt32 fontID)
{
    rtl::Reference<XFFont> pFont = new XFFont();
    m_FNMgr.Override(GetFontNameIndex(fontID), pFont);
    m_AttrMgr.Override(GetFontAttrIndex(fontID), pFont);
    return pFont;
}

/*
Called XFFont based on the override result of two font ids.
Refer to CFontManager::OverrideID
*/
//OUString LwpFontManager::GetOverrideStyle(sal_uInt32 fontID, sal_uInt32 overID)
rtl::Reference<XFFont> LwpFontManager::CreateOverrideFont(sal_uInt32 fontID, sal_uInt32 overID)
{
    rtl::Reference<XFFont> pFont = new XFFont();   //To be deleted by XFFontFactory
    if(fontID)
    {
        Override(fontID, pFont);
    }
    if(overID)
    {
        Override(overID, pFont);
    }
    return pFont;
}

void LwpFontManager::Override(sal_uInt32 fontID, rtl::Reference<XFFont> const & pFont)
{
    m_FNMgr.Override(GetFontNameIndex(fontID), pFont);
    m_AttrMgr.Override(GetFontAttrIndex(fontID), pFont);
}

OUString LwpFontManager::GetNameByID(sal_uInt32 fontID)
{
    return ( m_FNMgr.GetNameByIndex(GetFontNameIndex(fontID)) );//use font id for bullet?
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
