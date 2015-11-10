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

#ifndef INCLUDED_SD_INC_SDUNDO_HXX
#define INCLUDED_SD_INC_SDUNDO_HXX

#include <svl/undo.hxx>
#include "sddllapi.h"

class SdDrawDocument;

class SD_DLLPUBLIC SdUndoAction : public SfxUndoAction
{
public:
    TYPEINFO_OVERRIDE();
                            SdUndoAction(SdDrawDocument* pSdDrawDocument)
                                : mpDoc(pSdDrawDocument)  {}
    virtual                 ~SdUndoAction() {}

    void                    SetComment(const OUString& rStr) { maComment = rStr; }
    virtual OUString        GetComment() const override { return maComment; }
    virtual SdUndoAction*   Clone() const { return nullptr; }

protected:
    SdDrawDocument* mpDoc;
    OUString maComment;
};

#endif // INCLUDED_SD_INC_SDUNDO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
