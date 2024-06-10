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

#include <svx/svdpage.hxx>
#include <svx/svdundo.hxx>
#include <svx/svxdllapi.h>

class SdrUndoAction;
class SfxViewShell;

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
{
private:
    static sal_uInt32 m_nLastId;
    static sal_uInt32 nextID() { return m_nLastId++; }

protected:
    SdrPage* mpSdrPage;
    sal_uInt32 m_nId;

    css::geometry::RealPoint2D m_Position;
    css::geometry::RealSize2D m_Size;
    OUString m_Author;
    OUString m_Initials;
    css::util::DateTime m_DateTime;

    bool m_bIsFreeText = false;

    std::unique_ptr<SdrUndoAction> createUndoAnnotation();

public:
    Annotation(SdrPage* pPage)
        : mpSdrPage(pPage)
        , m_nId(nextID())
    {
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

    virtual OUString GetText() = 0;
    virtual void SetText(OUString const& rText) = 0;

    SdrModel* GetModel()
    {
        return mpSdrPage != nullptr ? &mpSdrPage->getSdrModelFromSdrPage() : nullptr;
    }
    SdrPage const* getPage() const { return mpSdrPage; }

    sal_uInt32 GetId() const { return m_nId; }

    void setIsFreeText(bool value) { m_bIsFreeText = value; }

    bool isFreeText() const { return m_bIsFreeText; }
};

//typedef std::vector<rtl::Reference<Annotation>> AnnotationVector;

} // namespace sdr::annotation

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
