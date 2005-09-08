/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cuisrchdlg.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 20:53:30 $
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
#ifndef _CUI_SRCHDLG_HXX
#define _CUI_SRCHDLG_HXX

// include ---------------------------------------------------------------


#ifndef _STDCTRL_HXX //autogen
#include <svtools/stdctrl.hxx>
#endif
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
#ifndef _SFX_CHILDWIN_HXX //autogen
#include <sfx2/childwin.hxx>
#endif
#ifndef _BASEDLGS_HXX
#include <sfx2/basedlgs.hxx>
#endif
#ifndef _SVARRAY_HXX //autogen
#include <svtools/svarray.hxx>
#endif
#define _SVSTDARR_STRINGSDTOR
#include <svtools/svstdarr.hxx>

#ifndef _SVEDIT_HXX //autogen
#include <svtools/svmedit.hxx>
#endif

#ifndef _SFX_SRCHDEFS_HXX_
#include <sfx2/srchdefs.hxx>
#endif

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
                            const SfxItemSet& rOptionsSet, USHORT nUniqueId,
                            INT32 nInitialFlags  );
    virtual ~SvxJSearchOptionsDialog();

    // Window
    virtual void    Activate();

    INT32           GetTransliterationFlags() const;
    void            SetTransliterationFlags( INT32 nSettings );
};

#endif
