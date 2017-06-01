/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_ACCESSIBILITY_INC_HELPER_ACCRESMGR_HXX
#define INCLUDED_ACCESSIBILITY_INC_HELPER_ACCRESMGR_HXX

#include <rtl/ustring.hxx>

class SimpleResMgr;

#define TK_RES_STRING(id) ::accessibility::TkResMgr::loadString(id)


// TkResMgr

namespace accessibility
{

class TkResMgr
{
    static std::locale* m_pImpl;

private:
    // no instantiation allowed
    TkResMgr() = delete;
    ~TkResMgr() { }

    // we'll instantiate one static member of the following class,
    // which in its dtor ensures that m_pImpl will be deleted
    class EnsureDelete
    {
    public:
        EnsureDelete() { }
        ~EnsureDelete();
    };
    friend class EnsureDelete;

protected:
    static void ensureImplExists();

public:
    // loads the string with the specified resource id
    static OUString loadString(const char *pResId);
};

}

#endif // INCLUDED_ACCESSIBILITY_INC_HELPER_ACCRESMGR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
