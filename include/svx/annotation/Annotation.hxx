/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <com/sun/star/geometry/RealPoint2D.hpp>
#include <com/sun/star/geometry/RealSize2D.hpp>
#include <com/sun/star/util/DateTime.hpp>

#include <svx/svdundo.hxx>
#include <svx/svxdllapi.h>

#include <com/sun/star/office/XAnnotation.hpp>
#include <comphelper/compbase.hxx>
#include <cppuhelper/propertysetmixin.hxx>
#include <svx/annotation/Annotation.hxx>
#include <svx/annotation/TextAPI.hxx>

class SdrUndoAction;
class SfxViewShell;
class SdrPage;

namespace sdr::annotation
{
class Annotation;

enum class CommentNotificationType
{
    Add,
    Modify,
    Remove
};

SVXCORE_DLLPUBLIC void LOKCommentNotify(CommentNotificationType nType,
                                        const SfxViewShell* pViewShell, Annotation& rAnnotation);
SVXCORE_DLLPUBLIC void LOKCommentNotifyAll(CommentNotificationType nType, Annotation& rAnnotation);

enum class AnnotationType
{
    None,
    Square,
    Polygon,
    Circle,
    Ink,
    Highlight,
    Line,
    FreeText,
};

struct CreationInfo
{
    AnnotationType meType = AnnotationType::None;

    std::vector<basegfx::B2DPolygon> maPolygons;
    basegfx::B2DRectangle maRectangle;

    float mnWidth = 0.0f;

    bool mbFillColor = false;
    Color maFillColor = COL_TRANSPARENT;

    bool mbColor = false;
    Color maColor = COL_TRANSPARENT;
};

struct SVXCORE_DLLPUBLIC AnnotationData
{
    css::geometry::RealPoint2D m_Position;
    css::geometry::RealSize2D m_Size;
    OUString m_Author;
    OUString m_Initials;
    css::util::DateTime m_DateTime;
    OUString m_Text;

    void get(Annotation& rAnnotation);
    void set(Annotation& rAnnotation);
};

class SVXCORE_DLLPUBLIC Annotation
    : public ::comphelper::WeakComponentImplHelper<css::office::XAnnotation>,
      public ::cppu::PropertySetMixin<css::office::XAnnotation>
{
private:
    static sal_uInt32 m_nLastId;
    static sal_uInt32 nextID() { return m_nLastId++; }

protected:
    SdrPage* mpPage;
    sal_uInt32 m_nId;

    css::geometry::RealPoint2D m_Position;
    css::geometry::RealSize2D m_Size;
    OUString m_Author;
    OUString m_Initials;
    css::util::DateTime m_DateTime;
    rtl::Reference<sdr::annotation::TextApiObject> m_TextRange;

    CreationInfo maCreationInfo;

    std::unique_ptr<SdrUndoAction> createUndoAnnotation();

public:
    Annotation(const css::uno::Reference<css::uno::XComponentContext>& context, SdrPage* pPage);
    Annotation(const Annotation&) = delete;
    Annotation& operator=(const Annotation&) = delete;

    // XInterface:
    virtual css::uno::Any SAL_CALL queryInterface(css::uno::Type const& type) override;
    virtual void SAL_CALL acquire() noexcept override
    {
        comphelper::WeakComponentImplHelper<css::office::XAnnotation>::acquire();
    }
    virtual void SAL_CALL release() noexcept override
    {
        comphelper::WeakComponentImplHelper<css::office::XAnnotation>::release();
    }

    css::geometry::RealPoint2D GetPosition() const { return m_Position; }
    void SetPosition(const css::geometry::RealPoint2D& rValue) { m_Position = rValue; }

    css::geometry::RealSize2D GetSize() const { return m_Size; }
    void SetSize(const css::geometry::RealSize2D& rValue) { m_Size = rValue; }

    OUString GetAuthor() const { return m_Author; }
    void SetAuthor(const OUString& rValue) { m_Author = rValue; }

    OUString GetInitials() const { return m_Initials; }
    void SetInitials(const OUString& rValue) { m_Initials = rValue; }

    css::util::DateTime GetDateTime() const { return m_DateTime; }
    void SetDateTime(const css::util::DateTime& rValue) { m_DateTime = rValue; }

    virtual css::uno::Reference<css::text::XText> SAL_CALL getTextRange() override;

    // override WeakComponentImplHelperBase::disposing()
    // This function is called upon disposing the component,
    // if your component needs special work when it becomes
    // disposed, do it here.
    virtual void disposing(std::unique_lock<std::mutex>& rGuard) override;

    OUString GetText();
    void SetText(OUString const& rText);

    SdrModel* GetModel() const;
    SdrPage const* getPage() const { return mpPage; }
    SdrPage* getPage() { return mpPage; }

    sal_uInt32 GetId() const { return m_nId; }

    CreationInfo const& getCreationInfo() { return maCreationInfo; }
    void setCreationInfo(CreationInfo const& rCreationInfo) { maCreationInfo = rCreationInfo; }
};

typedef std::vector<rtl::Reference<Annotation>> AnnotationVector;

} // namespace sdr::annotation

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
