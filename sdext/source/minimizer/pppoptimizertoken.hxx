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

#ifndef INCLUDED_SDEXT_SOURCE_MINIMIZER_PPPOPTIMIZERTOKEN_HXX
#define INCLUDED_SDEXT_SOURCE_MINIMIZER_PPPOPTIMIZERTOKEN_HXX

#include <rtl/ustring.hxx>

enum PPPOptimizerTokenEnum
{
    TK_btnNavFinish,

    TK_DocumentService,
    TK_FileSizeDestination,
    TK_FileSizeSource,
    TK_FilterName,
    TK_Flags,
    TK_DocumentFrame,
    TK_DialogParentWindow,
    TK_Name,
    TK_Progress,
    TK_Settings,
    TK_StatusDispatcher,
    TK_Type,
    TK_UIName,

    TK_JPEGCompression,
    TK_JPEGQuality,
    TK_RemoveCropArea,
    TK_ImageResolution,
    TK_EmbedLinkedGraphics,
    TK_OLEOptimization,
    TK_OLEOptimizationType,
    TK_DeleteUnusedMasterPages,
    TK_DeleteHiddenSlides,
    TK_DeleteNotesPages,
    TK_CustomShowName,
    TK_SaveAs,
    TK_SaveAsURL,
    TK_OpenNewDocument,
    TK_EstimatedFileSize,

    TK_Status,
    TK_Pages,

    TK_NotFound
};

PPPOptimizerTokenEnum TKGet(const OUString&);
OUString TKGet(const PPPOptimizerTokenEnum);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
