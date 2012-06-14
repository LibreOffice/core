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
#ifndef DBAUI_TABLEWINDOWTITLE_HXX
#define DBAUI_TABLEWINDOWTITLE_HXX

#include <vcl/fixed.hxx>

namespace dbaui
{
    class OTableWindow;
    class OTableWindowTitle : public FixedText
    {
        OTableWindow* m_pTabWin;

    protected:
        virtual void Command(const CommandEvent& rEvt);
        //  virtual void Paint( const Rectangle& rRect );
        virtual void MouseButtonDown( const MouseEvent& rEvt );
        virtual void KeyInput( const KeyEvent& rEvt );
        virtual void DataChanged( const DataChangedEvent& rDCEvt );

    public:
        OTableWindowTitle( OTableWindow* pParent );
        virtual ~OTableWindowTitle();
        virtual void LoseFocus();
        virtual void GetFocus();
        virtual void RequestHelp( const HelpEvent& rHEvt );
        // window override
        virtual void StateChanged( StateChangedType nStateChange );
    };
}
#endif // DBAUI_TABLEWINDOWTITLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
