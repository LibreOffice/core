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
#ifndef INCLUDED_SD_SOURCE_UI_UNOIDL_UNOPAGE_HXX
#define INCLUDED_SD_SOURCE_UI_UNOIDL_UNOPAGE_HXX

#include <com/sun/star/document/XLinkTargetSupplier.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/drawing/XMasterPageTarget.hpp>
#include <com/sun/star/presentation/XPresentationPage.hpp>
#include <com/sun/star/animations/XAnimationNodeSupplier.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/office/XAnnotationAccess.hpp>

#include <svl/itemprop.hxx>

#include <svx/unopage.hxx>
#include <svx/fmdpage.hxx>
#include <svx/svdpool.hxx>

#include <comphelper/servicehelper.hxx>

#include "unosrch.hxx"

class SdPage;
class SdrObject;
class SdXImpressDocument;
struct SfxItemPropertySimpleEntry;

/***********************************************************************
*                                                                      *
***********************************************************************/
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
    SdXImpressDocument* mpModel;
    SdrModel* mpSdrModel;
    sal_Int16 mnTempPageNumber; // for printing handouts

protected:
    friend class SdXImpressDocument;

    const SvxItemPropertySet*   mpPropSet;

    virtual void setBackground( const css::uno::Any& rValue ) throw(css::lang::IllegalArgumentException, std::exception);
    virtual void getBackground( css::uno::Any& rValue ) throw(std::exception);

    OUString getBookmarkURL() const;
    void setBookmarkURL( OUString& rURL );

    void SetLftBorder( sal_Int32 nValue );
    void SetRgtBorder( sal_Int32 nValue );
    void SetUppBorder( sal_Int32 nValue );
    void SetLwrBorder( sal_Int32 nValue );

    void SetWidth( sal_Int32 nWidth );
    void SetHeight( sal_Int32 nHeight );

    bool     mbIsImpressDocument;

    virtual void disposing() throw() override;

    css::uno::Any getNavigationOrder();
    void setNavigationOrder( const css::uno::Any& rValue );

    void throwIfDisposed() const throw (css::uno::RuntimeException );

public:
    SdGenericDrawPage( SdXImpressDocument* pModel, SdPage* pInPage, const SvxItemPropertySet* pSet ) throw();
    virtual ~SdGenericDrawPage() throw();

    // intern
    bool isValid() { return (SvxDrawPage::mpPage != nullptr) && (mpModel != nullptr); }

    SdPage* GetPage() const { return static_cast<SdPage*>(SvxDrawPage::mpPage); }
    SdXImpressDocument* GetModel() const;

    static const css::uno::Sequence< sal_Int8 > & getUnoTunnelId() throw();
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) throw(css::uno::RuntimeException, std::exception) override;

    // this is called whenever a SdrObject must be created for a empty api shape wrapper
    virtual SdrObject *_CreateSdrObject( const css::uno::Reference< css::drawing::XShape >& xShape )
        throw (css::uno::RuntimeException, std::exception) override;

    // SvxFmDrawPage
    virtual css::uno::Reference<css::drawing::XShape>  _CreateShape(SdrObject *pObj) const
        throw (css::uno::RuntimeException, std::exception) override;

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL release() throw() override;

    // XShapeCombiner
    virtual css::uno::Reference< css::drawing::XShape > SAL_CALL combine( const css::uno::Reference< css::drawing::XShapes >& xShapes ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL split( const css::uno::Reference< css::drawing::XShape >& xGroup ) throw(css::uno::RuntimeException, std::exception) override;

    // XShapeBinder
    virtual css::uno::Reference< css::drawing::XShape > SAL_CALL bind( const css::uno::Reference< css::drawing::XShapes >& xShapes ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL unbind( const css::uno::Reference< css::drawing::XShape >& xShape ) throw(css::uno::RuntimeException, std::exception) override;

    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue )
        throw (css::beans::UnknownPropertyException,
               css::beans::PropertyVetoException,
               css::lang::IllegalArgumentException,
               css::lang::WrappedTargetException,
               css::uno::RuntimeException,
               std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    // XMultiPropertySet
    virtual void SAL_CALL setPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Sequence< css::uno::Any >& aValues ) throw (css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::uno::Any > SAL_CALL getPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addPropertiesChangeListener( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePropertiesChangeListener( const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL firePropertiesChangeEvent( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;

    // XLinkTargetSupplier
    virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getLinks(  ) throw(css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;

    // XAnimationNodeSupplier
    virtual css::uno::Reference< css::animations::XAnimationNode > SAL_CALL getAnimationNode(  ) throw (css::uno::RuntimeException, std::exception) override;

    // XAnnotationAccess:
    virtual css::uno::Reference< css::office::XAnnotation > SAL_CALL createAndInsertAnnotation() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeAnnotation(const css::uno::Reference< css::office::XAnnotation > & annotation) throw (css::uno::RuntimeException, css::lang::IllegalArgumentException, std::exception) override;
    virtual css::uno::Reference< css::office::XAnnotationEnumeration > SAL_CALL createAnnotationEnumeration() throw (css::uno::RuntimeException, std::exception) override;
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
    virtual void setBackground( const css::uno::Any& rValue ) throw(css::lang::IllegalArgumentException) override;
    virtual void getBackground( css::uno::Any& rValue ) throw() override;
public:
    SdDrawPage( SdXImpressDocument* pModel, SdPage* pInPage ) throw();
    virtual ~SdDrawPage() throw();

    UNO3_GETIMPLEMENTATION_DECL( SdDrawPage )

    static OUString getPageApiName( SdPage* pPage );
    static OUString getPageApiNameFromUiName( const OUString& rUIName );
    static OUString getUiNameFromPageApiName( const OUString& rApiName );

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL acquire() throw() override;
    virtual void SAL_CALL release() throw() override;

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw(css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;

    // XMasterPageTarget
    virtual css::uno::Reference< css::drawing::XDrawPage > SAL_CALL getMasterPage(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setMasterPage( const css::uno::Reference< css::drawing::XDrawPage >& xMasterPage ) throw(css::uno::RuntimeException, std::exception) override;

    // XPresentationPage
    virtual css::uno::Reference< css::drawing::XDrawPage > SAL_CALL getNotesPage(  ) throw(css::uno::RuntimeException, std::exception) override;

    // XNamed
    virtual OUString SAL_CALL getName(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setName( const OUString& aName ) throw(css::uno::RuntimeException, std::exception) override;

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(css::uno::RuntimeException, std::exception) override ;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw(css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw(css::uno::RuntimeException, std::exception) override;

    // XShapes
    virtual void SAL_CALL add( const css::uno::Reference< css::drawing::XShape >& xShape ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL remove( const css::uno::Reference< css::drawing::XShape >& xShape )
        throw (css::uno::RuntimeException,
               std::exception) override;
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
    virtual void setBackground( const css::uno::Any& rValue ) throw( css::lang::IllegalArgumentException, std::exception  ) override;
    virtual void getBackground( css::uno::Any& rValue ) throw (std::exception) override;

public:
    SdMasterPage( SdXImpressDocument* pModel, SdPage* pInPage ) throw();
    virtual ~SdMasterPage() throw();

    UNO3_GETIMPLEMENTATION_DECL(SdMasterPage)

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL acquire() throw() override;
    virtual void SAL_CALL release() throw() override;

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw(css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(css::uno::RuntimeException, std::exception) override ;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw(css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw(css::uno::RuntimeException, std::exception) override;

    // XPresentationPage
    virtual css::uno::Reference< css::drawing::XDrawPage > SAL_CALL getNotesPage(  ) throw(css::uno::RuntimeException, std::exception) override;

    // XNamed
    virtual OUString SAL_CALL getName(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setName( const OUString& aName ) throw(css::uno::RuntimeException, std::exception) override;

    // XShapes
    virtual void SAL_CALL add( const css::uno::Reference< css::drawing::XShape >& xShape ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL remove( const css::uno::Reference< css::drawing::XShape >& xShape )
        throw (css::uno::RuntimeException,
               std::exception) override;
};

/***********************************************************************
*                                                                      *
***********************************************************************/
#include <cppuhelper/implbase.hxx>

class SdPageLinkTargets : public ::cppu::WeakImplHelper< css::container::XNameAccess,
                                                  css::lang::XServiceInfo >
{
private:
    css::uno::Reference< css::drawing::XDrawPage > mxPage;
    SdGenericDrawPage* mpUnoPage;

public:
    SdPageLinkTargets( SdGenericDrawPage* pUnoPage ) throw();
    virtual ~SdPageLinkTargets() throw();

    // intern
    SdrObject* FindObject( const OUString& rName ) const throw();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;

    // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) throw(css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) throw(css::uno::RuntimeException, std::exception) override;

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw(css::uno::RuntimeException, std::exception) override;
};

OUString getUiNameFromPageApiNameImpl( const OUString& rApiName );

#endif // INCLUDED_SD_SOURCE_UI_UNOIDL_UNOPAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
