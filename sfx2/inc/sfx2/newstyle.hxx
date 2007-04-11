/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: newstyle.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 21:25:28 $
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
#ifndef _NEWSTYLE_HXX
#define _NEWSTYLE_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_SFX2_DLLAPI_H
#include "sfx2/dllapi.h"
#endif

#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SV_COMBOBOX_HXX
#include <vcl/combobox.hxx>
#endif
#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif

class SfxStyleSheetBasePool;

class SFX2_DLLPUBLIC SfxNewStyleDlg : public ModalDialog
{
private:
    FixedLine               aColFL;
    ComboBox                aColBox;
    OKButton                aOKBtn;
    CancelButton            aCancelBtn;

    QueryBox                aQueryOverwriteBox;
    SfxStyleSheetBasePool&  rPool;

//#if 0 // _SOLAR__PRIVATE
    DECL_DLLPRIVATE_LINK( OKHdl, Control * );
    DECL_DLLPRIVATE_LINK( ModifyHdl, ComboBox * );
//#endif

public:
    SfxNewStyleDlg( Window* pParent, SfxStyleSheetBasePool& );
    ~SfxNewStyleDlg();

    String                  GetName() const { return aColBox.GetText().EraseLeadingChars(); }
};

#endif

