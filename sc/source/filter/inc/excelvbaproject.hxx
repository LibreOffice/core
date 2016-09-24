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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_EXCELVBAPROJECT_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_EXCELVBAPROJECT_HXX

#include <oox/ole/vbaproject.hxx>

namespace com { namespace sun { namespace star {
        namespace sheet { class XSpreadsheetDocument; }
} } }

namespace oox {
namespace xls {

/** Special implementation of the VBA project for the Excel filters. */
class ExcelVbaProject : public ::oox::ole::VbaProject
{
public:
    explicit            ExcelVbaProject(
                            const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                            const css::uno::Reference< css::sheet::XSpreadsheetDocument >& rxDocument );

protected:
    /** Adds dummy modules for sheets without imported code name. */
    virtual void        prepareImport() override;

private:
    css::uno::Reference< css::sheet::XSpreadsheetDocument >
                        mxDocument;
};

} // namespace xls
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
