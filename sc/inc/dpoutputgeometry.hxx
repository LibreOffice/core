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

#ifndef SC_DPOUTPUTGEOMETRY_HXX
#define SC_DPOUTPUTGEOMETRY_HXX

#include "address.hxx"
#include <vector>

class ScAddress;

class SC_DLLPUBLIC ScDPOutputGeometry
{
public:
    enum FieldType { Column, Row, Page, Data, None };
    enum ImportType { ODF, XLS };

    ScDPOutputGeometry(const ScRange& rOutRange, bool bShowFilter, ImportType eImportType);
    ~ScDPOutputGeometry();

    /**
     * @param nCount number of row fields, <b>excluding the data layout
     *               field if exists</b>.
     */
    void setRowFieldCount(sal_uInt32 nCount);
    void setColumnFieldCount(sal_uInt32 nCount);
    void setPageFieldCount(sal_uInt32 nCount);
    void setDataFieldCount(sal_uInt32 nCount);

    void getColumnFieldPositions(::std::vector<ScAddress>& rAddrs) const;
    void getRowFieldPositions(::std::vector<ScAddress>& rAddrs) const;
    void getPageFieldPositions(::std::vector<ScAddress>& rAddrs) const;

    SCROW getRowFieldHeaderRow() const;

    FieldType getFieldButtonType(const ScAddress& rPos) const;

private:
    ScDPOutputGeometry(); // disabled

private:
    ScRange     maOutRange;
    sal_uInt32  mnRowFields;    /// number of row fields (data layout field NOT included!)
    sal_uInt32  mnColumnFields;
    sal_uInt32  mnPageFields;
    sal_uInt32  mnDataFields;

    ImportType  meImportType;

    bool        mbShowFilter;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
