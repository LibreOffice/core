/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <rtl/ustring.hxx>
#include <o3tl/sorted_vector.hxx>
#include <tools/fontenum.hxx>

// A description of an imported font as LibreOffice sees it
// e.g. "Name SemiBold"
struct OfficeFontInfo
{
    sal_Int32 nUniqueIdent;
    OUString sFontName;
    FontWeight eFontWeight;
};

struct OfficeFontInfoCmp
{
    bool operator()(const OfficeFontInfo& rA, const OfficeFontInfo& rB) const
    {
        return rA.nUniqueIdent < rB.nUniqueIdent;
    }

    bool operator()(int nAUniqueIdent, const OfficeFontInfo& rB) const
    {
        return nAUniqueIdent < rB.nUniqueIdent;
    }

    bool operator()(const OfficeFontInfo& rA, int nBUniqueIdent) const
    {
        return rA.nUniqueIdent < nBUniqueIdent;
    }
};

typedef o3tl::sorted_vector<OfficeFontInfo, OfficeFontInfoCmp> ImportedFontMap;

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
