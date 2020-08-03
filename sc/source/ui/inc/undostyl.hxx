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

#include <memory>
#include <svl/style.hxx>
#include "undobase.hxx"

class ScDocShell;

class ScStyleSaveData
{
private:
    OUString        aName;
    OUString        aParent;
    std::unique_ptr<SfxItemSet>  xItems;

public:
    ScStyleSaveData();
    ScStyleSaveData( const ScStyleSaveData& rOther );
    ScStyleSaveData&    operator=( const ScStyleSaveData& rOther );

    void                InitFromStyle( const SfxStyleSheetBase* pSource );

    const OUString&     GetName() const     { return aName; }
    const OUString&     GetParent() const   { return aParent; }
    const SfxItemSet*   GetItems() const    { return xItems.get(); }
};

class ScUndoModifyStyle: public ScSimpleUndo
{
private:
    SfxStyleFamily  eFamily;
    ScStyleSaveData aOldData;
    ScStyleSaveData aNewData;

    static void     DoChange( ScDocShell* pDocSh,
                                const OUString& rName, SfxStyleFamily eStyleFamily,
                                const ScStyleSaveData& rData );

public:
                    ScUndoModifyStyle( ScDocShell* pDocSh,
                                        SfxStyleFamily eFam,
                                        const ScStyleSaveData& rOld,
                                        const ScStyleSaveData& rNew );
    virtual         ~ScUndoModifyStyle() override;

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;
};

class ScUndoApplyPageStyle: public ScSimpleUndo
{
public:
                    ScUndoApplyPageStyle( ScDocShell* pDocSh, const OUString& rNewStyle );
    virtual         ~ScUndoApplyPageStyle() override;

    void            AddSheetAction( SCTAB nTab, const OUString& rOld );

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    struct ApplyStyleEntry
    {
        SCTAB           mnTab;
        OUString        maOldStyle;
        explicit        ApplyStyleEntry( SCTAB nTab, const OUString& rOldStyle );
    };
    typedef ::std::vector< ApplyStyleEntry > ApplyStyleVec;

    ApplyStyleVec   maEntries;
    OUString        maNewStyle;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
