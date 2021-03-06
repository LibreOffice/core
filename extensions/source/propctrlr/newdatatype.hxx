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

#pragma once

#include <vcl/weld.hxx>

#include <set>
#include <vector>


namespace pcr
{
    //= NewDataTypeDialog
    class NewDataTypeDialog : public weld::GenericDialogController
    {
    private:
        std::set<OUString> m_aProhibitedNames;

        std::unique_ptr<weld::Entry> m_xName;
        std::unique_ptr<weld::Button> m_xOK;
    public:
        NewDataTypeDialog(weld::Window* _pParent, const OUString& _rNameBase,
            const std::vector< OUString >& _rProhibitedNames );
        virtual ~NewDataTypeDialog() override;

        OUString GetName() const { return m_xName->get_text(); }

    private:
        DECL_LINK(OnNameModified, weld::Entry&, void);
    };


} // namespace pcr


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
