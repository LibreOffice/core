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

#include <model/SlsPageDescriptor.hxx>
#include <model/SlsSharedPageDescriptor.hxx>

#include <xmloff/autolayout.hxx>

#include <sdundo.hxx>

class SdDrawDocument;
class SdPage;

class ModifyPageUndoAction final : public SdUndoAction
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

public:
    ModifyPageUndoAction(
        SdDrawDocument&         rTheDoc,
        SdPage*                 pThePage,
        const OUString&         aTheNewName,
        AutoLayout              eTheNewAutoLayout,
        bool                    bTheNewBckgrndVisible,
        bool                    bTheNewBckgrndObjsVisible);

    virtual ~ModifyPageUndoAction() override;
    virtual void Undo() override;
    virtual void Redo() override;
};

class ChangeSlideExclusionStateUndoAction final : public SdUndoAction
{
public:
    ChangeSlideExclusionStateUndoAction(SdDrawDocument& rDocument,
                                        const sd::slidesorter::model::PageDescriptor::State eState,
                                        const bool bOldStateValue);

    ChangeSlideExclusionStateUndoAction(
        SdDrawDocument& rDocument, const sd::slidesorter::model::SharedPageDescriptor& rpDescriptor,
        const sd::slidesorter::model::PageDescriptor::State eState, const bool bOldStateValue);

    virtual void Undo() override;
    virtual void Redo() override;

    virtual OUString GetComment() const override;

    void AddPageDescriptor(const sd::slidesorter::model::SharedPageDescriptor& rpDescriptor);

private:
    sd::slidesorter::model::PageDescriptor::State meState;
    bool mbOldStateValue;
    std::vector<sd::slidesorter::model::SharedPageDescriptor> mrpDescriptors;
    const OUString maComment;
};

class RenameLayoutTemplateUndoAction final : public SdUndoAction
{
public:
    RenameLayoutTemplateUndoAction(
        SdDrawDocument& rDocument,
        OUString aOldLayoutName,
        OUString aNewLayoutName);

    virtual void Undo() override;
    virtual void Redo() override;

    virtual OUString GetComment() const override;

private:
    OUString maOldName;
    OUString maNewName;
    const OUString maComment;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
