/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svx/annotation/Annotation.hxx>
#include <svx/svdpage.hxx>
#include <tools/json_writer.hxx>
#include <sfx2/viewsh.hxx>
#include <unotools/datetime.hxx>
#include <comphelper/lok.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>

using namespace css;

namespace sdr::annotation
{
namespace
{
OString lcl_LOKGetCommentPayload(CommentNotificationType nType, Annotation& rAnnotation)
{
    tools::JsonWriter aJsonWriter;
    {
        auto aCommentNode = aJsonWriter.startNode("comment");

        aJsonWriter.put(
            "action",
            (nType == CommentNotificationType::Add
                 ? "Add"
                 : (nType == CommentNotificationType::Remove
                        ? "Remove"
                        : (nType == CommentNotificationType::Modify ? "Modify" : "???"))));
        aJsonWriter.put("id", rAnnotation.GetId());

        if (nType != CommentNotificationType::Remove)
        {
            aJsonWriter.put("id", rAnnotation.GetId());
            aJsonWriter.put("author", rAnnotation.GetAuthor());
            aJsonWriter.put("dateTime", utl::toISO8601(rAnnotation.GetDateTime()));
            aJsonWriter.put("text", rAnnotation.GetText());
            SdrPage const* pPage = rAnnotation.getPage();
            sal_Int64 nHash = sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_IntPtr>(pPage));
            aJsonWriter.put("parthash", pPage ? OString::number(nHash) : OString());
            geometry::RealPoint2D const& rPoint = rAnnotation.GetPosition();
            geometry::RealSize2D const& rSize = rAnnotation.GetSize();
            tools::Rectangle aRectangle(
                Point(std::round(o3tl::toTwips(rPoint.X, o3tl::Length::mm)),
                      std::round(o3tl::toTwips(rPoint.Y, o3tl::Length::mm))),
                Size(std::round(o3tl::toTwips(rSize.Width, o3tl::Length::mm)),
                     std::round(o3tl::toTwips(rSize.Height, o3tl::Length::mm))));
            aJsonWriter.put("rectangle", aRectangle.toString());
        }
    }
    return aJsonWriter.finishAndGetAsOString();
}

class UndoAnnotation : public SdrUndoAction
{
public:
    explicit UndoAnnotation(Annotation& rAnnotation)
        : SdrUndoAction(*rAnnotation.GetModel())
        , mrAnnotation(rAnnotation)
    {
        maUndoData.get(mrAnnotation);
    }

    void Undo() override
    {
        maRedoData.get(mrAnnotation);
        maUndoData.set(mrAnnotation);
        LOKCommentNotifyAll(CommentNotificationType::Modify, mrAnnotation);
    }

    void Redo() override
    {
        maUndoData.get(mrAnnotation);
        maRedoData.set(mrAnnotation);
        LOKCommentNotifyAll(CommentNotificationType::Modify, mrAnnotation);
    }

protected:
    Annotation& mrAnnotation;
    AnnotationData maUndoData;
    AnnotationData maRedoData;
};

} // anonymous ns

void LOKCommentNotify(CommentNotificationType nType, const SfxViewShell* pViewShell,
                      Annotation& rAnnotation)
{
    // callbacks only if tiled annotations are explicitly turned off by LOK client
    if (!comphelper::LibreOfficeKit::isActive() || comphelper::LibreOfficeKit::isTiledAnnotations())
        return;

    OString aPayload = lcl_LOKGetCommentPayload(nType, rAnnotation);
    pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_COMMENT, aPayload);
}

void LOKCommentNotifyAll(CommentNotificationType nType, Annotation& rAnnotation)
{
    // callbacks only if tiled annotations are explicitly turned off by LOK client
    if (!comphelper::LibreOfficeKit::isActive() || comphelper::LibreOfficeKit::isTiledAnnotations())
        return;

    OString aPayload = lcl_LOKGetCommentPayload(nType, rAnnotation);

    const SfxViewShell* pViewShell = SfxViewShell::GetFirst();
    while (pViewShell)
    {
        pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_COMMENT, aPayload);
        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }
}

void AnnotationData::get(Annotation& rAnnotation)
{
    m_Position = rAnnotation.GetPosition();
    m_Size = rAnnotation.GetSize();
    m_Author = rAnnotation.GetAuthor();
    m_Initials = rAnnotation.GetInitials();
    m_DateTime = rAnnotation.GetDateTime();
    m_Text = rAnnotation.GetText();
}

void AnnotationData::set(Annotation& rAnnotation)
{
    rAnnotation.SetPosition(m_Position);
    rAnnotation.SetSize(m_Size);
    rAnnotation.SetAuthor(m_Author);
    rAnnotation.SetInitials(m_Initials);
    rAnnotation.SetDateTime(m_DateTime);
    rAnnotation.SetText(m_Text);
}

Annotation::Annotation(const css::uno::Reference<css::uno::XComponentContext>& rxContext,
                       SdrPage* pPage)
    : cppu::PropertySetMixin<office::XAnnotation>(rxContext, IMPLEMENTS_PROPERTY_SET,
                                                  uno::Sequence<OUString>())
    , mpPage(pPage)
    , m_nId(nextID())
{
}

sal_uInt32 Annotation::m_nLastId = 1;

SdrModel* Annotation::GetModel() const
{
    return mpPage != nullptr ? &mpPage->getSdrModelFromSdrPage() : nullptr;
}

uno::Any Annotation::queryInterface(uno::Type const& type)
{
    return comphelper::WeakComponentImplHelper<office::XAnnotation>::queryInterface(type);
}

std::unique_ptr<SdrUndoAction> Annotation::createUndoAnnotation()
{
    return std::make_unique<UndoAnnotation>(*this);
}

} // namespace sdr::annotation

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
