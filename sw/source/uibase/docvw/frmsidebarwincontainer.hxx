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
#include <memory>
#include <vector>

class SwFrame;
class SwFormatField;
namespace vcl { class Window; }
namespace sw { namespace annotation {
    class SwAnnotationWin;
} }

namespace sw { namespace sidebarwindows {

class FrameSidebarWinContainer;

class SwFrameSidebarWinContainer
{
    public:
        SwFrameSidebarWinContainer();
        ~SwFrameSidebarWinContainer();

        bool insert( const SwFrame& rFrame,
                     const SwFormatField& rFormatField,
                     sw::annotation::SwAnnotationWin& rSidebarWin );

        bool remove( const SwFrame& rFrame,
                     const sw::annotation::SwAnnotationWin& rSidebarWin );

        bool empty( const SwFrame& rFrame );

        sw::annotation::SwAnnotationWin* get( const SwFrame& rFrame,
                           const sal_Int32 nIndex );

        void getAll( const SwFrame& rFrame,
                     std::vector< vcl::Window* >* pSidebarWins );

    private:
        std::unique_ptr<FrameSidebarWinContainer> mpFrameSidebarWinContainer;
};

} } // eof of namespace sw::sidebarwindows::

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
