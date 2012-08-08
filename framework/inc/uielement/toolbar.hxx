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

#ifndef __FRAMEWORK_UIELEMENT_TOOLBAR_HXX_
#define __FRAMEWORK_UIELEMENT_TOOLBAR_HXX_

#include <vcl/toolbox.hxx>

namespace framework
{

class ToolBarManager;
class ToolBar : public ToolBox
{
    public:
                        ToolBar( Window* pParent, WinBits nWinBits );
        virtual         ~ToolBar();

        virtual void    Command ( const CommandEvent& rCEvt );
        virtual void    StateChanged( StateChangedType nType );
        virtual void    DataChanged( const DataChangedEvent& rDCEvt );

        // Toolbar manager
        void            SetToolBarManager( ToolBarManager* pTbMgr );
        ToolBarManager* GetToolBarManager() const { return m_pToolBarManager; }

        // Provide additional handlers to support external implementations
        void            SetCommandHdl( const Link& aLink ) { m_aCommandHandler = aLink; }
        const Link&     GetCommandHdl() const { return m_aCommandHandler; }
        void            SetStateChangedHdl( const Link& aLink ) { m_aStateChangedHandler = aLink; }
        const Link&     GetStateChangedHdl() const { return m_aStateChangedHandler; }
        void            SetDataChangedHdl( const Link& aLink ) { m_aDataChangedHandler = aLink; }
        const Link&     GetDataChangedHdl() { return m_aDataChangedHandler; }

    private:
        Link            m_aCommandHandler;
        Link            m_aStateChangedHandler;
        Link            m_aDataChangedHandler;
        ToolBarManager* m_pToolBarManager;
};

}

#endif // __FRAMEWORK_UIELEMENT_TOOLBAR_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
