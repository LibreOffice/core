/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _SDR_TABLE_DESIGN_HXX
#define _SDR_TABLE_DESIGN_HXX

#include "svx/svxdllapi.h"

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

extern SVX_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > CreateTableDesignFamily();

} }


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
