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

#include <vcl/event.hxx>
#include <vcl/ITiledRenderable.hxx>

#include <editeng/unoipset.hxx>

#include <comphelper/servicehelper.hxx>
#include <cppuhelper/implbase.hxx>
#include <sddllapi.h>

class SdDrawDocument;
class SdPage;

namespace sd {
class DrawDocShell;
class DrawViewShell;
}

extern OUString getPageApiName( SdPage const * pPage );
extern OUString getPageApiNameFromUiName( const OUString& rUIName );

/***********************************************************************
*                                                                      *
***********************************************************************/
class SD_DLLPUBLIC SdXImpressDocument : public SfxBaseModel, // implements SfxListener, OWEAKOBJECT & other
                           public SvxFmMSFactory,
                           public css::drawing::XDrawPageDuplicator,
                           public css::drawing::XLayerSupplier,
                           public css::drawing::XMasterPagesSupplier,
                           public css::drawing::XDrawPagesSupplier,
                           public css::presentation::XPresentationSupplier,
                           public css::presentation::XCustomPresentationSupplier,
                           public css::document::XLinkTargetSupplier,
                           public css::beans::XPropertySet,
                           public css::style::XStyleFamiliesSupplier,
                           public css::lang::XServiceInfo,
                           public css::ucb::XAnyCompareFactory,
                           public css::presentation::XHandoutMasterSupplier,
                           public css::view::XRenderable,
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

    /// @throws css::uno::RuntimeException
    SdPage* InsertSdPage( sal_uInt16 nPage, bool bDuplicate );

    const bool mbImpressDoc;
    bool mbClipBoard;

    css::uno::WeakReference< css::drawing::XDrawPages > mxDrawPagesAccess;
    css::uno::WeakReference< css::drawing::XDrawPages > mxMasterPagesAccess;
    css::uno::WeakReference< css::container::XNameAccess > mxLayerManager;
    css::uno::WeakReference< css::container::XNameContainer > mxCustomPresentationAccess;
    css::uno::WeakReference< css::i18n::XForbiddenCharacters > mxForbiddenCharacters;
    css::uno::Reference< css::container::XNameAccess > mxLinks;

    css::uno::Reference< css::uno::XInterface > mxDashTable;
    css::uno::Reference< css::uno::XInterface > mxGradientTable;
    css::uno::Reference< css::uno::XInterface > mxHatchTable;
    css::uno::Reference< css::uno::XInterface > mxBitmapTable;
    css::uno::Reference< css::uno::XInterface > mxTransGradientTable;
    css::uno::Reference< css::uno::XInterface > mxMarkerTable;
    css::uno::Reference< css::uno::XInterface > mxDrawingPool;

    const SvxItemPropertySet*   mpPropSet;

    css::uno::Sequence< css::uno::Type > maTypeSequence;

    OUString   maBuildId;

    void initializeDocument();

    sd::DrawViewShell* GetViewShell();

public:
    SdXImpressDocument( ::sd::DrawDocShell* pShell, bool bClipBoard ) throw();
    SdXImpressDocument( SdDrawDocument* pDoc, bool bClipBoard ) throw();
    virtual ~SdXImpressDocument() throw() override;

    static rtl::Reference< SdXImpressDocument > GetModel( SdDrawDocument const * pDoc );

    // intern
    bool operator==( const SdXImpressDocument& rModel ) const { return mpDoc == rModel.mpDoc; }
    bool operator!=( const SdXImpressDocument& rModel ) const { return mpDoc != rModel.mpDoc; }

    ::sd::DrawDocShell* GetDocShell() const { return mpDocShell; }
    SdDrawDocument* GetDoc() const { return mpDoc; }
    bool IsImpressDocument() const { return mbImpressDoc; }

    void SetModified() throw();

    css::uno::Reference< css::i18n::XForbiddenCharacters > getForbiddenCharsTable();

    // SfxListener
    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    UNO3_GETIMPLEMENTATION_DECL(SdXImpressDocument)

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    virtual void SAL_CALL acquire() throw() override;
    virtual void SAL_CALL release() throw() override;

    // XModel
    virtual void SAL_CALL lockControllers(  ) override;
    virtual void SAL_CALL unlockControllers(  ) override;
    virtual sal_Bool SAL_CALL hasControllersLocked(  ) override;
    virtual css::uno::Reference < css::container::XIndexAccess > SAL_CALL getViewData() override;
    virtual void SAL_CALL setViewData( const css::uno::Reference < css::container::XIndexAccess >& aData ) override;

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) override;

    // XDrawPageDuplicator
    virtual css::uno::Reference< css::drawing::XDrawPage > SAL_CALL duplicate( const css::uno::Reference< css::drawing::XDrawPage >& xPage ) override;

    // XDrawPagesSupplier
    virtual css::uno::Reference< css::drawing::XDrawPages > SAL_CALL getDrawPages(  ) override;

    // XMasterPagesSupplier
    virtual css::uno::Reference< css::drawing::XDrawPages > SAL_CALL getMasterPages(  ) override;

    // XLayerManagerSupplier
    virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getLayerManager(  ) override;

    // XCustomPresentationSupplier
    virtual css::uno::Reference< css::container::XNameContainer > SAL_CALL getCustomPresentations(  ) override;

    // XHandoutMasterSupplier
    virtual css::uno::Reference< css::drawing::XDrawPage > SAL_CALL getHandoutMasterPage(  ) override;

    // XPresentationSupplier
    virtual css::uno::Reference< css::presentation::XPresentation > SAL_CALL getPresentation(  ) override;

    // XMultiServiceFactory ( SvxFmMSFactory )
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstance( const OUString& aServiceSpecifier ) override;
    virtual css::uno::Reference<css::uno::XInterface> SAL_CALL
    createInstanceWithArguments(
        OUString const & ServiceSpecifier,
        css::uno::Sequence<css::uno::Any> const & Arguments) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getAvailableServiceNames(  ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    // XLinkTargetSupplier
    virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getLinks(  ) override;

    // XStyleFamiliesSupplier
    virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getStyleFamilies(  ) override;

    // XAnyCompareFactory
    virtual css::uno::Reference< css::ucb::XAnyCompare > SAL_CALL createAnyCompareByName( const OUString& PropertyName ) override;

    // XRenderable
    virtual sal_Int32 SAL_CALL getRendererCount( const css::uno::Any& aSelection, const css::uno::Sequence< css::beans::PropertyValue >& xOptions ) override;
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getRenderer( sal_Int32 nRenderer, const css::uno::Any& aSelection, const css::uno::Sequence< css::beans::PropertyValue >& xOptions ) override;
    virtual void SAL_CALL render( sal_Int32 nRenderer, const css::uno::Any& aSelection, const css::uno::Sequence< css::beans::PropertyValue >& xOptions ) override;

    // ITiledRenderable
    virtual void paintTile( VirtualDevice& rDevice,
                            int nOutputWidth,
                            int nOutputHeight,
                            int nTilePosX,
                            int nTilePosY,
                            long nTileWidth,
                            long nTileHeight ) override;
    virtual Size getDocumentSize() override;
    virtual void setPart(   int nPart ) override;
    virtual int  getPart() override;
    virtual int  getParts() override;
    virtual OUString getPartName( int nPart ) override;
    virtual OUString getPartHash( int nPart ) override;

    virtual void setPartMode( int nPartMode ) override;

    /// @see vcl::ITiledRenderable::initializeForTiledRendering().
    virtual void initializeForTiledRendering(const css::uno::Sequence<css::beans::PropertyValue>& rArguments) override;
    /// @see vcl::ITiledRenderable::postKeyEvent().
    virtual void postKeyEvent(int nType, int nCharCode, int nKeyCode) override;
    /// @see vcl::ITiledRenderable::postMouseEvent().
    virtual void postMouseEvent(int nType, int nX, int nY, int nCount, int nButtons, int nModifier) override;
    /// @see vcl::ITiledRenderable::setTextSelection().
    virtual void setTextSelection(int nType, int nX, int nY) override;
    /// @see vcl::ITiledRenderable::getTextSelection().
    virtual OString getTextSelection(const char* pMimeType, OString& rUsedMimeType) override;
    /// @see vcl::ITiledRenderable::setGraphicSelection().
    virtual void setGraphicSelection(int nType, int nX, int nY) override;
    /// @see lok::Document::resetSelection().
    virtual void resetSelection() override;
    /// @see vcl::ITiledRenderable::setClipboard().
    virtual void setClipboard(const css::uno::Reference<css::datatransfer::clipboard::XClipboard>& xClipboard) override;
    /// @see vcl::ITiledRenderable::isMimeTypeSupported().
    virtual bool isMimeTypeSupported() override;
    /// @see vcl::ITiledRenderable::getPointer().
    virtual Pointer getPointer() override;
    /// @see vcl::ITiledRenderable::getPostIts().
    virtual OUString getPostIts() override;

    // XComponent

    /** This dispose implementation releases the resources held by the
        called object and forwards the call to its base class.
        When close() has not yet been called then this is done first.  As a
        consequence the implementation has to cope with being called twice
        and still has to forward the second call to the base class.
        See also comments of issue 27847.
    */
    virtual void SAL_CALL dispose() override;
};

/***********************************************************************
*                                                                      *
***********************************************************************/

class SdDrawPagesAccess : public ::cppu::WeakImplHelper< css::drawing::XDrawPages, css::container::XNameAccess, css::lang::XServiceInfo, css::lang::XComponent >
{
private:
    SdXImpressDocument* mpModel;

public:
    SdDrawPagesAccess( SdXImpressDocument&  rMyModel ) throw();
    virtual ~SdDrawPagesAccess() throw() override;

    // XDrawPages
    virtual css::uno::Reference< css::drawing::XDrawPage > SAL_CALL insertNewByIndex( sal_Int32 nIndex ) override;
    virtual void SAL_CALL remove( const css::uno::Reference< css::drawing::XDrawPage >& xPage ) override;

    // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override;

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override ;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override;

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

    // XComponent
    virtual void SAL_CALL dispose(  ) override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;
};

/***********************************************************************
*                                                                      *
***********************************************************************/

class SdMasterPagesAccess : public ::cppu::WeakImplHelper< css::drawing::XDrawPages, css::lang::XServiceInfo, css::lang::XComponent >
{
private:
    SdXImpressDocument* mpModel;

public:
    SdMasterPagesAccess( SdXImpressDocument& rMyModel ) throw();
    virtual ~SdMasterPagesAccess() throw() override;

    // XDrawPages
    virtual css::uno::Reference< css::drawing::XDrawPage > SAL_CALL insertNewByIndex( sal_Int32 nIndex ) override;
    virtual void SAL_CALL remove( const css::uno::Reference< css::drawing::XDrawPage >& xPage ) override;

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override ;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override;

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

    // XComponent
    virtual void SAL_CALL dispose(  ) override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;
};

/***********************************************************************
*                                                                      *
***********************************************************************/

class SdDocLinkTargets : public ::cppu::WeakImplHelper< css::container::XNameAccess,
                                                         css::lang::XServiceInfo , css::lang::XComponent >
{
private:
    SdXImpressDocument* mpModel;

public:
    SdDocLinkTargets( SdXImpressDocument&   rMyModel ) throw();
    virtual ~SdDocLinkTargets() throw() override;

    // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override;

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XComponent
    virtual void SAL_CALL dispose(  ) override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;

    // intern
    /// @throws std::exception
    SdPage* FindPage( const OUString& rName ) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
