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
#if 1

#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <list>

class ScXMLImport;
class ScChartListenerCollection;
class ScDocument;

struct ScMyToFixupOLE
{
    com::sun::star::uno::Reference <com::sun::star::drawing::XShape> xShape;
    rtl::OUString sRangeList;
};

typedef std::list<ScMyToFixupOLE> ScMyToFixupOLEs;

class ScMyOLEFixer
{
    ScXMLImport&                rImport;
    ScMyToFixupOLEs             aShapes;
    ScChartListenerCollection*  pCollection;

    void CreateChartListener(ScDocument* pDoc,
        const rtl::OUString& rName,
        const rtl::OUString& rRangeList);
public:
    ScMyOLEFixer(ScXMLImport& rImport);
    ~ScMyOLEFixer();

    static sal_Bool IsOLE(com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& rShape);
    void    AddOLE(com::sun::star::uno::Reference <com::sun::star::drawing::XShape>& rShape,
                   const rtl::OUString &rRangeList);
    void    FixupOLEs();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
