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

#include "sal/config.h"

#include "boost/noncopyable.hpp"
#include "osl/time.h"

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/office/XAnnotation.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>

#include <comphelper/processfactory.hxx>
#include <cppuhelper/propertysetmixin.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>

#include "Annotation.hxx"
#include "drawdoc.hxx"
#include "notifydocumentevent.hxx"
#include "sdpage.hxx"
#include "textapi.hxx"

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
                   public ::cppu::PropertySetMixin< XAnnotation >,
                   private boost::noncopyable
{
public:
    explicit Annotation( const Reference< XComponentContext >& context, SdPage* pPage );

    SdPage* GetPage() const { return mpPage; }
    SdrModel* GetModel() { return (mpPage != nullptr) ? mpPage->GetModel() : nullptr; }

    // XInterface:
    virtual Any SAL_CALL queryInterface(Type const & type) throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL acquire() throw () override { ::cppu::WeakComponentImplHelper< XAnnotation >::acquire(); }
    virtual void SAL_CALL release() throw () override { ::cppu::WeakComponentImplHelper< XAnnotation >::release(); }

    // css::beans::XPropertySet:
    virtual Reference< XPropertySetInfo > SAL_CALL getPropertySetInfo() throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyValue(const OUString & aPropertyName, const Any & aValue) throw (RuntimeException, UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, std::exception) override;
    virtual Any SAL_CALL getPropertyValue(const OUString & PropertyName) throw (RuntimeException, UnknownPropertyException, WrappedTargetException, std::exception) override;
    virtual void SAL_CALL addPropertyChangeListener(const OUString & aPropertyName, const Reference< XPropertyChangeListener > & xListener) throw (RuntimeException, UnknownPropertyException, WrappedTargetException, std::exception) override;
    virtual void SAL_CALL removePropertyChangeListener(const OUString & aPropertyName, const Reference< XPropertyChangeListener > & aListener) throw (RuntimeException, UnknownPropertyException, WrappedTargetException, std::exception) override;
    virtual void SAL_CALL addVetoableChangeListener(const OUString & PropertyName, const Reference< XVetoableChangeListener > & aListener) throw (RuntimeException, UnknownPropertyException, WrappedTargetException, std::exception) override;
    virtual void SAL_CALL removeVetoableChangeListener(const OUString & PropertyName, const Reference< XVetoableChangeListener > & aListener) throw (RuntimeException, UnknownPropertyException, WrappedTargetException, std::exception) override;

    // css::office::XAnnotation:
    virtual css::uno::Any SAL_CALL getAnchor() throw (css::uno::RuntimeException, std::exception) override;
    virtual RealPoint2D SAL_CALL getPosition() throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL setPosition(const RealPoint2D & the_value) throw (RuntimeException, std::exception) override;
    virtual css::geometry::RealSize2D SAL_CALL getSize() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setSize( const css::geometry::RealSize2D& _size ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getAuthor() throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL setAuthor(const OUString & the_value) throw (RuntimeException, std::exception) override;
    virtual util::DateTime SAL_CALL getDateTime() throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL setDateTime(const util::DateTime & the_value) throw (RuntimeException, std::exception) override;
    virtual Reference< XText > SAL_CALL getTextRange() throw (RuntimeException, std::exception) override;

private:
    // destructor is private and will be called indirectly by the release call    virtual ~Annotation() {}

    void createChangeUndo();

    // override WeakComponentImplHelperBase::disposing()
    // This function is called upon disposing the component,
    // if your component needs special work when it becomes
    // disposed, do it here.
    virtual void SAL_CALL disposing() override;

    SdPage* mpPage;
    Reference< XComponentContext > m_xContext;
    mutable ::osl::Mutex m_aMutex;
    RealPoint2D m_Position;
    RealSize2D m_Size;
    OUString m_Author;
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
    bool mbInsert;
    int mnIndex;
};

struct AnnotationData
{
    RealPoint2D m_Position;
    RealSize2D m_Size;
    OUString m_Author;
    util::DateTime m_DateTime;

    void get( const rtl::Reference< Annotation >& xAnnotation )
    {
        m_Position = xAnnotation->getPosition();
        m_Size = xAnnotation->getSize();
        m_Author = xAnnotation->getAuthor();
        m_DateTime = xAnnotation->getDateTime();
    }

    void set( const rtl::Reference< Annotation >& xAnnotation )
    {
        xAnnotation->setPosition(m_Position);
        xAnnotation->setSize(m_Size);
        xAnnotation->setAuthor(m_Author);
        xAnnotation->setDateTime(m_DateTime);
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
    pPage->addAnnotation(xAnnotation);
}

Annotation::Annotation( const Reference< XComponentContext >& context, SdPage* pPage )
: ::cppu::WeakComponentImplHelper< XAnnotation >(m_aMutex)
, ::cppu::PropertySetMixin< XAnnotation >(context, static_cast< Implements >(IMPLEMENTS_PROPERTY_SET), Sequence< OUString >())
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

Any Annotation::queryInterface(Type const & type) throw (RuntimeException, std::exception)
{
    return ::cppu::WeakComponentImplHelper< XAnnotation>::queryInterface(type);
}

// com.sun.star.beans.XPropertySet:
Reference< XPropertySetInfo > SAL_CALL Annotation::getPropertySetInfo() throw (RuntimeException, std::exception)
{
    return ::cppu::PropertySetMixin< XAnnotation >::getPropertySetInfo();
}

void SAL_CALL Annotation::setPropertyValue(const OUString & aPropertyName, const Any & aValue) throw (RuntimeException, UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, std::exception)
{
    ::cppu::PropertySetMixin< XAnnotation >::setPropertyValue(aPropertyName, aValue);
}

Any SAL_CALL Annotation::getPropertyValue(const OUString & aPropertyName) throw (RuntimeException, UnknownPropertyException, WrappedTargetException, std::exception)
{
    return ::cppu::PropertySetMixin< XAnnotation >::getPropertyValue(aPropertyName);
}

void SAL_CALL Annotation::addPropertyChangeListener(const OUString & aPropertyName, const Reference< XPropertyChangeListener > & xListener) throw (RuntimeException, UnknownPropertyException, WrappedTargetException, std::exception)
{
    ::cppu::PropertySetMixin< XAnnotation >::addPropertyChangeListener(aPropertyName, xListener);
}

void SAL_CALL Annotation::removePropertyChangeListener(const OUString & aPropertyName, const Reference< XPropertyChangeListener > & xListener) throw (RuntimeException, UnknownPropertyException, WrappedTargetException, std::exception)
{
    ::cppu::PropertySetMixin< XAnnotation >::removePropertyChangeListener(aPropertyName, xListener);
}

void SAL_CALL Annotation::addVetoableChangeListener(const OUString & aPropertyName, const Reference< XVetoableChangeListener > & xListener) throw (RuntimeException, UnknownPropertyException, WrappedTargetException, std::exception)
{
    ::cppu::PropertySetMixin< XAnnotation >::addVetoableChangeListener(aPropertyName, xListener);
}

void SAL_CALL Annotation::removeVetoableChangeListener(const OUString & aPropertyName, const Reference< XVetoableChangeListener > & xListener) throw (RuntimeException, UnknownPropertyException, WrappedTargetException, std::exception)
{
    ::cppu::PropertySetMixin< XAnnotation >::removeVetoableChangeListener(aPropertyName, xListener);
}

Any SAL_CALL Annotation::getAnchor() throw (RuntimeException, std::exception)
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
RealPoint2D SAL_CALL Annotation::getPosition() throw (RuntimeException, std::exception)
{
    osl::MutexGuard g(m_aMutex);
    return m_Position;
}

void SAL_CALL Annotation::setPosition(const RealPoint2D & the_value) throw (RuntimeException, std::exception)
{
    prepareSet("Position", Any(), Any(), nullptr);
    {
        osl::MutexGuard g(m_aMutex);
        createChangeUndo();
        m_Position = the_value;
    }
}

// css::office::XAnnotation:
RealSize2D SAL_CALL Annotation::getSize() throw (RuntimeException, std::exception)
{
    osl::MutexGuard g(m_aMutex);
    return m_Size;
}

void SAL_CALL Annotation::setSize(const RealSize2D & the_value) throw (RuntimeException, std::exception)
{
    prepareSet("Size", Any(), Any(), nullptr);
    {
        osl::MutexGuard g(m_aMutex);
        createChangeUndo();
        m_Size = the_value;
    }
}

OUString SAL_CALL Annotation::getAuthor() throw (RuntimeException, std::exception)
{
    osl::MutexGuard g(m_aMutex);
    return m_Author;
}

void SAL_CALL Annotation::setAuthor(const OUString & the_value) throw (RuntimeException, std::exception)
{
    prepareSet("Author", Any(), Any(), nullptr);
    {
        osl::MutexGuard g(m_aMutex);
        createChangeUndo();
        m_Author = the_value;
    }
}

util::DateTime SAL_CALL Annotation::getDateTime() throw (RuntimeException, std::exception)
{
    osl::MutexGuard g(m_aMutex);
    return m_DateTime;
}

void SAL_CALL Annotation::setDateTime(const util::DateTime & the_value) throw (RuntimeException, std::exception)
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
    SdrModel* pModel = GetModel();
    if( pModel && pModel->IsUndoEnabled() )
        pModel->AddUndo( new UndoAnnotation( *this ) );

    if( pModel )
    {
        pModel->SetChanged();
        Reference< XInterface > xSource( static_cast<uno::XWeak*>( this ) );
        NotifyDocumentEvent( static_cast< SdDrawDocument* >( pModel ), "OnAnnotationChanged" , xSource );
    }
}

Reference< XText > SAL_CALL Annotation::getTextRange() throw (RuntimeException, std::exception)
{
    osl::MutexGuard g(m_aMutex);
    if( !m_TextRange.is() && (mpPage != nullptr) )
    {
        m_TextRange = TextApiObject::create( static_cast< SdDrawDocument* >( mpPage->GetModel() ) );
    }
    return Reference< XText >( m_TextRange.get() );
}

SdrUndoAction* CreateUndoInsertOrRemoveAnnotation( const Reference< XAnnotation >& xAnnotation, bool bInsert )
{
    Annotation* pAnnotation = dynamic_cast< Annotation* >( xAnnotation.get() );
    if( pAnnotation )
    {
        return new UndoInsertOrRemoveAnnotation( *pAnnotation, bInsert );
    }
    else
    {
        return nullptr;
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
        for( AnnotationVector::const_iterator iter = rVec.begin(); iter != rVec.end(); ++iter )
        {
            if( (*iter) == xAnnotation )
                break;

            mnIndex++;
        }
    }
}

void UndoInsertOrRemoveAnnotation::Undo()
{
    SdPage* pPage = mxAnnotation->GetPage();
    SdrModel* pModel = mxAnnotation->GetModel();
    if( pPage && pModel )
    {
        Reference< XAnnotation > xAnnotation( mxAnnotation.get() );
        if( mbInsert )
        {
            pPage->removeAnnotation( xAnnotation );
        }
        else
        {
            pPage->addAnnotation( xAnnotation, mnIndex );
        }
    }
}

void UndoInsertOrRemoveAnnotation::Redo()
{
    SdPage* pPage = mxAnnotation->GetPage();
    SdrModel* pModel = mxAnnotation->GetModel();
    if( pPage && pModel )
    {
        Reference< XAnnotation > xAnnotation( mxAnnotation.get() );

        if( mbInsert )
        {
            pPage->addAnnotation( xAnnotation, mnIndex );
        }
        else
        {
            pPage->removeAnnotation( xAnnotation );
        }
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
}

void UndoAnnotation::Redo()
{
    maUndoData.get( mxAnnotation );
    maRedoData.set( mxAnnotation );
}

} // namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
