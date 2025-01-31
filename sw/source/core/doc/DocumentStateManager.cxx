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
#include <DocumentStateManager.hxx>
#include <doc.hxx>
#include <DocumentStatisticsManager.hxx>
#include <IDocumentUndoRedo.hxx>
#include <DocumentLayoutManager.hxx>
#include <acorrect.hxx>

#if defined(YRS)
#include <AnnotationWin.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <docufld.hxx>
#include <PostItMgr.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <txtannotationfld.hxx>
#include <ndtxt.hxx>

#include <editeng/yrs.hxx>
#include <editeng/editview.hxx>
#include <editeng/outliner.hxx>

#include <sfx2/docfile.hxx>

#include <vcl/svapp.hxx>

#include <ucbhelper/content.hxx>

#include <sax/tools/converter.hxx>

#include <comphelper/diagnose_ex.hxx>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/connection/Acceptor.hpp>
#include <com/sun/star/connection/Connector.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/io/XSeekable.hpp>

#include <salhelper/thread.hxx>
#endif

namespace sw
{

#if defined(YRS)
using sw::annotation::SwAnnotationWin;

namespace {

enum class Message : sal_uInt8
{
    //Init,
    RequestStateVector,
    SendStateVector,
    SendStateDiff,
};

    YDoc* MakeYDoc()
    {
//        struct deleter { void operator()(YDoc *const p) const { ydoc_destroy(p); } };
//        std::unique_ptr<YDoc, deleter> *const pDoc(ydoc_new());
        YOptions o{yoptions()};
        o.encoding = Y_OFFSET_UTF16;
        YDoc *const pYDoc{ydoc_new_with_options(o)};
        return pYDoc;
    }

} // namespace

class YrsTransactionSupplier : public IYrsTransactionSupplier
{
private:
    friend class DocumentStateManager;
    YDoc * m_pYDoc { nullptr };
    Branch * m_pComments;
    Branch * m_pCursors;
    YTransaction * m_pCurrentReadTransaction { nullptr };
    YTransaction * m_pCurrentWriteTransaction { nullptr };
    int m_nLocalComments { 0 };
    ::std::map<OString, ::std::vector</*SwAnnotationWin*/SwAnnotationItem *>> m_Comments;

public:
    YrsTransactionSupplier();
    ~YrsTransactionSupplier();

    YDoc* GetYDoc() override { return m_pYDoc; }
    Branch* GetCommentMap() override { return m_pComments; }
    Branch* GetCursorMap() override { return m_pCursors; }
    YTransaction * GetReadTransaction() override;
    YTransaction * GetWriteTransaction() override;
    bool CommitTransaction(bool isForce = false) override;
    OString GenNewCommentId() override;
    decltype(m_Comments) const& GetComments() { return m_Comments; }
};

YrsTransactionSupplier::YrsTransactionSupplier()
    : m_pYDoc(MakeYDoc())
    // ymap implicitly calls transact_mut()
    , m_pComments(ymap(m_pYDoc, "comments"))
    , m_pCursors(ymap(m_pYDoc, "cursors"))
{
}

YrsTransactionSupplier::~YrsTransactionSupplier()
{
    assert(m_pCurrentWriteTransaction == nullptr);
    ydoc_destroy(m_pYDoc);
}

YTransaction * YrsTransactionSupplier::GetReadTransaction()
{
    if (m_pCurrentWriteTransaction)
    {
        return m_pCurrentWriteTransaction;
    }
    if (m_pCurrentReadTransaction)
    {
        return m_pCurrentReadTransaction;
    }
    m_pCurrentReadTransaction = ydoc_read_transaction(m_pYDoc);
    return m_pCurrentReadTransaction;
}

YTransaction * YrsTransactionSupplier::GetWriteTransaction()
{
    if (m_Mode != Mode::Edit)
    {
        return nullptr;
    }
    if (m_pCurrentWriteTransaction)
    {
        return m_pCurrentWriteTransaction;
    }
    if (m_pCurrentReadTransaction)
    {
        // commit it? or is it an error?
        assert(false);
    }
    m_pCurrentWriteTransaction = ydoc_write_transaction(m_pYDoc, 0, nullptr);
    return m_pCurrentWriteTransaction;
}

bool YrsTransactionSupplier::CommitTransaction(bool const isForce)
{
    bool ret{false};
    if (!isForce && m_Mode == Mode::Replay)
    {
        return ret;
    }
    if (m_pCurrentWriteTransaction)
    {
        assert(m_pCurrentReadTransaction == nullptr);
        ytransaction_commit(m_pCurrentWriteTransaction);
        m_pCurrentWriteTransaction = nullptr;
        ret = true;
    }
    if (m_pCurrentReadTransaction)
    {
        ytransaction_commit(m_pCurrentReadTransaction);
        m_pCurrentReadTransaction = nullptr;
    }
    return ret;
}

OString YrsTransactionSupplier::GenNewCommentId()
{
    auto const id{ydoc_id(m_pYDoc)};
    int counter = ++m_nLocalComments;
    return OString::number(id) + OString::number(counter);
}

class YrsThread : public ::salhelper::Thread
{
public:
    uno::Reference<connection::XConnection> m_xConnection;
    DocumentStateManager * m_pDSM;

public:
    YrsThread(uno::Reference<connection::XConnection> const& xConnection,
            DocumentStateManager & rDSM)
        : ::salhelper::Thread("yrs reader")
        , m_xConnection(xConnection)
        , m_pDSM(&rDSM)
    {
    }
    void execute() override
    {
//        uno::Reference<XConnection> xConnection = m_xAcceptor->accept("pipe,name=ytest");

        uno::Sequence<sal_Int8> buf;
        while (m_xConnection->read(buf, 4) == 4)
        {
            sal_Int32 const size{static_cast<sal_uInt8>(buf[0])
                | static_cast<sal_uInt8>(buf[1]) << 8
                | static_cast<sal_uInt8>(buf[2]) << 16
                | static_cast<sal_uInt8>(buf[3]) << 24};
            if (size == 0)
            {
                SAL_DEBUG("YYY 0");
                break;
            }
            SAL_DEBUG("YRS receive " << size);
            ::std::unique_ptr<uno::Sequence<sal_Int8>> pBuf{new uno::Sequence<sal_Int8>(size)};
            m_xConnection->read(*pBuf, size);
            Application::PostUserEvent(LINK(this, YrsThread, HandleMessage), pBuf.release());
        }
    }
    DECL_LINK(HandleMessage, void*, void);
};

namespace {

    struct ObserveState
    {
        YrsTransactionSupplier & rYrsSupplier;
        SwDoc & rDoc;
        YTransaction *const pTxn;
    };

} // namespace

#if 0
    path(Branch *const pComments, YPathSegment const*const pPath, int const len)
    {
        for (int i = 0; i < len; ++i)
        {
            switch (pPath[i]->tag)
            {
                case Y_EVENT_PATH_KEY:
                    pPath[i]->value->key
                    break;
                case Y_EVENT_PATH_INDEX:
                    pPath[i]->value->index
                    break;
                default:
                    assert(false);
            }
        }
        assert(pPath[0]->tag == Y_EVENT_PATH_KEY);
//nope        assert(strcmp(pPath[0]->value->key, "comments") == 0);
        if (len < 1)
            return;
        assert(pPath[1]->tag == Y_EVENT_PATH_INDEX);
        if (len < 2)
            return;
        switch (pPath[1]->value->index)
        {
            case 0:
                // position?
                break;
            case 1:
                // properties
                assert(pPath[2]->tag == Y_EVENT_PATH_KEY);
                break;
            case 2:
                // text
                assert(pPath[2]->tag == Y_EVENT_PATH_INDEX);
                break;
        }
    }
#endif

namespace {

    extern "C" void observe_comments(void *const pState, uint32_t count, YEvent const*const events)
    {
        ObserveState & rState{*static_cast<ObserveState*>(pState)};
                                            //FAIL YTransaction *const pTxn{rState.rYrsSupplier.GetWriteTransaction(true)};
        YTransaction *const pTxn{rState.pTxn};
// ??? that is TransactionMut - there is no way to construct YTransaction from it???                                            YTransaction *const pTxn{pEvent->txn};

        ::std::vector<::std::tuple<OString, uint64_t, uint64_t>> posUpdates;
        ::std::map<::std::pair<uint64_t, uint64_t>, std::pair<OString, Branch const*>> newComments;

        for (decltype(count) i = 0; i < count; ++i)
        {
            switch (events[i].tag)
            {
                case Y_TEXT:
                {
                    YTextEvent const*const pEvent{&events[i].content.text};
                    Branch const*const pText{ytext_event_target(pEvent)};
                    (void)pText;

                    uint32_t lenP{0};
                    YPathSegment *const pPath{ytext_event_path(pEvent, &lenP)};
                    yvalidate(lenP == 2);
                    yvalidate(pPath[1].tag == Y_EVENT_PATH_INDEX);
                    yvalidate(pPath[1].value.index == 2);
                    yvalidate(pPath[0].tag == Y_EVENT_PATH_KEY);
                    OString const commentId{pPath[0].value.key};

                    ypath_destroy(pPath, lenP);
#if 0
                    auto const it{rState.rYrsSupplier.GetComments().find(commentId)};
                    assert(it != rState.rYrsSupplier.GetComments().end());
#endif

                    SwAnnotationWin & rWin{*rState.rYrsSupplier.GetComments().find(commentId)->second.front()->mpPostIt};
                    rWin.GetOutlinerView()->GetEditView().YrsApplyEEDelta(rState.pTxn, pEvent);
                    if ((rWin.GetStyle() & WB_DIALOGCONTROL) == 0)
                    {
                        rWin.UpdateData(); // not active window, force update
                    }
                    else
                    {   // apparently this repaints active window
                        rWin.GetOutlinerView()->GetEditView().Invalidate();
                    }
                }
                break;
                case Y_ARRAY:
                {
                    YArrayEvent const*const pEvent{&events[i].content.array};
                    Branch const*const pArray{yarray_event_target(pEvent)};
                    (void)pArray;
                    uint32_t lenP{0};
                    YPathSegment *const pPath{yarray_event_path(pEvent, &lenP)};
                    yvalidate(lenP == 2);
                    yvalidate(pPath[0].tag == Y_EVENT_PATH_KEY);
                    yvalidate(pPath[1].tag == Y_EVENT_PATH_INDEX);
                    yvalidate(pPath[1].value.index == 0); // this means position update, TODO anything else here?
                    // check that pArray is really that comment's position array
                    assert(pArray == yarray_get(
                        ymap_get(rState.rYrsSupplier.GetCommentMap(), pTxn, pPath[0].value.key)->value.y_type,
                        pTxn, pPath[1].value.index)->value.y_type);
                    OString const commentId{pPath[0].value.key};
                    uint32_t lenC{0};
                    YEventChange *const pChange{yarray_event_delta(pEvent, &lenC)};
                    // position update looks like this
                    yvalidate(lenC == 2);
                    yvalidate(pChange[0].tag == Y_EVENT_CHANGE_DELETE);
                    yvalidate(pChange[0].len == 2);
                    yvalidate(pChange[1].tag == Y_EVENT_CHANGE_ADD);
                    yvalidate(pChange[1].len == 2);
                    yvalidate(pChange[1].values[0].tag == Y_JSON_INT);
                    yvalidate(pChange[1].values[1].tag == Y_JSON_INT);
                    posUpdates.emplace_back(commentId, pChange[1].values[0].value.integer, pChange[1].values[1].value.integer);
                    yevent_delta_destroy(pChange, lenC);
                    ypath_destroy(pPath, lenP);
                }
                break;
                case Y_MAP:
                {
                    // new comment: lenP = 0, lenK = 1, Y_EVENT_KEY_CHANGE_ADD, Y_ARRAY
                    YMapEvent const*const pEvent{&events[i].content.map};
                    Branch const*const pMap{ymap_event_target(pEvent)};
                    (void)pMap;
                    yvalidate(pMap == rState.rYrsSupplier.GetCommentMap());
                    uint32_t lenP{0};
                    YPathSegment *const pPath{ymap_event_path(pEvent, &lenP)};
                    yvalidate(lenP == 0);
                    uint32_t lenK{0};
                    YEventKeyChange *const pChange{ymap_event_keys(pEvent, &lenK)};
                    for (decltype(lenK) j = 0; j < lenK; ++j)
                    {
                        switch (pChange[j].tag)
                        {
                            case Y_EVENT_KEY_CHANGE_ADD:
#if 1
                                switch (pChange[j].new_value->tag)
                                {
                                    case Y_JSON_INT:
                                        {
                                            int64_t const number{pChange[j].new_value->value.integer};
                                            (void)number;
                                            assert(false);
                                        }
                                        break;
                                    case Y_ARRAY:
                                        {
                                            Branch const*const pArray{pChange[j].new_value->value.y_type};
                                            // new comment
                                            yvalidate(yarray_len(pArray) == 3);
                                            ::std::unique_ptr<YOutput, YOutputDeleter> const pPos{yarray_get(pArray, pTxn, 0)};
                                            yvalidate(pPos->tag == Y_ARRAY);
                                            ::std::unique_ptr<YOutput, YOutputDeleter> const pNode{yarray_get(pPos->value.y_type, pTxn, 0)};
                                            yvalidate(pNode->tag == Y_JSON_INT);
                                            ::std::unique_ptr<YOutput, YOutputDeleter> const pContent{yarray_get(pPos->value.y_type, pTxn, 1)};
                                            yvalidate(pContent->tag == Y_JSON_INT);

                                            if (!newComments.insert({{pNode->value.integer, pContent->value.integer}, {pChange[j].key, pArray}}).second)
                                            {
                                                abort();
                                            }
                                        }
                                        break;
                                    case Y_MAP:
                                        {
                                            Branch const*const pMap2{pChange[j].new_value->value.y_type};
                                            (void)pMap2;
                                            assert(false);
                                        }
                                        break;
                                    case Y_TEXT:
                                        {
                                            // ???
                                            Branch const*const pText{pChange[j].new_value->value.y_type};
                                            (void)pText;
                                            assert(false);
                                        }
                                        break;
                                    default:
                                        assert(false);
                                }
#endif
                                break;
                            case Y_EVENT_KEY_CHANGE_DELETE:
                                switch (pChange[j].old_value->tag)
                                {
                                    case Y_ARRAY:
                                    {
                                        OString const commentId{pChange[j].key};
                                        auto const it{rState.rYrsSupplier.GetComments().find(commentId)};
                                        yvalidate(it != rState.rYrsSupplier.GetComments().end());
                                        SwWrtShell *const pShell{dynamic_cast<SwWrtShell*>(rState.rDoc.getIDocumentLayoutAccess().GetCurrentViewShell())};
                                        pShell->Push();
                                        *pShell->GetCursor()->GetPoint() = it->second.front()->GetAnchorPosition();
                                        pShell->SetMark();
                                        pShell->Right(SwCursorSkipMode::Chars, true, 1, false, false);
                                        pShell->DelRight();
                                        pShell->Pop(SwCursorShell::PopMode::DeleteStack);
                                        rState.rDoc.getIDocumentState().RemoveCommentImpl(commentId);
                                        break;
                                    }
                                    default:
                                        assert(false);
                                }
                                break;
                            case Y_EVENT_KEY_CHANGE_UPDATE:
                                assert(false);
                                break;
                            default:
                                assert(false);
                        }
                    }
                    yevent_keys_destroy(pChange, lenK);
                    ypath_destroy(pPath, lenP);
                }
                break;
                default:
                    assert(false);
            }
        }

        // insert into each node in order of content index! else pos is wrong
        for (auto const& it : newComments)
        {
            Branch const*const pArray{it.second.second};
            // new comment
            yvalidate(yarray_len(pArray) == 3);
            ::std::unique_ptr<YOutput, YOutputDeleter> const pPos{yarray_get(pArray, pTxn, 0)};
            yvalidate(pPos->tag == Y_ARRAY);
            ::std::unique_ptr<YOutput, YOutputDeleter> const pProps{yarray_get(pArray, pTxn, 1)};
            yvalidate(pProps->tag == Y_MAP);
            ::std::unique_ptr<YOutput, YOutputDeleter> const pText{yarray_get(pArray, pTxn, 2)};
            yvalidate(pText->tag == Y_TEXT);
#if 0
            ::std::unique_ptr<YOutput, YOutputDeleter> const pNode{yarray_get(pPos->value.y_type, pTxn, 0)};
            yvalidate(pNode->tag == Y_JSON_INT);
            ::std::unique_ptr<YOutput, YOutputDeleter> const pContent{yarray_get(pPos->value.y_type, pTxn, 1)};
            yvalidate(pContent->tag == Y_JSON_INT);
#endif
            ::std::optional<SwPosition> oStartPos;
            if (yarray_len(pPos->value.y_type) == 4)
            {
                ::std::unique_ptr<YOutput, YOutputDeleter> const pStartNode{
                    yarray_get(pPos->value.y_type, pTxn, 2)};
                ::std::unique_ptr<YOutput, YOutputDeleter> const pStartContent{
                    yarray_get(pPos->value.y_type, pTxn, 3)};
                auto const node{pStartNode->value.integer};
                yvalidate(SwNodeOffset{node} < rState.rDoc.GetNodes().Count());
                SwNode & rStartNode{*rState.rDoc.GetNodes()[SwNodeOffset{node}]};
                yvalidate(rStartNode.IsTextNode());
                auto const content{pStartContent->value.integer};
                yvalidate(content <= rStartNode.GetTextNode()->Len());
                oStartPos.emplace(*rStartNode.GetTextNode(), static_cast<sal_Int32>(content));
            }

            OUString author;
            {
                ::std::unique_ptr<YOutput, YOutputDeleter> const pAuthor{ymap_get(pProps->value.y_type, pTxn, "author")};
                yvalidate(!pAuthor || pAuthor->tag == Y_JSON_STR);
                if (pAuthor && pAuthor->tag == Y_JSON_STR)
                {
                    author = OUString(pAuthor->value.str, pAuthor->len, RTL_TEXTENCODING_UTF8);
                }
            }
            OUString initials;
            {
                ::std::unique_ptr<YOutput, YOutputDeleter> const pInitials{ymap_get(pProps->value.y_type, pTxn, "initials")};
                yvalidate(!pInitials|| pInitials->tag == Y_JSON_STR);
                if (pInitials && pInitials->tag == Y_JSON_STR)
                {
                    initials = OUString(pInitials->value.str, pInitials->len, RTL_TEXTENCODING_UTF8);
                }
            }
            DateTime date{DateTime::SYSTEM};
            {
                ::std::unique_ptr<YOutput, YOutputDeleter> const pDate{ymap_get(pProps->value.y_type, pTxn, "date")};
                yvalidate(!pDate|| pDate->tag == Y_JSON_STR);
                if (pDate && pDate->tag == Y_JSON_STR)
                {
                    util::DateTime unoDate;
                    if (::sax::Converter::parseDateTime(unoDate, pDate->value.str))
                    {
                        date = unoDate;
                    }
                }
            }
            bool isResolved{false};
            {
                ::std::unique_ptr<YOutput, YOutputDeleter> const pResolved{ymap_get(pProps->value.y_type, pTxn, "resolved")};
                yvalidate(!pResolved || pResolved->tag == Y_JSON_BOOL);
                if (pResolved && pResolved->tag == Y_JSON_BOOL)
                {
                    isResolved = pResolved->value.flag == Y_TRUE;
                }
            }
            yvalidate(SwNodeOffset{it.first.first} < rState.rDoc.GetNodes().Count());
            SwNode & rNode{*rState.rDoc.GetNodes()[SwNodeOffset{it.first.first}]};
            yvalidate(rNode.IsTextNode());
            yvalidate(it.first.second <= o3tl::make_unsigned(rNode.GetTextNode()->Len()));
            SwPosition anchorPos{*rNode.GetTextNode(), static_cast<sal_Int32>(it.first.second)};
            SAL_DEBUG("YRS " << anchorPos);
            SwWrtShell *const pShell{dynamic_cast<SwWrtShell*>(rState.rDoc.getIDocumentLayoutAccess().GetCurrentViewShell())};
            SwPostItFieldType* pType = static_cast<SwPostItFieldType*>(pShell->GetFieldType(0, SwFieldIds::Postit));
            auto pField{
                new SwPostItField(
                    pType,
                    author,
                    "", // content
                    initials,
                    OUString(), // name
                    date)};
            pField->SetResolved(isResolved);

//                                            rState.rYrsSupplier.InsertComment(pChange[j].key, anchorPos, pField);
            pShell->Push();
            *pShell->GetCursor()->GetPoint() = anchorPos;
            if (oStartPos)
            {
                pShell->SetMark();
                *pShell->GetCursor()->GetMark() = *oStartPos;
            }
            else
            {
                pShell->ClearMark();
            }
            bool const b = pShell->InsertField2Impl(*pField, nullptr, nullptr);
            assert(b);
            pShell->Pop(SwCursorShell::PopMode::DeleteCurrent);
            --anchorPos.nContent;
            OString const commentId{it.second.first};
            rState.rDoc.getIDocumentState().AddCommentImpl(anchorPos, commentId);

            rState.rYrsSupplier.GetComments().find(commentId)->second.front()->mpPostIt->GetOutlinerView()->GetEditView().YrsReadEEState(rState.pTxn);
        }

        // comments inserted, now check position updates for consistency
        for (auto & rUpdate : posUpdates)
        {
            auto const it{rState.rYrsSupplier.GetComments().find(::std::get<0>(rUpdate))};
            yvalidate(it != rState.rYrsSupplier.GetComments().end());
            SwPosition const pos{it->second.front()->GetAnchorPosition()};
            yvalidate(o3tl::make_unsigned(pos.GetNodeIndex().get()) == ::std::get<1>(rUpdate));
            yvalidate(o3tl::make_unsigned(pos.GetContentIndex()) == ::std::get<2>(rUpdate));
        }
    }
    extern "C" void observe_cursors(void *const pState, uint32_t count, YEvent const*const events)
    {
#if 1
        (void) pState;
        (void) count;
        (void) events;
#endif
    }

    void writeLength(sal_Int8 *& rpBuf, sal_Int32 const len)
    {
        *rpBuf = (len >>  0) & 0xFF;
        ++rpBuf;
        *rpBuf = (len >>  8) & 0xFF;
        ++rpBuf;
        *rpBuf = (len >> 16) & 0xFF;
        ++rpBuf;
        *rpBuf = (len >> 24) & 0xFF;
        ++rpBuf;
    }
} // namespace

    IMPL_LINK(YrsThread, HandleMessage, void*, pVoid, void)
    {
        SAL_DEBUG("YRS HandleMessage");
        DBG_TESTSOLARMUTEX();
        if (!m_pDSM)
        {
            SAL_DEBUG("m_pDSM died");
            return;
        }
        // wrap this, not strictly needed but can't hurt?
        m_pDSM->m_rDoc.getIDocumentLayoutAccess().GetCurrentViewShell()->StartAllAction();
        ::std::unique_ptr<uno::Sequence<sal_Int8>> const pBuf{static_cast<uno::Sequence<sal_Int8>*>(pVoid)};
        uint32_t const length{pBuf->size()};
        assert(length != 0);
        switch ((*pBuf)[0])
        {
            case ::std::underlying_type_t<Message>(Message::RequestStateVector):
            {
                SAL_DEBUG("sending state vector");
                YTransaction *const pTxn{m_pDSM->m_pYrsSupplier->GetWriteTransaction()};
                uint32_t len{0};
                char * pSV = ytransaction_state_vector_v1(pTxn, &len);
                uno::Sequence<sal_Int8> buf(5 + len);
                sal_Int8 * it{buf.getArray()};
                writeLength(it, len+1);
                *it = ::std::underlying_type_t<Message>(Message::SendStateVector);
                ++it;
                ::std::copy(pSV, pSV+len, it);
                try
                {
                    m_xConnection->write(buf);
                }
                catch (io::IOException const&)
                {
                    TOOLS_WARN_EXCEPTION("sw", "YRS HandleMessage");
                }
                m_pDSM->m_pYrsSupplier->CommitTransaction();
                ybinary_destroy(pSV, len);
                break;
            }
            case ::std::underlying_type_t<Message>(Message::SendStateVector):
            {
                SAL_DEBUG("received state vector");
                YTransaction *const pTxn{m_pDSM->m_pYrsSupplier->GetWriteTransaction()};
                uint32_t len{0};
                char * pUpdate = ytransaction_state_diff_v1(pTxn,
                    reinterpret_cast<char const*>(pBuf->begin()) + 1, length - 1, &len);
                uno::Sequence<sal_Int8> buf(5 + len);
                sal_Int8 * it{buf.getArray()};
                writeLength(it, len+1);
                *it = ::std::underlying_type_t<Message>(Message::SendStateDiff);
                ++it;
                ::std::copy(pUpdate, pUpdate+len, it);
                try
                {
                    m_xConnection->write(buf);
                }
                catch (io::IOException const&)
                {
                    TOOLS_WARN_EXCEPTION("sw", "YRS HandleMessage");
                }
                m_pDSM->m_pYrsSupplier->CommitTransaction();
                ybinary_destroy(pUpdate, len);
                break;
            }
            case ::std::underlying_type_t<Message>(Message::SendStateDiff):
            {
                SAL_DEBUG("apply update: " << yupdate_debug_v1(reinterpret_cast<char const*>(pBuf->begin()) + 1, length - 1));
                YTransaction *const pTxn{m_pDSM->m_pYrsSupplier->GetWriteTransaction()};
                m_pDSM->m_pYrsSupplier->SetMode(IYrsTransactionSupplier::Mode::Replay);
                auto const err = ytransaction_apply(pTxn, reinterpret_cast<char const*>(pBuf->begin()) + 1, length - 1);
                if (err != 0)
                {
                    SAL_DEBUG("ytransaction_apply error " << err);
                    abort();
                }
                // let's have one observe_deep instead of a observe on every
                // YText - need to be notified on new comments being created...
                ObserveState state{*m_pDSM->m_pYrsSupplier, m_pDSM->m_rDoc, pTxn};
                YSubscription *const pSubComments = yobserve_deep(m_pDSM->m_pYrsSupplier->GetCommentMap(), &state, observe_comments);
                YSubscription *const pSubCursors = yobserve_deep(m_pDSM->m_pYrsSupplier->GetCursorMap(), &state, observe_cursors);
                m_pDSM->m_pYrsSupplier->CommitTransaction(true);
                m_pDSM->m_pYrsSupplier->SetMode(IYrsTransactionSupplier::Mode::Edit);
                yunobserve(pSubComments);
                yunobserve(pSubCursors);
                break;
            }
        }
        //yweak_observe for cursors ???
        //ydoc_observe_updates_v1 -> no, that seems useless
        //ydoc_observe_after_transaction ??? too
        m_pDSM->m_rDoc.getIDocumentLayoutAccess().GetCurrentViewShell()->EndAllAction();
    }

    IYrsTransactionSupplier::Mode DocumentStateManager::SetYrsMode(IYrsTransactionSupplier::Mode const mode)
    {
        return m_pYrsSupplier->SetMode(mode);
    }

    void DocumentStateManager::AddCommentImpl(/*YTransaction *const pTxn, YDoc const*const pDoc,*/ SwPosition const& rAnchorPos/*, SwPosition pEnd, SwPostIt pComment*/, OString const& commentId)
    {
        SAL_DEBUG("YRS AddCommentImpl");
        ::std::vector<SwAnnotationItem *> items;
        // ??? TODO how should this work for multiple viewshells? every shell has its own EditEngine? unclear.
        for (SwViewShell & rShell : m_rDoc.getIDocumentLayoutAccess().GetCurrentViewShell()->GetRingContainer())
        {
//unnecessary?            if (SwWrtShell *const pShell{dynamic_cast<SwWrtShell*>(&rShell)})
            {
                for (::std::unique_ptr<SwAnnotationItem> const& it : *rShell.GetPostItMgr())
                {
                    if (it->GetAnchorPosition() == rAnchorPos)
                    {
                        items.emplace_back(it.get());
#if 0
                        assert(rShell.GetPostItMgr()->GetLatestPostItField() == it->GetFormatField().GetField());
                        SwAnnotationWin *const pWin{
                            /*rShell.GetPostItMgr().GetSidebarWin(it->GetFormatField().GetField())*/
                            /*it->mpPostIt.get()*/
                            /*rShell.GetPostItMgr()->GetOrCreateAnnotationWindow(*it)*/
                            rShell.GetPostItMgr()->GetOrCreateAnnotationWindowForLatestPostItField()
                        };
#endif
                        // for a loaded document, GetOrCreateAnnotationWindowForLatestPostItField() cannot be used
                        SwAnnotationWin *const pWin{
                            rShell.GetPostItMgr()->GetOrCreateAnnotationWindow(*it)};
                        assert(pWin);
                        pWin->GetOutlinerView()->GetEditView().SetYrsCommentId(m_pYrsSupplier.get(), commentId);
                    }
                }
            }
        }
        m_pYrsSupplier->m_Comments.emplace(commentId, items);
    }

    void DocumentStateManager::AddComment(SwPosition const& rPos,
        ::std::optional<SwPosition> const oAnchorStart, SwPostItField const& rField,
        bool const isInsert)
    {
        // need to SetYrsTransactionSupplier somewhere - but its missing the 1st para insert ? ... but that needs handling anyway bc loaded doc ?
        // ... SetYrsTransactionSupplier could insert current state in Y?
        // ... or: pass Y to ctor? but does that work for file import?
        SAL_DEBUG("YRS AddComment " << rPos);
        OString const commentId{m_pYrsSupplier->GenNewCommentId()};
        // this calls EditViewInvalidate so prevent destroying pTxn
        AddCommentImpl(rPos, commentId);
        //SwPosition const anchorPos{rPos.nContent, -1};
        YTransaction *const pTxn{m_pYrsSupplier->GetWriteTransaction()};
        // first, adjust position of all other comments in the paragraph
        if (isInsert)
        {
#if 0 // no way to id?
            SwTextNode const*const pNode{rPos.GetNode().GetTextNode()};
            for (SwTextAttr const*const pHint : pNode->GetSwpHints())
            {
                if (pHint->Which() == RES_TXTATR_ANNOTATION
                    && rPos.GetContentIndex() <= pHint->GetStart())
                {
                    auto const pField{dynamic_cast<const SwPostItField*>(GetFormatField().GetField())};
                }
            }
#endif
            for (auto const& it : m_pYrsSupplier->m_Comments)
            {
                // TODO update the starts too???
                SwPosition const& rItPos{it.second.front()->GetAnchorPosition()};
                if (rPos.nNode == rItPos.nNode && rPos.nContent <= rItPos.nContent
                    && it.second.front()->mpPostIt->GetOutlinerView()->GetEditView().GetCommentId() != commentId)
                {
                    ::std::unique_ptr<YOutput, YOutputDeleter> const pComment{ymap_get(m_pYrsSupplier->m_pComments, pTxn, it.first.getStr())};
                    assert(pComment);
                    ::std::unique_ptr<YOutput, YOutputDeleter> const pPos{yarray_get(pComment->value.y_type, pTxn, 0)};
                    assert(pPos);
                    ::std::unique_ptr<YOutput, YOutputDeleter> const pPosN{yarray_get(pPos->value.y_type, pTxn, 0)};
                    ::std::unique_ptr<YOutput, YOutputDeleter> const pPosC{yarray_get(pPos->value.y_type, pTxn, 1)};
                    // SwTextNode::Update already moved rItPos
                    assert(pPosN->value.integer == rItPos.GetNodeIndex().get());
                    assert(pPosC->value.integer + 1 == rItPos.GetContentIndex());
                    yarray_remove_range(pPos->value.y_type, pTxn, 0, 2);
                    YInput const anchorNode{yinput_long(rItPos.GetNodeIndex().get())};
                    YInput const anchorContent{yinput_long(rItPos.GetContentIndex())};
                    YInput posArray[]{anchorNode, anchorContent/*, anchorStartNode, anchorStartContent*/};
                    yarray_insert_range(pPos->value.y_type, pTxn, 0, posArray, 2);
                }
            }
        }
        YInput const anchorNode{yinput_long(rPos.GetNodeIndex().get())};
        YInput const anchorContent{yinput_long(rPos.GetContentIndex())};
        YInput const anchorStartNode{oAnchorStart ? yinput_long(oAnchorStart->GetNodeIndex().get()) : yinput_undefined()};
        YInput const anchorStartContent{oAnchorStart ? yinput_long(oAnchorStart->GetContentIndex()) : yinput_undefined()};
        YInput posArray[]{anchorNode, anchorContent, anchorStartNode, anchorStartContent};
        YInput const anchor{yinput_yarray(posArray, oAnchorStart ? 4 : 2)};
        // TODO SwPostItField parent stuff?
        OString const authorString{OUStringToOString(rField.GetPar1(), RTL_TEXTENCODING_UTF8)};
        OString const initialsString{OUStringToOString(rField.GetInitials(), RTL_TEXTENCODING_UTF8)};
        OUStringBuffer dateBuf;
        ::sax::Converter::convertDateTime(dateBuf, rField.GetDateTime().GetUNODateTime(), nullptr, true);
        OString const dateString{OUStringToOString(dateBuf.makeStringAndClear(), RTL_TEXTENCODING_UTF8)};
        char const*const propsNames[]{ "author", "initials", "date", "resolved" };
        YInput const author{yinput_string(authorString.getStr())};
        YInput const initials{yinput_string(initialsString.getStr())};
        YInput const date{yinput_string(dateString.getStr())};
        YInput const resolved{yinput_bool(rField.GetResolved() ? Y_TRUE : Y_FALSE)};
        YInput propsArray[]{author, initials, date, resolved};
        YInput const properties{yinput_ymap(const_cast<char**>(propsNames), propsArray, 4)};
        YInput const text{yinput_ytext(const_cast<char*>(""))};
        YInput commentArray[]{anchor, properties, text};
        YInput const comment{yinput_yarray(commentArray, 3)};
        ymap_insert(m_pYrsSupplier->m_pComments, pTxn, commentId.getStr(), &comment);
        // just use first one?
        // or check which one is active = (GetStyle() & WB_DIALOGCONTROL)
        m_pYrsSupplier->m_Comments.find(commentId)->second.front()->mpPostIt->GetOutlinerView()->GetEditView().YrsWriteEEState();

        // either update the cursors here, or wait for round-trip?
        //Branch *const pCursors(ymap(pDoc.get(), "cursors"));
//do it in 1 caller so that load document can batch it?    CommitModified(); // SetModified is called earlier
    }

    void DocumentStateManager::RemoveCommentImpl(OString const& rCommentId)
    {
        assert(!rCommentId.isEmpty());
        m_pYrsSupplier->m_Comments.erase(rCommentId);
    }

    void DocumentStateManager::RemoveComment(SwPosition const& rPos, OString const& rCommentId)
    {
        SAL_DEBUG("YRS RemoveComment");
        RemoveCommentImpl(rCommentId);
        YTransaction *const pTxn{m_pYrsSupplier->GetWriteTransaction()};
        if (!pTxn)
        {
            return;
        }

//        Branch *const pComments{ymap(m_pYrsSupplier->GetYDoc(), "comments")};
        ymap_remove(m_pYrsSupplier->m_pComments, pTxn, rCommentId.getStr());

        // first, adjust position of all other comments in the paragraph
        for (auto const& it : m_pYrsSupplier->m_Comments)
        {
            // TODO update the starts too???
            SwPosition const& rItPos{it.second.front()->GetAnchorPosition()};
            if (rPos.nNode == rItPos.nNode && rPos.nContent <= rItPos.nContent)
            {
                ::std::unique_ptr<YOutput, YOutputDeleter> const pComment{ymap_get(m_pYrsSupplier->m_pComments, pTxn, it.first.getStr())};
                assert(pComment);
                ::std::unique_ptr<YOutput, YOutputDeleter> const pPos{yarray_get(pComment->value.y_type, pTxn, 0)};
                assert(pPos);
                ::std::unique_ptr<YOutput, YOutputDeleter> const pPosN{yarray_get(pPos->value.y_type, pTxn, 0)};
                ::std::unique_ptr<YOutput, YOutputDeleter> const pPosC{yarray_get(pPos->value.y_type, pTxn, 1)};
                assert(pPosN->value.integer == rItPos.GetNodeIndex().get());
                assert(pPosC->value.integer == rItPos.GetContentIndex());
                yarray_remove_range(pPos->value.y_type, pTxn, 0, 2);
                YInput const anchorNode{yinput_long(rItPos.GetNodeIndex().get())};
                YInput const anchorContent{yinput_long(rItPos.GetContentIndex() - 1)};
                YInput posArray[]{anchorNode, anchorContent/*, anchorStartNode, anchorStartContent*/};
                yarray_insert_range(pPos->value.y_type, pTxn, 0, posArray, 2);
            }
        }
        // either update the cursors here, or wait for round-trip?
    }

    //void UpdateCursor(YTransaction *const pTxn, YDoc *const pYDoc)
    void DocumentStateManager::UpdateCursor()
    {
        SAL_DEBUG("YRS UpdateCursor");
        YTransaction *const pTxn{m_pYrsSupplier->GetWriteTransaction()};
        YDoc *const pYDoc{m_pYrsSupplier->GetYDoc()};
//        Branch *const pCursors{ymap(pYDoc, "cursors")};
        auto const id{ydoc_id(pYDoc)};
        SwWrtShell *const pShell{dynamic_cast<SwWrtShell*>(m_rDoc.getIDocumentLayoutAccess().GetCurrentViewShell())};
        if (sw::annotation::SwAnnotationWin *const pWin{
                pShell->GetView().GetPostItMgr()->GetActiveSidebarWin()})
        {
            // TODO StickyIndex cannot be inserted into YDoc ?
            ESelection const sel{pWin->GetOutlinerView()->GetSelection()};
            ::std::vector<YInput> positions;
            // the ID of the comment
            positions.push_back(yinput_string(pWin->GetOutlinerView()->GetEditView().GetCommentId().getStr()));
            positions.push_back(yinput_long(sel.start.nPara));
            positions.push_back(yinput_long(sel.start.nIndex));
            if (pWin->GetOutlinerView()->HasSelection())
            {
                positions.push_back(yinput_long(sel.end.nPara));
                positions.push_back(yinput_long(sel.end.nIndex));
                YInput const input{yinput_yarray(positions.data(), 5)};
                ymap_insert(m_pYrsSupplier->m_pCursors, pTxn, OString::number(id).getStr(), &input);
            }
            else
            {
                YInput const input{yinput_yarray(positions.data(), 3)};
                ymap_insert(m_pYrsSupplier->m_pCursors, pTxn, OString::number(id).getStr(), &input);
            }
        }
        else
        if (!pShell->IsStdMode()
//            || pShell->IsObjSelected()
            || pShell->IsTableMode()
            || pShell->IsBlockMode())
        {
            YInput const input{yinput_null()};
            ymap_insert(m_pYrsSupplier->m_pCursors, pTxn, OString::number(id).getStr(), &input);
        }
        else
        {
            SwPaM const*const pCursor{pShell->GetCursor()};
            ::std::vector<YInput> positions;
            positions.push_back(yinput_long(pCursor->GetPoint()->GetNodeIndex().get()));
            positions.push_back(yinput_long(pCursor->GetPoint()->GetContentIndex()));
            if (pCursor->HasMark())
            {
                positions.push_back(yinput_long(pCursor->GetMark()->GetNodeIndex().get()));
                positions.push_back(yinput_long(pCursor->GetMark()->GetContentIndex()));
                YInput const input{yinput_yarray(positions.data(), 4)};
                ymap_insert(m_pYrsSupplier->m_pCursors, pTxn, OString::number(id).getStr(), &input);
            }
            else
            {
                YInput const input{yinput_yarray(positions.data(), 2)};
                ymap_insert(m_pYrsSupplier->m_pCursors, pTxn, OString::number(id).getStr(), &input);
            }
        }
    }

void DocumentStateManager::InitAcceptor()
{
    if (!getenv("YRSACCEPT") || m_pYrsReader.is())
    {
        return;
    }
    try
    {
        auto const conn = u"pipe,name=ytest"_ustr;
        auto const xContext{comphelper::getProcessComponentContext()};
        SAL_DEBUG("YRS accept");
        m_xAcceptor = css::connection::Acceptor::create(xContext);
        // TODO move to thread?
        uno::Reference<connection::XConnection> xConnection = m_xAcceptor->accept(conn);
        uno::Sequence<sal_Int8> buf(4);
        uno::Sequence<sal_Int8> data;
        if (SfxMedium const*const pMedium{m_rDoc.GetDocShell()->GetMedium()})
        {
            OUString const url{pMedium->GetOrigURL()};
            if (!url.isEmpty())
            {
                try
                {
                    SAL_DEBUG("YRS send file: " << url);
                    ::ucbhelper::Content temp{url, {}, xContext};
                    uno::Reference<io::XInputStream> const xInStream{temp.openStreamNoLock()};
                    uno::Reference<io::XSeekable> const xSeekable{xInStream, uno::UNO_QUERY};
                    if (xSeekable.is())
                    {
                        auto const len(xSeekable->getLength() - xSeekable->getPosition());
                        if (xInStream->readBytes(data, len) != len)
                        {
                            throw uno::RuntimeException(u"short readBytes"_ustr);
                        }
                    }
                    else
                    {
                        ::std::vector<uno::Sequence<sal_Int8>> bufs;
                        bool isDone{false};
                        do
                        {
                            bufs.emplace_back();
                            isDone = xInStream->readSomeBytes(bufs.back(), 65536) == 0;
                        } while (!isDone);
                        sal_Int32 nSize{0};
                        for (auto const& rBuf : bufs)
                        {
                            if (o3tl::checked_add(nSize, rBuf.getLength(), nSize))
                            {
                                throw std::bad_alloc(); // too large for Sequence
                            }
                        }
                        size_t nCopied{0};
                        data.realloc(nSize);
                        for (auto const& rBuf : bufs)
                        {
                            ::std::memcpy(data.getArray() + nCopied, rBuf.getConstArray(), rBuf.getLength());
                            nCopied += rBuf.getLength(); // can't overflow
                        }
                    }
                    sal_Int8 * it{buf.getArray()};
                    writeLength(it, data.getLength());
                }
                catch (...)
                {
                    abort();
                }
            }
        }
        xConnection->write(buf);
        xConnection->write(data);
        m_pYrsReader = new YrsThread(xConnection, *this);
        //m_xAcceptor->stopAccepting();
        //m_xConnection = xConnection;
        m_pYrsReader->launch();
        SAL_DEBUG("YRS started");
        // inserting comments needs sidebar wins so needs a view shell first
        SwFieldType & rType{*m_rDoc.getIDocumentFieldsAccess().GetFieldType(SwFieldIds::Postit, OUString(), false)};
        std::vector<SwFormatField*> fields;
        rType.GatherFields(fields);
        for (SwFormatField const*const pField : fields)
        {
            SwPostItField const& rField{*dynamic_cast<SwPostItField const*>(pField->GetField())};
            SwTextAnnotationField const& rHint{*static_cast<SwTextAnnotationField const*>(pField->GetTextField())};
            SwPosition const pos{rHint.GetTextNode(), rHint.GetStart()};
            ::std::optional<SwPosition> oAnchorStart;
            if (::sw::mark::AnnotationMark const*const pMark{rHint.GetAnnotationMark()})
            {
                oAnchorStart.emplace(pMark->GetMarkStart());
            }
            AddComment(pos, oAnchorStart, rField, false);
        }
        // initiate sync of comments to other side
    //AddComment would have done this    m_pYrsSupplier->GetWriteTransaction();
        CommitModified();
    }
    catch (uno::Exception const&) // exception here will cause UAF from SwView later
    {
        DBG_UNHANDLED_EXCEPTION("sw");
        abort();
    }
}

void DocumentStateManager::InitConnector(uno::Any const& raConnection)
{
    assert(!m_pYrsReader);
    uno::Reference<connection::XConnection> xConnection;
    raConnection >>= xConnection;
    assert(xConnection.is());

    // delete all fields that were loaded from a document - they must be
    // inserted via yrs
    // cannot call SwPostItMgr::Delete() because no view shell yet
    SwFieldType & rType{*m_rDoc.getIDocumentFieldsAccess().GetFieldType(SwFieldIds::Postit, OUString(), false)};
    std::vector<SwFormatField*> fields;
    rType.GatherFields(fields);
    for (SwFormatField *const pField : fields)
    {
        SwTextField const*const pHint{pField->GetTextField()};
        if (pHint)
        {
            SwTextField::DeleteTextField(*pHint);
        }
    }
    // don't request comments to be sent here - let the other side just commit, that will send them
    // TODO ... but we get an undo action per comment, which is unfortunate

    m_pYrsReader = new YrsThread(xConnection, *this);
    m_pYrsReader->launch();
    SAL_DEBUG("YRS started (InitConnector)");
}

#endif

DocumentStateManager::DocumentStateManager( SwDoc& i_rSwdoc ) :
    m_rDoc( i_rSwdoc ),
    mbEnableSetModified(true),
    mbModified(false),
    mbUpdateExpField(false),
    mbNewDoc(false),
    mbInCallModified(false)
{
#if defined(YRS)
    m_pYrsSupplier.reset(new YrsTransactionSupplier);
#if 0
    auto const conn = u"pipe,name=ytest"_ustr;
    auto const xContext{comphelper::getProcessComponentContext()};
    if (getenv("YRSACCEPT"))
    {
        SAL_DEBUG("YRS accept");
        m_xAcceptor = css::connection::Acceptor::create(xContext);
        // TODO move to thread?
        uno::Reference<connection::XConnection> xConnection = m_xAcceptor->accept(conn);
        // TODO write file - but cant get model, too early
        // move it to loadfinished ?
            uno::Sequence<sal_Int8> buf(4);
            xConnection->write(buf);
        m_pYrsReader = new YrsThread(xConnection, *this);
        //m_xAcceptor->stopAccepting();
        //m_xConnection = xConnection;
        m_pYrsReader->launch();
        SAL_DEBUG("YRS started");
    }
#endif
#if 0
    else
    {
        SAL_DEBUG("YRS connect");
        m_xConnector = css::connection::Connector::create(xContext);
        uno::Reference<connection::XConnection> xConnection = m_xConnector->connect(conn);
        m_pYrsReader = new YrsThread(xConnection, *this);
        //m_xConnection = xConnection;
        m_pYrsReader->launch();
        SAL_DEBUG("YRS started");
    }
#endif
#endif
}

DocumentStateManager::~DocumentStateManager()
{
#if defined(YRS)
    if (m_pYrsReader) // not in e.g. AutoText docs
    {
        m_pYrsReader->m_xConnection->close();
        m_pYrsReader->m_pDSM = nullptr;
        m_pYrsReader->join();
    }
#endif
}

void DocumentStateManager::SetModified()
{
    if (!IsEnableSetModified())
        return;

    m_rDoc.GetDocumentLayoutManager().ClearSwLayouterEntries();
    mbModified = true;
    m_rDoc.GetDocumentStatisticsManager().SetDocStatModified( true );
    if( m_rDoc.GetOle2Link().IsSet() )
    {
        mbInCallModified = true;
        m_rDoc.GetOle2Link().Call( true );
        mbInCallModified = false;
    }

    if( m_rDoc.GetAutoCorrExceptWord() && !m_rDoc.GetAutoCorrExceptWord()->IsDeleted() )
        m_rDoc.DeleteAutoCorrExceptWord();

#if defined(YRS)
    SAL_DEBUG("YRS SetModified");
    // FIXME: this is called only on LoseFocus! not while editing comment
    CommitModified();
#endif
}

#if defined(YRS)
void DocumentStateManager::CommitModified()
{
    if (m_pYrsSupplier->CommitTransaction())
    {
        uno::Sequence<sal_Int8> buf(5);
        sal_Int8 * it{buf.getArray()};
        writeLength(it, 1);
        *it = ::std::underlying_type_t<Message>(Message::RequestStateVector);
        try {
            m_pYrsReader->m_xConnection->write(buf);
        }
        catch (io::IOException const&)
        {
            TOOLS_WARN_EXCEPTION("sw", "YRS CommitTransaction");
            m_pYrsReader->m_xConnection->close();
        }
    }
}
#endif


void DocumentStateManager::ResetModified()
{
    // give the old and new modified state to the link
    //  Bit 0:  -> old state
    //  Bit 1:  -> new state
    bool bOldModified = mbModified;
    mbModified = false;
    m_rDoc.GetDocumentStatisticsManager().SetDocStatModified( false );
    m_rDoc.GetIDocumentUndoRedo().SetUndoNoModifiedPosition();
    if( bOldModified && m_rDoc.GetOle2Link().IsSet() )
    {
        mbInCallModified = true;
        m_rDoc.GetOle2Link().Call( false );
        mbInCallModified = false;
    }
}

bool DocumentStateManager::IsModified() const
{
    return mbModified;
}

bool DocumentStateManager::IsEnableSetModified() const
{
    return mbEnableSetModified;
}

void DocumentStateManager::SetEnableSetModified(bool bEnableSetModified)
{
    mbEnableSetModified = bEnableSetModified;
}

bool DocumentStateManager::IsInCallModified() const
{
    return mbInCallModified;
}

bool DocumentStateManager::IsUpdateExpField() const
{
    return mbUpdateExpField;
}

bool DocumentStateManager::IsNewDoc() const
{
    return mbNewDoc;
}

void DocumentStateManager::SetNewDoc(bool b)
{
    mbNewDoc = b;
}

void DocumentStateManager::SetUpdateExpFieldStat(bool b)
{
    mbUpdateExpField = b;
}

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
