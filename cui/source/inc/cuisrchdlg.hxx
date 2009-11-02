/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cuisrchdlg.hxx,v $
 * $Revision: 1.5 $
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
#ifndef _CUI_SRCHDLG_HXX
#define _CUI_SRCHDLG_HXX

// include ---------------------------------------------------------------


#include <svtools/stdctrl.hxx>
#ifndef _COMBOBOX_HXX //autogen
#include <vcl/combobox.hxx>
#endif
#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif
#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#include <sfx2/childwin.hxx>
#include <sfx2/basedlgs.hxx>
#include <svl/svarray.hxx>
#define _SVSTDARR_STRINGSDTOR
#include <svl/svstdarr.hxx>
#include <svtools/svmedit.hxx>
#include <sfx2/srchdefs.hxx>

class SvxJSearchOptionsPage;

class SvxJSearchOptionsDialog : public SfxSingleTabDialog
{
    INT32                   nInitialTlFlags;
    SvxJSearchOptionsPage  *pPage;

    // disallow copy-constructor and assignment-operator for now
    SvxJSearchOptionsDialog( const SvxJSearchOptionsDialog & );
    SvxJSearchOptionsDialog & operator == ( const SvxJSearchOptionsDialog & );

public:
    SvxJSearchOptionsDialog( Window *pParent,
                            const SfxItemSet& rOptionsSet,
                            INT32 nInitialFlags  );
    virtual ~SvxJSearchOptionsDialog();

    // Window
    virtual void    Activate();

    INT32           GetTransliterationFlags() const;
    void            SetTransliterationFlags( INT32 nSettings );
};

#endif
