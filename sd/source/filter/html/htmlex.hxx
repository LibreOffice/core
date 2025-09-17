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

#include <resltn.hxx>
#include <rtl/ustrbuf.hxx>
#include <tools/color.hxx>
#include <tools/solar.h>
#include <vcl/errinf.hxx>
#include <unotools/resmgr.hxx>

#include <memory>
#include <string_view>
#include <vector>

namespace sd { class DrawDocShell; }

class OutlinerParaObject;
class SfxItemSet;
class SfxProgress;
class SdrOutliner;
class SdPage;
class HtmlState;
class SdrTextObj;
class SdrObjGroup;
namespace sdr::table { class SdrTableObj; }
class SdrPage;
class SdDrawDocument;

/// this class exports an Impress Document as a HTML Presentation.
class HtmlExport final
{
    std::vector< SdPage* > maPages;

    OUString maPath;

    SdDrawDocument* mpDoc;
    ::sd::DrawDocShell* mpDocSh;

    std::unique_ptr<SfxProgress> mpProgress;
    sal_uInt16 mnSdPageCount;
    sal_uInt16 mnPagesWritten;
    OUString maIndex;
    OUString maDocFileName;

    OUString maExportPath; ///< output directory or URL.

    void InitProgress( sal_uInt16 nProgrCount );
    void ResetProgress();

    /// Output document metadata.
    OUString DocumentMetadata() const;

    void Init();
    void ExportSingleDocument();

    bool WriteHtml( std::u16string_view rFileName, std::u16string_view rHtmlData );

 public:
    HtmlExport(OUString aPath,
               SdDrawDocument* pExpDoc,
               sd::DrawDocShell* pDocShell);

    static OUString CreateTextForTitle(SdrOutliner* pOutliner, SdPage* pPage);
    static void WriteObjectGroup(OUStringBuffer& aStr, SdrObjGroup const * pObjectGroup,
                                 SdrOutliner* pOutliner, bool bHeadLine);
    static void WriteTable(OUStringBuffer& aStr, sdr::table::SdrTableObj const * pTableObject,
                           SdrOutliner* pOutliner);
    static void WriteOutlinerParagraph(OUStringBuffer& aStr, SdrOutliner* pOutliner,
                                       OutlinerParaObject const * pOutlinerParagraphObject,
                                       bool bHeadLine);

    ~HtmlExport();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
