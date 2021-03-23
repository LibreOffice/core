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

#include <com/sun/star/document/XLinkTargetSupplier.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/drawing/XMasterPageTarget.hpp>
#include <com/sun/star/drawing/XShapeCombiner.hpp>
#include <com/sun/star/drawing/XShapeBinder.hpp>
#include <com/sun/star/presentation/XPresentationPage.hpp>
#include <com/sun/star/animations/XAnimationNodeSupplier.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/office/XAnnotationAccess.hpp>

#include <svx/unopage.hxx>
#include <svx/fmdpage.hxx>

#include <comphelper/servicehelper.hxx>
#include <cppuhelper/implbase.hxx>

#include "unosrch.hxx"
#include <sdpage.hxx>

class SdrObject;
class SdXImpressDocument;

class SdGenericDrawPage : public SvxFmDrawPage,
                          public SdUnoSearchReplaceShape,
                          public css::drawing::XShapeCombiner,
                          public css::drawing::XShapeBinder,
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

    void UpdateModel();

protected:
    friend class SdXImpressDocument;

    const SvxItemPropertySet*   mpPropSet;

    /// @throws css::lang::IllegalArgumentException
    virtual void setBackground( const css::uno::Any& rValue );
    /// @throws std::exception
    virtual void getBackground( css::uno::Any& rValue );

    OUString getBookmarkURL() const;
    void setBookmarkURL( OUString const & rURL );

    void SetLeftBorder( sal_Int32 nValue );
    void SetRightBorder( sal_Int32 nValue );
    void SetUpperBorder( sal_Int32 nValue );
    void SetLowerBorder( sal_Int32 nValue );

    void SetWidth( sal_Int32 nWidth );
    void SetHeight( sal_Int32 nHeight );

    bool IsImpressDocument() const;

    virtual void disposing() throw() override;

    css::uno::Any getNavigationOrder();
    void setNavigationOrder( const css::uno::Any& rValue );

    /// @throws css::uno::RuntimeException
    void throwIfDisposed() const;

public:
    SdGenericDrawPage(SdXImpressDocument* pModel, SdPage* pInPage, const SvxItemPropertySet* pSet);
    virtual ~SdGenericDrawPage() throw() override;

    // intern
    bool isValid() const { return (SvxDrawPage::mpPage != nullptr) && (mpModel != nullptr); }

    SdPage* GetPage() const { return static_cast<SdPage*>(SvxDrawPage::mpPage); }
    SdXImpressDocument* GetModel() const;

    static const css::uno::Sequence< sal_Int8 > & getUnoTunnelId() throw();
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;

    // this is called whenever a SdrObject must be created for an empty api shape wrapper
    virtual SdrObject *CreateSdrObject_( const css::uno::Reference< css::drawing::XShape >& xShape ) override;

    // SvxFmDrawPage
    virtual css::uno::Reference<css::drawing::XShape>  CreateShape(SdrObject *pObj) const override;

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    virtual void SAL_CALL release() throw() override;

    // XShapeCombiner
    virtual css::uno::Reference< css::drawing::XShape > SAL_CALL combine( const css::uno::Reference< css::drawing::XShapes >& xShapes ) override;
    virtual void SAL_CALL split( const css::uno::Reference< css::drawing::XShape >& xGroup ) override;

    // XShapeBinder
    virtual css::uno::Reference< css::drawing::XShape > SAL_CALL bind( const css::uno::Reference< css::drawing::XShapes >& xShapes ) override;
    virtual void SAL_CALL unbind( const css::uno::Reference< css::drawing::XShape >& xShape ) override;

    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    // XMultiPropertySet
    virtual void SAL_CALL setPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Sequence< css::uno::Any >& aValues ) override;
    virtual css::uno::Sequence< css::uno::Any > SAL_CALL getPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames ) override;
    virtual void SAL_CALL addPropertiesChangeListener( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertiesChangeListener( const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;
    virtual void SAL_CALL firePropertiesChangeEvent( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;

    // XLinkTargetSupplier
    virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getLinks(  ) override;

    // XServiceInfo
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XAnimationNodeSupplier
    virtual css::uno::Reference< css::animations::XAnimationNode > SAL_CALL getAnimationNode(  ) override;

    // XAnnotationAccess:
    virtual css::uno::Reference< css::office::XAnnotation > SAL_CALL createAndInsertAnnotation() override;
    virtual void SAL_CALL removeAnnotation(const css::uno::Reference< css::office::XAnnotation > & annotation) override;
    virtual css::uno::Reference< css::office::XAnnotationEnumeration > SAL_CALL createAnnotationEnumeration() override;
};

/***********************************************************************
*                                                                      *
***********************************************************************/

class SdDrawPage : public css::drawing::XMasterPageTarget,
                   public css::presentation::XPresentationPage,
                   public SdGenericDrawPage
{
private:
    css::uno::Sequence< css::uno::Type > maTypeSequence;

protected:
    virtual void setBackground( const css::uno::Any& rValue ) override;
    virtual void getBackground( css::uno::Any& rValue ) override;
public:
    SdDrawPage(SdXImpressDocument* pModel, SdPage* pInPage);
    virtual ~SdDrawPage() throw() override;

    UNO3_GETIMPLEMENTATION_DECL( SdDrawPage )

    static OUString getPageApiName( SdPage const * pPage );
    static OUString getPageApiNameFromUiName( const OUString& rUIName );
    static OUString getUiNameFromPageApiName( const OUString& rApiName );

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    virtual void SAL_CALL acquire() throw() override;
    virtual void SAL_CALL release() throw() override;

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XMasterPageTarget
    virtual css::uno::Reference< css::drawing::XDrawPage > SAL_CALL getMasterPage(  ) override;
    virtual void SAL_CALL setMasterPage( const css::uno::Reference< css::drawing::XDrawPage >& xMasterPage ) override;

    // XPresentationPage
    virtual css::uno::Reference< css::drawing::XDrawPage > SAL_CALL getNotesPage(  ) override;

    // XNamed
    virtual OUString SAL_CALL getName(  ) override;
    virtual void SAL_CALL setName( const OUString& aName ) override;

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override ;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override;

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

    // XShapes
    virtual void SAL_CALL add( const css::uno::Reference< css::drawing::XShape >& xShape ) override;
    virtual void SAL_CALL remove( const css::uno::Reference< css::drawing::XShape >& xShape ) override;
};

/***********************************************************************
*                                                                      *
***********************************************************************/

class SdMasterPage : public css::presentation::XPresentationPage,
                     public SdGenericDrawPage
{
private:
    css::uno::Sequence< css::uno::Type > maTypeSequence;
protected:
    virtual void setBackground( const css::uno::Any& rValue ) override;
    virtual void getBackground( css::uno::Any& rValue ) override;

public:
    SdMasterPage(SdXImpressDocument* pModel, SdPage* pInPage);
    virtual ~SdMasterPage() throw() override;

    UNO3_GETIMPLEMENTATION_DECL(SdMasterPage)

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    virtual void SAL_CALL acquire() throw() override;
    virtual void SAL_CALL release() throw() override;

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override ;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override;

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

    // XPresentationPage
    virtual css::uno::Reference< css::drawing::XDrawPage > SAL_CALL getNotesPage(  ) override;

    // XNamed
    virtual OUString SAL_CALL getName(  ) override;
    virtual void SAL_CALL setName( const OUString& aName ) override;

    // XShapes
    virtual void SAL_CALL add( const css::uno::Reference< css::drawing::XShape >& xShape ) override;
    virtual void SAL_CALL remove( const css::uno::Reference< css::drawing::XShape >& xShape ) override;
};

/***********************************************************************
*                                                                      *
***********************************************************************/

class SdPageLinkTargets : public ::cppu::WeakImplHelper< css::container::XNameAccess,
                                                  css::lang::XServiceInfo >
{
private:
    css::uno::Reference< css::drawing::XDrawPage > mxPage;
    SdGenericDrawPage* mpUnoPage;

public:
    SdPageLinkTargets( SdGenericDrawPage* pUnoPage ) throw();
    virtual ~SdPageLinkTargets() throw() override;

    // intern
    SdrObject* FindObject( std::u16string_view rName ) const throw();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override;

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;
};

OUString getUiNameFromPageApiNameImpl( const OUString& rApiName );

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
