/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <com/sun/star/document/XLinkTargetSupplier.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/drawing/XMasterPageTarget.hpp>
#include <com/sun/star/presentation/XPresentationPage.hpp>
#include <com/sun/star/animations/XAnimationNodeSupplier.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/office/XAnnotationAccess.hpp>

#include <svx/unopage.hxx>

#include <comphelper/servicehelper.hxx>
#include <cppuhelper/implbase.hxx>

#include "unosrch.hxx"
#include <sdpage.hxx>

class SdrObject;
class SdXImpressDocument;
class SdMasterPage;

/** The shape type a presentation object of the given kind claims - what the placeholder is, which
    can differ from what currently represents it on the page. */
OUString GetPresObjShapeType(PresObjKind eKind);

class SdGenericDrawPage : public SvxDrawPage,
                          public SdUnoSearchReplaceShape,
                          public css::container::XNamed,
                          public css::beans::XPropertySet,
                          public css::beans::XMultiPropertySet,
                          public css::animations::XAnimationNodeSupplier,
                          public css::office::XAnnotationAccess,
                          public css::document::XLinkTargetSupplier
{
private:
    SdXImpressDocument* mpDocModel;
    SdrModel* mpSdrModel;
    bool      mbIsImpressDocument;
    sal_Int16 mnTempPageNumber; // for printing handouts
    cpo::uno::Any  mSlideLayout;  // import slide layout pptx
    const SvxItemPropertySet*   mpPropSet;

    void UpdateModel();

protected:
    friend class SdXImpressDocument;

    /// @throws css::lang::IllegalArgumentException
    virtual void setBackground( const cpo::uno::Any& rValue );
    /// @throws std::exception
    virtual void getBackground( cpo::uno::Any& rValue );

    OUString getBookmarkURL() const;
    void setBookmarkURL( std::u16string_view rURL );
    OUString getSourceModifiedTime() const;
    void setSourceModifiedTime( const OUString& rTime );
    OUString getSourcePageGuid() const;
    void setSourcePageGuid( const OUString& rGuid );

    void SetLeftBorder( sal_Int32 nValue );
    void SetRightBorder( sal_Int32 nValue );
    void SetUpperBorder( sal_Int32 nValue );
    void SetLowerBorder( sal_Int32 nValue );

    void SetWidth( sal_Int32 nWidth );
    void SetHeight( sal_Int32 nHeight );

    bool IsImpressDocument() const;

    virtual void disposing() noexcept override;

    cpo::uno::Any getNavigationOrder();
    void setNavigationOrder( const cpo::uno::Any& rValue );

    /// @throws cpo::uno::RuntimeException
    void throwIfDisposed() const;

public:
    SdGenericDrawPage(SdXImpressDocument* pModel, SdPage* pInPage, const SvxItemPropertySet* pSet);
    virtual ~SdGenericDrawPage() noexcept override;

    // internal
    bool isValid() const { return (SvxDrawPage::mpPage != nullptr) && (mpModel != nullptr); }

    SdPage* GetPage() const { return static_cast<SdPage*>(SvxDrawPage::mpPage); }
    SdXImpressDocument* GetModel() const;

    static const cpo::uno::Sequence< sal_Int8 > & getUnoTunnelId() noexcept;
    virtual sal_Int64 getSomething( const cpo::uno::Sequence< sal_Int8 >& aIdentifier ) override;

    // this is called whenever a SdrObject must be created for an empty api shape wrapper
    virtual rtl::Reference<SdrObject> CreateSdrObject_( const cpo::uno::Reference< css::drawing::XShape >& xShape ) override;

    // SvxDrawPage
    virtual cpo::uno::Reference<css::drawing::XShape>  CreateShape(SdrObject *pObj) const override;

    // XInterface
    virtual void acquire() noexcept override { SvxDrawPage::acquire(); }
    virtual void release() noexcept override { SvxDrawPage::release(); }
    virtual cpo::uno::Any queryInterface( const cpo::uno::Type & rType ) override;

    // XPropertySet
    virtual cpo::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo() override;
    virtual void setPropertyValue( const OUString& aPropertyName, const cpo::uno::Any& aValue ) override;
    virtual cpo::uno::Any getPropertyValue( const OUString& PropertyName ) override;
    virtual void addPropertyChangeListener( const OUString& aPropertyName, const cpo::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void removePropertyChangeListener( const OUString& aPropertyName, const cpo::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void addVetoableChangeListener( const OUString& PropertyName, const cpo::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void removeVetoableChangeListener( const OUString& PropertyName, const cpo::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    // XMultiPropertySet
    virtual void setPropertyValues( const cpo::uno::Sequence< OUString >& aPropertyNames, const cpo::uno::Sequence< cpo::uno::Any >& aValues ) override;
    virtual cpo::uno::Sequence< cpo::uno::Any > getPropertyValues( const cpo::uno::Sequence< OUString >& aPropertyNames ) override;
    virtual void addPropertiesChangeListener( const cpo::uno::Sequence< OUString >& aPropertyNames, const cpo::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;
    virtual void removePropertiesChangeListener( const cpo::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;
    virtual void firePropertiesChangeEvent( const cpo::uno::Sequence< OUString >& aPropertyNames, const cpo::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;

    // XLinkTargetSupplier
    virtual cpo::uno::Reference< css::container::XNameAccess > getLinks(  ) override;

    // XServiceInfo
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

    // XAnimationNodeSupplier
    virtual cpo::uno::Reference< css::animations::XAnimationNode > getAnimationNode(  ) override;

    // XAnnotationAccess:
    virtual cpo::uno::Reference< css::office::XAnnotation > createAndInsertAnnotation() override;
    virtual void removeAnnotation(const cpo::uno::Reference< css::office::XAnnotation > & annotation) override;
    virtual cpo::uno::Reference< css::office::XAnnotationEnumeration > createAnnotationEnumeration() override;
};

/***********************************************************************
*                                                                      *
***********************************************************************/

class SdDrawPage final : public css::drawing::XMasterPageTarget,
                   public css::presentation::XPresentationPage,
                   public SdGenericDrawPage
{
private:
    cpo::uno::Sequence< cpo::uno::Type > maTypeSequence;

    virtual void setBackground( const cpo::uno::Any& rValue ) override;
    virtual void getBackground( cpo::uno::Any& rValue ) override;
public:
    SdDrawPage(SdXImpressDocument* pModel, SdPage* pInPage);
    virtual ~SdDrawPage() noexcept override;

    UNO3_GETIMPLEMENTATION_DECL( SdDrawPage )

    static OUString getPageApiName( SdPage const * pPage );
    static OUString getPageApiNameFromUiName( const OUString& rUIName );
    static OUString getUiNameFromPageApiName( const OUString& rApiName );

    // XInterface
    virtual cpo::uno::Any queryInterface( const cpo::uno::Type & rType ) override;
    virtual void acquire() noexcept override;
    virtual void release() noexcept override;

    // XTypeProvider
    virtual cpo::uno::Sequence< cpo::uno::Type > getTypes() override;
    virtual cpo::uno::Sequence< sal_Int8 > getImplementationId() override;

    // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

    // XMasterPageTarget
    virtual cpo::uno::Reference< css::drawing::XDrawPage > getMasterPage(  ) override;
    virtual void setMasterPage( const cpo::uno::Reference< css::drawing::XDrawPage >& xMasterPage ) override;

    // XPresentationPage
    virtual cpo::uno::Reference< css::drawing::XDrawPage > getNotesPage(  ) override;

    // XNamed
    virtual OUString getName(  ) override;
    virtual void setName( const OUString& aName ) override;

    // XIndexAccess
    virtual sal_Int32 getCount() override ;
    virtual cpo::uno::Any getByIndex( sal_Int32 Index ) override;

    // XElementAccess
    virtual cpo::uno::Type getElementType() override;
    virtual bool hasElements() override;

    // XShapes
    virtual void add( const cpo::uno::Reference< css::drawing::XShape >& xShape ) override;
    virtual void remove( const cpo::uno::Reference< css::drawing::XShape >& xShape ) override;

    SdMasterPage* getSdMasterPage();
};

/***********************************************************************
*                                                                      *
***********************************************************************/

class SdMasterPage final : public css::presentation::XPresentationPage,
                     public SdGenericDrawPage
{
private:
    cpo::uno::Sequence< cpo::uno::Type > maTypeSequence;

    virtual void setBackground( const cpo::uno::Any& rValue ) override;
    virtual void getBackground( cpo::uno::Any& rValue ) override;

public:
    SdMasterPage(SdXImpressDocument* pModel, SdPage* pInPage);
    virtual ~SdMasterPage() noexcept override;

    UNO3_GETIMPLEMENTATION_DECL(SdMasterPage)

    // XInterface
    virtual cpo::uno::Any queryInterface( const cpo::uno::Type & rType ) override;
    virtual void acquire() noexcept override;
    virtual void release() noexcept override;

    // XTypeProvider
    virtual cpo::uno::Sequence< cpo::uno::Type > getTypes() override;
    virtual cpo::uno::Sequence< sal_Int8 > getImplementationId() override;

    // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

    // XIndexAccess
    virtual sal_Int32 getCount() override ;
    virtual cpo::uno::Any getByIndex( sal_Int32 Index ) override;

    // XElementAccess
    virtual cpo::uno::Type getElementType() override;
    virtual bool hasElements() override;

    // XPresentationPage
    virtual cpo::uno::Reference< css::drawing::XDrawPage > getNotesPage(  ) override;

    // XNamed
    virtual OUString getName(  ) override;
    virtual void setName( const OUString& aName ) override;

    // XShapes
    virtual void add( const cpo::uno::Reference< css::drawing::XShape >& xShape ) override;
    virtual void remove( const cpo::uno::Reference< css::drawing::XShape >& xShape ) override;
};

/***********************************************************************
*                                                                      *
***********************************************************************/

class SdPageLinkTargets final : public ::cppu::WeakImplHelper< css::container::XNameAccess,
                                                  css::lang::XServiceInfo >
{
private:
    rtl::Reference< SdGenericDrawPage > mxPage;

public:
    SdPageLinkTargets( SdGenericDrawPage* pUnoPage ) noexcept;
    virtual ~SdPageLinkTargets() noexcept override;

    // internal
    SdrObject* FindObject( std::u16string_view rName ) const noexcept;

    // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

    // XNameAccess
    virtual cpo::uno::Any getByName( const OUString& aName ) override;
    virtual cpo::uno::Sequence< OUString > getElementNames() override;
    virtual bool hasByName( const OUString& aName ) override;

    // XElementAccess
    virtual cpo::uno::Type getElementType() override;
    virtual bool hasElements() override;
};

OUString getUiNameFromPageApiNameImpl( const OUString& rApiName );

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
