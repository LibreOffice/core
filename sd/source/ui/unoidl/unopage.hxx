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
#ifndef _SD_UNOPAGE_HXX
#define _SD_UNOPAGE_HXX

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
                          public ::com::sun::star::drawing::XShapeCombiner,
                          public ::com::sun::star::drawing::XShapeBinder,
                          public ::com::sun::star::container::XNamed,
                          public ::com::sun::star::beans::XPropertySet,
                          public ::com::sun::star::beans::XMultiPropertySet,
                          public ::com::sun::star::animations::XAnimationNodeSupplier,
                             public ::com::sun::star::office::XAnnotationAccess,
                          public ::com::sun::star::document::XLinkTargetSupplier
{
private:
    SdXImpressDocument* mpModel;
    SdrModel* mpSdrModel;
    sal_Int16 mnTempPageNumber; // for printing handouts

protected:
    friend class SdXImpressDocument;

    const SvxItemPropertySet*   mpPropSet;

    virtual void setBackground( const ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::lang::IllegalArgumentException);
    virtual void getBackground( ::com::sun::star::uno::Any& rValue ) throw();

    OUString getBookmarkURL() const;
    void setBookmarkURL( OUString& rURL );

    void SetLftBorder( sal_Int32 nValue );
    void SetRgtBorder( sal_Int32 nValue );
    void SetUppBorder( sal_Int32 nValue );
    void SetLwrBorder( sal_Int32 nValue );

    void SetWidth( sal_Int32 nWidth );
    void SetHeight( sal_Int32 nHeight );

    bool     mbIsImpressDocument;

    virtual void disposing() throw();

    ::com::sun::star::uno::Any getNavigationOrder();
    void setNavigationOrder( const ::com::sun::star::uno::Any& rValue );

    void throwIfDisposed() const throw (::com::sun::star::uno::RuntimeException );

public:
    SdGenericDrawPage( SdXImpressDocument* pModel, SdPage* pInPage, const SvxItemPropertySet* pSet ) throw();
    virtual ~SdGenericDrawPage() throw();

    // intern
    sal_Bool isValid() { return (SvxDrawPage::mpPage != NULL) && (mpModel != NULL); }

    SdPage* GetPage() const { return (SdPage*)SvxDrawPage::mpPage; }
    SdXImpressDocument* GetModel() const;

    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId() throw();
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    // this is called whenever a SdrObject must be created for a empty api shape wrapper
    virtual SdrObject *_CreateSdrObject( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape ) throw();

    // SvxFmDrawPage
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >  _CreateShape( SdrObject *pObj ) const throw ();

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL release() throw();

    // XShapeCombiner
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > SAL_CALL combine( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xShapes ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL split( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xGroup ) throw(::com::sun::star::uno::RuntimeException);

    // XShapeBinder
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > SAL_CALL bind( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xShapes ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL unbind( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape ) throw(::com::sun::star::uno::RuntimeException);

    // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XMultiPropertySet
    virtual void SAL_CALL setPropertyValues( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aValues ) throw (::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL getPropertyValues( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertiesChangeListener( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertiesChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL firePropertiesChangeEvent( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);

    // XLinkTargetSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getLinks(  ) throw(::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    // XAnimationNodeSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode > SAL_CALL getAnimationNode(  ) throw (::com::sun::star::uno::RuntimeException);

    // XAnnotationAccess:
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::office::XAnnotation > SAL_CALL createAndInsertAnnotation() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeAnnotation(const ::com::sun::star::uno::Reference< ::com::sun::star::office::XAnnotation > & annotation) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::office::XAnnotationEnumeration > SAL_CALL createAnnotationEnumeration() throw (::com::sun::star::uno::RuntimeException);
};

/***********************************************************************
*                                                                      *
***********************************************************************/

class SdDrawPage : public ::com::sun::star::drawing::XMasterPageTarget,
                   public ::com::sun::star::presentation::XPresentationPage,
                   public SdGenericDrawPage
{
private:
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > maTypeSequence;

protected:
    virtual void setBackground( const ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::lang::IllegalArgumentException);
    virtual void getBackground( ::com::sun::star::uno::Any& rValue ) throw();
public:
    SdDrawPage( SdXImpressDocument* pModel, SdPage* pInPage ) throw();
    virtual ~SdDrawPage() throw();

    UNO3_GETIMPLEMENTATION_DECL( SdDrawPage )

    static OUString getPageApiName( SdPage* pPage );
    static OUString getPageApiNameFromUiName( const OUString& rUIName );
    static OUString getUiNameFromPageApiName( const OUString& rApiName );

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire() throw();
    virtual void SAL_CALL release() throw();

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    // XMasterPageTarget
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > SAL_CALL getMasterPage(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setMasterPage( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& xMasterPage ) throw(::com::sun::star::uno::RuntimeException);

    // XPresentationPage
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > SAL_CALL getNotesPage(  ) throw(::com::sun::star::uno::RuntimeException);

    // XNamed
    virtual OUString SAL_CALL getName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setName( const OUString& aName ) throw(::com::sun::star::uno::RuntimeException);

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException) ;
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

    // XShapes
    virtual void SAL_CALL add( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL remove( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape ) throw(::com::sun::star::uno::RuntimeException);
};

/***********************************************************************
*                                                                      *
***********************************************************************/

class SdMasterPage : public ::com::sun::star::presentation::XPresentationPage,
                     public SdGenericDrawPage
{
private:
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > maTypeSequence;
protected:
    virtual void setBackground( const ::com::sun::star::uno::Any& rValue ) throw( ::com::sun::star::lang::IllegalArgumentException  );
    virtual void getBackground( ::com::sun::star::uno::Any& rValue ) throw();

public:
    SdMasterPage( SdXImpressDocument* pModel, SdPage* pInPage ) throw();
    virtual ~SdMasterPage() throw();

    UNO3_GETIMPLEMENTATION_DECL(SdMasterPage)

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire() throw();
    virtual void SAL_CALL release() throw();

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException) ;
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

    // XPresentationPage
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > SAL_CALL getNotesPage(  ) throw(::com::sun::star::uno::RuntimeException);

    // XNamed
    virtual OUString SAL_CALL getName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setName( const OUString& aName ) throw(::com::sun::star::uno::RuntimeException);

    // XShapes
    virtual void SAL_CALL add( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL remove( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape ) throw(::com::sun::star::uno::RuntimeException);
};


/***********************************************************************
*                                                                      *
***********************************************************************/
#include <cppuhelper/implbase2.hxx>

class SdPageLinkTargets : public ::cppu::WeakImplHelper2< ::com::sun::star::container::XNameAccess,
                                                  ::com::sun::star::lang::XServiceInfo >
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > mxPage;
    SdGenericDrawPage* mpUnoPage;

public:
    SdPageLinkTargets( SdGenericDrawPage* pUnoPage ) throw();
    virtual ~SdPageLinkTargets() throw();

    // intern
    SdrObject* FindObject( const OUString& rName ) const throw();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const OUString& aName ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) throw(::com::sun::star::uno::RuntimeException);

    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);
};

#endif // _SD_UNOPAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
