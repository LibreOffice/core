/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <vcl/event.hxx>
#include <annotationmark.hxx>
#include <svtools/ctrlbox.hxx>
#include <rtl/ustring.hxx>
#include <memory>

#include <sfx2/sidebar/PanelLayout.hxx>
#include <sfx2/sidebar/ControllerItem.hxx>
#include <AnnotationWin.hxx>
#include <svl/poolitem.hxx>
#include <tools/link.hxx>
#include <vcl/weld.hxx>
#include <PostItMgr.hxx>
#include <postithelper.hxx>
#include <view.hxx>
#include <tools/date.hxx>
#include <tools/datetime.hxx>
#include <tools/time.hxx>

class SwWrtShell;
class SwView;
class SwPostItField;
class SwFormatField;
class SwAnnotationWin;
class SfxBroadcaster;
class SwPostItMgr;
class SwAnnotationItem;

namespace sw::sidebar
{
class CommentsPanel;
class Comment final
{
    friend class CommentsPanel;

private:
    std::unique_ptr<weld::Builder> mxBuilder;
    std::unique_ptr<weld::Container> mxContainer;
    std::unique_ptr<weld::Expander> mxExpander;
    std::unique_ptr<weld::Label> mxAuthor;
    std::unique_ptr<weld::Label> mxDate;
    std::unique_ptr<weld::Label> mxTime;
    std::unique_ptr<weld::Button> mxReply;
    std::unique_ptr<weld::CheckButton> mxResolve;
    std::unique_ptr<weld::TextView> mxTextView;

    sw::sidebar::CommentsPanel& mrCommentsPanel;
    OUString msText;
    OUString msAuthor;
    Date maDate;
    tools::Time maTime;

    void makeEditable() { mxTextView->set_editable(true); }

public:
    Comment(weld::Container* pParent, CommentsPanel& rCommentsPanel);
    ~Comment();
    weld::Widget* get_widget() const { return mxContainer.get(); }
    weld::TextView* getTextView() const { return mxTextView.get(); }

    DECL_LINK(ReplyClicked, weld::Button&, void);
    DECL_LINK(ResolveClicked, weld::Toggleable&, void);
    DECL_LINK(OnFocusOut, weld::Widget&, void);
    DECL_LINK(ContextMenuHdl, const MouseEvent&, bool);

    bool mbResolved;

    void InitControls(const SwPostItField* pPostItField);
    void SetCommentText(OUString sText) { msText = std::move(sText); }
    const OUString& GetAuthor() { return msAuthor; }
    const Date& GetDate() { return maDate; }
};

class Thread final
{
    friend class CommentsPanel;

private:
    std::unique_ptr<weld::Builder> mxBuilder;
    std::unique_ptr<weld::Container> mxContainer;
    std::unique_ptr<weld::Expander> mxExpander;
    std::unique_ptr<weld::Box> mxCommentBox;

public:
    Thread(weld::Container* pParent);
    ~Thread();
    weld::Widget* get_widget() const { return mxContainer.get(); }
    weld::Expander* getExpander() const { return mxExpander.get(); }
    weld::Box* getCommentBoxWidget() const { return mxCommentBox.get(); }

    sal_uInt16 mnComments = 0;
};

class CommentsPanel : public PanelLayout,
                      public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface,
                      public SfxListener
{
public:
    static std::unique_ptr<PanelLayout> Create(weld::Widget* pParent);

    virtual void NotifyItemUpdate(const sal_uInt16 nSId, const SfxItemState eState,
                                  const SfxPoolItem* pState) override;

    virtual void GetControlState(const sal_uInt16 /*nSId*/,
                                 boost::property_tree::ptree& /*rState*/) override{};

    CommentsPanel(weld::Widget* pParent);
    virtual ~CommentsPanel() override;

    void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override;

    void EditComment(Comment* pComment);
    void ToggleResolved(Comment* pComment);
    void ReplyComment(Comment* pComment);
    void DeleteComment(Comment* pComment);
    void DeleteThread(Comment* pComment);
    void ResolveThread(Comment* pComment);

    DECL_LINK(FilterByAuthor, weld::ComboBox&, void);
    DECL_LINK(FilterByDate, SvtCalendarBox&, void);
    DECL_LINK(ResetDate, weld::Button&, void);
    DECL_LINK(ShowTimeHdl, weld::Toggleable&, void);
    DECL_LINK(ShowResolvedHdl, weld::Toggleable&, void);
    DECL_LINK(SortHdl, weld::Toggleable&, void);

    // utility functions
    static OUString FormatDate(Date& rDate);
    static OUString FormatTime(tools::Time& rTime);

    weld::Toggleable& getShowResolved() { return *mxShowResolved; }

private:
    SwDoc* mpDoc;
    SwPostItMgr* mpPostItMgr;

    std::unordered_map<sal_uInt32, std::unique_ptr<Thread>> mpThreadsMap;
    std::unordered_map<sal_uInt32, std::unique_ptr<Comment>> mpCommentsMap;
    std::unordered_set<OUString> mpAuthorSet;

    std::unique_ptr<weld::ComboBox> mxFilterAuthor;
    std::unique_ptr<SvtCalendarBox> mxFilterDate;
    std::unique_ptr<weld::Button> mxResetDate;
    std::unique_ptr<weld::CheckButton> mxShowTime;
    std::unique_ptr<weld::CheckButton> mxShowResolved;
    std::unique_ptr<weld::CheckButton> mxShowReference;
    std::unique_ptr<weld::RadioButton> mxSortbyPosition;
    std::unique_ptr<weld::RadioButton> mxSortbyTime;
    std::unique_ptr<weld::Box> mxThreadsContainer;

    sal_uInt16 mnThreads = 0;
    bool mbDateSelected = false;

    // utility functions
    sw::annotation::SwAnnotationWin* getRootCommentWin(const SwFormatField* pField);
    static sal_uInt32 getPostItId(sw::annotation::SwAnnotationWin* pAnnotationWin);
    sw::annotation::SwAnnotationWin* getAnnotationWin(Comment* pComment);
    static bool comp_dateTime(SwFormatField* a, SwFormatField* b);
    static SwPosition getAnchorPosition(SwFormatField* pField);
    static bool comp_position(SwFormatField* a, SwFormatField* b);

    // event handlers (To sync with AnnotationWin)
    void populateComments();
    void addComment(const SwFormatField* pField);
    void deleteComment(sal_uInt32 nId);
    void setResolvedStatus(sw::annotation::SwAnnotationWin* pAnnotationWin);
    static void editComment(SwPostItField* pPostItField, Comment* pComment);
    static OUString getReferenceText(SwTextNode* pTextNode, sw::mark::AnnotationMark* pMark);
    void setReferenceText(sal_uInt32 nRootId);

    void populateAuthorComboBox();
};

} //end of namespace sw::sidebar
