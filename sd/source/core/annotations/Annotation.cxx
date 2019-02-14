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

#include <boost/property_tree/json_parser.hpp>

#include <com/sun/star/office/XAnnotation.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/lok.hxx>
#include <cppuhelper/propertysetmixin.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>

#include <unotools/datetime.hxx>

#include <sfx2/viewsh.hxx>
#include <svx/svdundo.hxx>

#include <LibreOfficeKit/LibreOfficeKitEnums.h>

#include <Annotation.hxx>
#include <drawdoc.hxx>
#include <notifydocumentevent.hxx>
#include <sdpage.hxx>
#include <textapi.hxx>

namespace com { namespace sun { namespace star { namespace uno { class XComponentContext; } } } }

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::office;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::geometry;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star;

namespace sd {

class Annotation : private ::cppu::BaseMutex,
                   public ::cppu::WeakComponentImplHelper< XAnnotation>,
                   public ::cppu::PropertySetMixin< XAnnotation >
{
public:
    explicit Annotation( const Reference< XComponentContext >& context, SdPage* pPage );
    Annotation(const Annotation&) = delete;
    Annotation& operator=(const Annotation&) = delete;

    static sal_uInt32 m_nLastId;

    SdPage* GetPage() const { return mpPage; }
    SdrModel* GetModel() { return (mpPage != nullptr) ? &mpPage->getSdrModelFromSdrPage() : nullptr; }
    sal_uInt32 GetId() const { return m_nId; }

    // XInterface:
    virtual Any SAL_CALL queryInterface(Type const & type) override;
    virtual void SAL_CALL acquire() throw () override { ::cppu::WeakComponentImplHelper< XAnnotation >::acquire(); }
    virtual void SAL_CALL release() throw () override { ::cppu::WeakComponentImplHelper< XAnnotation >::release(); }

    // css::beans::XPropertySet:
    virtual Reference< XPropertySetInfo > SAL_CALL getPropertySetInfo() override;
    virtual void SAL_CALL setPropertyValue(const OUString & aPropertyName, const Any & aValue) override;
    virtual Any SAL_CALL getPropertyValue(const OUString & PropertyName) override;
    virtual void SAL_CALL addPropertyChangeListener(const OUString & aPropertyName, const Reference< XPropertyChangeListener > & xListener) override;
    virtual void SAL_CALL removePropertyChangeListener(const OUString & aPropertyName, const Reference< XPropertyChangeListener > & aListener) override;
    virtual void SAL_CALL addVetoableChangeListener(const OUString & PropertyName, const Reference< XVetoableChangeListener > & aListener) override;
    virtual void SAL_CALL removeVetoableChangeListener(const OUString & PropertyName, const Reference< XVetoableChangeListener > & aListener) override;

    // css::office::XAnnotation:
    virtual css::uno::Any SAL_CALL getAnchor() override;
    virtual RealPoint2D SAL_CALL getPosition() override;
    virtual void SAL_CALL setPosition(const RealPoint2D & the_value) override;
    virtual css::geometry::RealSize2D SAL_CALL getSize() override;
    virtual void SAL_CALL setSize( const css::geometry::RealSize2D& _size ) override;
    virtual OUString SAL_CALL getAuthor() override;
    virtual void SAL_CALL setAuthor(const OUString & the_value) override;
    virtual OUString SAL_CALL getInitials() override;
    virtual void SAL_CALL setInitials(const OUString & the_value) override;
    virtual util::DateTime SAL_CALL getDateTime() override;
    virtual void SAL_CALL setDateTime(const util::DateTime & the_value) override;
    virtual Reference< XText > SAL_CALL getTextRange() override;

    void createChangeUndo();

private:
    // destructor is private and will be called indirectly by the release call    virtual ~Annotation() {}

    // override WeakComponentImplHelperBase::disposing()
    // This function is called upon disposing the component,
    // if your component needs special work when it becomes
    // disposed, do it here.
    virtual void SAL_CALL disposing() override;

    sal_uInt32 const m_nId;
    SdPage* mpPage;
    RealPoint2D m_Position;
    RealSize2D m_Size;
    OUString m_Author;
    OUString m_Initials;
    util::DateTime m_DateTime;
    rtl::Reference< TextApiObject > m_TextRange;
};

class UndoInsertOrRemoveAnnotation : public SdrUndoAction
{
public:
    UndoInsertOrRemoveAnnotation( Annotation& rAnnotation, bool bInsert );

    virtual void Undo() override;
    virtual void Redo() override;

protected:
    rtl::Reference< Annotation > mxAnnotation;
    bool const mbInsert;
    int mnIndex;
};

struct AnnotationData
{
    RealPoint2D m_Position;
    RealSize2D m_Size;
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
        Reference<XText> xText(xAnnotation->getTextRange());
        m_Text = xText->getString();
    }

    void set( const rtl::Reference< Annotation >& xAnnotation )
    {
        xAnnotation->setPosition(m_Position);
        xAnnotation->setSize(m_Size);
        xAnnotation->setAuthor(m_Author);
        xAnnotation->setInitials(m_Initials);
        xAnnotation->setDateTime(m_DateTime);
        Reference<XText> xText(xAnnotation->getTextRange());
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

void createAnnotation( Reference< XAnnotation >& xAnnotation, SdPage* pPage )
{
    xAnnotation.set(
        new Annotation(comphelper::getProcessComponentContext(), pPage));
    pPage->addAnnotation(xAnnotation, -1);
}

sal_uInt32 Annotation::m_nLastId = 1;

Annotation::Annotation( const Reference< XComponentContext >& context, SdPage* pPage )
: ::cppu::WeakComponentImplHelper< XAnnotation >(m_aMutex)
, ::cppu::PropertySetMixin< XAnnotation >(context, IMPLEMENTS_PROPERTY_SET, Sequence< OUString >())
, m_nId( m_nLastId++ )
, mpPage( pPage )
{
}

// override WeakComponentImplHelperBase::disposing()
// This function is called upon disposing the component,
// if your component needs special work when it becomes
// disposed, do it here.
void SAL_CALL Annotation::disposing()
{
    mpPage = nullptr;
    if( m_TextRange.is() )
    {
        m_TextRange->dispose();
        m_TextRange.clear();
    }
}

Any Annotation::queryInterface(Type const & type)
{
    return ::cppu::WeakComponentImplHelper< XAnnotation>::queryInterface(type);
}

// com.sun.star.beans.XPropertySet:
Reference< XPropertySetInfo > SAL_CALL Annotation::getPropertySetInfo()
{
    return ::cppu::PropertySetMixin< XAnnotation >::getPropertySetInfo();
}

void SAL_CALL Annotation::setPropertyValue(const OUString & aPropertyName, const Any & aValue)
{
    ::cppu::PropertySetMixin< XAnnotation >::setPropertyValue(aPropertyName, aValue);
}

Any SAL_CALL Annotation::getPropertyValue(const OUString & aPropertyName)
{
    return ::cppu::PropertySetMixin< XAnnotation >::getPropertyValue(aPropertyName);
}

void SAL_CALL Annotation::addPropertyChangeListener(const OUString & aPropertyName, const Reference< XPropertyChangeListener > & xListener)
{
    ::cppu::PropertySetMixin< XAnnotation >::addPropertyChangeListener(aPropertyName, xListener);
}

void SAL_CALL Annotation::removePropertyChangeListener(const OUString & aPropertyName, const Reference< XPropertyChangeListener > & xListener)
{
    ::cppu::PropertySetMixin< XAnnotation >::removePropertyChangeListener(aPropertyName, xListener);
}

void SAL_CALL Annotation::addVetoableChangeListener(const OUString & aPropertyName, const Reference< XVetoableChangeListener > & xListener)
{
    ::cppu::PropertySetMixin< XAnnotation >::addVetoableChangeListener(aPropertyName, xListener);
}

void SAL_CALL Annotation::removeVetoableChangeListener(const OUString & aPropertyName, const Reference< XVetoableChangeListener > & xListener)
{
    ::cppu::PropertySetMixin< XAnnotation >::removeVetoableChangeListener(aPropertyName, xListener);
}

Any SAL_CALL Annotation::getAnchor()
{
    osl::MutexGuard g(m_aMutex);
    Any aRet;
    if( mpPage )
    {
        Reference< XDrawPage > xPage( mpPage->getUnoPage(), UNO_QUERY );
        aRet <<= xPage;
    }
    return aRet;
}

// css::office::XAnnotation:
RealPoint2D SAL_CALL Annotation::getPosition()
{
    osl::MutexGuard g(m_aMutex);
    return m_Position;
}

void SAL_CALL Annotation::setPosition(const RealPoint2D & the_value)
{
    prepareSet("Position", Any(), Any(), nullptr);
    {
        osl::MutexGuard g(m_aMutex);
        createChangeUndo();
        m_Position = the_value;
    }
}

// css::office::XAnnotation:
RealSize2D SAL_CALL Annotation::getSize()
{
    osl::MutexGuard g(m_aMutex);
    return m_Size;
}

void SAL_CALL Annotation::setSize(const RealSize2D & the_value)
{
    prepareSet("Size", Any(), Any(), nullptr);
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
    prepareSet("Author", Any(), Any(), nullptr);
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
    prepareSet("Initials", Any(), Any(), nullptr);
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
    prepareSet("DateTime", Any(), Any(), nullptr);
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
        pModel->AddUndo( std::make_unique<UndoAnnotation>( *this ) );

    if( pModel )
    {
        pModel->SetChanged();
        Reference< XInterface > xSource( static_cast<uno::XWeak*>( this ) );
        NotifyDocumentEvent(
            static_cast< SdDrawDocument& >( *pModel ),
            "OnAnnotationChanged" ,
            xSource );
    }
}

Reference< XText > SAL_CALL Annotation::getTextRange()
{
    osl::MutexGuard g(m_aMutex);
    if( !m_TextRange.is() && (mpPage != nullptr) )
    {
        m_TextRange = TextApiObject::create( static_cast< SdDrawDocument* >( &mpPage->getSdrModelFromSdrPage() ) );
    }
    return Reference< XText >( m_TextRange.get() );
}

std::unique_ptr<SdrUndoAction> CreateUndoInsertOrRemoveAnnotation( const Reference< XAnnotation >& xAnnotation, bool bInsert )
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

void CreateChangeUndo(const css::uno::Reference< css::office::XAnnotation >& xAnnotation)
{
    Annotation* pAnnotation = dynamic_cast<Annotation*>(xAnnotation.get());
    if (pAnnotation)
        pAnnotation->createChangeUndo();
}

sal_uInt32 getAnnotationId(const Reference<XAnnotation>& xAnnotation)
{
    Annotation* pAnnotation = dynamic_cast<Annotation*>(xAnnotation.get());
    sal_uInt32 nId = 0;
    if (pAnnotation)
        nId = pAnnotation->GetId();
    return nId;
}

const SdPage* getAnnotationPage(const Reference<XAnnotation>& xAnnotation)
{
    Annotation* pAnnotation = dynamic_cast<Annotation*>(xAnnotation.get());
    if (pAnnotation)
        return pAnnotation->GetPage();
    return nullptr;
}

namespace
{
std::string lcl_LOKGetCommentPayload(CommentNotificationType nType, Reference<XAnnotation> const & rxAnnotation)
{
    boost::property_tree::ptree aAnnotation;
    aAnnotation.put("action", (nType == CommentNotificationType::Add ? "Add" :
                               (nType == CommentNotificationType::Remove ? "Remove" :
                                (nType == CommentNotificationType::Modify ? "Modify" : "???"))));
    aAnnotation.put("id", sd::getAnnotationId(rxAnnotation));
    if (nType != CommentNotificationType::Remove && rxAnnotation.is())
    {
        aAnnotation.put("id", sd::getAnnotationId(rxAnnotation));
        aAnnotation.put("author", rxAnnotation->getAuthor());
        aAnnotation.put("dateTime", utl::toISO8601(rxAnnotation->getDateTime()));
        uno::Reference<text::XText> xText(rxAnnotation->getTextRange());
        aAnnotation.put("text", xText->getString());
        const SdPage* pPage = sd::getAnnotationPage(rxAnnotation);
        aAnnotation.put("parthash", pPage ? OString::number(pPage->GetHashCode()) : OString());
    }

    boost::property_tree::ptree aTree;
    aTree.add_child("comment", aAnnotation);
    std::stringstream aStream;
    boost::property_tree::write_json(aStream, aTree);

    return aStream.str();
}
} // anonymous ns

void LOKCommentNotify(CommentNotificationType nType, const SfxViewShell* pViewShell, Reference<XAnnotation> const & rxAnnotation)
{
    // callbacks only if tiled annotations are explicitly turned off by LOK client
    if (!comphelper::LibreOfficeKit::isActive() || comphelper::LibreOfficeKit::isTiledAnnotations())
        return ;

    std::string aPayload = lcl_LOKGetCommentPayload(nType, rxAnnotation);
    pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_COMMENT, aPayload.c_str());
}

void LOKCommentNotifyAll(CommentNotificationType nType, Reference<XAnnotation> const & rxAnnotation)
{
    // callbacks only if tiled annotations are explicitly turned off by LOK client
    if (!comphelper::LibreOfficeKit::isActive() || comphelper::LibreOfficeKit::isTiledAnnotations())
        return ;

    std::string aPayload = lcl_LOKGetCommentPayload(nType, rxAnnotation);

    const SfxViewShell* pViewShell = SfxViewShell::GetFirst();
    while (pViewShell)
    {
        pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_COMMENT, aPayload.c_str());
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
        Reference< XAnnotation > xAnnotation( &rAnnotation );

        const AnnotationVector& rVec = pPage->getAnnotations();
        auto iter = std::find(rVec.begin(), rVec.end(), xAnnotation);
        mnIndex += std::distance(rVec.begin(), iter);
    }
}

void UndoInsertOrRemoveAnnotation::Undo()
{
    SdPage* pPage = mxAnnotation->GetPage();
    SdrModel* pModel = mxAnnotation->GetModel();
    if( !(pPage && pModel) )
        return;

    Reference< XAnnotation > xAnnotation( mxAnnotation.get() );
    if( mbInsert )
    {
        pPage->removeAnnotation( xAnnotation );
    }
    else
    {
        pPage->addAnnotation( xAnnotation, mnIndex );
        LOKCommentNotifyAll( CommentNotificationType::Add, xAnnotation );
    }
}

void UndoInsertOrRemoveAnnotation::Redo()
{
    SdPage* pPage = mxAnnotation->GetPage();
    SdrModel* pModel = mxAnnotation->GetModel();
    if( !(pPage && pModel) )
        return;

    Reference< XAnnotation > xAnnotation( mxAnnotation.get() );

    if( mbInsert )
    {
        pPage->addAnnotation( xAnnotation, mnIndex );
        LOKCommentNotifyAll( CommentNotificationType::Add, xAnnotation );
    }
    else
    {
        pPage->removeAnnotation( xAnnotation );
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
    Reference< XAnnotation > xAnnotation( mxAnnotation.get() );
    LOKCommentNotifyAll( CommentNotificationType::Modify, xAnnotation );
}

void UndoAnnotation::Redo()
{
    maUndoData.get( mxAnnotation );
    maRedoData.set( mxAnnotation );
    Reference< XAnnotation > xAnnotation( mxAnnotation.get() );
    LOKCommentNotifyAll( CommentNotificationType::Modify, xAnnotation );
}

} // namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
