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

#include <sal/log.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdotable.hxx>
#include <svx/svdograf.hxx>
#include <tools/debug.hxx>

#include <drawdoc.hxx>
#include <DrawDocShell.hxx>
#include <Outliner.hxx>
#include <sdpage.hxx>

#include "htmlex.hxx"
#include <sdhtmlfilter.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::document;

using namespace sdr::table;

namespace
{
void CreateTitleDescription(SdrObject* pObject, OUStringBuffer& rHtml)
{
    rHtml.append("<h2>");
    rHtml.append(pObject->GetTitle());
    rHtml.append("</h2>\n\r<p>");
    rHtml.append(pObject->GetDescription());
    rHtml.append("</p>\n\r");
}
}

void SdHTMLFilter::ExportPage(SdrOutliner* pOutliner, SdPage* pPage, OUStringBuffer& rHtml)
{
    if (!pPage || !pOutliner)
    {
        return;
    }

    // page title
    OUString sTitleText(HtmlExport::CreateTextForTitle(pOutliner, pPage));

    rHtml.append("<h1>" + sTitleText + "</h1>\r\n");

    for (const rtl::Reference<SdrObject>& pObject : *pPage)
    {
        PresObjKind eKind = pPage->GetPresObjKind(pObject.get());

        switch (eKind)
        {
            case PresObjKind::NONE:
            {
                if (pObject->GetObjIdentifier() == SdrObjKind::Group)
                {
                    SdrObjGroup* pObjectGroup = static_cast<SdrObjGroup*>(pObject.get());
                    HtmlExport::WriteObjectGroup(rHtml, pObjectGroup, pOutliner, false);
                }
                else if (pObject->GetObjIdentifier() == SdrObjKind::Table)
                {
                    SdrTableObj* pTableObject = static_cast<SdrTableObj*>(pObject.get());
                    HtmlExport::WriteTable(rHtml, pTableObject, pOutliner);
                }
                else
                {
                    if (pObject->GetOutlinerParaObject())
                    {
                        HtmlExport::WriteOutlinerParagraph(rHtml, pOutliner,
                                                           pObject->GetOutlinerParaObject(), false);
                    }
                    else
                    {
                        CreateTitleDescription(pObject.get(), rHtml);
                    }
                }
            }
            break;

            case PresObjKind::Table:
            {
                SdrTableObj* pTableObject = static_cast<SdrTableObj*>(pObject.get());
                HtmlExport::WriteTable(rHtml, pTableObject, pOutliner);
            }
            break;

            case PresObjKind::Text:
            case PresObjKind::Outline:
            {
                SdrTextObj* pTextObject = static_cast<SdrTextObj*>(pObject.get());
                if (pTextObject->IsEmptyPresObj())
                    continue;
                HtmlExport::WriteOutlinerParagraph(rHtml, pOutliner,
                                                   pTextObject->GetOutlinerParaObject(), true);
            }
            break;

            default:
                CreateTitleDescription(pObject.get(), rHtml);
                break;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
