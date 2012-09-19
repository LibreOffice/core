/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _SD_REMOTEDIALOG_HXX_
#define _SD_REMOTEDIALOG_HXX_

#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/field.hxx>
#include <svtools/svtreebx.hxx>

#include "RemoteDialogClientBox.hxx"

namespace sd
{

class RemoteDialog : public ModalDialog
{
private:
    OKButton        mButtonConnect;
    CancelButton    mButtonCancel;
    ClientBox       mClientBox;
    // Whether discoverability was enabled befor the dialog started.
    bool            mPreviouslyDiscoverable;

    DECL_DLLPRIVATE_LINK( HandleConnectButton, void * );
    DECL_LINK( CloseHdl, void * );
public:
    RemoteDialog( Window* pWindow );
    ~RemoteDialog();

};

}

#endif // _SD_REMOTEDIALOG_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
