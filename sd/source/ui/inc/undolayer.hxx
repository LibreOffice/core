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

#ifndef _SD_UNDOLAYER_HXX
#define _SD_UNDOLAYER_HXX

#include "sdundo.hxx"

class SdDrawDocument;
class SdrLayer;

/************************************************************************/

class SdLayerModifyUndoAction : public SdUndoAction
{

public:
    TYPEINFO();
    SdLayerModifyUndoAction( SdDrawDocument* _pDoc, SdrLayer* pLayer,
    const String& rOldLayerName, const String& rOldLayerTitle, const String& rOldLayerDesc, bool bOldIsVisible, bool bOldIsLocked, bool bOldIsPrintable,
    const String& rNewLayerName, const String& rNewLayerTitle, const String& rNewLayerDesc, bool bNewIsVisible, bool bNewIsLocked, bool bNewIsPrintable );

    virtual void Undo();
    virtual void Redo();

private:
    SdrLayer* mpLayer;
    String maOldLayerName;
    String maOldLayerTitle;
    String maOldLayerDesc;
    bool mbOldIsVisible;
    bool mbOldIsLocked;
    bool mbOldIsPrintable;
    String maNewLayerName;
    String maNewLayerTitle;
    String maNewLayerDesc;
    bool mbNewIsVisible;
    bool mbNewIsLocked;
    bool mbNewIsPrintable;
};

#endif      // _SD_UNDOLAYER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
