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

#include <sal/config.h>

#include <Annotation.hxx>
#include <drawdoc.hxx>

#include <com/sun/star/drawing/XDrawPage.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/lok.hxx>

#include <unotools/datetime.hxx>

#include <sfx2/viewsh.hxx>
#include <svx/svdundo.hxx>

#include <LibreOfficeKit/LibreOfficeKitEnums.h>

#include <notifydocumentevent.hxx>


using namespace css;

namespace com::sun::star::uno { class XComponentContext; }

namespace sd
{

namespace
{

/** Undo/redo insertion or removal of an annotation to/from the document */
class UndoInsertOrRemoveAnnotation : public SdrUndoAction
{
public:
    UndoInsertOrRemoveAnnotation(rtl::Reference<sdr::annotation::Annotation>& xAnnotation, bool bInsert);

    virtual void Undo() override;
    virtual void Redo() override;

protected:
    rtl::Reference<sdr::annotation::Annotation> mxAnnotation;
    bool mbInsert;
    int mnIndex = 0;
};

}

/** Creates an annotation */
rtl::Reference<sdr::annotation::Annotation> createAnnotation(SdPage* pPage)
{
    return rtl::Reference<Annotation>(new Annotation(comphelper::getProcessComponentContext(), pPage));
}

/** Creates an annotation and adds it to the page */
rtl::Reference<sdr::annotation::Annotation> createAnnotationAndAddToPage(SdPage* pPage)
{
    rtl::Reference<sdr::annotation::Annotation> xAnnotation = createAnnotation(pPage);
    pPage->addAnnotation(xAnnotation, -1);
    return xAnnotation;
}

Annotation::Annotation(const uno::Reference<uno::XComponentContext>& context, SdrPage* pPage)
    : sdr::annotation::Annotation(context, pPage)
{
}

// com.sun.star.beans.XPropertySet:
uno::Reference<beans::XPropertySetInfo> SAL_CALL Annotation::getPropertySetInfo()
{
    return ::cppu::PropertySetMixin<office::XAnnotation>::getPropertySetInfo();
}

void SAL_CALL Annotation::setPropertyValue(const OUString & aPropertyName, const uno::Any & aValue)
{
    ::cppu::PropertySetMixin<office::XAnnotation>::setPropertyValue(aPropertyName, aValue);
}

uno::Any SAL_CALL Annotation::getPropertyValue(const OUString & aPropertyName)
{
    return ::cppu::PropertySetMixin<office::XAnnotation>::getPropertyValue(aPropertyName);
}

void SAL_CALL Annotation::addPropertyChangeListener(const OUString & aPropertyName, const uno::Reference<beans::XPropertyChangeListener> & xListener)
{
    ::cppu::PropertySetMixin<office::XAnnotation>::addPropertyChangeListener(aPropertyName, xListener);
}

void SAL_CALL Annotation::removePropertyChangeListener(const OUString & aPropertyName, const uno::Reference<beans::XPropertyChangeListener> & xListener)
{
    ::cppu::PropertySetMixin<office::XAnnotation>::removePropertyChangeListener(aPropertyName, xListener);
}

void SAL_CALL Annotation::addVetoableChangeListener(const OUString & aPropertyName, const uno::Reference<beans::XVetoableChangeListener> & xListener)
{
    ::cppu::PropertySetMixin<office::XAnnotation>::addVetoableChangeListener(aPropertyName, xListener);
}

void SAL_CALL Annotation::removeVetoableChangeListener(const OUString & aPropertyName, const uno::Reference<beans::XVetoableChangeListener> & xListener)
{
    ::cppu::PropertySetMixin<office::XAnnotation>::removeVetoableChangeListener(aPropertyName, xListener);
}

uno::Any SAL_CALL Annotation::getAnchor()
{
    osl::MutexGuard g(m_aMutex);
    uno::Any aRet;
    if( mpPage )
    {
        uno::Reference<drawing::XDrawPage> xPage( mpPage->getUnoPage(), uno::UNO_QUERY );
        aRet <<= xPage;
    }
    return aRet;
}

// css::office::XAnnotation:
geometry::RealPoint2D SAL_CALL Annotation::getPosition()
{
    osl::MutexGuard g(m_aMutex);
    return m_Position;
}

void SAL_CALL Annotation::setPosition(const geometry::RealPoint2D & the_value)
{
    prepareSet("Position", uno::Any(), uno::Any(), nullptr);
    {
        osl::MutexGuard g(m_aMutex);
        createChangeUndo();
        m_Position = the_value;
    }
}

// css::office::XAnnotation:
geometry::RealSize2D SAL_CALL Annotation::getSize()
{
    osl::MutexGuard g(m_aMutex);
    return m_Size;
}

void SAL_CALL Annotation::setSize(const geometry::RealSize2D & the_value)
{
    prepareSet("Size", uno::Any(), uno::Any(), nullptr);
    {
        osl::MutexGuard g(m_aMutex);
        createChangeUndo();
        m_Size = the_value;
    }
}

OUString SAL_CALL Annotation::getAuthor()
{
    osl::MutexGuard g(m_aMutex);
    return m_Author;
}

void SAL_CALL Annotation::setAuthor(const OUString & the_value)
{
    prepareSet("Author", uno::Any(), uno::Any(), nullptr);
    {
        osl::MutexGuard g(m_aMutex);
        createChangeUndo();
        m_Author = the_value;
    }
}

OUString SAL_CALL Annotation::getInitials()
{
    osl::MutexGuard g(m_aMutex);
    return m_Initials;
}

void SAL_CALL Annotation::setInitials(const OUString & the_value)
{
    prepareSet("Initials", uno::Any(), uno::Any(), nullptr);
    {
        osl::MutexGuard g(m_aMutex);
        createChangeUndo();
        m_Initials = the_value;
    }
}

util::DateTime SAL_CALL Annotation::getDateTime()
{
    osl::MutexGuard g(m_aMutex);
    return m_DateTime;
}

void SAL_CALL Annotation::setDateTime(const util::DateTime & the_value)
{
    prepareSet("DateTime", uno::Any(), uno::Any(), nullptr);
    {
        osl::MutexGuard g(m_aMutex);
        createChangeUndo();
        m_DateTime = the_value;
    }
}

void Annotation::createChangeUndo()
{
    SdrModel* pModel = GetModel(); // TTTT should use reference
    if( pModel && pModel->IsUndoEnabled() )
        pModel->AddUndo(createUndoAnnotation());

    if( pModel )
    {
        pModel->SetChanged();
        uno::Reference< XInterface > xSource( static_cast<uno::XWeak*>( this ) );
        NotifyDocumentEvent(
            static_cast< SdDrawDocument& >( *pModel ),
            "OnAnnotationChanged" ,
            xSource );
    }
}

rtl::Reference<sdr::annotation::Annotation> Annotation::clone(SdrPage* pTargetPage)
{
    rtl::Reference<sdr::annotation::Annotation> aNewAnnotation;
    aNewAnnotation = new sd::Annotation(comphelper::getProcessComponentContext(), pTargetPage);
    aNewAnnotation->setPosition(getPosition());
    aNewAnnotation->setSize(getSize());
    aNewAnnotation->setAuthor(getAuthor());
    aNewAnnotation->setInitials(getInitials());
    aNewAnnotation->setDateTime(getDateTime());
    aNewAnnotation->setCreationInfo(getCreationInfo());

    uno::Reference<css::text::XTextCopy> xSourceRange (getTextRange(), uno::UNO_QUERY);
    uno::Reference<css::text::XTextCopy> xRange (aNewAnnotation->getTextRange(), uno::UNO_QUERY);
    if (xSourceRange.is() && xRange.is())
        xRange->copyText(xSourceRange);

    return aNewAnnotation;
}

std::unique_ptr<SdrUndoAction> CreateUndoInsertOrRemoveAnnotation(rtl::Reference<sdr::annotation::Annotation>& xAnnotation, bool bInsert)
{
    if (xAnnotation)
    {
        return std::make_unique<UndoInsertOrRemoveAnnotation>(xAnnotation, bInsert);
    }
    else
    {
        return nullptr;
    }
}

UndoInsertOrRemoveAnnotation::UndoInsertOrRemoveAnnotation(rtl::Reference<sdr::annotation::Annotation>& xAnnotation, bool bInsert)
    : SdrUndoAction(*xAnnotation->GetModel())
    , mxAnnotation(xAnnotation)
    , mbInsert(bInsert)
{
    SdrPage const* pPage = mxAnnotation->getPage();
    if (pPage)
    {
        sdr::annotation::AnnotationVector const& rVector = pPage->getAnnotations();
        auto iterator = std::find(rVector.begin(), rVector.end(), mxAnnotation);
        mnIndex += std::distance(rVector.begin(), iterator);
    }
}

void UndoInsertOrRemoveAnnotation::Undo()
{
    SdrPage* pPage = mxAnnotation->getPage();
    SdrModel* pModel = mxAnnotation->GetModel();
    if (!pPage || !pModel)
        return;

    if (mbInsert)
    {
        pPage->removeAnnotationNoNotify(mxAnnotation);
    }
    else
    {
        pPage->addAnnotation(mxAnnotation, mnIndex);
        LOKCommentNotifyAll(sdr::annotation::CommentNotificationType::Add, *mxAnnotation);
    }
}

void UndoInsertOrRemoveAnnotation::Redo()
{
    SdrPage* pPage = mxAnnotation->getPage();
    SdrModel* pModel = mxAnnotation->GetModel();
    if (!pPage || !pModel)
        return;

    if (mbInsert)
    {
        pPage->addAnnotationNoNotify(mxAnnotation, mnIndex);
        LOKCommentNotifyAll(sdr::annotation::CommentNotificationType::Add, *mxAnnotation);
    }
    else
    {
        pPage->removeAnnotationNoNotify(mxAnnotation);
    }
}

} // namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
