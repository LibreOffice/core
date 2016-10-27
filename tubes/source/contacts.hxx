/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TUBES_SOURCE_CONTACTS_HXX
#define INCLUDED_TUBES_SOURCE_CONTACTS_HXX

#include <sal/config.h>

#include <tubes/manager.hxx>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/lstbox.hxx>

class Collaboration;

namespace tubes {

class TubeContacts : public ModelessDialog
{
    VclPtr<PushButton>             mpBtnDemo;
    VclPtr<PushButton>             mpBtnBuddy;
    VclPtr<PushButton>             mpBtnGroup;
    VclPtr<PushButton>             mpBtnInvite;
    VclPtr<PushButton>             mpBtnListen;
    VclPtr<ListBox>                mpList;
    Collaboration*          mpCollaboration;

    DECL_LINK( BtnDemoHdl, Button*, void );
    DECL_LINK( BtnConnectHdl, Button*, void );
    DECL_LINK( BtnGroupHdl, Button*, void );
    DECL_LINK( BtnInviteHdl, Button*, void );
    DECL_STATIC_LINK( TubeContacts, BtnListenHdl, Button*, void );

    AccountContactPairV maACs;

    void Invite();

    void StartDemoSession();

    void StartBuddySession();

    void StartGroupSession();

public:
    explicit TubeContacts( Collaboration* pCollaboration );

    virtual ~TubeContacts() override;

    virtual void dispose() override;

    void Populate();
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
