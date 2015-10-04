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

#ifndef INCLUDED_SC_SOURCE_UI_INC_DAPIDATA_HXX
#define INCLUDED_SC_SOURCE_UI_INC_DAPIDATA_HXX

#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/combobox.hxx>

struct ScImportSourceDesc;

class ScDataPilotDatabaseDlg : public ModalDialog
{
private:
    VclPtr<ListBox>  m_pLbDatabase;
    VclPtr<ComboBox> m_pCbObject;
    VclPtr<ListBox>  m_pLbType;

    void    FillObjects();

    DECL_LINK_TYPED( SelectHdl, ListBox&, void );

public:
    ScDataPilotDatabaseDlg(vcl::Window* pParent);
    virtual ~ScDataPilotDatabaseDlg();
    virtual void dispose() SAL_OVERRIDE;

    void    GetValues( ScImportSourceDesc& rDesc );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
