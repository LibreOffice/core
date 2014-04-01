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
#ifndef _NEWERVERSIONWARNING_HXX
#define _NEWERVERSIONWARNING_HXX

#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/dialog.hxx>

#define RET_ASK_LATER   short( 100 )

namespace uui
{
    class NewerVersionWarningDialog : public ModalDialog
    {
    private:
        FixedImage      m_aImage;
        FixedText       m_aInfoText;
        FixedLine       m_aButtonLine;
        PushButton      m_aUpdateBtn;
        CancelButton    m_aLaterBtn;

        OUString m_sVersion;

        DECL_LINK(UpdateHdl, void *);
        DECL_LINK(LaterHdl, void *);

        void            InitButtonWidth();

    public:
        NewerVersionWarningDialog( Window* pParent, const OUString& rVersion, ResMgr& rResMgr );
        virtual ~NewerVersionWarningDialog();
    };
} // namespace uui

#endif // #ifndef _NEWERVERSIONWARNING_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
