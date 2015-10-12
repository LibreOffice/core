/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_SFX2_CHECKIN_HXX
#define INCLUDED_SFX2_CHECKIN_HXX

#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/edit.hxx>

class SfxCheckinDialog : public ModalDialog
{
    private:
        VclPtr<Edit>       m_pCommentED;
        VclPtr<CheckBox>   m_pMajorCB;

        VclPtr<OKButton>   m_pOKBtn;

        DECL_DLLPRIVATE_LINK_TYPED(OKHdl, Button*, void);

    public:
        SfxCheckinDialog( vcl::Window* pParent );
        virtual ~SfxCheckinDialog();
        virtual void dispose() override;

        OUString GetComment( );
        bool IsMajor( );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
