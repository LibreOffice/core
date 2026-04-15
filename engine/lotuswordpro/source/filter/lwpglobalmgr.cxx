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
#include <lwpglobalmgr.hxx>
#include <osl/thread.hxx>
std::map<sal_uInt32, LwpGlobalMgr*> LwpGlobalMgr::m_ThreadMap;
LwpGlobalMgr::LwpGlobalMgr(LwpSvStream* pSvStream)
{
    if (pSvStream)
        m_pObjFactory.reset(new LwpObjectFactory(pSvStream));
    m_pBookmarkMgr.reset(new LwpBookmarkMgr);
    m_pChangeMgr.reset(new LwpChangeMgr);
    m_pXFFontFactory.reset(new XFFontFactory);
    m_pXFStyleManager.reset(new XFStyleManager);
}

LwpGlobalMgr::~LwpGlobalMgr()
{
    m_pObjFactory.reset();
    m_pBookmarkMgr.reset();
    m_pChangeMgr.reset();
    m_pXFFontFactory.reset();
    m_pXFStyleManager.reset();
    m_EditorAttrMap.clear();
}

LwpGlobalMgr* LwpGlobalMgr::GetInstance(LwpSvStream* pSvStream)
{
    sal_uInt32 nThreadID = osl::Thread::getCurrentIdentifier();
    auto iter = m_ThreadMap.find(nThreadID);
    if (iter == m_ThreadMap.end())
    {
        LwpGlobalMgr* pInstance = new LwpGlobalMgr(pSvStream);
        m_ThreadMap[nThreadID] = pInstance;
        return pInstance;
    }
    else
        return iter->second;
}

void LwpGlobalMgr::DeleteInstance()
{
    sal_uInt32 nThreadID = osl::Thread::getCurrentIdentifier();
    auto iter = m_ThreadMap.find(nThreadID);
    if (iter != m_ThreadMap.end())
    {
        delete iter->second;
        iter->second = nullptr;
        m_ThreadMap.erase(iter);
    }
}

void LwpGlobalMgr::SetEditorAttrMap(sal_uInt16 nID, LwpEditorAttr* pAttr)
{
    m_EditorAttrMap[nID].reset(pAttr);
}

OUString LwpGlobalMgr::GetEditorName(sal_uInt8 nID)
{
    auto iter = m_EditorAttrMap.find(nID);
    if (iter != m_EditorAttrMap.end())
        return iter->second->cName.str();
    return OUString();
}

XFColor LwpGlobalMgr::GetHighlightColor(sal_uInt8 nID)
{
    auto iter = m_EditorAttrMap.find(nID);
    if (iter != m_EditorAttrMap.end())
    {
        LwpColor aLwpColor = iter->second->cHiLiteColor;
        XFColor aColor(aLwpColor.GetRed(), aLwpColor.GetGreen(), aLwpColor.GetBlue());
        return aColor;
    }
    else
    {
        XFColor aColor(255, 255, 0); //yellow
        return aColor;
    }
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
