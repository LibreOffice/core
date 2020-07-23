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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_TABLEDESIGNHELPBAR_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_TABLEDESIGNHELPBAR_HXX

#include "IClipBoardTest.hxx"
#include <vcl/InterimItemWindow.hxx>

namespace dbaui
{
    class OTableDesignHelpBar final : public InterimItemWindow, public IClipboardTest
    {
    private:
        std::unique_ptr<weld::TextView> m_xTextWin;

    public:
        OTableDesignHelpBar( vcl::Window* pParent );
        virtual ~OTableDesignHelpBar() override;
        virtual void dispose() override;

        void SetHelpText( const OUString& rText );

        // IClipboardTest
        virtual bool isCutAllowed() override;
        virtual bool isCopyAllowed() override;
        virtual bool isPasteAllowed() override;

        virtual void copy() override;
        virtual void cut() override;
        virtual void paste() override;
    };
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
