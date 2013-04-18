/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _SFX_CHECKIN_HXX
#define _SFX_CHECKIN_HXX

#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/edit.hxx>

class SfxCheckinDialog : public ModalDialog
{
    private:
        Edit*       m_pCommentED;
        CheckBox*   m_pMajorCB;

        OKButton*   m_pOKBtn;

        DECL_DLLPRIVATE_LINK(OKHdl, void *);

    public:
        SfxCheckinDialog( Window* pParent );

        OUString GetComment( );
        bool IsMajor( );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
