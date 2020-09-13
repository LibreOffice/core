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

#include <vcl/transfer.hxx>
#include <memory>

namespace dbaui
{
    class OTableRow;
    class OTableRowExchange : public TransferableHelper
    {
        std::vector< std::shared_ptr<OTableRow> > m_vTableRow;
    public:
        OTableRowExchange(const std::vector< std::shared_ptr<OTableRow> >& _rvTableRow);
    protected:
        virtual void        AddSupportedFormats() override;
        virtual bool GetData( const css::datatransfer::DataFlavor& rFlavor, const OUString& rDestDoc ) override;
        virtual bool        WriteObject( tools::SvRef<SotStorageStream>& rxOStm, void* pUserObject, sal_uInt32 nUserObjectId, const css::datatransfer::DataFlavor& rFlavor ) override;
        virtual void        ObjectReleased() override;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
