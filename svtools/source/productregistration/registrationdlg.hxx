/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SVTOOLS_REGISTRATIONDLG_HXX
#define SVTOOLS_REGISTRATIONDLG_HXX

#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>

//........................................................................
namespace svt
{
//........................................................................

    //====================================================================
    //= RegistrationDialog
    //====================================================================
    class RegistrationDialog : public ModalDialog
    {
    public:
        enum UserResponse
        {
            urRegisterNow,
            urRegisterLater,
            urRegisterNever,
            urAlreadyRegistered
        };

    private:
        UserResponse    m_eResponse;

        FixedImage      m_aLogo;
        FixedText       m_aIntro;

        RadioButton     m_aNow;
        RadioButton     m_aLater;
        RadioButton     m_aNever;
        RadioButton     m_aAlreadyDone;

        FixedLine       m_aSeparator;

        OKButton        m_aOK;
        HelpButton      m_aHelp;

    public:
        RegistrationDialog( Window* _pWindow, const ResId& _rResId, bool _bEvalVersion );

        virtual short   Execute();
        virtual long    PreNotify( NotifyEvent& rNEvt );

        inline  UserResponse    getResponse() const { return m_eResponse; }
    };



//........................................................................
}// namespace svt
//........................................................................

#endif // SVTOOLS_REGISTRATIONDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
