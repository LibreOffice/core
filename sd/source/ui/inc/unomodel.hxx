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
#ifndef INCLUDED_SD_SOURCE_UI_INC_UNOMODEL_HXX
#define INCLUDED_SD_SOURCE_UI_INC_UNOMODEL_HXX

#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/document/XLinkTargetSupplier.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPageSummarizer.hpp>
#include <com/sun/star/drawing/XDrawPageDuplicator.hpp>
#include <com/sun/star/drawing/XLayerSupplier.hpp>
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#include <com/sun/star/presentation/XPresentationSupplier.hpp>
#include <com/sun/star/presentation/XCustomPresentationSupplier.hpp>
#include <com/sun/star/drawing/XLayerManager.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/presentation/XPresentation.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/ucb/XAnyCompareFactory.hpp>
#include <com/sun/star/i18n/XForbiddenCharacters.hpp>
#include <com/sun/star/presentation/XHandoutMasterSupplier.hpp>
#include <com/sun/star/view/XRenderable.hpp>
#include <com/sun/star/util/MeasureUnit.hpp>

#include <rtl/ref.hxx>

#include <svl/lstner.hxx>
#include <sfx2/sfxbasemodel.hxx>
#include <svx/fmdmod.hxx>

#include <vcl/ITiledRenderable.hxx>

#include <editeng/unoipset.hxx>

#include <comphelper/servicehelper.hxx>
#include <sddllapi.h>

class SdDrawDocument;
class SdPage;

namespace sd {
class DrawDocShell;
class DrawViewShell;
}

extern OUString getPageApiName( SdPage* pPage );
extern OUString getPageApiNameFromUiName( const OUString& rUIName );

/***********************************************************************
*                                                                      *
***********************************************************************/
class SD_DLLPUBLIC SdXImpressDocument : public SfxBaseModel, // implements SfxListener, OWEAKOBJECT & other
                           public SvxFmMSFactory,
                           public ::com::sun::star::drawing::XDrawPageDuplicator,
                           public ::com::sun::star::drawing::XLayerSupplier,
                           public ::com::sun::star::drawing::XMasterPagesSupplier,
                           public ::com::sun::star::drawing::XDrawPagesSupplier,
                           public ::com::sun::star::presentation::XPresentationSupplier,
                           public ::com::sun::star::presentation::XCustomPresentationSupplier,
                           public ::com::sun::star::document::XLinkTargetSupplier,
                           public ::com::sun::star::beans::XPropertySet,
                           public ::com::sun::star::style::XStyleFamiliesSupplier,
                           public ::com::sun::star::lang::XServiceInfo,
                           public ::com::sun::star::ucb::XAnyCompareFactory,
                           public ::com::sun::star::presentation::XHandoutMasterSupplier,
                           public ::com::sun::star::view::XRenderable,
                           public vcl::ITiledRenderable
{
    friend class SdDrawPagesAccess;
    friend class SdMasterPagesAccess;
    friend class SdLayerManager;

private:
    ::sd::DrawDocShell* mpDocShell;
    SdDrawDocument* mpDoc;
    bool mbDisposed;

    css::uno::Reference<css::uno::XInterface> create(
        OUString const & aServiceSpecifier, OUString const & referer);

    SdPage* InsertSdPage( sal_uInt16 nPage, bool bDuplicate = false )
        throw (css::uno::RuntimeException, std::exception);

    const bool mbImpressDoc;
    bool mbClipBoard;

    ::com::sun::star::uno::WeakReference< ::com::sun::star::drawing::XDrawPages > mxDrawPagesAccess;
    ::com::sun::star::uno::WeakReference< ::com::sun::star::drawing::XDrawPages > mxMasterPagesAccess;
    ::com::sun::star::uno::WeakReference< ::com::sun::star::container::XNameAccess > mxLayerManager;
    ::com::sun::star::uno::WeakReference< ::com::sun::star::container::XNameContainer > mxCustomPresentationAccess;
    ::com::sun::star::uno::WeakReference< ::com::sun::star::container::XNameAccess > mxStyleFamilies;
    ::com::sun::star::uno::WeakReference< ::com::sun::star::presentation::XPresentation > mxPresentation;
    ::com::sun::star::uno::WeakReference< ::com::sun::star::i18n::XForbiddenCharacters > mxForbidenCharacters;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > mxLinks;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > mxDashTable;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > mxGradientTable;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > mxHatchTable;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > mxBitmapTable;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > mxTransGradientTable;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > mxMarkerTable;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > mxDrawingPool;

    const SvxItemPropertySet*   mpPropSet;

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > maTypeSequence;

    OUString   maBuildId;

    void initializeDocument();

    sd::DrawViewShell* GetViewShell();

public:
    SdXImpressDocument( ::sd::DrawDocShell* pShell, bool bClipBoard = false ) throw();
    SdXImpressDocument( SdDrawDocument* pDoc, bool bClipBoard = false ) throw();
    virtual ~SdXImpressDocument() throw();

    static rtl::Reference< SdXImpressDocument > GetModel( SdDrawDocument* pDoc );

    // intern
    bool operator==( const SdXImpressDocument& rModel ) const { return mpDoc == rModel.mpDoc; }
    bool operator!=( const SdXImpressDocument& rModel ) const { return mpDoc != rModel.mpDoc; }

    ::sd::DrawDocShell* GetDocShell() const { return mpDocShell; }
    SdDrawDocument* GetDoc() const { return mpDoc; }
    bool IsImpressDocument() const { return mbImpressDoc; }

    void SetModified( bool bModified = true ) throw();

    ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XForbiddenCharacters > getForbiddenCharsTable();

    // SfxListener
    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) SAL_OVERRIDE;

    UNO3_GETIMPLEMENTATION_DECL(SdXImpressDocument)

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL acquire() throw() SAL_OVERRIDE;
    virtual void SAL_CALL release() throw() SAL_OVERRIDE;

    // XModel
    virtual void SAL_CALL lockControllers(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL unlockControllers(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasControllersLocked(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference < ::com::sun::star::container::XIndexAccess > SAL_CALL getViewData() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setViewData( const ::com::sun::star::uno::Reference < ::com::sun::star::container::XIndexAccess >& aData ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XDrawPageDuplicator
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > SAL_CALL duplicate( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& xPage ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XDrawPagesSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPages > SAL_CALL getDrawPages(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XMasterPagesSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPages > SAL_CALL getMasterPages(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XLayerManagerSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getLayerManager(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XCustomPresentationSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > SAL_CALL getCustomPresentations(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XHandoutMasterSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > SAL_CALL getHandoutMasterPage(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XPresentationSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::presentation::XPresentation > SAL_CALL getPresentation(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XMultiServiceFactory ( SvxFmMSFactory )
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstance( const OUString& aServiceSpecifier ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Reference<css::uno::XInterface> SAL_CALL
    createInstanceWithArguments(
        OUString const & ServiceSpecifier,
        css::uno::Sequence<css::uno::Any> const & Arguments)
        throw (css::uno::Exception, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getAvailableServiceNames(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XLinkTargetSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getLinks(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XStyleFamiliesSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getStyleFamilies(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XAnyCompareFactory
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XAnyCompare > SAL_CALL createAnyCompareByName( const OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XRenderable
    virtual sal_Int32 SAL_CALL getRendererCount( const ::com::sun::star::uno::Any& aSelection, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& xOptions ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getRenderer( sal_Int32 nRenderer, const ::com::sun::star::uno::Any& aSelection, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& xOptions ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL render( sal_Int32 nRenderer, const ::com::sun::star::uno::Any& aSelection, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& xOptions ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ITiledRenderable
    virtual void paintTile( VirtualDevice& rDevice,
                            int nOutputWidth,
                            int nOutputHeight,
                            int nTilePosX,
                            int nTilePosY,
                            long nTileWidth,
                            long nTileHeight ) SAL_OVERRIDE;
    virtual Size getDocumentSize() SAL_OVERRIDE;
    virtual void setPart(   int nPart ) SAL_OVERRIDE;
    virtual int  getPart() SAL_OVERRIDE;
    virtual int  getParts() SAL_OVERRIDE;
    virtual OUString getPartName( int nPart ) SAL_OVERRIDE;
    virtual void setPartMode( int nPartMode ) SAL_OVERRIDE;

    /// @see vcl::ITiledRenderable::initializeForTiledRendering().
    virtual void initializeForTiledRendering() SAL_OVERRIDE;
    /// @see vcl::ITiledRenderable::registerCallback().
    virtual void registerCallback(LibreOfficeKitCallback pCallback, void* pData) SAL_OVERRIDE;
    /// @see vcl::ITiledRenderable::postKeyEvent().
    virtual void postKeyEvent(int nType, int nCharCode, int nKeyCode) SAL_OVERRIDE;
    /// @see vcl::ITiledRenderable::postMouseEvent().
    virtual void postMouseEvent(int nType, int nX, int nY, int nCount) SAL_OVERRIDE;
    /// @see vcl::ITiledRenderable::setTextSelection().
    virtual void setTextSelection(int nType, int nX, int nY) SAL_OVERRIDE;
    /// @see vcl::ITiledRenderable::getTextSelection().
    virtual OString getTextSelection(const char* pMimeType) SAL_OVERRIDE;
    /// @see vcl::ITiledRenderable::setGraphicSelection().
    virtual void setGraphicSelection(int nType, int nX, int nY) SAL_OVERRIDE;
    /// @see lok::Document::resetSelection().
    virtual void resetSelection() SAL_OVERRIDE;

    // XComponent

    /** This dispose implementation releases the resources held by the
        called object and forwards the call to its base class.
        When close() has not yet been called then this is done first.  As a
        consequence the implementation has to cope with being called twice
        and still has to forward the second call to the base class.
        See also comments of issue 27847.
    */
    virtual void SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};

/***********************************************************************
*                                                                      *
***********************************************************************/

#include <cppuhelper/implbase4.hxx>

class SdDrawPagesAccess : public ::cppu::WeakImplHelper4< ::com::sun::star::drawing::XDrawPages, ::com::sun::star::container::XNameAccess, ::com::sun::star::lang::XServiceInfo, ::com::sun::star::lang::XComponent >
{
private:
    SdXImpressDocument* mpModel;

public:
    SdDrawPagesAccess( SdXImpressDocument&  rMyModel ) throw();
    virtual ~SdDrawPagesAccess() throw();

    // XDrawPages
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > SAL_CALL insertNewByIndex( sal_Int32 nIndex ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL remove( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& xPage ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const OUString& aName ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE ;
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XComponent
    virtual void SAL_CALL dispose(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};

/***********************************************************************
*                                                                      *
***********************************************************************/

#include <cppuhelper/implbase3.hxx>

class SdMasterPagesAccess : public ::cppu::WeakImplHelper3< ::com::sun::star::drawing::XDrawPages, ::com::sun::star::lang::XServiceInfo, ::com::sun::star::lang::XComponent >
{
private:
    SdXImpressDocument* mpModel;

public:
    SdMasterPagesAccess( SdXImpressDocument& rMyModel ) throw();
    virtual ~SdMasterPagesAccess() throw();

    // XDrawPages
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > SAL_CALL insertNewByIndex( sal_Int32 nIndex )
        throw (::com::sun::star::uno::RuntimeException,
               std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL remove( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& xPage ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE ;
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XComponent
    virtual void SAL_CALL dispose(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};

/***********************************************************************
*                                                                      *
***********************************************************************/

class SdDocLinkTargets : public ::cppu::WeakImplHelper3< ::com::sun::star::container::XNameAccess,
                                                         ::com::sun::star::lang::XServiceInfo , ::com::sun::star::lang::XComponent >
{
private:
    SdXImpressDocument* mpModel;

public:
    SdDocLinkTargets( SdXImpressDocument&   rMyModel ) throw();
    virtual ~SdDocLinkTargets() throw();

    // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const OUString& aName ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XComponent
    virtual void SAL_CALL dispose(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // intern
    SdPage* FindPage( const OUString& rName ) const throw();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
