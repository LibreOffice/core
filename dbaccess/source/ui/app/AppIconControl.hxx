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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_APP_APPICONCONTROL_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_APP_APPICONCONTROL_HXX

#include <svtools/ivctrl.hxx>
#include <vcl/transfer.hxx>

namespace dbaui
{
    class IControlActionListener;
    class OApplicationIconControl   :public SvtIconChoiceCtrl
                                    ,public DropTargetHelper
    {
        IControlActionListener*     m_pActionListener;

    public:
        explicit OApplicationIconControl(vcl::Window* _pParent);
        virtual ~OApplicationIconControl() override;
        virtual void dispose() override;

        void                    setControlActionListener( IControlActionListener* _pListener ) { m_pActionListener = _pListener; }

    protected:
        // DropTargetHelper overridables
        virtual sal_Int8    AcceptDrop( const AcceptDropEvent& _rEvt ) override;
        virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& _rEvt ) override;
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_UI_APP_APPICONCONTROL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
