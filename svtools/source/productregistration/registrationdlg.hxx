/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: registrationdlg.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 16:42:11 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SVTOOLS_REGISTRATIONDLG_HXX
#define SVTOOLS_REGISTRATIONDLG_HXX

#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif

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
