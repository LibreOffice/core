/*************************************************************************
 *
 *  $RCSfile: dlgass.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: af $ $Date: 2001-04-06 12:23:52 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef INC_DLGASS
#define INC_DLGASS

#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#ifdef VCL
#include <svtools/svmedit.hxx>
#endif

#ifndef INC_ASSCLASS
#include "assclass.hxx"
#endif

#ifndef _SV_LSTBOX_HXX //autogen    // !!!TEMP
#include <vcl/lstbox.hxx>
#endif

#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif

enum OutputType { OUTPUT_PAGE, OUTPUT_OVERHEAD, OUTPUT_SLIDE, OUTPUT_PRESENTATION, OUTPUT_ORIGINAL };
enum StartType { ST_EMPTY, ST_TEMPLATE, ST_OPEN };

class AssistentDlgImpl;

class AssistentDlg:public ModalDialog
{
private:
    AssistentDlgImpl* m_pImpl;

public:
    AssistentDlg(Window* pParent, BOOL bAutoPilot);
    ~AssistentDlg();

    DECL_LINK( FinishHdl, OKButton * );
        //erstellt die eingestellte Praesentation und beendet den
        //Assistenten

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
