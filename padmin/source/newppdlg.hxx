/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: newppdlg.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 16:26:00 $
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

#ifndef _PAD_NEWPPDLG_HXX_
#define _PAD_NEWPPDLG_HXX_

#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_COMBOBOX_HXX
#include <vcl/combobox.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_GROUP_HXX
#include <vcl/group.hxx>
#endif

namespace psp { class PPDParser; }

namespace padmin {

    class PPDImportDialog : public ModalDialog
    {
        OKButton            m_aOKBtn;
        CancelButton        m_aCancelBtn;
        FixedText           m_aPathTxt;
        ComboBox            m_aPathBox;
        PushButton          m_aSearchBtn;
        FixedText           m_aDriverTxt;
        MultiListBox        m_aDriverLB;

        FixedLine           m_aPathGroup;
        FixedLine           m_aDriverGroup;

        String              m_aLoadingPPD;

        DECL_LINK( ClickBtnHdl, PushButton* );
        DECL_LINK( SelectHdl, ComboBox* );
        DECL_LINK( ModifyHdl, ComboBox* );

        void Import();
    public:
        PPDImportDialog( Window* pParent );
        ~PPDImportDialog();
    };

} // namespace

#endif // _NEWPPDLG_HXX
