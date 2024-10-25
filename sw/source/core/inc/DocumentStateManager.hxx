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

#include <IDocumentState.hxx>

#if defined(YRS)
#include <rtl/ref.hxx>
#include <com/sun/star/connection/XAcceptor.hpp>
#include <com/sun/star/connection/XConnector.hpp>
struct SwPosition;
#endif

class SwDoc;


namespace sw {

#if defined(YRS)
class YrsThread;
class YrsTransactionSupplier;
#endif

class DocumentStateManager final : public IDocumentState
{

public:
    DocumentStateManager( SwDoc& i_rSwdoc );
    ~DocumentStateManager();

    void SetModified() override;
    void ResetModified() override;
    bool IsModified() const override;
    bool IsEnableSetModified() const override;
    void SetEnableSetModified(bool bEnableSetModified) override;
    bool IsInCallModified() const override;
    bool IsUpdateExpField() const override;
    bool IsNewDoc() const override;
    void SetNewDoc(bool b) override;
    void SetUpdateExpFieldStat(bool b) override;

private:

    DocumentStateManager(DocumentStateManager const&) = delete;
    DocumentStateManager& operator=(DocumentStateManager const&) = delete;

    SwDoc& m_rDoc;

    bool mbEnableSetModified; //< FALSE: changing document modification status (temporarily) locked
    bool mbModified      ;    //< TRUE: document has changed.
    bool mbUpdateExpField;    //< TRUE: Update expression fields.
    bool mbNewDoc        ;    //< TRUE: new Doc.
    bool mbInCallModified;    //< TRUE: in Set/Reset-Modified link.

#if defined(YRS)
    friend class YrsThread;
    ::rtl::Reference<YrsThread> m_pYrsReader;
    css::uno::Reference<css::connection::XAcceptor> m_xAcceptor;
//    css::uno::Reference<css::connection::XConnector> m_xConnector;
    ::std::unique_ptr<YrsTransactionSupplier> m_pYrsSupplier;
//    YDoc * m_pYDoc { nullptr };
    // TODO move this to PostItMgr?
//    int m_nLocalComments { 0 };

public:
    void InitAcceptor() override;
    void InitConnector(css::uno::Any const& raConnector) override;
    IYrsTransactionSupplier::Mode SetYrsMode(IYrsTransactionSupplier::Mode mode) override;
    void CommitModified() override;

    void AddCommentImpl(/*YTransaction *const pTxn, YDoc const*const pDoc,*/ SwPosition const& rPos, OString const& rCommentId/*, SwPosition pEnd, SwPostIt pComment*/) override;
    void AddComment(SwPosition const& rPos, ::std::optional<SwPosition> oAnchorStart,
            SwPostItField const& rField, bool isInsert) override;
    void RemoveCommentImpl(OString const& rCommentId) override;
    void RemoveComment(SwPosition const& rPos, OString const& rCommentId) override;
    void UpdateCursor() override;
#endif
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
