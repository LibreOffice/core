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

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <svx/VectorObjectContent.hxx>
#include <svx/sdrhandlerails.hxx>
#include <svx/svdobjkind.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <drawinglayer/primitive2d/Primitive2DContainer.hxx>
#include <drawinglayer/processor2d/Primitive2dJsonProcessor.hxx>
#include <drawinglayer/processor2d/VectorResourceCache.hxx>
#include <tools/gen.hxx>
#include <tools/Guid.hxx>
#include <rtl/ref.hxx>
#include <unotools/weakref.hxx>

#include <memory>
#include <unordered_map>
#include <optional>
#include <unordered_set>
#include <vector>

#include <vcl/BinaryDataContainer.hxx>
#include <vcl/graph.hxx>

#include <sfx2/sfxbasemodel.hxx>
#include <svx/fmdmod.hxx>

#include <vcl/ITiledRenderable.hxx>
#include <vcl/dropcache.hxx>

#include <comphelper/servicehelper.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/weakref.hxx>
#include "sddllapi.h"

namespace com::sun::star::i18n { class XForbiddenCharacters; }
namespace avmedia { struct MediaTempFile; }

class SdrGrafObj;
class SdrObject;
class SdrPage;
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
class SvxDrawPage;

namespace sd {
class DrawDocShell;
class DrawViewShell;
class SlideshowLayerRenderer;
class SlideShow;
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
                           public vcl::ITiledRenderable,
                           public CacheOwner
{
    friend class SdDrawPagesAccess;
    friend class SdMasterPagesAccess;
    friend class SdLayerManager;

public:
    /// Cache of graphics from vector rendering, keyed by checksum.
    std::unordered_map<sal_Int64, Graphic>& getBitmapCache()
    {
        return maVectorResources.getBitmapCache();
    }

    /// Cache of font files used by vector-rendering text, keyed by a
    /// content hash of the font bytes.
    std::unordered_map<sal_uInt64, BinaryDataContainer>& getVectorFontCache()
    {
        return maVectorResources.getFontCache();
    }

    /// What each font key resolved to, once per document.
    std::unordered_map<OUString, drawinglayer::Primitive2dJsonProcessor::ResolvedFace>&
    getVectorFontFaceByKey()
    {
        return maVectorResources.getFaceByKey();
    }

    /// The graphics and fonts the vector primitives of this document name.
    drawinglayer::VectorResourceCache& getVectorResources() { return maVectorResources; }

    /// What was last written for one object.
    using VectorObjectContent = svx::vector::ObjectContent;

    /// Names the version space the part versions count in. Two versions stand for the content
    /// of the same model only when they carry the same epoch.
    SD_DLLPUBLIC sal_Int32 getVectorEpoch() const;

    /// Content version of the part that stands for the page, counted up on each object
    /// change. 0 when nothing changed since the document was opened.
    sal_uInt64 getVectorPartVersion(const SdrPage& rPage) const;

    /// True when the object with the given unique id last changed on the
    /// part at a version later than nSince.
    bool isVectorObjectChangedSince(const SdrPage& rPage, sal_uInt64 nObjectId,
                                    sal_uInt64 nSince) const;

    /// True when the set of objects on the part, or the order they paint in, changed after the
    /// given version.
    bool isVectorOrderChangedSince(const SdrPage& rPage, sal_uInt64 nSince) const;

    /// The objects a change asked for a fresh look at, taken out of the part's state.
    std::unordered_set<sal_uInt64> takeVectorDirtyObjects(const SdrPage& rPage);

    /// Records what is being written for one object. Counts the part's version up and returns
    /// true when the content differs from what was recorded before, false when the object looks
    /// the same and sits where it did.
    bool recordVectorObjectContent(const SdrPage& rPage, sal_uInt64 nObjectId,
                                   const VectorObjectContent& rContent);

    /// What was last written for the object on the part, or nullptr when nothing was.
    const VectorObjectContent* findVectorObjectContent(const SdrPage& rPage,
                                                       sal_uInt64 nObjectId) const;

    /// Records what is being written for one object without touching any version. Writing an
    /// object is what makes it the content the client holds, whether the write was a full
    /// response or a delta.
    void noteVectorObjectWritten(const SdrPage& rPage, sal_uInt64 nObjectId,
                                 const VectorObjectContent& rContent);

    /// Drops what was recorded for an object that is no longer on the part.
    void forgetVectorObject(const SdrPage& rPage, sal_uInt64 nObjectId);

    /// The ids everything the part has recorded is keyed by, in no particular order.
    std::vector<sal_uInt64> getVectorRecordedIds(const SdrPage& rPage) const;

    /// Records the order the objects of the part paint in. When it differs from the order
    /// recorded before, counts the part's version up, remembers that version as the one the
    /// order last moved at, and returns true. The first order recorded moves nothing.
    bool recordVectorPaintOrder(const SdrPage& rPage, const std::vector<sal_uInt64>& rOrder);

    /// The text edit running on rEdited shows something other than it did. Tells the views of
    /// the part it sits on, and the write that follows counts the version up if the text really
    /// moved. Only the entry carrying the edit is affected, so no object is recorded against.
    void notifyTextEditChanged(const SdrObject& rEdited);

    /// True when the part's master page last changed at a version later
    /// than nSince.
    bool isVectorMasterChangedSince(const SdrPage& rPage, sal_uInt64 nSince) const;

    /// True when a change marked the page entry, or nothing was recorded for it yet. Clears
    /// the mark.
    bool takeVectorPageDirty(const SdrPage& rPage);

    /// Records what is written for the page entry: the content behind the objects and the id
    /// of the master part the page names. When that differs from the last record, counts the
    /// part's version up, remembers it as the version the entry last changed at, and returns
    /// true. The first record for a part moves nothing.
    bool recordVectorPageContent(const SdrPage& rPage, const VectorObjectContent& rContent,
                                 const OString& rMasterPartId,
                                 const std::vector<sal_Int32>& rMasterHiddenLayers);

    /// Content state of one vector-rendering part: its current version, the version the last
    /// delta written for it was computed up to, the version at which its master page last
    /// changed, and, per object unique id, the version at which that object last changed.
    ///
    /// maObjectContent holds what was last written per object, and maDirtyObjects the objects a
    /// change asked for a fresh look at. An object stays in maDirtyObjects until a write
    /// compares it against maObjectContent. A memory trim empties maObjectContent and
    /// moPageContent while the versions stay, so until the next write records an object again
    /// it has nothing to be compared against and counts as changed.
    struct VectorPartState
    {
        sal_uInt64 mnVersion = 0;
        /// Where the readers of the part stand: a delta written for the part steps from here
        /// and ends at the version it carries. The first response to serve the part sets it,
        /// since nothing holds the part before that. One delta is written per part rather than
        /// one per reader, so the mark counts for the part.
        sal_uInt64 mnLastSentVersion = 0;
        /// True once a response has served the part, so a reader may hold it. The first full
        /// response leaves the version where it found it, which can be zero, so the version
        /// alone does not say whether anyone holds the part.
        bool mbServed = false;
        /// The version at which the page entry last changed: the page's own properties, its
        /// background, the master it names or the master content it carries inline.
        sal_uInt64 mnMasterChangeVersion = 0;
        /// The version at which the set of objects on the part, or the order they paint in,
        /// last changed. A client whose content is newer than this already holds the order.
        sal_uInt64 mnOrderChangeVersion = 0;
        /// True while a change marked the page entry for comparison.
        bool mbPageDirty = false;
        /// What was last written for the page entry, and the id of the master part it named.
        /// Nothing before the part was first written.
        std::optional<VectorObjectContent> moPageContent;
        OString maPageMasterPartId;
        /// The layers of the master the page leaves out, as last written.
        std::vector<sal_Int32> maPageMasterHiddenLayers;
        std::unordered_map<sal_uInt64, sal_uInt64> maObjectChangeVersions;
        std::unordered_map<sal_uInt64, VectorObjectContent> maObjectContent;
        std::unordered_set<sal_uInt64> maDirtyObjects;
        /// The ids of the painted objects in the order they were last written, without the page
        /// entry. Nothing before the part was first written.
        std::optional<std::vector<sal_uInt64>> maPaintOrder;
    };

private:
    ::sd::DrawDocShell* mpDocShell;
    SdDrawDocument* mpDoc;
    bool mbDisposed;

    std::unique_ptr<sd::SlideshowLayerRenderer> mpSlideshowLayerRenderer;
    /// The graphics and fonts the vector primitives name, and the epoch their versions count in.
    drawinglayer::VectorResourceCache maVectorResources;

    struct AnimatedGifTempFile
    {
        sal_uInt64 mnChecksum = 0;
        OUString maUrl;
        std::shared_ptr<avmedia::MediaTempFile> mpTempFile;
    };

    /// Extracted animated GIF files, keyed by the graphic object's unique id.
    mutable std::unordered_map<sal_uInt64, AnimatedGifTempFile> maAnimatedGifCache;

    struct ClickSoundTempFile
    {
        OUString maPackageUrl;
        OUString maMimeType;
        OUString maUrl;
        std::shared_ptr<avmedia::MediaTempFile> mpTempFile;
    };

    /// Extracted click-action sounds, keyed by the shape's unique id.
    mutable std::unordered_map<sal_uInt64, ClickSoundTempFile> maClickSoundCache;

    /// Vector content state, keyed by the GUID of the page it belongs to, which is also the
    /// page's part id. The state stays with the page when pages are inserted, removed or moved
    /// and its index in the page list changes.
    std::unordered_map<tools::Guid, VectorPartState> maVectorParts;

    /// Writes the primitives of the page rPartId names in the page list nMode. A non-negative
    /// nSinceVersion asks for the delta against that version instead of the whole page. A push
    /// steps from the version the part was last pushed at and moves that mark afterwards. A
    /// pull leaves the mark alone unless nothing has served the part yet.
    void writeVectorPrimitives(tools::JsonWriter& rJsonWriter, const OString& rPartId,
                               sal_Int32 nMode, sal_Int64 nSinceVersion, bool bPush);

    cpo::uno::Reference<cpo::uno::XInterface> create(
        OUString const & aServiceSpecifier, OUString const & referer);

    /// @throws cpo::uno::RuntimeException
    SdPage* InsertSdPage( sal_uInt16 nPage, bool bDuplicate );

    const bool mbImpressDoc;
    bool mbClipBoard;

    unotools::WeakReference< SdDrawPagesAccess > mxDrawPagesAccess;
    unotools::WeakReference< SdMasterPagesAccess > mxMasterPagesAccess;
    unotools::WeakReference< SdLayerManager > mxLayerManager;
    unotools::WeakReference< SdXCustomPresentationAccess > mxCustomPresentationAccess;
    unotools::WeakReference< SdUnoForbiddenCharsTable > mxForbiddenCharacters;
    unotools::WeakReference< SdDocLinkTargets > mxLinks;

    cpo::uno::Reference< cpo::uno::XInterface > mxDashTable;
    cpo::uno::Reference< cpo::uno::XInterface > mxGradientTable;
    cpo::uno::Reference< cpo::uno::XInterface > mxHatchTable;
    cpo::uno::Reference< cpo::uno::XInterface > mxBitmapTable;
    cpo::uno::Reference< cpo::uno::XInterface > mxTransGradientTable;
    cpo::uno::Reference< cpo::uno::XInterface > mxMarkerTable;
    cpo::uno::Reference< cpo::uno::XInterface > mxDrawingPool;

    const SvxItemPropertySet*   mpPropSet;

    cpo::uno::Sequence< cpo::uno::Type > maTypeSequence;

    OUString   maBuildId;

    bool mbPaintTextEdit;

    void initializeDocument();

    /// Creates file URL of the animated GIF extracted from rGraphicObject
    OUString getOrCreateAnimatedGifUrl(const SdrGrafObj& rGraphicObject) const;

    /// Creates file URL of the click action sound copied out of the document package
    OUString getOrCreateClickSoundUrl(const SdrObject& rObject, const OUString& rPackageUrl,
                                      OUString& rMimeType) const;

    void getShapeClickAction(const cpo::uno::Reference<css::drawing::XShape>& xShape,
                             ::tools::JsonWriter& rJsonWriter) const;

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

    cpo::uno::Reference< css::i18n::XForbiddenCharacters > getForbiddenCharsTable();

    // SfxListener
    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    UNO3_GETIMPLEMENTATION_DECL(SdXImpressDocument)

    // XInterface
    virtual cpo::uno::Any queryInterface( const cpo::uno::Type & rType ) override;
    SD_DLLPUBLIC virtual void acquire() noexcept override;
    SD_DLLPUBLIC virtual void release() noexcept override;

    // XModel
    virtual void lockControllers(  ) override;
    virtual void unlockControllers(  ) override;
    virtual bool hasControllersLocked(  ) override;
    virtual cpo::uno::Reference < css::container::XIndexAccess > getViewData() override;
    virtual void setViewData( const cpo::uno::Reference < css::container::XIndexAccess >& aData ) override;

    // XTypeProvider
    virtual cpo::uno::Sequence< cpo::uno::Type > getTypes(  ) override;
    virtual cpo::uno::Sequence< sal_Int8 > getImplementationId(  ) override;

    // XDrawPageDuplicator
    virtual cpo::uno::Reference< css::drawing::XDrawPage > duplicate( const cpo::uno::Reference< css::drawing::XDrawPage >& xPage ) override;

    // XDrawPagesSupplier
    SD_DLLPUBLIC virtual cpo::uno::Reference< css::drawing::XDrawPages > getDrawPages(  ) override;

    // XMasterPagesSupplier
    virtual cpo::uno::Reference< css::drawing::XDrawPages > getMasterPages(  ) override;

    // XLayerManagerSupplier
    virtual cpo::uno::Reference< css::container::XNameAccess > getLayerManager(  ) override;

    // XCustomPresentationSupplier
    virtual cpo::uno::Reference< css::container::XNameContainer > getCustomPresentations(  ) override;

    // XHandoutMasterSupplier
    virtual cpo::uno::Reference< css::drawing::XDrawPage > getHandoutMasterPage(  ) override;

    // XPresentationSupplier
    virtual cpo::uno::Reference< css::presentation::XPresentation > getPresentation(  ) override;

    // XMultiServiceFactory ( SvxFmMSFactory )
    SD_DLLPUBLIC virtual cpo::uno::Reference< cpo::uno::XInterface > createInstance( const OUString& aServiceSpecifier ) override;
    virtual cpo::uno::Reference<cpo::uno::XInterface>
    createInstanceWithArguments(
        OUString const & ServiceSpecifier,
        cpo::uno::Sequence<cpo::uno::Any> const & Arguments) override;
    virtual cpo::uno::Sequence< OUString > getAvailableServiceNames(  ) override;

    // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

    // XPropertySet
    virtual cpo::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo(  ) override;
    virtual void setPropertyValue( const OUString& aPropertyName, const cpo::uno::Any& aValue ) override;
    virtual cpo::uno::Any getPropertyValue( const OUString& PropertyName ) override;
    virtual void addPropertyChangeListener( const OUString& aPropertyName, const cpo::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void removePropertyChangeListener( const OUString& aPropertyName, const cpo::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void addVetoableChangeListener( const OUString& PropertyName, const cpo::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void removeVetoableChangeListener( const OUString& PropertyName, const cpo::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    // XLinkTargetSupplier
    virtual cpo::uno::Reference< css::container::XNameAccess > getLinks(  ) override;

    // XStyleFamiliesSupplier
    virtual cpo::uno::Reference< css::container::XNameAccess > getStyleFamilies(  ) override;

    // XAnyCompareFactory
    virtual cpo::uno::Reference< css::ucb::XAnyCompare > createAnyCompareByName( const OUString& PropertyName ) override;

    // XRenderable
    virtual sal_Int32 getRendererCount( const cpo::uno::Any& aSelection, const cpo::uno::Sequence< css::beans::PropertyValue >& xOptions ) override;
    virtual cpo::uno::Sequence< css::beans::PropertyValue > getRenderer( sal_Int32 nRenderer, const cpo::uno::Any& aSelection, const cpo::uno::Sequence< css::beans::PropertyValue >& xOptions ) override;
    virtual void render( sal_Int32 nRenderer, const cpo::uno::Any& aSelection, const cpo::uno::Sequence< css::beans::PropertyValue >& xOptions ) override;

    rtl::Reference< sd::SlideShow > getSlideShow();

    // ITiledRenderable
    SD_DLLPUBLIC virtual void paintTile( VirtualDevice& rDevice,
                            int nOutputWidth,
                            int nOutputHeight,
                            int nTilePosX,
                            int nTilePosY,
                            tools::Long nTileWidth,
                            tools::Long nTileHeight ) override;
    SD_DLLPUBLIC virtual Size getDocumentSize() override;
    virtual Size getPartSize(int part) override;
    virtual void getAllPartSize(::tools::JsonWriter& rJsonWriter) override;
    SD_DLLPUBLIC virtual void setPart(   int nPart, bool bAllowChangeFocus = true ) override;
    SD_DLLPUBLIC virtual int  getPart() override;
    SD_DLLPUBLIC virtual int  getParts() override;
    SD_DLLPUBLIC virtual OUString getPartName( int nPart ) override;
    SD_DLLPUBLIC virtual OUString getPartHash( int nPart ) override;
    SD_DLLPUBLIC virtual VclPtr<vcl::Window> getDocWindow() override;
    bool isMasterViewMode();

    /// @see vcl::ITiledRenderable::setPartMode().
    virtual void setPartMode( COKitPartMode ePartMode ) override;
    /// @see vcl::ITiledRenderable::getEditMode().
    SD_DLLPUBLIC virtual int getEditMode() override;
    /// @see vcl::ITiledRenderable::setEditMode().
    SD_DLLPUBLIC virtual void setEditMode(int) override;

    /// @see vcl::ITiledRenderable::setDrawnFromModel().
    virtual void setDrawnFromModel(bool bDrawnFromModel) override;

    /// @see CacheOwner::dropCaches(). Drops what the model remembers of the vector content of
    /// every part. The versions stay.
    SD_DLLPUBLIC virtual bool dropCaches() override;
    /// @see CacheOwner::dumpState().
    virtual void dumpState(rtl::OStringBuffer& rState) override;
    /// @see CacheOwner::getCacheName().
    virtual OUString getCacheName() const override;
    /// @see vcl::ITiledRenderable::initializeForTiledRendering().
    SD_DLLPUBLIC virtual void initializeForTiledRendering(const cpo::uno::Sequence<css::beans::PropertyValue>& rArguments) override;
    /// @see vcl::ITiledRenderable::postKeyEvent().
    SD_DLLPUBLIC virtual void postKeyEvent(COKitKeyEventType eType, int nCharCode,
                                           int nKeyCode) override;
    /// @see vcl::ITiledRenderable::postMouseEvent().
    SD_DLLPUBLIC virtual void postMouseEvent(COKitMouseEventType eType, int nX, int nY, int nCount,
                                             int nButtons, int nModifier) override;
    /// @see vcl::ITiledRenderable::setTextSelection().
    SD_DLLPUBLIC virtual void setTextSelection(COKitSetTextSelectionType eType, int nX, int nY) override;
    /// @see vcl::ITiledRenderable::getSelection().
    SD_DLLPUBLIC virtual cpo::uno::Reference<css::datatransfer::XTransferable> getSelection() override;
    /// @see vcl::ITiledRenderable::setGraphicSelection().
    SD_DLLPUBLIC virtual void setGraphicSelection(COKitSetGraphicSelectionType eType, int nX, int nY) override;
    /// @see COKitDocument::resetSelection().
    SD_DLLPUBLIC virtual void resetSelection() override;
    /// @see vcl::ITiledRenderable::setClientVisibleArea().
    SD_DLLPUBLIC virtual void setClientVisibleArea(const tools::Rectangle& rRectangle) override;
    /// @see vcl::ITiledRenderable::setPageZoom().
    virtual void setPageZoom(int nPageZoom) override;
    /// @see vcl::ITiledRenderable::setClipboard().
    virtual void setClipboard(const cpo::uno::Reference<css::datatransfer::clipboard::XClipboard>& xClipboard) override;
    /// @see vcl::ITiledRenderable::isMimeTypeSupported().
    virtual bool isMimeTypeSupported() override;
    /// @see vcl::ITiledRenderable::getPointer().
    virtual PointerStyle getPointer() override;
    /// @see vcl::ITiledRenderable::getPostIts().
    SD_DLLPUBLIC virtual void getPostIts(tools::JsonWriter& /*rJsonWriter*/) override;
    /// @see vcl::ITiledRenderable::selectPart().
    virtual void selectPart(int nPart, int nSelect) override;
    /// @see vcl::ITiledRenderable::moveSelectedParts().
    virtual void moveSelectedParts(int nPosition, bool bDuplicate, int nIntoSection) override;
    /// @see vcl::ITiledRenderable::getPartInfo().
    virtual std::string getPartInfo(int nPart) override;
    /// @see vcl::ITiledRenderable::getPartId().
    virtual OString getPartId(int nPart, int nMode) override;
    /// @see vcl::ITiledRenderable::getPartIndex().
    virtual int getPartIndex(std::string_view rPartId, int nMode) override;
    /// @see vcl::ITiledRenderable::isDisposed().
    virtual bool isDisposed() const override
    {
        return mbDisposed;
    }
    /// @see vcl::ITiledRenderable::setPaintTextEdit().
    virtual void setPaintTextEdit(bool bPaint) override { mbPaintTextEdit = bPaint; }
    /// @see vcl::ITiledRenderable::getViewRenderState().
    SD_DLLPUBLIC OString getViewRenderState(const SfxViewShell* pViewShell = nullptr) override;

    /// @see vcl::ITiledRenderable::supportsCommand().
    SD_DLLPUBLIC virtual bool supportsCommand(std::u16string_view rCommand) override;

    SD_DLLPUBLIC virtual void getCommandValues(tools::JsonWriter& rJsonWriter, std::string_view rCommand) override;

    /// @see vcl::ITiledRenderable::pushVectorPrimitivesDelta().
    SD_DLLPUBLIC void pushVectorPrimitivesDelta(tools::JsonWriter& rJsonWriter,
                                                std::string_view rPartId, int nMode) override;

    /// @see vcl::ITiledRenderable::getPresentationInfo().
    SD_DLLPUBLIC std::string getPresentationInfo(bool bAllyState = false) const override;
    /// @see vcl::ITiledRenderable::createSlideRenderer().
    SD_DLLPUBLIC bool createSlideRenderer(
        const OString& rSlideHash,
        sal_Int32 nSlideNumber, sal_Int32& nViewWidth, sal_Int32& nViewHeight,
        bool bRenderBackground, bool bRenderMasterPage) override;
    /// @see vcl::ITiledRenderable::renderNextSlideLayer().
    SD_DLLPUBLIC bool renderNextSlideLayer(unsigned char* pBuffer, bool& bIsBitmapLayer, double& rScale, std::string& rJsonMsg) override;
    /// @see vcl::ITiledRenderable::insertPagesFromFile().
    SD_DLLPUBLIC bool insertPagesFromFile(const OUString& rFileUrl,
                                          const OString& rJsonOptions) override;
    /// @see vcl::ITiledRenderable::getSlideLinks().
    SD_DLLPUBLIC bool getSlideLinks(tools::JsonWriter& rJsonWriter) override;
    /// @see vcl::ITiledRenderable::refreshSlideLinks().
    SD_DLLPUBLIC sal_Int32 refreshSlideLinks(const OUString& rSourceName,
                                             const OUString& rFileUrl,
                                             const OUString& rLastModifiedTime,
                                             std::vector<OString>* pNotUpdated = nullptr,
                                             sal_Int32 nPageIndex = -1) override;
    /// @see vcl::ITiledRenderable::breakSlideLink().
    SD_DLLPUBLIC bool breakSlideLink(sal_Int32 nIndex) override;

    /// @see vcl::ITiledRenderable::getSlideIndexOfGuid().
    SD_DLLPUBLIC sal_Int32 getSlideIndexOfGuid(const OUString& rGuid) override;
    /// @see vcl::ITiledRenderable::exportPages().
    SD_DLLPUBLIC bool exportPages(const std::vector<sal_Int32>& rPages,
                                  const OUString& rFileUrl) override;

    SD_DLLPUBLIC rtl::Reference< SdDrawPagesAccess > getSdDrawPages();

    // XComponent

    /** This dispose implementation releases the resources held by the
        called object and forwards the call to its base class.
        When close() has not yet been called then this is done first.  As a
        consequence the implementation has to cope with being called twice
        and still has to forward the second call to the base class.
        See also comments of issue 27847.
    */
    virtual void dispose() override;
};

/***********************************************************************
*                                                                      *
***********************************************************************/

class SAL_DLLPUBLIC_RTTI SdDrawPagesAccess final : public ::cppu::WeakImplHelper< css::drawing::XDrawPages, css::container::XNameAccess, css::lang::XServiceInfo, css::lang::XComponent >
{
private:
    SdXImpressDocument* mpModel;

public:
    SdDrawPagesAccess( SdXImpressDocument&  rMyModel ) noexcept;
    virtual ~SdDrawPagesAccess() noexcept override;

    // XDrawPages
    virtual cpo::uno::Reference< css::drawing::XDrawPage > insertNewByIndex( sal_Int32 nIndex ) override;
    virtual void remove( const cpo::uno::Reference< css::drawing::XDrawPage >& xPage ) override;

    // XNameAccess
    virtual cpo::uno::Any getByName( const OUString& aName ) override;
    virtual cpo::uno::Sequence< OUString > getElementNames() override;
    virtual bool hasByName( const OUString& aName ) override;

    // XIndexAccess
    virtual sal_Int32 getCount() override ;
    virtual cpo::uno::Any getByIndex( sal_Int32 Index ) override;

    // XElementAccess
    virtual cpo::uno::Type getElementType() override;
    virtual bool hasElements() override;

    // XServiceInfo
    virtual OUString getImplementationName(  ) override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames(  ) override;

    // XComponent
    virtual void dispose(  ) override;
    virtual void addEventListener( const cpo::uno::Reference< css::lang::XEventListener >& xListener ) override;
    virtual void removeEventListener( const cpo::uno::Reference< css::lang::XEventListener >& aListener ) override;

    SD_DLLPUBLIC SdGenericDrawPage* getDrawPageByIndex( sal_Int32 Index );
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
    virtual cpo::uno::Reference< css::drawing::XDrawPage > insertNewByIndex( sal_Int32 nIndex ) override;
    virtual void remove( const cpo::uno::Reference< css::drawing::XDrawPage >& xPage ) override;

    // XDrawPages2
    virtual cpo::uno::Reference< ::css::drawing::XDrawPage > insertNamedNewByIndex( sal_Int32 nIndex, const OUString& sName ) override;

    // XIndexAccess
    virtual sal_Int32 getCount() override ;
    virtual cpo::uno::Any getByIndex( sal_Int32 Index ) override;

    // XElementAccess
    virtual cpo::uno::Type getElementType() override;
    virtual bool hasElements() override;

    // XServiceInfo
    virtual OUString getImplementationName(  ) override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames(  ) override;

    // XComponent
    virtual void dispose(  ) override;
    virtual void addEventListener( const cpo::uno::Reference< css::lang::XEventListener >& xListener ) override;
    virtual void removeEventListener( const cpo::uno::Reference< css::lang::XEventListener >& aListener ) override;

private:
    rtl::Reference< SvxDrawPage > insertNewImpl( sal_Int32 nIndex, std::optional<OUString> oName );
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
    virtual cpo::uno::Any getByName( const OUString& aName ) override;
    virtual cpo::uno::Sequence< OUString > getElementNames() override;
    virtual bool hasByName( const OUString& aName ) override;

    // XElementAccess
    virtual cpo::uno::Type getElementType() override;
    virtual bool hasElements() override;

    // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

    // XComponent
    virtual void dispose(  ) override;
    virtual void addEventListener( const cpo::uno::Reference< css::lang::XEventListener >& xListener ) override;
    virtual void removeEventListener( const cpo::uno::Reference< css::lang::XEventListener >& aListener ) override;
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
    virtual cpo::uno::Reference< css::container::XNameAccess > getLinks() override;

    // css::lang::XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString> getSupportedServiceNames() override;

    // css::beans::XPropertySet
    virtual cpo::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo() override;
    virtual void setPropertyValue(const OUString& aPropertyName,
                                           const cpo::uno::Any& aValue) override;
    virtual cpo::uno::Any getPropertyValue(const OUString& PropertyName) override;
    virtual void addPropertyChangeListener(const OUString& aPropertyName,
                          const cpo::uno::Reference< css::beans::XPropertyChangeListener > & xListener) override;
    virtual void removePropertyChangeListener(const OUString& aPropertyName,
                          const cpo::uno::Reference< css::beans::XPropertyChangeListener > & aListener) override;
    virtual void addVetoableChangeListener(const OUString& PropertyName,
                          const cpo::uno::Reference< css::beans::XVetoableChangeListener > & aListener) override;
    virtual void removeVetoableChangeListener(const OUString& PropertyName,
                          const cpo::uno::Reference< css::beans::XVetoableChangeListener > & aListener) override;
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
    virtual cpo::uno::Any getByName(const OUString& aName) override;
    virtual cpo::uno::Sequence< OUString> getElementNames() override;
    virtual bool hasByName(const OUString& aName) override;

    // css::container::XElementAccess
    virtual cpo::uno::Type getElementType() override;
    virtual bool hasElements() override;

    // css::lang::XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService(const OUString& ServiceName) override;
    virtual cpo::uno::Sequence< OUString> getSupportedServiceNames() override;

    // internal
    /// @throws std::exception
    SdPage* FindPage( std::u16string_view rName ) const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
