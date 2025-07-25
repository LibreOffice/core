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

#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/document/XLinkTargetSupplier.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPages2.hpp>
#include <com/sun/star/drawing/XDrawPageDuplicator.hpp>
#include <com/sun/star/drawing/XLayerSupplier.hpp>
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#include <com/sun/star/presentation/XPresentationSupplier.hpp>
#include <com/sun/star/presentation/XCustomPresentationSupplier.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/ucb/XAnyCompareFactory.hpp>
#include <com/sun/star/presentation/XHandoutMasterSupplier.hpp>
#include <com/sun/star/view/XRenderable.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <rtl/ref.hxx>
#include <unotools/weakref.hxx>

#include <sfx2/sfxbasemodel.hxx>
#include <svx/fmdmod.hxx>

#include <vcl/ITiledRenderable.hxx>

#include <comphelper/servicehelper.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/weakref.hxx>
#include "sddllapi.h"

namespace com::sun::star::i18n { class XForbiddenCharacters; }

class SdDrawDocument;
class SdPage;
class SvxItemPropertySet;
class SdUnoForbiddenCharsTable;
class SdDrawPagesAccess;
class SdMasterPagesAccess;
class SdLayerManager;
class SdXCustomPresentationAccess;
class SdDocLinkTargets;
class SdGenericDrawPage;

namespace sd {
class DrawDocShell;
class DrawViewShell;
class SlideshowLayerRenderer;
}

extern OUString getPageApiName( SdPage const * pPage );
extern OUString getPageApiNameFromUiName( const OUString& rUIName );

class SAL_DLLPUBLIC_RTTI SdXImpressDocument final : public SfxBaseModel, // implements SfxListener, OWEAKOBJECT & other
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

    std::unique_ptr<sd::SlideshowLayerRenderer> mpSlideshowLayerRenderer;

    css::uno::Reference<css::uno::XInterface> create(
        OUString const & aServiceSpecifier, OUString const & referer);

    /// @throws css::uno::RuntimeException
    SdPage* InsertSdPage( sal_uInt16 nPage, bool bDuplicate );

    const bool mbImpressDoc;
    bool mbClipBoard;

    unotools::WeakReference< SdDrawPagesAccess > mxDrawPagesAccess;
    unotools::WeakReference< SdMasterPagesAccess > mxMasterPagesAccess;
    unotools::WeakReference< SdLayerManager > mxLayerManager;
    unotools::WeakReference< SdXCustomPresentationAccess > mxCustomPresentationAccess;
    unotools::WeakReference< SdUnoForbiddenCharsTable > mxForbiddenCharacters;
    unotools::WeakReference< SdDocLinkTargets > mxLinks;

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

    bool mbPaintTextEdit;

    void initializeDocument();

    SAL_RET_MAYBENULL sd::DrawViewShell* GetViewShell();

    /** abstract SdrModel provider */
    virtual SdrModel& getSdrModelFromUnoModel() const override;

public:
    SdXImpressDocument(::sd::DrawDocShell* pShell, bool bClipBoard);
    SdXImpressDocument(SdDrawDocument* pDoc, bool bClipBoard);
    virtual ~SdXImpressDocument() noexcept override;

    static rtl::Reference< SdXImpressDocument > GetModel( SdDrawDocument const & rDoc );

    // internal
    bool operator==( const SdXImpressDocument& rModel ) const { return mpDoc == rModel.mpDoc; }
    bool operator!=( const SdXImpressDocument& rModel ) const { return mpDoc != rModel.mpDoc; }

    ::sd::DrawDocShell* GetDocShell() const { return mpDocShell; }
    SdDrawDocument* GetDoc() const { return mpDoc; }
    bool IsImpressDocument() const { return mbImpressDoc; }

    void SetModified() noexcept;

    css::uno::Reference< css::i18n::XForbiddenCharacters > getForbiddenCharsTable();

    // SfxListener
    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    UNO3_GETIMPLEMENTATION_DECL(SdXImpressDocument)

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    SD_DLLPUBLIC virtual void SAL_CALL acquire() noexcept override;
    SD_DLLPUBLIC virtual void SAL_CALL release() noexcept override;

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
    SD_DLLPUBLIC virtual css::uno::Reference< css::drawing::XDrawPages > SAL_CALL getDrawPages(  ) override;

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
    SD_DLLPUBLIC virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstance( const OUString& aServiceSpecifier ) override;
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
    SD_DLLPUBLIC virtual void paintTile( VirtualDevice& rDevice,
                            int nOutputWidth,
                            int nOutputHeight,
                            int nTilePosX,
                            int nTilePosY,
                            tools::Long nTileWidth,
                            tools::Long nTileHeight ) override;
    virtual Size getDocumentSize() override;
    SD_DLLPUBLIC virtual void setPart(   int nPart, bool bAllowChangeFocus = true ) override;
    SD_DLLPUBLIC virtual int  getPart() override;
    SD_DLLPUBLIC virtual int  getParts() override;
    SD_DLLPUBLIC virtual OUString getPartName( int nPart ) override;
    SD_DLLPUBLIC virtual OUString getPartHash( int nPart ) override;
    SD_DLLPUBLIC virtual VclPtr<vcl::Window> getDocWindow() override;
    bool isMasterViewMode();

    /// @see vcl::ITiledRenderable::setPartMode().
    virtual void setPartMode( int nPartMode ) override;
    /// @see vcl::ITiledRenderable::getEditMode().
    virtual int getEditMode() override;
    /// @see vcl::ITiledRenderable::setEditMode().
    virtual void setEditMode(int) override;
    /// @see vcl::ITiledRenderable::initializeForTiledRendering().
    SD_DLLPUBLIC virtual void initializeForTiledRendering(const css::uno::Sequence<css::beans::PropertyValue>& rArguments) override;
    /// @see vcl::ITiledRenderable::postKeyEvent().
    SD_DLLPUBLIC virtual void postKeyEvent(int nType, int nCharCode, int nKeyCode) override;
    /// @see vcl::ITiledRenderable::postMouseEvent().
    SD_DLLPUBLIC virtual void postMouseEvent(int nType, int nX, int nY, int nCount, int nButtons, int nModifier) override;
    /// @see vcl::ITiledRenderable::setTextSelection().
    SD_DLLPUBLIC virtual void setTextSelection(int nType, int nX, int nY) override;
    /// @see vcl::ITiledRenderable::getSelection().
    SD_DLLPUBLIC virtual css::uno::Reference<css::datatransfer::XTransferable> getSelection() override;
    /// @see vcl::ITiledRenderable::setGraphicSelection().
    SD_DLLPUBLIC virtual void setGraphicSelection(int nType, int nX, int nY) override;
    /// @see lok::Document::resetSelection().
    SD_DLLPUBLIC virtual void resetSelection() override;
    /// @see vcl::ITiledRenderable::setClientVisibleArea().
    virtual void setClientVisibleArea(const tools::Rectangle& rRectangle) override;
    /// @see vcl::ITiledRenderable::setClipboard().
    virtual void setClipboard(const css::uno::Reference<css::datatransfer::clipboard::XClipboard>& xClipboard) override;
    /// @see vcl::ITiledRenderable::isMimeTypeSupported().
    virtual bool isMimeTypeSupported() override;
    /// @see vcl::ITiledRenderable::getPointer().
    virtual PointerStyle getPointer() override;
    /// @see vcl::ITiledRenderable::getPostIts().
    virtual void getPostIts(tools::JsonWriter& /*rJsonWriter*/) override;
    /// @see vcl::ITiledRenderable::selectPart().
    virtual void selectPart(int nPart, int nSelect) override;
    /// @see vcl::ITiledRenderable::moveSelectedParts().
    virtual void moveSelectedParts(int nPosition, bool bDuplicate) override;
    /// @see vcl::ITiledRenderable::getPartInfo().
    virtual OUString getPartInfo(int nPart) override;
    /// @see vcl::ITiledRenderable::isDisposed().
    virtual bool isDisposed() const override
    {
        return mbDisposed;
    }
    /// @see vcl::ITiledRenderable::setPaintTextEdit().
    virtual void setPaintTextEdit(bool bPaint) override { mbPaintTextEdit = bPaint; }
    /// @see vcl::ITiledRenderable::getViewRenderState().
    SD_DLLPUBLIC OString getViewRenderState(SfxViewShell* pViewShell = nullptr) override;

    SD_DLLPUBLIC virtual void getCommandValues(tools::JsonWriter& rJsonWriter, std::string_view rCommand) override;

    /// @see vcl::ITiledRenderable::getPresentationInfo().
    SD_DLLPUBLIC OString getPresentationInfo() const override;
    /// @see vcl::ITiledRenderable::createSlideRenderer().
    SD_DLLPUBLIC bool createSlideRenderer(
        const OString& rSlideHash,
        sal_Int32 nSlideNumber, sal_Int32& nViewWidth, sal_Int32& nViewHeight,
        bool bRenderBackground, bool bRenderMasterPage) override;
    /// @see vcl::ITiledRenderable::postSlideshowCleanup().
    SD_DLLPUBLIC void postSlideshowCleanup() override;
    /// @see vcl::ITiledRenderable::renderNextSlideLayer().
    SD_DLLPUBLIC bool renderNextSlideLayer(unsigned char* pBuffer, bool& bIsBitmapLayer, double& rScale, OUString& rJsonMsg) override;

    rtl::Reference< SdDrawPagesAccess > getSdDrawPages();

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

class SdDrawPagesAccess final : public ::cppu::WeakImplHelper< css::drawing::XDrawPages, css::container::XNameAccess, css::lang::XServiceInfo, css::lang::XComponent >
{
private:
    SdXImpressDocument* mpModel;

public:
    SdDrawPagesAccess( SdXImpressDocument&  rMyModel ) noexcept;
    virtual ~SdDrawPagesAccess() noexcept override;

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

    SdGenericDrawPage* getDrawPageByIndex( sal_Int32 Index );
};

/***********************************************************************
*                                                                      *
***********************************************************************/

class SdMasterPagesAccess final : public ::cppu::WeakImplHelper< css::drawing::XDrawPages2, css::lang::XServiceInfo, css::lang::XComponent >
{
private:
    SdXImpressDocument* mpModel;

public:
    SdMasterPagesAccess( SdXImpressDocument& rMyModel ) noexcept;
    virtual ~SdMasterPagesAccess() noexcept override;

    // XDrawPages
    virtual css::uno::Reference< css::drawing::XDrawPage > SAL_CALL insertNewByIndex( sal_Int32 nIndex ) override;
    virtual void SAL_CALL remove( const css::uno::Reference< css::drawing::XDrawPage >& xPage ) override;

    // XDrawPages2
    virtual css::uno::Reference< ::css::drawing::XDrawPage > SAL_CALL insertNamedNewByIndex( sal_Int32 nIndex, const OUString& sName ) override;

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

private:
    css::uno::Reference< ::css::drawing::XDrawPage > insertNewImpl( sal_Int32 nIndex, std::optional<OUString> oName );
};

/***********************************************************************
*                                                                      *
***********************************************************************/

enum SdLinkTargetType
{
    Page = 0,
    Notes,
    Handout,
    MasterPage,
    Count
};

class SdDocLinkTargets final : public ::cppu::WeakImplHelper< css::container::XNameAccess,
                                                         css::lang::XServiceInfo , css::lang::XComponent >
{
private:
    SdXImpressDocument* mpModel;
    OUString aNames[SdLinkTargetType::Count];

public:
    SdDocLinkTargets(SdXImpressDocument& rMyModel);
    virtual ~SdDocLinkTargets() noexcept override;

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
};

class SdDocLinkTargetType final : public ::cppu::WeakImplHelper< css::document::XLinkTargetSupplier,
                                                             css::beans::XPropertySet,
                                                             css::lang::XServiceInfo >
{
    SdXImpressDocument* mpModel;
    sal_uInt16 mnType;
    OUString maName;

public:
    SdDocLinkTargetType(SdXImpressDocument* pModel, sal_uInt16 nT);

    // css::document::XLinkTargetSupplier
    virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getLinks() override;

    // css::lang::XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString> SAL_CALL getSupportedServiceNames() override;

    // css::beans::XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() override;
    virtual void SAL_CALL setPropertyValue(const OUString& aPropertyName,
                                           const css::uno::Any& aValue) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(const OUString& PropertyName) override;
    virtual void SAL_CALL addPropertyChangeListener(const OUString& aPropertyName,
                          const css::uno::Reference< css::beans::XPropertyChangeListener > & xListener) override;
    virtual void SAL_CALL removePropertyChangeListener(const OUString& aPropertyName,
                          const css::uno::Reference< css::beans::XPropertyChangeListener > & aListener) override;
    virtual void SAL_CALL addVetoableChangeListener(const OUString& PropertyName,
                          const css::uno::Reference< css::beans::XVetoableChangeListener > & aListener) override;
    virtual void SAL_CALL removeVetoableChangeListener(const OUString& PropertyName,
                          const css::uno::Reference< css::beans::XVetoableChangeListener > & aListener) override;
};

class SdDocLinkTarget final : public ::cppu::WeakImplHelper< css::container::XNameAccess,
                                                             css::lang::XServiceInfo >
{
private:
    SdXImpressDocument* mpModel;
    sal_uInt16 mnType;

public:
    SdDocLinkTarget( SdXImpressDocument* pModel, sal_uInt16 nT );

    // css::container::XNameAccess
    virtual css::uno::Any SAL_CALL getByName(const OUString& aName) override;
    virtual css::uno::Sequence< OUString> SAL_CALL getElementNames() override;
    virtual sal_Bool SAL_CALL hasByName(const OUString& aName) override;

    // css::container::XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

    // css::lang::XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString> SAL_CALL getSupportedServiceNames() override;

    // internal
    /// @throws std::exception
    SdPage* FindPage( std::u16string_view rName ) const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
