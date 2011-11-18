/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 *  Copyright (C) 2011 Markus Mohrhard <markus.mohrhard@googlemail.com> (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */
#include <svtools/headbar.hxx>
#include <svtools/svtabbx.hxx>

struct ScRangeNameLine
{
    rtl::OUString aName;
    rtl::OUString aExpression;
    rtl::OUString aScope;
};

//Implements the table for the manage names dialog
//TODO: cache the lines for performance improvements
//otherwise handling of a large set of range names might get extremely slow
//Need some sort of a filter to handle several range names
SC_DLLPUBLIC class ScRangeManagerTable : public SvTabListBox
{
private:
    HeaderBar maHeaderBar;
    ScRangeName* mpGlobalRangeName;
    std::map<rtl::OUString, ScRangeName*> maTabRangeNames;
    rtl::OUString maGlobalString;
public:
    ScRangeManagerTable( Window* pParent, ScRangeName* pGlobalRangeName, std::map<rtl::OUString, ScRangeName*> aTabRangeNames );
    ~ScRangeManagerTable() {};

    void addEntry( const ScRangeNameLine& rLine );

    void GetCurrentLine(ScRangeNameLine& rLine);
    void UpdateEntries();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
