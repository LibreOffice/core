/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_UI_INC_SEARCHRESULTS_HXX
#define INCLUDED_SC_SOURCE_UI_INC_SEARCHRESULTS_HXX

#include <vcl/dialog.hxx>
#include <sfx2/childwin.hxx>
#include <svtools/simptabl.hxx>

class ScDocument;
class ScRangeList;
class SvTreeListBox;

namespace sc {

class SearchResultsDlg : public ModelessDialog
{
    VclPtr<SvSimpleTable> mpList;
    VclPtr<FixedText> mpSearchResults;
    OUString const aSkipped;
    SfxBindings* const mpBindings;
    ScDocument* mpDoc;

    DECL_LINK( ListSelectHdl, SvTreeListBox*, void );
public:
    SearchResultsDlg( SfxBindings* _pBindings, vcl::Window* pParent );
    virtual ~SearchResultsDlg() override;
    virtual void dispose() override;

    void FillResults( ScDocument* pDoc, const ScRangeList& rMatchedRanges, bool bCellNotes );

    virtual bool Close() override;
};

class SearchResultsDlgWrapper : public SfxChildWindow
{
public:
    SearchResultsDlgWrapper(
        vcl::Window* _pParent, sal_uInt16 nId, SfxBindings* pBindings, SfxChildWinInfo* pInfo );

    virtual ~SearchResultsDlgWrapper() override;

    SFX_DECL_CHILDWINDOW_WITHID(SearchResultsDlgWrapper);
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
