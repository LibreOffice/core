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

#ifndef INCLUDED_SD_SOURCE_UI_INC_UNDOHEADERFOOTER_HXX
#define INCLUDED_SD_SOURCE_UI_INC_UNDOHEADERFOOTER_HXX

#include "sdundo.hxx"
#include "sdpage.hxx"
#include "sddllapi.h"

class SdDrawDocument;

/************************************************************************/

class SD_DLLPUBLIC SdHeaderFooterUndoAction : public SdUndoAction
{
    SdPage*     mpPage;

    const sd::HeaderFooterSettings  maOldSettings;
    const sd::HeaderFooterSettings  maNewSettings;

public:
    SdHeaderFooterUndoAction( SdDrawDocument* pDoc, SdPage* pPage, const sd::HeaderFooterSettings& rNewSettings );
    virtual ~SdHeaderFooterUndoAction();

    virtual void Undo() override;
    virtual void Redo() override;
};

#endif // INCLUDED_SD_SOURCE_UI_INC_UNDOHEADERFOOTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
