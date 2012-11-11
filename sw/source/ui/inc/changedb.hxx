/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _CHANGEDB_HXX
#define _CHANGEDB_HXX


#include <vcl/bitmap.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <svtools/stdctrl.hxx>
#include <svtools/treelistbox.hxx>
#include <svx/stddlg.hxx>
#include "dbtree.hxx"

class SwFldMgr;
class SwView;
class SwWrtShell;
struct SwDBData;

/*--------------------------------------------------------------------
     Description: exchange database at fields
 --------------------------------------------------------------------*/
class SwChangeDBDlg: public SvxStandardDialog
{
    FixedLine       aDBListFL;
    FixedText       aUsedDBFT;
    FixedText       aAvailDBFT;
    SvTreeListBox   aUsedDBTLB;
    SwDBTreeList    aAvailDBTLB;
    PushButton      aAddDBPB;
    FixedInfo       aDescFT;
    FixedText       aDocDBTextFT;
    FixedText       aDocDBNameFT;
    OKButton        aOKBT;
    CancelButton    aCancelBT;
    HelpButton      aHelpBT;

    ImageList       aImageList;

    SwWrtShell      *pSh;
    SwFldMgr        *pMgr;

    DECL_LINK(TreeSelectHdl, void * = 0);
    DECL_LINK(ButtonHdl, void *);
    DECL_LINK(AddDBHdl, void *);

    virtual void    Apply();
    void            UpdateFlds();
    void            FillDBPopup();
    SvTreeListEntry*    Insert(const String& rDBName);
    void            ShowDBName(const SwDBData& rDBData);

public:
    SwChangeDBDlg(SwView& rVw);
    ~SwChangeDBDlg();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
