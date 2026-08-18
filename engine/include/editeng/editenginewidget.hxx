/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <editeng/editengdllapi.h>
#include <rtl/ustring.hxx>
#include <vcl/customweld.hxx>
#include <vcl/idle.hxx>

class EditEngine;
class EditStatus;
class EditView;
class Outliner;
class SfxItemSet;

namespace tools
{
class JsonWriter;
}

/**
 * Adapts an EditEngine and its EditView to a client-rendered custom widget.
 *
 * The custom widget type is "editengine". DumpWidgetData() writes the text model as JSON:
 * one entry per paragraph, each carrying its text, its outline depth and bullet text, its
 * alignment, and a list of character runs with the formatting that applies to them. The caret
 * or selection travels with it as paragraph plus index pairs, which map straight onto
 * ESelection.
 *
 * HandleCustomEvent() takes input back. Keystrokes arrive as UNO key codes and become real
 * KeyEvent objects handed to EditView::PostKeyEvent, so paragraph splitting, outline promote
 * and demote, word navigation, autocorrect and undo grouping all behave as they do on the
 * desktop. Composed text and pasted text cannot be expressed as a keystroke and arrive as
 * their own command.
 *
 * The EditEngine and the EditView are owned elsewhere and must outlive the controller. The owner
 * also decides when a new model goes out, by calling QueueUpdate(). The controller does not take
 * the EditEngine modify or status handlers over, because the owner usually needs them itself.
 */
class EDITENG_DLLPUBLIC EditEngineWidgetController : public weld::CustomClientWidgetController
{
private:
    EditView& m_rEditView;
    EditEngine& m_rEditEngine;
    /// Supplies outline depth and bullet text. Null for a plain EditEngine.
    Outliner* m_pOutliner;
    /// Collects the model changes of one burst of keystrokes into a single message.
    Idle m_aUpdateIdle;

    DECL_DLLPRIVATE_LINK(UpdateIdleHdl, Timer*, void);

    void DumpParagraph(tools::JsonWriter& rWriter, sal_Int32 nPara);
    void DumpRuns(tools::JsonWriter& rWriter, sal_Int32 nPara, const SfxItemSet& rParagraphBaseline);

    bool HandleKey(std::u16string_view rData);
    bool HandleSelection(std::u16string_view rData);
    bool HandleText(std::u16string_view rData);

public:
    EditEngineWidgetController(EditView& rEditView, Outliner* pOutliner = nullptr);
    virtual ~EditEngineWidgetController() override;

    virtual OUString GetCustomWidgetType() const override { return u"editengine"_ustr; }
    virtual void DumpWidgetData(tools::JsonWriter& rWriter) override;
    virtual bool HandleCustomEvent(const OUString& rCmd, const OUString& rData) override;

    EditView& GetEditView() { return m_rEditView; }
    EditEngine& GetEditEngine() { return m_rEditEngine; }

    /// Queues a message carrying the current model to the client. Repeated calls before the queue
    /// drains collapse into one message, so a burst of keystrokes costs a single update.
    void QueueUpdate();

protected:
    /// Written under an "extra" node, for whatever the concrete widget adds to the payload.
    virtual void DumpExtraData(tools::JsonWriter& /*rWriter*/) {}

    /// Handles a command outside the generic set. Returns true when the command was consumed.
    virtual bool HandleExtraEvent(const OUString& /*rCmd*/, const OUString& /*rData*/)
    {
        return false;
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
