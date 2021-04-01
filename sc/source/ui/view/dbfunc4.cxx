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

#include <svx/svditer.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdpage.hxx>
#include <osl/diagnose.h>

#include <dbfunc.hxx>
#include <drwlayer.hxx>
#include <document.hxx>

using namespace com::sun::star;

sal_uInt16 ScDBFunc::DoUpdateCharts(const ScAddress& rPos, ScDocument& rDoc, bool bAllCharts)
{
    ScDrawLayer* pModel = rDoc.GetDrawLayer();
    if (!pModel)
        return 0;

    sal_uInt16 nFound = 0;

    sal_uInt16 nPageCount = pModel->GetPageCount();
    for (sal_uInt16 nPageNo = 0; nPageNo < nPageCount; nPageNo++)
    {
        SdrPage* pPage = pModel->GetPage(nPageNo);
        OSL_ENSURE(pPage, "Page ?");

        SdrObjListIter aIter(pPage, SdrIterMode::DeepNoGroups);
        SdrObject* pObject = aIter.Next();
        while (pObject)
        {
            if (pObject->GetObjIdentifier() == OBJ_OLE2 && ScDocument::IsChart(pObject))
            {
                OUString aName = static_cast<SdrOle2Obj*>(pObject)->GetPersistName();
                bool bHit = true;
                if (!bAllCharts)
                {
                    ScRangeList aRanges;
                    bool bColHeaders = false;
                    bool bRowHeaders = false;
                    rDoc.GetOldChartParameters(aName, aRanges, bColHeaders, bRowHeaders);
                    bHit = aRanges.In(rPos);
                }
                if (bHit)
                {
                    rDoc.UpdateChart(aName);
                    ++nFound;
                }
            }
            pObject = aIter.Next();
        }
    }
    return nFound;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
