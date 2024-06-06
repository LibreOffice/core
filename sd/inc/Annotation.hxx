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
#include <svx/annotation/Annotation.hxx>

#include "sdpage.hxx"
#include "sddllapi.h"

#include <basegfx/polygon/b2dpolygon.hxx>
#include <tools/color.hxx>

class SdrUndoAction;

namespace com::sun::star::office { class XAnnotation; }

namespace com::sun::star::uno { template <typename > class Reference; }

class SfxViewShell;

namespace sd
{

rtl::Reference<sdr::annotation::Annotation> createAnnotation(SdPage* pPage);
rtl::Reference<sdr::annotation::Annotation> createAnnotationAndAddToPage(SdPage* pPage);

std::unique_ptr<SdrUndoAction> CreateUndoInsertOrRemoveAnnotation(rtl::Reference<sdr::annotation::Annotation>& xAnnotation, bool bInsert);

class SAL_DLLPUBLIC_RTTI Annotation final : public sdr::annotation::Annotation
{
public:
    explicit Annotation( const css::uno::Reference<css::uno::XComponentContext>& context, SdPage* pPage );
    Annotation(const Annotation&) = delete;
    Annotation& operator=(const Annotation&) = delete;

    virtual ~Annotation();

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

    void createChangeUndo();

private:
    void createChangeUndoImpl(std::unique_lock<std::mutex>& g);
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
