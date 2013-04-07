/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _PAD_NEWPPDLG_HXX_
#define _PAD_NEWPPDLG_HXX_

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
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

        std::list< OUString >  m_aImportedFiles;
    public:
        PPDImportDialog( Window* pParent );
        ~PPDImportDialog();

        const std::list< OUString >& getImportedFiles() const
        { return m_aImportedFiles; }
    };

} // namespace

#endif // _NEWPPDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
