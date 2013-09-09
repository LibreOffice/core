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
class ScDocument;
class ScRangeList;
class SvSimpleTable;

class SearchResults : public ModelessDialog
{
    ScDocument *mpDoc;
    SvSimpleTable *mpList;
    DECL_LINK( ListSelectHdl, void * );
public:
    SearchResults(ScDocument *);
    virtual ~SearchResults();
    void Show(const ScRangeList &);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
