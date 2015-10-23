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

#ifndef INCLUDED_SVX_SDR_TABLE_TABLEDESIGN_HXX
#define INCLUDED_SVX_SDR_TABLE_TABLEDESIGN_HXX

#include <svx/svxdllapi.h>

#include <com/sun/star/container/XNameAccess.hpp>

namespace sdr { namespace table {

const sal_Int32 first_row_style = 0;
const sal_Int32 last_row_style = 1;
const sal_Int32 first_column_style = 2;
const sal_Int32 last_column_style = 3;
const sal_Int32 even_rows_style = 4;
const sal_Int32 odd_rows_style = 5;
const sal_Int32 even_columns_style = 6;
const sal_Int32 odd_columns_style = 7;
const sal_Int32 body_style = 8;
const sal_Int32 background_style = 9;
const sal_Int32 style_count = 10;

extern SVX_DLLPUBLIC css::uno::Reference< css::container::XNameAccess > CreateTableDesignFamily();

} }


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
