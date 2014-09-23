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
#ifndef INCLUDED_SW_SOURCE_UIBASE_DOCVW_FRMSIDEBARWINCONTAINER_HXX
#define INCLUDED_SW_SOURCE_UIBASE_DOCVW_FRMSIDEBARWINCONTAINER_HXX

#include <sal/types.h>
#include <vector>

class SwFrm;
class SwFmtFld;
namespace vcl { class Window; }

namespace sw { namespace sidebarwindows {

class SwSidebarWin;
class FrmSidebarWinContainer;

class SwFrmSidebarWinContainer
{
    public:
        SwFrmSidebarWinContainer();
        ~SwFrmSidebarWinContainer();

        bool insert( const SwFrm& rFrm,
                     const SwFmtFld& rFmtFld,
                     SwSidebarWin& rSidebarWin );

        bool remove( const SwFrm& rFrm,
                     const SwSidebarWin& rSidebarWin );

        bool empty( const SwFrm& rFrm );

        SwSidebarWin* get( const SwFrm& rFrm,
                           const sal_Int32 nIndex );

        void getAll( const SwFrm& rFrm,
                     std::vector< vcl::Window* >* pSidebarWins );

    private:
        FrmSidebarWinContainer* mpFrmSidebarWinContainer;
};

} } // eof of namespace sw::sidebarwindows::

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
