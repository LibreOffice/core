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
/** Undo/redo a modification of an annotation - change of annotation data */
class UndoAnnotation : public SdrUndoAction
{
public:
    explicit UndoAnnotation(Annotation& rAnnotation)
        : SdrUndoAction(*rAnnotation.GetModel())
        , mrAnnotation(rAnnotation)
    {
        mrAnnotation.toData(maUndoData);
    }

    void Undo() override
    {
        mrAnnotation.toData(maRedoData);
        mrAnnotation.fromData(maUndoData);
        LOKCommentNotifyAll(CommentNotificationType::Modify, mrAnnotation);
    }

    void Redo() override
    {
        mrAnnotation.toData(maUndoData);
        mrAnnotation.fromData(maRedoData);
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

    OString aPayload = rAnnotation.ToJSON(nType);
    pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_COMMENT, aPayload);
}

void LOKCommentNotifyAll(CommentNotificationType nType, Annotation& rAnnotation)
{
    // callbacks only if tiled annotations are explicitly turned off by LOK client
    if (!comphelper::LibreOfficeKit::isActive() || comphelper::LibreOfficeKit::isTiledAnnotations())
        return;

    OString aPayload = rAnnotation.ToJSON(nType);

    const SfxViewShell* pViewShell = SfxViewShell::GetFirst();
    while (pViewShell)
    {
        pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_COMMENT, aPayload);
        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }
}

void Annotation::toData(AnnotationData& rData)
{
    std::unique_lock g(m_aMutex);
    rData.m_Position = m_Position;
    rData.m_Size = m_Size;
    rData.m_Author = m_Author;
    rData.m_Initials = m_Initials;
    rData.m_DateTime = m_DateTime;
    rData.m_Text = GetTextImpl(g);
}

void Annotation::fromData(const AnnotationData& rData)
{
    std::unique_lock g(m_aMutex);
    m_Position = rData.m_Position;
    m_Size = rData.m_Size;
    m_Author = rData.m_Author;
    m_Initials = rData.m_Initials;
    m_DateTime = rData.m_DateTime;
    SetTextImpl(rData.m_Text, g);
}

Annotation::Annotation(const css::uno::Reference<css::uno::XComponentContext>& rxContext,
                       SdrPage* pPage)
    : cppu::PropertySetMixin<office::XAnnotation>(rxContext, IMPLEMENTS_PROPERTY_SET,
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
    return comphelper::WeakComponentImplHelper<office::XAnnotation>::queryInterface(type);
}

std::unique_ptr<SdrUndoAction> Annotation::createUndoAnnotation()
{
    return std::make_unique<UndoAnnotation>(*this);
}

OUString Annotation::GetText()
{
    std::unique_lock g(m_aMutex);
    return GetTextImpl(g);
}

OUString Annotation::GetTextImpl(const std::unique_lock<std::mutex>& g)
{
    uno::Reference<text::XText> xText(getTextRangeImpl(g));
    return xText->getString();
}

void Annotation::SetTextImpl(OUString const& rText, const std::unique_lock<std::mutex>& g)
{
    uno::Reference<text::XText> xText(getTextRangeImpl(g));
    return xText->setString(rText);
}

uno::Reference<text::XText> SAL_CALL Annotation::getTextRange()
{
    std::unique_lock g(m_aMutex);
    return getTextRangeImpl(g);
}

uno::Reference<text::XText> Annotation::getTextRangeImpl(const std::unique_lock<std::mutex>& /*g*/)
{
    if (!m_TextRange.is() && mpPage != nullptr)
    {
        m_TextRange = sdr::annotation::TextApiObject::create(&mpPage->getSdrModelFromSdrPage());
    }
    return m_TextRange;
}

void Annotation::SetPosition(const css::geometry::RealPoint2D& rValue)
{
    std::unique_lock g(m_aMutex);
    m_Position = rValue;
}

void Annotation::SetSize(const css::geometry::RealSize2D& rValue)
{
    std::unique_lock g(m_aMutex);
    m_Size = rValue;
}

void Annotation::disposing(std::unique_lock<std::mutex>& /*rGuard*/)
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

OString Annotation::ToJSON(CommentNotificationType nType)
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

        std::unique_lock g(m_aMutex);
        sal_uInt64 id = maUniqueID.getID();
        aJsonWriter.put("id", id);
        if (nType != CommentNotificationType::Remove)
        {
            aJsonWriter.put("id", id);
            aJsonWriter.put("author", m_Author);
            aJsonWriter.put("dateTime", utl::toISO8601(m_DateTime));
            aJsonWriter.put("text", GetTextImpl(g));
            aJsonWriter.put("parthash",
                            mpPage ? OString::number(mpPage->GetUniqueID()) : OString());
            tools::Rectangle aRectangle(
                Point(std::round(o3tl::toTwips(m_Position.X, o3tl::Length::mm)),
                      std::round(o3tl::toTwips(m_Position.Y, o3tl::Length::mm))),
                Size(std::round(o3tl::toTwips(m_Size.Width, o3tl::Length::mm)),
                     std::round(o3tl::toTwips(m_Size.Height, o3tl::Length::mm))));
            aJsonWriter.put("rectangle", aRectangle.toString());
        }
    }

    return aJsonWriter.finishAndGetAsOString();
}

} // namespace sdr::annotation

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
