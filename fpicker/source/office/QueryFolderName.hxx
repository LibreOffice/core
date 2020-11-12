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
#ifndef INCLUDED_FPICKER_SOURCE_OFFICE_QUERYFOLDERNAME_HXX
#define INCLUDED_FPICKER_SOURCE_OFFICE_QUERYFOLDERNAME_HXX

#include <vcl/weld.hxx>

class QueryFolderNameDialog : public weld::GenericDialogController
{
private:
    std::unique_ptr<weld::Entry> m_xNameEdit;
    std::unique_ptr<weld::Button> m_xOKBtn;

    DECL_LINK(OKHdl, weld::Button&, void);
    DECL_LINK(NameHdl, weld::Entry&, void);

public:
    QueryFolderNameDialog(weld::Window* _pParent, const OUString& rTitle,
                          const OUString& rDefaultText);
    virtual ~QueryFolderNameDialog() override;
    OUString GetName() const { return m_xNameEdit->get_text(); }
};

#endif // INCLUDED_FPICKER_SOURCE_OFFICE_QUERYFOLDERNAME_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
