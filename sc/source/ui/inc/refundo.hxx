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

#include <memory>

class ScDocument;
class ScDBCollection;
class ScRangeName;
class ScPrintRangeSaver;
class ScDPCollection;
class ScDetOpList;
class ScChartListenerCollection;
class ScAreaLinkSaveCollection;
class ScUnoRefList;

class ScRefUndoData
{
private:
    std::unique_ptr<ScDBCollection>             pDBCollection;
    std::unique_ptr<ScRangeName>                pRangeName;
    std::unique_ptr<ScPrintRangeSaver>          pPrintRanges;
    std::unique_ptr<ScDPCollection>             pDPCollection;
    std::unique_ptr<ScDetOpList>                pDetOpList;
    std::unique_ptr<ScChartListenerCollection>  pChartListenerCollection;
    std::unique_ptr<ScAreaLinkSaveCollection>   pAreaLinks;
    std::unique_ptr<ScUnoRefList> pUnoRefs;

public:
                ScRefUndoData( const ScDocument* pDoc );
                ~ScRefUndoData();

    void        DeleteUnchanged( const ScDocument* pDoc );
    void        DoUndo( ScDocument* pDoc, bool bUndoRefFirst );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
