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

#ifndef INCLUDED_SD_SOURCE_UI_INC_UNMODPG_HXX
#define INCLUDED_SD_SOURCE_UI_INC_UNMODPG_HXX

#include "sdundo.hxx"
#include "pres.hxx"

class SdDrawDocument;
class SdPage;

class ModifyPageUndoAction : public SdUndoAction
{
    SdPage*         mpPage;
    OUString        maOldName;
    OUString        maNewName;
    AutoLayout      meOldAutoLayout;
    AutoLayout      meNewAutoLayout;
    bool        mbOldBckgrndVisible;
    bool        mbNewBckgrndVisible;
    bool        mbOldBckgrndObjsVisible;
    bool        mbNewBckgrndObjsVisible;

    OUString        maComment;

public:
    ModifyPageUndoAction(
        SdDrawDocument*         pTheDoc,
        SdPage*                 pThePage,
        const OUString&         aTheNewName,
        AutoLayout              eTheNewAutoLayout,
        bool                    bTheNewBckgrndVisible,
        bool                    bTheNewBckgrndObjsVisible);

    virtual ~ModifyPageUndoAction();
    virtual void Undo() override;
    virtual void Redo() override;

    virtual OUString GetComment() const override;
};

class RenameLayoutTemplateUndoAction : public SdUndoAction
{
public:
    RenameLayoutTemplateUndoAction(
        SdDrawDocument* pDocument,
        const OUString& rOldLayoutName,
        const OUString& rNewLayoutName);

    virtual void Undo() override;
    virtual void Redo() override;

    virtual OUString GetComment() const override;

private:
    OUString maOldName;
    OUString maNewName;
    const OUString maComment;
};

#endif // INCLUDED_SD_SOURCE_UI_INC_UNMODPG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
