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

#include <sal/config.h>
#include <sal/types.h>
#include <memory>

#include <com/sun/star/office/XAnnotation.hpp>
#include <comphelper/compbase.hxx>
#include <cppuhelper/propertysetmixin.hxx>

#include "sdpage.hxx"
#include "textapi.hxx"
#include "sddllapi.h"

#include <basegfx/polygon/b2dpolygon.hxx>
#include <tools/color.hxx>

class SdrUndoAction;

namespace com::sun::star::office {
    class XAnnotation;
}

namespace com::sun::star::uno { template <typename > class Reference; }

class SfxViewShell;

namespace sd {

enum class CommentNotificationType { Add, Modify, Remove };

void createAnnotation( rtl::Reference< Annotation >& xAnnotation, SdPage* pPage );

std::unique_ptr<SdrUndoAction> CreateUndoInsertOrRemoveAnnotation( const rtl::Reference< sd::Annotation >& xAnnotation, bool bInsert );

void LOKCommentNotify(CommentNotificationType nType, const SfxViewShell* pViewShell,
        rtl::Reference<sd::Annotation> const & rxAnnotation);

void LOKCommentNotifyAll(CommentNotificationType nType,
        rtl::Reference<sd::Annotation> const & rxAnnotation);

struct SD_DLLPUBLIC CustomAnnotationMarker
{
    Color maLineColor;
    Color maFillColor;
    float mnLineWidth;
    std::vector<basegfx::B2DPolygon> maPolygons;
};

class SAL_DLLPUBLIC_RTTI Annotation final :
                   public ::comphelper::WeakComponentImplHelper<css::office::XAnnotation>,
                   public ::cppu::PropertySetMixin<css::office::XAnnotation>
{
public:
    explicit Annotation( const css::uno::Reference<css::uno::XComponentContext>& context, SdPage* pPage );
    Annotation(const Annotation&) = delete;
    Annotation& operator=(const Annotation&) = delete;

    static sal_uInt32 m_nLastId;

    SdPage* GetPage() const { return mpPage; }
    SdrModel* GetModel() { return (mpPage != nullptr) ? &mpPage->getSdrModelFromSdrPage() : nullptr; }
    sal_uInt32 GetId() const { return m_nId; }

    // XInterface:
    virtual css::uno::Any SAL_CALL queryInterface(css::uno::Type const & type) override;
    virtual void SAL_CALL acquire() noexcept override { ::comphelper::WeakComponentImplHelper<css::office::XAnnotation>::acquire(); }
    virtual void SAL_CALL release() noexcept override { ::comphelper::WeakComponentImplHelper<css::office::XAnnotation>::release(); }

    // css::beans::XPropertySet:
    virtual css::uno::Reference<css::beans::XPropertySetInfo> SAL_CALL getPropertySetInfo() override;
    virtual void SAL_CALL setPropertyValue(const OUString & aPropertyName, const css::uno::Any & aValue) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(const OUString & PropertyName) override;
    virtual void SAL_CALL addPropertyChangeListener(const OUString & aPropertyName, const css::uno::Reference<css::beans::XPropertyChangeListener> & xListener) override;
    virtual void SAL_CALL removePropertyChangeListener(const OUString & aPropertyName, const css::uno::Reference<css::beans::XPropertyChangeListener> & aListener) override;
    virtual void SAL_CALL addVetoableChangeListener(const OUString & PropertyName, const css::uno::Reference<css::beans::XVetoableChangeListener> & aListener) override;
    virtual void SAL_CALL removeVetoableChangeListener(const OUString & PropertyName, const css::uno::Reference<css::beans::XVetoableChangeListener> & aListener) override;

    // css::office::XAnnotation:
    virtual css::uno::Any SAL_CALL getAnchor() override;
    SD_DLLPUBLIC virtual css::geometry::RealPoint2D SAL_CALL getPosition() override;
    virtual void SAL_CALL setPosition(const css::geometry::RealPoint2D & the_value) override;
    virtual css::geometry::RealSize2D SAL_CALL getSize() override;
    virtual void SAL_CALL setSize(const css::geometry::RealSize2D& _size) override;
    SD_DLLPUBLIC virtual OUString SAL_CALL getAuthor() override;
    virtual void SAL_CALL setAuthor(const OUString & the_value) override;
    SD_DLLPUBLIC virtual OUString SAL_CALL getInitials() override;
    virtual void SAL_CALL setInitials(const OUString & the_value) override;
    SD_DLLPUBLIC virtual css::util::DateTime SAL_CALL getDateTime() override;
    virtual void SAL_CALL setDateTime(const css::util::DateTime & the_value) override;
    SD_DLLPUBLIC virtual css::uno::Reference<css::text::XText> SAL_CALL getTextRange() override;

    void createChangeUndo();

    void createCustomAnnotationMarker()
    {
        m_pCustomAnnotationMarker = std::make_unique<CustomAnnotationMarker>();
    }

    CustomAnnotationMarker& getCustomAnnotationMarker()
    {
        return *m_pCustomAnnotationMarker;
    }

    bool hasCustomAnnotationMarker() const
    {
        return bool(m_pCustomAnnotationMarker);
    }

    void setIsFreeText(bool value) { m_bIsFreeText = value; }

    bool isFreeText() const { return m_bIsFreeText; }

private:
    // destructor is private and will be called indirectly by the release call    virtual ~Annotation() {}

    // override WeakComponentImplHelperBase::disposing()
    // This function is called upon disposing the component,
    // if your component needs special work when it becomes
    // disposed, do it here.
    virtual void disposing(std::unique_lock<std::mutex>& rGuard) override;

    void createChangeUndoImpl(std::unique_lock<std::mutex>& g);

    sal_uInt32 m_nId;
    SdPage* mpPage;
    css::geometry::RealPoint2D m_Position;
    css::geometry::RealSize2D m_Size;
    OUString m_Author;
    OUString m_Initials;
    css::util::DateTime m_DateTime;
    rtl::Reference<TextApiObject> m_TextRange;

    std::unique_ptr<CustomAnnotationMarker> m_pCustomAnnotationMarker;
    bool m_bIsFreeText;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
