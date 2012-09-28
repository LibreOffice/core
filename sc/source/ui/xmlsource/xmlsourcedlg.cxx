/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xmlsourcedlg.hxx"
#include "xmlsourcedlg.hrc"

#include "scresid.hxx"

ScXMLSourceTree::ScXMLSourceTree(Window* pParent, const ResId& rResId) :
    SvTreeListBox(pParent, rResId),
    maImgElemDefault(ScResId(IMG_ELEMENT_DEFAULT)),
    maImgElemRepeat(ScResId(IMG_ELEMENT_REPEAT))
{
    SetDefaultExpandedEntryBmp(maImgElemDefault);
    SetDefaultCollapsedEntryBmp(maImgElemDefault);

    SvLBoxEntry* p = InsertEntry(OUString("Test1"));
    p = InsertEntry(OUString("Test2"));
    SvLBoxEntry* pPar = p;
    p = InsertEntry(OUString("Test3"), p);
    Expand(pPar);
    pPar = p;
    p = InsertEntry(OUString("Test4"), p);
    Expand(pPar);
}

ScXMLSourceDlg::ScXMLSourceDlg(Window* pParent) :
    ModalDialog(pParent, ScResId(RID_SCDLG_XML_SOURCE)),
    maFlSourceFile(this, ScResId(FL_SOURCE_FILE)),
    maBtnSelectSource(this, ScResId(BTN_SELECT_SOURCE_FILE)),
    maFtSourceFile(this, ScResId(FT_SOURCE_FILE)),
    maFtMapXmlDoc(this, ScResId(FL_MAP_XML_TO_DOCUMENT)),
    maLbTree(this, ScResId(LB_SOURCE_TREE)),
    maBtnCancel(this, ScResId(BTN_CANCEL)),
    maImgFileOpen(ScResId(IMG_FILE_OPEN))
{
    maBtnSelectSource.SetModeImage(maImgFileOpen);
    FreeResource();
}

ScXMLSourceDlg::~ScXMLSourceDlg()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
