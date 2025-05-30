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

#include <sdundo.hxx>

class SdDrawDocument;
class SdrLayer;

/************************************************************************/

class SdLayerModifyUndoAction final : public SdUndoAction
{

public:
    SdLayerModifyUndoAction( SdDrawDocument& rDoc, SdrLayer* pLayer,
    OUString aOldLayerName, OUString aOldLayerTitle, OUString aOldLayerDesc, bool bOldIsVisible, bool bOldIsLocked, bool bOldIsPrintable,
    OUString aNewLayerName, OUString aNewLayerTitle, OUString aNewLayerDesc, bool bNewIsVisible, bool bNewIsLocked, bool bNewIsPrintable );

    virtual void Undo() override;
    virtual void Redo() override;

private:
    SdrLayer* mpLayer;
    OUString maOldLayerName;
    OUString maOldLayerTitle;
    OUString maOldLayerDesc;
    bool mbOldIsVisible;
    bool mbOldIsLocked;
    bool mbOldIsPrintable;
    OUString maNewLayerName;
    OUString maNewLayerTitle;
    OUString maNewLayerDesc;
    bool mbNewIsVisible;
    bool mbNewIsLocked;
    bool mbNewIsPrintable;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
