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

#include "address.hxx"
#include <vector>

class SC_DLLPUBLIC ScDPOutputGeometry
{
public:
    enum FieldType { Column = 0, Row, Page, Data, None };

    ScDPOutputGeometry() = delete;
    ScDPOutputGeometry(const ScRange& rOutRange, bool bShowFilter);
    ~ScDPOutputGeometry();

    /**
     * @param nCount number of row fields.
     */
    void setRowFieldCount(sal_uInt32 nCount);
    void setColumnFieldCount(sal_uInt32 nCount);
    void setPageFieldCount(sal_uInt32 nCount);
    void setDataFieldCount(sal_uInt32 nCount);
    void setDataLayoutType(FieldType eType);
    void setHeaderLayout(bool bHeaderLayout);
    void setCompactMode(bool bCompactMode);

    void getColumnFieldPositions(::std::vector<ScAddress>& rAddrs) const;
    void getRowFieldPositions(::std::vector<ScAddress>& rAddrs) const;
    void getPageFieldPositions(::std::vector<ScAddress>& rAddrs) const;

    SCROW getRowFieldHeaderRow() const;

    std::pair<FieldType, size_t> getFieldButtonType(const ScAddress& rPos) const;

private:
    void adjustFieldsForDataLayout(sal_uInt32& rColumnFields, sal_uInt32& rRowFields) const;

private:
    ScRange     maOutRange;
    sal_uInt32  mnRowFields;    /// number of row fields
    sal_uInt32  mnColumnFields;
    sal_uInt32  mnPageFields;
    sal_uInt32  mnDataFields;
    FieldType   meDataLayoutType;
    bool        mbShowFilter;
    bool        mbHeaderLayout;
    bool        mbCompactMode; // MSO only setting
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
