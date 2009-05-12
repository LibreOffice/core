/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: newppdlg.hxx,v $
 * $Revision: 1.4 $
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

#ifndef _PAD_NEWPPDLG_HXX_
#define _PAD_NEWPPDLG_HXX_

#include <vcl/dialog.hxx>
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <vcl/combobox.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/group.hxx>

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
