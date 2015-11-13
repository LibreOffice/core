/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_QA_SORT_HELPER_QAHELPER_HXX
#define INCLUDED_SC_QA_SORT_HELPER_QAHELPER_HXX

// Unfortunately requires linkage to sc/ internals so
// can't live in qahelper itself.
#include "inputopt.hxx"

/**
 * Temporarily set the sorting type.
 */
class SortTypeSetter {
    bool mbSortRefUpdate;
public:
    explicit SortTypeSetter(bool bSortRefUpdate)
    {
        mbSortRefUpdate = changeTo(bSortRefUpdate);
    }
    static bool changeTo(bool bSortRefUpdate)
    {
        ScInputOptions aInputOptions = SC_MOD()->GetInputOptions();
        bool bRet = aInputOptions.GetSortRefUpdate();
        aInputOptions.SetSortRefUpdate(bSortRefUpdate);
        SC_MOD()->SetInputOptions(aInputOptions);
        return bRet;
    }
    virtual ~SortTypeSetter()
    {
        changeTo(mbSortRefUpdate);
    }
};

class SortRefNoUpdateSetter : private SortTypeSetter
{
public:
    SortRefNoUpdateSetter() : SortTypeSetter(false) {}
};

class SortRefUpdateSetter : private SortTypeSetter
{
public:
    SortRefUpdateSetter() : SortTypeSetter(true) {}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
