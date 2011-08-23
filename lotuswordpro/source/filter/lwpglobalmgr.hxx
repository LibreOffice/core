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
 Nov 2005			Created
 ************************************************************************/
#ifndef _LWPGLOBALMGR_HXX_
#define _LWPGLOBALMGR_HXX_

#include <map>
#include "lwpheader.hxx"
#include "lwpsvstream.hxx"
#include "lwpobjfactory.hxx"
#include "lwpbookmarkmgr.hxx"
#include "lwpchangemgr.hxx"
#include "xfilter/xffontfactory.hxx"
#include "xfilter/xfstylemanager.hxx"
#include "lwpdocdata.hxx"
#include "xfilter/xfcolor.hxx"

class LwpGlobalMgr
{
public:
    ~LwpGlobalMgr();
    static LwpGlobalMgr* GetInstance(LwpSvStream* pSvStream=NULL);
    static void DeleteInstance();
    LwpObjectFactory* GetLwpObjFactory(){return m_pObjFactory;}
    LwpBookmarkMgr* GetLwpBookmarkMgr(){return m_pBookmarkMgr;}
    LwpChangeMgr* GetLwpChangeMgr(){return m_pChangeMgr;}
    XFFontFactory* GetXFFontFactory(){return m_pXFFontFactory;}
    XFStyleManager* GetXFStyleManager(){return m_pXFStyleManager;}
    void SetEditorAttrMap(sal_uInt16 nID, LwpEditorAttr* pAttr);
    OUString GetEditorName(sal_uInt8 nID);
    XFColor GetHighLightColor(sal_uInt8 nID);
private:
    LwpGlobalMgr(LwpSvStream* pSvStream);
private:
    static std::map< sal_uInt32,LwpGlobalMgr* > m_ThreadMap;
    LwpObjectFactory* m_pObjFactory;
    LwpBookmarkMgr* m_pBookmarkMgr;
    LwpChangeMgr* m_pChangeMgr;
    XFFontFactory* m_pXFFontFactory;
    XFStyleManager* m_pXFStyleManager;
    std::map< sal_uInt16,LwpEditorAttr* > m_EditorAttrMap;
};

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
