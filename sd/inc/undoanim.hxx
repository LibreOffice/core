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

#include <com/sun/star/animations/XAnimationNode.hpp>
#include <svx/svdundo.hxx>
#include <memory>

#include "sdundo.hxx"

class SdPage;

namespace sd
{
struct UndoAnimationImpl;

class UndoAnimation final : public SdrUndoAction
{
public:
    UndoAnimation(SdDrawDocument* pDoc, SdPage* pThePage);
    virtual ~UndoAnimation() override;

    virtual void Undo() override;
    virtual void Redo() override;

    virtual OUString GetComment() const override;

private:
    std::unique_ptr<UndoAnimationImpl> mpImpl;
};

struct UndoAnimationPathImpl;
class UndoAnimationPath final : public SdrUndoAction
{
public:
    UndoAnimationPath(SdDrawDocument* pDoc, SdPage* pThePage,
                      const css::uno::Reference<css::animations::XAnimationNode>& xNode);
    virtual ~UndoAnimationPath() override;

    virtual void Undo() override;
    virtual void Redo() override;

    virtual OUString GetComment() const override;

private:
    std::unique_ptr<UndoAnimationPathImpl> mpImpl;
};

struct UndoTransitionImpl;

class UndoTransition final : public SdUndoAction
{
public:
    UndoTransition(SdDrawDocument* pDoc, SdPage* pThePage);
    virtual ~UndoTransition() override;

    virtual void Undo() override;
    virtual void Redo() override;

    virtual OUString GetComment() const override;

private:
    std::unique_ptr<UndoTransitionImpl> mpImpl;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
