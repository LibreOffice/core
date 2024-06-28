/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svx/annotation/Annotation.hxx>
#include <svx/annotation/ObjectAnnotationData.hxx>
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
            aJsonWriter.put("parthash", pPage ? OString::number(pPage->GetUniqueID()) : OString());
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

/** Undo/redo a modification of an annotation - change of annotation data */
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
    : cppu::WeakComponentImplHelper<office::XAnnotation>(m_aMutex)
    , cppu::PropertySetMixin<office::XAnnotation>(rxContext, IMPLEMENTS_PROPERTY_SET,
                                                  uno::Sequence<OUString>())
    , mpPage(pPage)
{
}

SdrModel* Annotation::GetModel() const
{
    return mpPage != nullptr ? &mpPage->getSdrModelFromSdrPage() : nullptr;
}

uno::Any Annotation::queryInterface(uno::Type const& type)
{
    return ::cppu::WeakComponentImplHelper<office::XAnnotation>::queryInterface(type);
}

std::unique_ptr<SdrUndoAction> Annotation::createUndoAnnotation()
{
    return std::make_unique<UndoAnnotation>(*this);
}

OUString Annotation::GetText()
{
    uno::Reference<text::XText> xText(getTextRange());
    return xText->getString();
}

void Annotation::SetText(OUString const& rText)
{
    uno::Reference<text::XText> xText(getTextRange());
    return xText->setString(rText);
}

uno::Reference<text::XText> SAL_CALL Annotation::getTextRange()
{
    osl::MutexGuard g(m_aMutex);
    if (!m_TextRange.is() && mpPage != nullptr)
        m_TextRange = sdr::annotation::TextApiObject::create(&mpPage->getSdrModelFromSdrPage());

    return m_TextRange;
}

// override WeakComponentImplHelperBase::disposing()
// This function is called upon disposing the component,
// if your component needs special work when it becomes
// disposed, do it here.
void SAL_CALL Annotation::disposing()
{
    mpPage = nullptr;
    if (m_TextRange.is())
    {
        m_TextRange->dispose();
        m_TextRange.clear();
    }
}

SdrObject* Annotation::findAnnotationObject()
{
    SdrPage const* pPage = getPage();

    if (!pPage)
        return nullptr;

    for (size_t i = 0; i < pPage->GetObjCount(); ++i)
    {
        SdrObject* pObject = pPage->GetObj(i);
        if (pObject->isAnnotationObject()
            && pObject->getAnnotationData()->mxAnnotation.get() == this)
            return pObject;
    }
    return nullptr;
}

} // namespace sdr::annotation

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
