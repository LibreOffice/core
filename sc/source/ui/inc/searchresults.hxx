/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_UI_SEARCHRESULTS_HXX
#define SC_UI_SEARCHRESULTS_HXX

#include <vcl/dialog.hxx>
#include <sfx2/childwin.hxx>

class ScDocument;
class ScRangeList;
class SvSimpleTable;

namespace sc {

class SearchResultsDlg : public ModelessDialog
{
    SvSimpleTable *mpList;
    SfxBindings* mpBindings;
    sal_uInt16 mnId;

    ScDocument* mpDoc;

    DECL_LINK( ListSelectHdl, void * );
public:
    SearchResultsDlg( SfxBindings* _pBindings, Window* pParent, sal_uInt16 nId );
    virtual ~SearchResultsDlg();

    void FillResults( ScDocument* pDoc, const ScRangeList& rMatchedRanges );

    virtual sal_Bool Close();
};

class SearchResultsDlgWrapper : public SfxChildWindow
{
public:
    SearchResultsDlgWrapper(
        Window* _pParent, sal_uInt16 nId, SfxBindings* pBindings, SfxChildWinInfo* pInfo );

    virtual ~SearchResultsDlgWrapper();

    SFX_DECL_CHILDWINDOW_WITHID(SearchResultsDlgWrapper);
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
