/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __SC_XMLSOURCEDLG_HXX__
#define __SC_XMLSOURCEDLG_HXX__

#include "vcl/button.hxx"
#include "vcl/dialog.hxx"
#include "vcl/fixed.hxx"
#include "svtools/svtreebx.hxx"

#include "expftext.hxx"

#include <boost/scoped_ptr.hpp>

class ScDocument;

class ScXMLSourceTree : public SvTreeListBox
{
public:
    ScXMLSourceTree(Window* pParent, const ResId& rResId);
};

class ScXMLSourceDlg : public ModalDialog
{
    FixedLine maFlSourceFile;
    ImageButton maBtnSelectSource;
    ScExpandedFixedText maFtSourceFile;

    FixedLine maFtMapXmlDoc;

    ScXMLSourceTree maLbTree;

    CancelButton maBtnCancel;

    Image maImgFileOpen;
    Image maImgElemDefault;
    Image maImgElemRepeat;

    ScDocument* mpDoc;

public:
    ScXMLSourceDlg(Window* pParent, ScDocument* pDoc);
    virtual ~ScXMLSourceDlg();

private:

    void SelectSourceFile();
    void LoadSourceFileStructure(const OUString& rPath);

    DECL_LINK(BtnPressedHdl, Button*);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
