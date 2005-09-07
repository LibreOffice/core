/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: nfltdlg.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:07:12 $
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
#ifndef _SFXNFLTDLG_HXX
#define _SFXNFLTDLG_HXX


#ifndef _SV_DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#ifndef _SV_LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
class SfxFilterMatcher;

class SfxMedium;
class SfxFilterDialog : public ModalDialog
{
private:
    FixedLine               aMimeTypeFL;
    FixedText               aMIMEType;
    FixedText               aExtension;
    FixedLine               aFilterFL;
    ListBox                 aListbox;
    OKButton                aOkBtn;
    CancelButton            aCancelBtn;
    HelpButton              aHelpBtn;

    const SfxFilterMatcher& rMatcher;

    DECL_LINK( OkHdl, Control * );

public:
    SfxFilterDialog( Window * pParent,
                     SfxMedium* pMed,
                     const SfxFilterMatcher &rMatcher,
                     const String *pDefFilter, BOOL bShowConvert = TRUE );

    String GetSelectEntry() const;
};

#endif

