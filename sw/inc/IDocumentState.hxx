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

#if defined(YRS)
#include <com/sun/star/uno/Any.h>
#include <editeng/yrs.hxx>
#include <optional>
struct SwPosition;
class SwPostItField;
#endif

/** Get information about the current document state
 */
class IDocumentState
{
public:
    /** Must be called manually at changes of format.
    */
    virtual void SetModified() = 0;

    virtual void ResetModified() = 0;

    /** Changes of document?
    */
    virtual bool IsModified() const = 0;

    virtual bool IsInCallModified() const = 0;

    virtual bool IsUpdateExpField() const = 0;

    virtual bool IsNewDoc() const = 0;

    virtual void SetNewDoc(bool b) = 0;

    virtual void SetUpdateExpFieldStat(bool b) = 0;

    virtual bool IsEnableSetModified() const = 0;
    virtual void SetEnableSetModified(bool bEnableSetModified) = 0;

#if defined(YRS)
    virtual void InitAcceptor() = 0;
    virtual void InitConnector(css::uno::Any const& raConnector) = 0;
    virtual IYrsTransactionSupplier::Mode SetYrsMode(IYrsTransactionSupplier::Mode mode) = 0;
    virtual void CommitModified() = 0;

    virtual void AddCommentImpl(SwPosition const& rPos, OString const& rCommentId) = 0;
    virtual void AddComment(SwPosition const& rPos, ::std::optional<SwPosition> oAnchorStart,
                            SwPostItField const& rField, bool isInsert)
        = 0;
    virtual void RemoveCommentImpl(rtl::OString const& rCommentId) = 0;
    virtual void RemoveComment(SwPosition const& rPos, rtl::OString const& rCommentId) = 0;
    virtual void UpdateCursor() = 0;
#endif

protected:
    virtual ~IDocumentState(){};
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
