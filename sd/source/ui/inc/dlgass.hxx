/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlgass.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 17:41:57 $
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

#ifndef INC_DLGASS
#define INC_DLGASS

#ifndef _GROUP_HXX
#include <vcl/group.hxx>
#endif
#ifndef _EDIT_HXX
#include <vcl/edit.hxx>
#endif
#ifndef _FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#include <svtools/svmedit.hxx>

#ifndef INC_ASSCLASS
#include "assclass.hxx"
#endif

#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif

#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif

#include "sdenumdef.hxx"

class AssistentDlgImpl;

class AssistentDlg:public ModalDialog
{
private:
    AssistentDlgImpl* mpImpl;

public:
    AssistentDlg(Window* pParent, BOOL bAutoPilot);
    ~AssistentDlg();

    DECL_LINK( FinishHdl, OKButton * );

    SfxObjectShellLock GetDocument();
    String GetTopic() const;
    String GetUserName() const;
    String GetInformation() const;
    OutputType GetOutputMedium() const;
    BOOL IsSummary() const;
    StartType GetStartType() const;
    String GetDocPath() const;
    BOOL GetStartWithFlag() const;

    BOOL IsDocEmpty() const;
    String GetPassword();
};

#endif
