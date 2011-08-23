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
#ifndef _LWPBULLETSTYLEMGR_HXX
#define _LWPBULLETSTYLEMGR_HXX

#include "lwpheader.hxx"
#include "lwpfoundry.hxx"
#include "lwpobjid.hxx"
#include "lwpnumberingoverride.hxx"
#include <utility>
#include "xfilter/xfcontentcontainer.hxx"

class XFListStyle;
class XFList;
class LwpPara;
class LwpSilverBullet;
class XFStyleManager;
class LwpIndentOverride;
class LwpBulletOverride;
class LwpFribParaNumber;

class LwpBulletStyleMgr
{
public:
    LwpBulletStyleMgr();
    virtual ~LwpBulletStyleMgr();
    rtl::OUString RegisterBulletStyle(LwpPara* pPara, LwpBulletOverride* pBullOver,
        LwpIndentOverride* pIndent);
//	rtl::OUString RegisterBulletStyle(const rtl::OUString& rContent, sal_uInt32 nFontID, LwpIndentOverride* pIndent);
//	rtl::OUString AddStyleToList(LwpSilverBullet* pSilverBullet, sal_uInt16 nLevel);
    inline void SetFoundry(LwpFoundry* pFoundry);
    inline void SetContinueFlag(sal_Bool bFlag);
    void OutputBulletListHeader(IXFStream* pOutputStream, sal_Bool bIsOrdered,
        const rtl::OUString& rStyleName, sal_Int16 nLevel, sal_Bool bIsBulletSkiped);
    //Added by Helen
    XFContentContainer* AddBulletList(XFContentContainer* pCont, sal_Bool bIsOrdered,
        const rtl::OUString& rStyleName, sal_Int16 nLevel, sal_Bool bIsBulletSkiped);
    //End of Add
    void OutputBulletListTail(IXFStream* pOutputStream, sal_uInt16 nLevel);
    inline void SetCurrentPos(sal_uInt16 nNewPos);
    inline void SetCurrentSilverBullet(const LwpObjectID& rNewID);
    inline LwpObjectID GetCurrentSilverBullet();
    inline sal_uInt16 GetCurrentPos() const;
    inline void SetCurrentNumOver(const LwpNumberingOverride& rOther);
    inline LwpNumberingOverride* GetCurrentNumOver();

private:
//	void CreateNewListStyle(XFListStyle*& pListStyle, XFStyleManager* pXFStyleMgr);
    rtl::OUString GetDivisionName();
    rtl::OUString GetSectionName(LwpPara* pPara);

private:
//	std::vector <XFListStyle*> m_aBulletStyleList;
    typedef std::pair<LwpBulletOverride, LwpObjectID> OverridePair;
    std::vector <rtl::OUString> m_vStyleNameList;
    std::vector <OverridePair> m_vIDsPairList;
    rtl::OUString m_aCurrentStyleName;
    LwpFoundry* m_pFoundry;
    XFList* m_pBulletList;
//	UChar32 m_nCurrentChar;
//	rtl::OUString m_strCurrentFontName;
//	rtl::OUString m_strCurrentNumberingName;
    sal_Bool m_bContinue;
    sal_Bool m_bIsBulletSkipped;
    LwpObjectID m_aCurrentNumberingID;
    LwpNumberingOverride m_aCurrentNumOverride;
    sal_uInt16 m_nCurrentPos;

};

inline void LwpBulletStyleMgr::SetFoundry(LwpFoundry* pFoundry)
{
    m_pFoundry = pFoundry;
}
inline void LwpBulletStyleMgr::SetContinueFlag(sal_Bool bFlag)
{
    m_bContinue = bFlag;
}

inline void LwpBulletStyleMgr::SetCurrentPos(sal_uInt16 nNewPos)
{
    m_nCurrentPos = nNewPos;
}
inline void LwpBulletStyleMgr::SetCurrentSilverBullet(const LwpObjectID& rNewID)
{
    m_aCurrentNumberingID = rNewID;
}

inline LwpObjectID LwpBulletStyleMgr::GetCurrentSilverBullet()
{
    return m_aCurrentNumberingID;
}

inline sal_uInt16 LwpBulletStyleMgr::GetCurrentPos() const
{
    return m_nCurrentPos;
}

inline void LwpBulletStyleMgr::SetCurrentNumOver(const LwpNumberingOverride& rOther)
{
    m_aCurrentNumOverride = rOther;
}
inline LwpNumberingOverride* LwpBulletStyleMgr::GetCurrentNumOver()
{
    return &m_aCurrentNumOverride;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
