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
#ifndef DBAUI_APPICONCONTROL_HXX
#define DBAUI_APPICONCONTROL_HXX

#include <svtools/ivctrl.hxx>
#include <svtools/transfer.hxx>

namespace dbaui
{
    class IControlActionListener;
    class OApplicationIconControl   :public SvtIconChoiceCtrl
                                    ,public DropTargetHelper
    {
        Point                       m_aMousePos;
        IControlActionListener*     m_pActionListener;

    public:
        OApplicationIconControl(Window* _pParent);
        virtual ~OApplicationIconControl();

        void                    setControlActionListener( IControlActionListener* _pListener ) { m_pActionListener = _pListener; }
        IControlActionListener* getControlActionListener( ) const { return m_pActionListener; }

    protected:
        // DropTargetHelper overridables
        virtual sal_Int8    AcceptDrop( const AcceptDropEvent& _rEvt );
        virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& _rEvt );
    };
}
#endif // DBAUI_APPICONCONTROL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
