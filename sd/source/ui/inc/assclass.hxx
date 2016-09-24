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

#ifndef INCLUDED_SD_SOURCE_UI_INC_ASSCLASS_HXX
#define INCLUDED_SD_SOURCE_UI_INC_ASSCLASS_HXX

#include <vector>
#include <memory>

#include "sddllapi.h"

#define MAX_PAGES 10

namespace vcl { class Window; }

class SD_DLLPUBLIC Assistent
{
    /** contains for every page the controls, which have to be
        connected? correctly */
    std::vector<VclPtr<vcl::Window> > maPages[MAX_PAGES];

    /// number of pages
    int mnPages;

    int mnCurrentPage;

    std::unique_ptr<bool[]> mpPageStatus;

public:

    Assistent(int nNoOfPage);

    bool IsEnabled ( int nPage ) const;
    void EnablePage( int nPage );
    void DisablePage( int nPage );

    /// adds a control to the specified page
    bool InsertControl(int nDestPage, vcl::Window* pUsedControl);

    void NextPage();

    void PreviousPage();

    bool GotoPage(const int nPageToGo);

    bool IsLastPage() const;

    bool IsFirstPage() const;

    int  GetCurrentPage() const { return mnCurrentPage;}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
