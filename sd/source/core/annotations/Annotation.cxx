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

#include <tools/json_writer.hxx>

using namespace css;

namespace com::sun::star::uno { class XComponentContext; }

namespace sd {

namespace {

class UndoInsertOrRemoveAnnotation : public SdrUndoAction
{
public:
    UndoInsertOrRemoveAnnotation( Annotation& rAnnotation, bool bInsert );

    virtual void Undo() override;
    virtual void Redo() override;

protected:
    rtl::Reference< Annotation > mxAnnotation;
    bool mbInsert;
    int mnIndex;
};

struct AnnotationData
{
    geometry::RealPoint2D m_Position;
    geometry::RealSize2D m_Size;
    OUString m_Author;
    OUString m_Initials;
    util::DateTime m_DateTime;
    OUString m_Text;

    void get( const rtl::Reference< Annotation >& xAnnotation )
    {
        m_Position = xAnnotation->getPosition();
        m_Size = xAnnotation->getSize();
        m_Author = xAnnotation->getAuthor();
        m_Initials = xAnnotation->getInitials();
        m_DateTime = xAnnotation->getDateTime();
        uno::Reference<text::XText> xText(xAnnotation->getTextRange());
        m_Text = xText->getString();
    }

    void set( const rtl::Reference< Annotation >& xAnnotation )
    {
        xAnnotation->setPosition(m_Position);
        xAnnotation->setSize(m_Size);
        xAnnotation->setAuthor(m_Author);
        xAnnotation->setInitials(m_Initials);
        xAnnotation->setDateTime(m_DateTime);
        uno::Reference<text::XText> xText(xAnnotation->getTextRange());
        xText->setString(m_Text);
    }
};

class UndoAnnotation : public SdrUndoAction
{
public:
    explicit UndoAnnotation( Annotation& rAnnotation );

    virtual void Undo() override;
    virtual void Redo() override;

protected:
    rtl::Reference< Annotation > mxAnnotation;
    AnnotationData maUndoData;
    AnnotationData maRedoData;
};

}

void createAnnotation(rtl::Reference<Annotation>& xAnnotation, SdPage* pPage )
{
    xAnnotation.set(
        new Annotation(comphelper::getProcessComponentContext(), pPage));
    pPage->addAnnotation(xAnnotation, -1);
}

sal_uInt32 Annotation::m_nLastId = 1;

Annotation::Annotation(const uno::Reference<uno::XComponentContext>& context, SdPage* pPage)
    : ::cppu::PropertySetMixin<office::XAnnotation>(context, IMPLEMENTS_PROPERTY_SET,
                                                    uno::Sequence<OUString>())
    , m_nId(m_nLastId++)
    , mpPage(pPage)
    , m_bIsFreeText(false)
{
}

// override WeakComponentImplHelperBase::disposing()
// This function is called upon disposing the component,
// if your component needs special work when it becomes
// disposed, do it here.
void Annotation::disposing(std::unique_lock<std::mutex>& /*rGuard*/)
{
    mpPage = nullptr;
    if( m_TextRange.is() )
    {
        m_TextRange->dispose();
        m_TextRange.clear();
    }
}

uno::Any Annotation::queryInterface(css::uno::Type const & type)
{
    return ::comphelper::WeakComponentImplHelper<office::XAnnotation>::queryInterface(type);
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
    std::unique_lock g(m_aMutex);
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
    std::unique_lock g(m_aMutex);
    return m_Position;
}

void SAL_CALL Annotation::setPosition(const geometry::RealPoint2D & the_value)
{
    prepareSet("Position", uno::Any(), uno::Any(), nullptr);
    {
        std::unique_lock g(m_aMutex);
        createChangeUndoImpl(g);
        m_Position = the_value;
    }
}

// css::office::XAnnotation:
geometry::RealSize2D SAL_CALL Annotation::getSize()
{
    std::unique_lock g(m_aMutex);
    return m_Size;
}

void SAL_CALL Annotation::setSize(const geometry::RealSize2D & the_value)
{
    prepareSet("Size", uno::Any(), uno::Any(), nullptr);
    {
        std::unique_lock g(m_aMutex);
        createChangeUndoImpl(g);
        m_Size = the_value;
    }
}

OUString SAL_CALL Annotation::getAuthor()
{
    std::unique_lock g(m_aMutex);
    return m_Author;
}

void SAL_CALL Annotation::setAuthor(const OUString & the_value)
{
    prepareSet("Author", uno::Any(), uno::Any(), nullptr);
    {
        std::unique_lock g(m_aMutex);
        createChangeUndoImpl(g);
        m_Author = the_value;
    }
}

OUString SAL_CALL Annotation::getInitials()
{
    std::unique_lock g(m_aMutex);
    return m_Initials;
}

void SAL_CALL Annotation::setInitials(const OUString & the_value)
{
    prepareSet("Initials", uno::Any(), uno::Any(), nullptr);
    {
        std::unique_lock g(m_aMutex);
        createChangeUndoImpl(g);
        m_Initials = the_value;
    }
}

util::DateTime SAL_CALL Annotation::getDateTime()
{
    std::unique_lock g(m_aMutex);
    return m_DateTime;
}

void SAL_CALL Annotation::setDateTime(const util::DateTime & the_value)
{
    prepareSet("DateTime", uno::Any(), uno::Any(), nullptr);
    {
        std::unique_lock g(m_aMutex);
        createChangeUndoImpl(g);
        m_DateTime = the_value;
    }
}

void Annotation::createChangeUndo()
{
    std::unique_lock g(m_aMutex);
    createChangeUndoImpl(g);
}

void Annotation::createChangeUndoImpl(std::unique_lock<std::mutex>& g)
{
    SdrModel* pModel = GetModel(); // TTTT should use reference
    if( pModel && pModel->IsUndoEnabled() )
    {
        g.unlock(); // AddUndo calls back into Annotation
        pModel->AddUndo( std::make_unique<UndoAnnotation>( *this ) );
        g.lock();
    }

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

uno::Reference<text::XText> SAL_CALL Annotation::getTextRange()
{
    std::unique_lock g(m_aMutex);
    if( !m_TextRange.is() && (mpPage != nullptr) )
    {
        m_TextRange = TextApiObject::create( static_cast< SdDrawDocument* >( &mpPage->getSdrModelFromSdrPage() ) );
    }
    return m_TextRange;
}

std::unique_ptr<SdrUndoAction> CreateUndoInsertOrRemoveAnnotation( const uno::Reference<office::XAnnotation>& xAnnotation, bool bInsert )
{
    Annotation* pAnnotation = dynamic_cast< Annotation* >( xAnnotation.get() );
    if( pAnnotation )
    {
        return std::make_unique< UndoInsertOrRemoveAnnotation >( *pAnnotation, bInsert );
    }
    else
    {
        return nullptr;
    }
}

sal_uInt32 getAnnotationId(const uno::Reference<office::XAnnotation>& xAnnotation)
{
    Annotation* pAnnotation = dynamic_cast<Annotation*>(xAnnotation.get());
    sal_uInt32 nId = 0;
    if (pAnnotation)
        nId = pAnnotation->GetId();
    return nId;
}

const SdPage* getAnnotationPage(const uno::Reference<office::XAnnotation>& xAnnotation)
{
    Annotation* pAnnotation = dynamic_cast<Annotation*>(xAnnotation.get());
    if (pAnnotation)
        return pAnnotation->GetPage();
    return nullptr;
}

namespace
{
OString lcl_LOKGetCommentPayload(CommentNotificationType nType, uno::Reference<office::XAnnotation> const & rxAnnotation)
{
    ::tools::JsonWriter aJsonWriter;
    {
        auto aCommentNode = aJsonWriter.startNode("comment");

        aJsonWriter.put("action", (nType == CommentNotificationType::Add ? "Add" :
                                (nType == CommentNotificationType::Remove ? "Remove" :
                                    (nType == CommentNotificationType::Modify ? "Modify" : "???"))));
        aJsonWriter.put("id", sd::getAnnotationId(rxAnnotation));

        if (nType != CommentNotificationType::Remove && rxAnnotation.is())
        {
            aJsonWriter.put("id", sd::getAnnotationId(rxAnnotation));
            aJsonWriter.put("author", rxAnnotation->getAuthor());
            aJsonWriter.put("dateTime", utl::toISO8601(rxAnnotation->getDateTime()));
            uno::Reference<text::XText> xText(rxAnnotation->getTextRange());
            aJsonWriter.put("text", xText->getString());
            const SdPage* pPage = sd::getAnnotationPage(rxAnnotation);
            aJsonWriter.put("parthash", pPage ? OString::number(pPage->GetHashCode()) : OString());
            geometry::RealPoint2D const & rPoint = rxAnnotation->getPosition();
            geometry::RealSize2D const & rSize = rxAnnotation->getSize();
            ::tools::Rectangle aRectangle(Point(std::round(o3tl::toTwips(rPoint.X, o3tl::Length::mm)),
                                                std::round(o3tl::toTwips(rPoint.Y, o3tl::Length::mm))),
                                          Size(std::round(o3tl::toTwips(rSize.Width, o3tl::Length::mm)),
                                               std::round(o3tl::toTwips(rSize.Height, o3tl::Length::mm))));
            aJsonWriter.put("rectangle", aRectangle.toString());
        }
    }
    return aJsonWriter.finishAndGetAsOString();
}
} // anonymous ns

void LOKCommentNotify(CommentNotificationType nType, const SfxViewShell* pViewShell, uno::Reference<office::XAnnotation> const & rxAnnotation)
{
    // callbacks only if tiled annotations are explicitly turned off by LOK client
    if (!comphelper::LibreOfficeKit::isActive() || comphelper::LibreOfficeKit::isTiledAnnotations())
        return ;

    OString aPayload = lcl_LOKGetCommentPayload(nType, rxAnnotation);
    pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_COMMENT, aPayload);
}

void LOKCommentNotifyAll(CommentNotificationType nType, uno::Reference<office::XAnnotation> const & rxAnnotation)
{
    // callbacks only if tiled annotations are explicitly turned off by LOK client
    if (!comphelper::LibreOfficeKit::isActive() || comphelper::LibreOfficeKit::isTiledAnnotations())
        return ;

    OString aPayload = lcl_LOKGetCommentPayload(nType, rxAnnotation);

    const SfxViewShell* pViewShell = SfxViewShell::GetFirst();
    while (pViewShell)
    {
        pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_COMMENT, aPayload);
        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }
}

UndoInsertOrRemoveAnnotation::UndoInsertOrRemoveAnnotation( Annotation& rAnnotation, bool bInsert )
: SdrUndoAction( *rAnnotation.GetModel() )
, mxAnnotation( &rAnnotation )
, mbInsert( bInsert )
, mnIndex( 0 )
{
    SdPage* pPage = rAnnotation.GetPage();
    if( pPage )
    {
        const AnnotationVector& rVec = pPage->getAnnotations();
        auto iter = std::find(rVec.begin(), rVec.end(), &rAnnotation);
        mnIndex += std::distance(rVec.begin(), iter);
    }
}

void UndoInsertOrRemoveAnnotation::Undo()
{
    SdPage* pPage = mxAnnotation->GetPage();
    SdrModel* pModel = mxAnnotation->GetModel();
    if( !(pPage && pModel) )
        return;

    if( mbInsert )
    {
        pPage->removeAnnotation( mxAnnotation );
    }
    else
    {
        pPage->addAnnotation( mxAnnotation, mnIndex );
        uno::Reference<office::XAnnotation> xAnnotation( mxAnnotation );
        LOKCommentNotifyAll( CommentNotificationType::Add, xAnnotation );
    }
}

void UndoInsertOrRemoveAnnotation::Redo()
{
    SdPage* pPage = mxAnnotation->GetPage();
    SdrModel* pModel = mxAnnotation->GetModel();
    if( !(pPage && pModel) )
        return;

    if( mbInsert )
    {
        pPage->addAnnotation( mxAnnotation, mnIndex );
        uno::Reference<office::XAnnotation> xAnnotation( mxAnnotation );
        LOKCommentNotifyAll( CommentNotificationType::Add, xAnnotation );
    }
    else
    {
        pPage->removeAnnotation( mxAnnotation );
    }
}

UndoAnnotation::UndoAnnotation( Annotation& rAnnotation )
: SdrUndoAction( *rAnnotation.GetModel() )
, mxAnnotation( &rAnnotation )
{
    maUndoData.get( mxAnnotation );
}

void UndoAnnotation::Undo()
{
    maRedoData.get( mxAnnotation );
    maUndoData.set( mxAnnotation );
    LOKCommentNotifyAll( CommentNotificationType::Modify, mxAnnotation );
}

void UndoAnnotation::Redo()
{
    maUndoData.get( mxAnnotation );
    maRedoData.set( mxAnnotation );
    LOKCommentNotifyAll( CommentNotificationType::Modify, mxAnnotation );
}

} // namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
