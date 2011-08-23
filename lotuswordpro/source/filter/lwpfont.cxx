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
    pStrm->QuickRead(&m_nAttrBits, sizeof(m_nAttrBits));
    pStrm->QuickRead(&m_nAttrOverrideBits, sizeof(m_nAttrOverrideBits));
    pStrm->QuickRead(&m_nAttrApplyBits, sizeof(m_nAttrApplyBits));
    pStrm->QuickRead(&m_nAttrOverrideBits2, sizeof(m_nAttrOverrideBits2));
    pStrm->QuickRead(&m_nAttrApplyBits2, sizeof(m_nAttrApplyBits2));
    pStrm->QuickRead(&m_nCase, sizeof(m_nCase));
    pStrm->QuickRead(&m_nUnder, sizeof(m_nUnder));
    pStrm->SkipExtra();
}

#include "xfilter/xfdefs.hxx"
void LwpFontAttrEntry::Override( XFFont*pFont )
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
            pFont->SetPosition(TRUE);
    }

    if (IsSubOverridden())
    {
        if(Is(SUBSCRIPT))
            pFont->SetPosition(FALSE);
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
            case UNDER_DONTCARE:	//fall through
            case UNDER_OFF:			//fall through
            case UNDER_STYLE:		//fall through
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
        case CASE_STYLE:		//fall through
        case CASE_DONTCARE:	//fall through
        default:
            //do nothing
            ;
        }
    }

    //Please note that, put the SmallCaps setting after the case setting,
    //for SmallCaps has higher priority than LowerCase but low
    if (IsSmallCapsOverridden())
    {
        if( pFont->GetTransform()!=enumXFTransformUpper )	//SmallCaps should not override upper case
        {
            if(Is(SMALLCAPS))
                pFont->SetTransform(enumXFTransformSmallCaps);
        }
    }

    // TODO: tightness
    //if (IsTightnessOverridden())
    //	pFont->SetTightness(cTightness);*/
}

BOOL LwpFontAttrEntry::Is(sal_uInt16 Attr)
{
    return (0 != (m_nAttrBits & Attr));
}

BOOL LwpFontAttrEntry::IsBoldOverridden()
{
    return (0 != (m_nAttrOverrideBits & BOLD));
}

BOOL LwpFontAttrEntry::IsItalicOverridden()
{
    return (0 != (m_nAttrOverrideBits & ITALIC));
}
BOOL LwpFontAttrEntry::IsStrikeThruOverridden()
{
    return (0 != (m_nAttrOverrideBits & STRIKETHRU));
}
BOOL LwpFontAttrEntry::IsSmallCapsOverridden()
{
    return (0 != (m_nAttrOverrideBits & SMALLCAPS));
}
BOOL LwpFontAttrEntry::IsSuperOverridden()
{
    return (0 != (m_nAttrOverrideBits & SUPERSCRIPT));

}
BOOL LwpFontAttrEntry::IsSubOverridden()
{
    return (0 != (m_nAttrOverrideBits & SUBSCRIPT));

}

BOOL LwpFontAttrEntry::IsSuperSubOverridden()
{
    return (0 != (m_nAttrOverrideBits
        & (SUPERSCRIPT | SUBSCRIPT)));
}

BOOL LwpFontAttrEntry::IsUnderlineOverridden()
{
    return (0 != (m_nAttrOverrideBits2 & UNDER));
}
BOOL LwpFontAttrEntry::IsCaseOverridden()
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
    if(m_FaceName.str().getLength()==0) return;
    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    XFFontDecl aFontDecl1( m_FaceName.str(), m_FaceName.str(), false );
    pXFStyleManager->AddFontDecl(aFontDecl1);
}

LwpFontTable::LwpFontTable()
    : m_pFontEntries(NULL)
{}

void LwpFontTable::Read(LwpObjectStream *pStrm)
{
    m_pFontEntries = NULL;
    pStrm->QuickRead(&m_nCount, sizeof(m_nCount));
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
    assert(index>0);
    if (index < 1)//add by , for fix crash,2005/12/20
        return OUString::createFromAscii("");//add end
    return m_pFontEntries[index-1].GetFaceName();
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
    pStrm->QuickRead(&m_nOverrideBits, sizeof(m_nOverrideBits));
    pStrm->QuickRead(&m_nApplyBits, sizeof(m_nApplyBits));
    pStrm->QuickRead(&m_nPointSize, sizeof(m_nPointSize));
    pStrm->QuickRead(&m_nOverstrike, sizeof(m_nOverstrike));
    pStrm->QuickRead(&m_nTightness, sizeof(m_nTightness));
    m_Color.Read(pStrm);
    m_BackColor.Read(pStrm);
    pStrm->SkipExtra();

    //Read data of LwpFontNameEntry
    pStrm->QuickRead(&m_nFaceName, sizeof(m_nFaceName));
    pStrm->QuickRead(&m_nAltFaceName, sizeof(m_nAltFaceName));
    pStrm->SkipExtra();
}
#include "xfilter/xfcolor.hxx"
void LwpFontNameEntry::Override(XFFont* pFont)
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
    //	pFont->SetTightness(cTightness);
}

BOOL LwpFontNameEntry::IsFaceNameOverridden()
{
    return (0 != (m_nOverrideBits & FACENAME));
}

BOOL LwpFontNameEntry::IsAltFaceNameOverridden()
{
    return (0 != (m_nOverrideBits & ALTFACENAME));
}

BOOL LwpFontNameEntry::IsPointSizeOverridden()
{
    return (0 != (m_nOverrideBits & POINTSIZE));
}

BOOL LwpFontNameEntry::IsColorOverridden()
{
    return (0 != (m_nOverrideBits & COLOR));
}

BOOL LwpFontNameEntry::IsBackgroundColorOverridden()
{
    return (0 != (m_nOverrideBits & BKCOLOR));
}

BOOL LwpFontNameEntry::IsTightnessOverridden()
{
    return (0 != (m_nOverrideBits & TIGHTNESS));
}

BOOL LwpFontNameEntry::IsAnythingOverridden()
{
    return (0 != (m_nOverrideBits & ALL_BITS));
}


LwpFontNameManager::LwpFontNameManager()
    :m_pFontNames(NULL)
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
    pStrm->QuickRead(&m_nCount, sizeof(m_nCount));
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

void	LwpFontNameManager::Override(sal_uInt16 index, XFFont* pFont)
    //index: start from 1
{
    if(index<1) return;
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

/*void LwpFontNameManager::Override(sal_uInt16 baseID, sal_uInt16 overID, XFFont* pFont)
{
    //Override
    m_pFontNames[overID-1]. Override(&m_pFontNames[baseID-1], pFont);
}

void LwpFontAttrManager::Override(sal_uInt16 baseID, sal_uInt16 overID, XFFont*pFont)
{
    m_pFontAttrs[overID-1]. Override(&m_pFontAttrs[baseID-1], pFont);
}*/

void LwpFontAttrManager::Read(LwpObjectStream *pStrm) {
    pStrm->QuickRead(&m_nCount, sizeof(m_nCount));
    m_pFontAttrs = new LwpFontAttrEntry[m_nCount];

    for(sal_uInt16 i=0; i<m_nCount; i++)
    {
        m_pFontAttrs[i].Read(pStrm);
    }
    pStrm->SkipExtra();
}

void	LwpFontAttrManager::Override(sal_uInt16 index, XFFont* pFont)
    //index: start from 1
{
    if(index<1) return;
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
    XFTextStyle* pStyle = new XFTextStyle();		//to be deleted by XFStyleManager
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
XFFont* LwpFontManager::CreateFont(sal_uInt32 fontID)
{
    XFFont* pFont = new XFFont();
    SetDfltFont(pFont);
    m_FNMgr.Override(GetFontNameIndex(fontID), pFont);
    m_AttrMgr.Override(GetFontAttrIndex(fontID), pFont);
    return pFont;
}

/*
Called XFFont based on the override result of two font ids.
Refer to CFontManager::OverrideID
*/
//OUString LwpFontManager::GetOverrideStyle(sal_uInt32 fontID, sal_uInt32 overID)
XFFont* LwpFontManager::CreateOverrideFont(sal_uInt32 fontID, sal_uInt32 overID)
{
    XFFont* pFont = new XFFont();	//To be deleted by XFFontFactory
    SetDfltFont(pFont);
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

void LwpFontManager::Override(sal_uInt32 fontID, XFFont* pFont)
{
    m_FNMgr.Override(GetFontNameIndex(fontID), pFont);
    m_AttrMgr.Override(GetFontAttrIndex(fontID), pFont);
}


/*
Called by VO_PARA to get style name based on the Para Style ID
1) Get style from LwpParaStyleMap based on the LwpObjectID of VO_PARASTYLE.
2) Prerequisite: VO_PARASTYLE should call AddStyle first.
3) Return empty string if no style found.
//To be replaced by OUString LwpStyleManager::GetStyle(const LwpObjectID &styleID)
*/
/*OUString LwpFontManager::GetParaStyle(const LwpObjectID &styleID)
{
    LwpParaStyleMap::const_iterator it =  m_StyleList.find(styleID);
    if (it != m_StyleList.end()) {
        return((*it).second);
    }
    return OUString();
}*/

void LwpFontManager::SetDfltFont(XFFont* /*pFont*/)
{
    // TODO:  Set default XFFont values in lwp filter
    //color:black, background color: transparent, Orientation: left to right, top to bottom, others: 0
    //Refer to CFontDescription::CFontDescription(void)
    //Did not find anything special
    //pFont->SetColor(XFColor(0,0,0));
}

OUString LwpFontManager::GetNameByID(sal_uInt32 fontID)
{
    return ( m_FNMgr.GetNameByIndex(GetFontNameIndex(fontID)) );//use font id for bullet? note by ,2005/7/5
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
