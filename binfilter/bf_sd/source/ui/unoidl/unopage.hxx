/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SD_UNOPAGE_HXX
#define _SD_UNOPAGE_HXX

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/document/XLinkTargetSupplier.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/drawing/XMasterPageTarget.hpp>
#include <com/sun/star/presentation/XPresentationPage.hpp>
#include <cppuhelper/interfacecontainer.hxx>

#include <bf_svtools/itemprop.hxx>
#include <bf_svx/unopage.hxx>
#include <bf_svx/fmdpage.hxx>
#include <bf_svx/svdpool.hxx>

#include <comphelper/servicehelper.hxx>

namespace binfilter {

struct SfxItemPropertyMap;

class SdPage;
class SvxShape;
class SdrObject;

#ifdef SVX_LIGHT
#define SvxFmDrawPage SvxDrawPage
#endif

class SdGenericDrawPageMutex
{
protected:
    ::osl::Mutex maMutex;
};

/***********************************************************************
*                                                                      *
***********************************************************************/
class SdGenericDrawPage : public SvxFmDrawPage,
                          public ::com::sun::star::container::XNamed,
                          public ::com::sun::star::beans::XPropertySet,
                          public ::com::sun::star::document::XLinkTargetSupplier,
                          public ::com::sun::star::lang::XComponent,
                          public SdGenericDrawPageMutex
{
protected:
    friend class SdXImpressDocument;

    SvxItemPropertySet	maPropSet;
    SdXImpressDocument* mpModel;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > mxModel;

    virtual void setBackground( const ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::lang::IllegalArgumentException);
    virtual void getBackground( ::com::sun::star::uno::Any& rValue ) throw();

    ::rtl::OUString getBookmarkURL() const;
    void setBookmarkURL( ::rtl::OUString& rURL );

    void SetLftBorder( sal_Int32 nValue );
    void SetRgtBorder( sal_Int32 nValue );
    void SetUppBorder( sal_Int32 nValue );
    void SetLwrBorder( sal_Int32 nValue );

    void SetWidth( sal_Int32 nWidth );
    void SetHeight( sal_Int32 nHeight );

    sal_Bool mbHasBackgroundObject;

    cppu::OBroadcastHelper mrBHelper;

    virtual void disposing() throw();

public:
    SdGenericDrawPage( SdXImpressDocument* pModel, SdPage* pInPage, const SfxItemPropertyMap* pMap ) throw();
    virtual ~SdGenericDrawPage() throw();

    // intern
    void Invalidate() { pPage = NULL; mpModel = NULL; mxModel = NULL; }
    sal_Bool isValid() { return (pPage != NULL) && (mpModel != NULL); }

    SdPage* GetPage() const { return (SdPage*)pPage; }
    SdXImpressDocument* GetModel() const { return mpModel; }

        static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId() throw();
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    // this is called whenever a SdrObject must be created for a empty api shape wrapper
    virtual SdrObject *_CreateSdrObject( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape ) throw();

    // SvxFmDrawPage
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >  _CreateShape( SdrObject *pObj ) const throw ();

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL release() throw();

    // XComponent
    virtual void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw(::com::sun::star::uno::RuntimeException);

    // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XLinkTargetSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getLinks(  ) throw(::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);
};
} //namespace binfilter
#endif

/***********************************************************************
*                                                                      *
***********************************************************************/
namespace binfilter {//STRIP009
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

    static ::rtl::OUString getPageApiName( SdPage* pPage );
    static ::rtl::OUString getPageApiNameFromUiName( const String& rUIName );
    static String getUiNameFromPageApiName( const ::rtl::OUString& rApiName );

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire() throw();
    virtual void SAL_CALL release() throw();

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    // XMasterPageTarget
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > SAL_CALL getMasterPage(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setMasterPage( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& xMasterPage ) throw(::com::sun::star::uno::RuntimeException);

    // XPresentationPage
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > SAL_CALL getNotesPage(  ) throw(::com::sun::star::uno::RuntimeException);

    // XNamed
    virtual ::rtl::OUString SAL_CALL getName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setName( const ::rtl::OUString& aName ) throw(::com::sun::star::uno::RuntimeException);

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
    SdrObject* mpBackgroundObj;

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
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException) ;
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

    // XPresentationPage
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > SAL_CALL getNotesPage(  ) throw(::com::sun::star::uno::RuntimeException);

    // XNamed
    virtual ::rtl::OUString SAL_CALL getName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setName( const ::rtl::OUString& aName ) throw(::com::sun::star::uno::RuntimeException);

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
    SdrObject* FindObject( const String& rName ) const throw();

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw(::com::sun::star::uno::RuntimeException);

    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);
};

} //namespace binfilter
