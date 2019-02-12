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
#ifndef INCLUDED_SW_INC_UNOTXDOC_HXX
#define INCLUDED_SW_INC_UNOTXDOC_HXX

#include "swdllapi.h"
#include <sfx2/sfxbasemodel.hxx>

#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/style/XAutoStylesSupplier.hpp>
#include <com/sun/star/document/XLinkTargetSupplier.hpp>
#include <com/sun/star/document/XRedlinesSupplier.hpp>
#include <com/sun/star/text/XNumberingRulesSupplier.hpp>
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#include <com/sun/star/text/XEndnotesSupplier.hpp>
#include <com/sun/star/text/XTextSectionsSupplier.hpp>
#include <com/sun/star/text/XLineNumberingProperties.hpp>
#include <com/sun/star/text/XChapterNumberingSupplier.hpp>
#include <com/sun/star/text/XPagePrintable.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/XTextGraphicObjectsSupplier.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/XDocumentIndexesSupplier.hpp>
#include <com/sun/star/text/XBookmarksSupplier.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextEmbeddedObjectsSupplier.hpp>
#include <com/sun/star/text/XReferenceMarksSupplier.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/util/XReplaceable.hpp>
#include <com/sun/star/util/XRefreshable.hpp>
#include <com/sun/star/util/XLinkUpdate.hpp>
#include <com/sun/star/view/XRenderable.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/xforms/XFormsSupplier.hpp>
#include <com/sun/star/text/XFlatParagraphIteratorProvider.hpp>
#include <com/sun/star/document/XDocumentLanguages.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <o3tl/deleter.hxx>
#include <rtl/ref.hxx>
#include <svx/fmdmod.hxx>
#include <editeng/UnoForbiddenCharsTable.hxx>
#include <cppuhelper/implbase.hxx>
#include <vcl/ITiledRenderable.hxx>
#include <com/sun/star/tiledrendering/XTiledRenderable.hpp>
#include <com/sun/star/text/XPasteBroadcaster.hpp>

#include "unobaseclass.hxx"
#include "viewopt.hxx"

#include <deque>

class SwDoc;
class SwDocShell;
class UnoActionContext;
class SwXBodyText;
class SwXDrawPage;
class SwUnoCursor;
class SwXDocumentPropertyHelper;
class SfxViewFrame;
class SwPrintUIOptions;
class SwPrintData;
class SwRenderData;
class SwViewShell;
class SfxItemPropertySet;
namespace com { namespace sun { namespace star { namespace container { class XNameContainer; } } } }
namespace com { namespace sun { namespace star { namespace frame { class XController; } } } }
namespace com { namespace sun { namespace star { namespace lang { struct Locale; } } } }
namespace com { namespace sun { namespace star { namespace uno { class XAggregation; } } } }

namespace com { namespace sun { namespace star { namespace util { class XReplaceDescriptor; } } } }

typedef cppu::WeakImplHelper
<
    css::text::XTextDocument,
    css::text::XLineNumberingProperties,
    css::text::XChapterNumberingSupplier,
    css::text::XNumberingRulesSupplier,
    css::text::XFootnotesSupplier,
    css::text::XEndnotesSupplier,
    css::util::XReplaceable,
    css::text::XPagePrintable,
    css::text::XReferenceMarksSupplier,
    css::text::XTextTablesSupplier,
    css::text::XTextFramesSupplier,
    css::text::XBookmarksSupplier,
    css::text::XTextSectionsSupplier,
    css::text::XTextGraphicObjectsSupplier,
    css::text::XTextEmbeddedObjectsSupplier,
    css::text::XTextFieldsSupplier,
    css::style::XStyleFamiliesSupplier,
    css::style::XAutoStylesSupplier,
    css::lang::XServiceInfo,
    css::drawing::XDrawPageSupplier,
    css::drawing::XDrawPagesSupplier,
    css::text::XDocumentIndexesSupplier,
    css::beans::XPropertySet,
    css::beans::XPropertyState,
    css::document::XLinkTargetSupplier,
    css::document::XRedlinesSupplier,
    css::util::XRefreshable,
    css::util::XLinkUpdate,
    css::view::XRenderable,
    css::xforms::XFormsSupplier,
    css::text::XFlatParagraphIteratorProvider,
    css::document::XDocumentLanguages,
    css::util::XCloneable,
    css::text::XPasteBroadcaster
>
SwXTextDocumentBaseClass;

class SW_DLLPUBLIC SwXTextDocument : public SwXTextDocumentBaseClass,
    public SvxFmMSFactory,
    public SfxBaseModel,
    public vcl::ITiledRenderable,
    public css::tiledrendering::XTiledRenderable
{
private:
    class Impl;
    ::sw::UnoImplPtr<Impl> m_pImpl;

    std::deque<std::unique_ptr<UnoActionContext, o3tl::default_delete<UnoActionContext>>> maActionArr;

    const SfxItemPropertySet* pPropSet;

    SwDocShell*             pDocShell;
    bool                    bObjectValid;

    SwXDrawPage*            pDrawPage;
    css::uno::Reference< css::drawing::XDrawPage >              mxXDrawPage;

    css::uno::Reference< css::text::XText >                     xBodyText;
    SwXBodyText*                                                pBodyText;
    css::uno::Reference< css::uno::XAggregation >               xNumFormatAgg;

    css::uno::Reference< css::container::XIndexAccess >         mxXNumberingRules;
    css::uno::Reference< css::container::XIndexAccess >         mxXFootnotes;
    css::uno::Reference< css::beans::XPropertySet >             mxXFootnoteSettings;
    css::uno::Reference< css::container::XIndexAccess >         mxXEndnotes;
    css::uno::Reference< css::beans::XPropertySet >             mxXEndnoteSettings;
    css::uno::Reference< css::container::XNameAccess >          mxXReferenceMarks;
    css::uno::Reference< css::container::XEnumerationAccess >   mxXTextFieldTypes;
    css::uno::Reference< css::container::XNameAccess >          mxXTextFieldMasters;
    css::uno::Reference< css::container::XNameAccess >          mxXTextSections;
    css::uno::Reference< css::container::XNameAccess >          mxXBookmarks;
    css::uno::Reference< css::container::XNameAccess >          mxXTextTables;
    css::uno::Reference< css::container::XNameAccess >          mxXTextFrames;
    css::uno::Reference< css::container::XNameAccess >          mxXGraphicObjects;
    css::uno::Reference< css::container::XNameAccess >          mxXEmbeddedObjects;
    css::uno::Reference< css::container::XNameAccess >          mxXStyleFamilies;
    mutable css::uno::Reference< css::style::XAutoStyles >      mxXAutoStyles;
    css::uno::Reference< css::container::XIndexReplace >        mxXChapterNumbering;
    css::uno::Reference< css::container::XIndexAccess >         mxXDocumentIndexes;

    css::uno::Reference< css::beans::XPropertySet >             mxXLineNumberingProperties;
    css::uno::Reference< css::container::XNameAccess >          mxLinkTargetSupplier;
    css::uno::Reference< css::container::XEnumerationAccess >   mxXRedlines;

    //temporary frame to enable PDF export if no valid view is available
    SfxViewFrame*                                   m_pHiddenViewFrame;
    rtl::Reference<SwXDocumentPropertyHelper>       mxPropertyHelper;

    std::unique_ptr<SwPrintUIOptions>               m_pPrintUIOptions;
    std::unique_ptr<SwRenderData>                   m_pRenderData;

    void                    GetNumberFormatter();

    css::uno::Reference<css::uno::XInterface> create(
        OUString const & rServiceName,
        css::uno::Sequence<css::uno::Any> const * arguments);

    // used for XRenderable implementation
    SfxViewShell *  GuessViewShell( /* out */ bool &rbIsSwSrcView, const css::uno::Reference< css::frame::XController >& rController = css::uno::Reference< css::frame::XController >() );
    SwDoc *         GetRenderDoc( SfxViewShell *&rpView, const css::uno::Any& rSelection, bool bIsPDFExport );
    SfxViewShell *  GetRenderView( bool &rbIsSwSrcView, const css::uno::Sequence< css::beans::PropertyValue >& rxOptions, bool bIsPDFExport );

    OUString           maBuildId;

    // boolean for XPagePrintable
    // set in XPagePrintable::printPages(..) to indicate that the PagePrintSettings
    // has to be applied in XRenderable::getRenderer(..) through which the printing
    // is implemented.
    bool bApplyPagePrintSettingsFromXPagePrintable;

    using SfxBaseModel::addEventListener;
    using SfxBaseModel::removeEventListener;

protected:
    /** abstract SdrModel provider */
    virtual SdrModel& getSdrModelFromUnoModel() const override;

    virtual ~SwXTextDocument() override;
public:
    SwXTextDocument(SwDocShell* pShell);

    void NotifyRefreshListeners();
    virtual     css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) override;
    virtual void SAL_CALL acquire(  ) throw() override;
    virtual void SAL_CALL release(  ) throw() override;

    //XWeak
    virtual css::uno::Reference< css::uno::XAdapter > SAL_CALL queryAdapter(  ) override;

    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;

    static const css::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;

    //XTextDocument
    virtual css::uno::Reference< css::text::XText >  SAL_CALL getText() override;
    virtual void SAL_CALL reformat() override;

    //XModel
    virtual sal_Bool SAL_CALL attachResource( const OUString& aURL, const css::uno::Sequence< css::beans::PropertyValue >& aArgs ) override;
    virtual OUString SAL_CALL getURL(  ) override;
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getArgs(  ) override;
    virtual void SAL_CALL connectController( const css::uno::Reference< css::frame::XController >& xController ) override;
    virtual void SAL_CALL disconnectController( const css::uno::Reference< css::frame::XController >& xController ) override;
    virtual void SAL_CALL lockControllers(  ) override;
    virtual void SAL_CALL unlockControllers(  ) override;
    virtual sal_Bool SAL_CALL hasControllersLocked(  ) override;
    virtual css::uno::Reference< css::frame::XController > SAL_CALL getCurrentController(  ) override;
    virtual void SAL_CALL setCurrentController( const css::uno::Reference< css::frame::XController >& xController ) override;
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getCurrentSelection(  ) override;

    //XComponent
    virtual void SAL_CALL dispose() override;
    virtual void SAL_CALL addEventListener(const css::uno::Reference< css::lang::XEventListener > & aListener) override;
    virtual void SAL_CALL removeEventListener(const css::uno::Reference< css::lang::XEventListener > & aListener) override;

    //XCloseable
    virtual void SAL_CALL close( sal_Bool bDeliverOwnership ) override;

    //XLineNumberingProperties
    virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL getLineNumberingProperties() override;

    //XChapterNumberingSupplier
    virtual css::uno::Reference< css::container::XIndexReplace >  SAL_CALL getChapterNumberingRules() override;

    //XNumberingRulesSupplier
    virtual css::uno::Reference< css::container::XIndexAccess > SAL_CALL getNumberingRules() override;

    //XFootnotesSupplier
    virtual css::uno::Reference< css::container::XIndexAccess >  SAL_CALL getFootnotes() override;
    virtual css::uno::Reference< css::beans::XPropertySet >  SAL_CALL getFootnoteSettings() override;

    //XEndnotesSupplier
    virtual css::uno::Reference< css::container::XIndexAccess >  SAL_CALL getEndnotes() override;
    virtual css::uno::Reference< css::beans::XPropertySet >  SAL_CALL getEndnoteSettings() override;

    //XReplaceable
    virtual css::uno::Reference< css::util::XReplaceDescriptor >  SAL_CALL createReplaceDescriptor() override;
    virtual sal_Int32 SAL_CALL replaceAll(const css::uno::Reference< css::util::XSearchDescriptor > & xDesc) override;

    //XSearchable
    virtual css::uno::Reference< css::util::XSearchDescriptor >  SAL_CALL createSearchDescriptor() override;
    virtual css::uno::Reference< css::container::XIndexAccess >  SAL_CALL findAll(const css::uno::Reference< css::util::XSearchDescriptor > & xDesc) override;
    virtual css::uno::Reference< css::uno::XInterface >  SAL_CALL findFirst(const css::uno::Reference< css::util::XSearchDescriptor > & xDesc) override;
    virtual css::uno::Reference< css::uno::XInterface >  SAL_CALL findNext(const css::uno::Reference< css::uno::XInterface > & xStartAt, const css::uno::Reference< css::util::XSearchDescriptor > & xDesc) override;

    //XPagePrintable
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getPagePrintSettings() override;
    virtual void SAL_CALL setPagePrintSettings(const css::uno::Sequence< css::beans::PropertyValue >& aSettings) override;
    virtual void SAL_CALL printPages(const css::uno::Sequence< css::beans::PropertyValue >& xOptions) override;

    //XReferenceMarksSupplier
    virtual css::uno::Reference< css::container::XNameAccess >  SAL_CALL getReferenceMarks() override;

    // css::text::XTextFieldsSupplier
    virtual css::uno::Reference< css::container::XEnumerationAccess >  SAL_CALL getTextFields() override;
    virtual css::uno::Reference< css::container::XNameAccess >  SAL_CALL getTextFieldMasters() override;

    // css::text::XTextEmbeddedObjectsSupplier
    virtual css::uno::Reference< css::container::XNameAccess >  SAL_CALL getEmbeddedObjects() override;

    // css::text::XBookmarksSupplier
    virtual css::uno::Reference< css::container::XNameAccess >  SAL_CALL getBookmarks() override;

    // css::text::XTextSectionsSupplier
    virtual css::uno::Reference< css::container::XNameAccess >  SAL_CALL getTextSections() override;

    // css::text::XTextTablesSupplier
    virtual css::uno::Reference< css::container::XNameAccess >  SAL_CALL getTextTables() override;

    // css::text::XTextGraphicObjectsSupplier
    virtual css::uno::Reference< css::container::XNameAccess >  SAL_CALL getGraphicObjects() override;

    // css::text::XTextFramesSupplier
    virtual css::uno::Reference< css::container::XNameAccess >  SAL_CALL getTextFrames() override;

    //XStyleFamiliesSupplier
    virtual css::uno::Reference< css::container::XNameAccess >  SAL_CALL getStyleFamilies() override;

    //XAutoStylesSupplier
    virtual css::uno::Reference< css::style::XAutoStyles > SAL_CALL getAutoStyles(  ) override;

    //XMultiServiceFactory
    virtual css::uno::Reference< css::uno::XInterface >  SAL_CALL createInstance(const OUString& ServiceSpecifier) override;
    virtual css::uno::Reference< css::uno::XInterface >  SAL_CALL createInstanceWithArguments(const OUString& ServiceSpecifier,
                const css::uno::Sequence< css::uno::Any >& Arguments) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getAvailableServiceNames() override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // css::drawing::XDrawPageSupplier
    virtual css::uno::Reference< css::drawing::XDrawPage >  SAL_CALL getDrawPage() override;

    // css::drawing::XDrawPagesSupplier
    virtual css::uno::Reference< css::drawing::XDrawPages > SAL_CALL getDrawPages() override;

    // css::text::XDocumentIndexesSupplier
    virtual css::uno::Reference< css::container::XIndexAccess >  SAL_CALL getDocumentIndexes() override;

    //XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    //XPropertyState
    virtual css::beans::PropertyState SAL_CALL getPropertyState( const OUString& rPropertyName ) override;
    virtual css::uno::Sequence< css::beans::PropertyState > SAL_CALL getPropertyStates( const css::uno::Sequence< OUString >& rPropertyNames ) override;
    virtual void SAL_CALL setPropertyToDefault( const OUString& rPropertyName ) override;
    virtual css::uno::Any SAL_CALL getPropertyDefault( const OUString& rPropertyName ) override;

    //XLinkTargetSupplier
    virtual css::uno::Reference< css::container::XNameAccess >  SAL_CALL getLinks() override;

    //XRedlinesSupplier
    virtual css::uno::Reference< css::container::XEnumerationAccess > SAL_CALL getRedlines(  ) override;

    // css::util::XRefreshable
    virtual void SAL_CALL refresh() override;
    virtual void SAL_CALL addRefreshListener(const css::uno::Reference< css::util::XRefreshListener > & l) override;
    virtual void SAL_CALL removeRefreshListener(const css::uno::Reference< css::util::XRefreshListener > & l) override;

    // css::util::XLinkUpdate,
    virtual void SAL_CALL updateLinks(  ) override;

    // css::view::XRenderable
    virtual sal_Int32 SAL_CALL getRendererCount( const css::uno::Any& aSelection, const css::uno::Sequence< css::beans::PropertyValue >& xOptions ) override;
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getRenderer( sal_Int32 nRenderer, const css::uno::Any& aSelection, const css::uno::Sequence< css::beans::PropertyValue >& xOptions ) override;
    virtual void SAL_CALL render( sal_Int32 nRenderer, const css::uno::Any& aSelection, const css::uno::Sequence< css::beans::PropertyValue >& xOptions ) override;

    // css::xforms::XFormsSupplier
    virtual css::uno::Reference< css::container::XNameContainer > SAL_CALL getXForms(  ) override;

    // css::document::XDocumentLanguages
    virtual css::uno::Sequence< css::lang::Locale > SAL_CALL getDocumentLanguages( ::sal_Int16 nScriptTypes, ::sal_Int16 nCount ) override;

    // css::text::XFlatParagraphIteratorProvider:
    virtual css::uno::Reference< css::text::XFlatParagraphIterator > SAL_CALL getFlatParagraphIterator(::sal_Int32 nTextMarkupType, sal_Bool bAutomatic ) override;

    // css::util::XCloneable
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone(  ) override;

    // css::text::XPasteBroadcaster
    void SAL_CALL addPasteEventListener(
        const ::css::uno::Reference<::css::text::XPasteListener>& xListener) override;
    void SAL_CALL removePasteEventListener(
        const ::css::uno::Reference<::css::text::XPasteListener>& xListener) override;

    /// @see vcl::ITiledRenderable::paintTile().
    virtual void paintTile( VirtualDevice &rDevice,
                            int nOutputWidth,
                            int nOutputHeight,
                            int nTilePosX,
                            int nTilePosY,
                            long nTileWidth,
                            long nTileHeight ) override;
    /// @see vcl::ITiledRenderable::getDocumentSize().
    virtual Size getDocumentSize() override;
    /// @see vcl::ITiledRenderable::setPart().
    virtual void setPart(int nPart) override;
    /// @see vcl::ITiledRenderable::getParts().
    virtual int getParts() override;
    /// @see vcl::ITiledRenderable::getPart().
    virtual int getPart() override;
    /// @see vcl::ITiledRenderable::getPartName().
    virtual OUString getPartName(int nPart) override;
    /// @see vcl::ITiledRenderable::getPartHash().
    virtual OUString getPartHash(int nPart) override;
    /// @see vcl::ITiledRenderable::getDocWindow().
    virtual VclPtr<vcl::Window> getDocWindow() override;
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
    /// @see vcl::ITiledRenderable::resetSelection().
    virtual void resetSelection() override;
    /// @see vcl::ITiledRenderable::getPartPageRectangles().
    virtual OUString getPartPageRectangles() override;
    /// @see vcl::ITiledRenderable::setClipboard().
    virtual void setClipboard(const css::uno::Reference<css::datatransfer::clipboard::XClipboard>& xClipboard) override;
    /// @see vcl::ITiledRenderable::isMimeTypeSupported().
    virtual bool isMimeTypeSupported() override;
    /// @see vcl::ITiledRenderable::setClientVisibleArea().
    virtual void setClientVisibleArea(const tools::Rectangle& rRectangle) override;
    /// @see vcl::ITiledRenderable::setClientZoom.
    virtual void setClientZoom(int nTilePixelWidth_, int nTilePixelHeight_, int nTileTwipWidth_, int nTileTwipHeight_) override;
    /// @see vcl::ITiledRenderable::getPointer().
    virtual PointerStyle getPointer() override;
    /// @see vcl::ITiledRenderable::getTrackedChanges().
    OUString getTrackedChanges() override;
    /// @see vcl::ITiledRenderable::getTrackedChangeAuthors().
    OUString getTrackedChangeAuthors() override;

    OUString getRulerState() override;
    /// @see vcl::ITiledRenderable::getPostIts().
    OUString getPostIts() override;

    // css::tiledrendering::XTiledRenderable
    virtual void SAL_CALL paintTile( const ::css::uno::Any& Parent, ::sal_Int32 nOutputWidth, ::sal_Int32 nOutputHeight, ::sal_Int32 nTilePosX, ::sal_Int32 nTilePosY, ::sal_Int32 nTileWidth, ::sal_Int32 nTileHeight ) override;

    void                        Invalidate();
    void                        Reactivate(SwDocShell* pNewDocShell);
    SwXDocumentPropertyHelper * GetPropertyHelper ();
    bool                    IsValid() const {return bObjectValid;}

    void                        InitNewDoc();

    SwUnoCursor* CreateCursorForSearch(css::uno::Reference< css::text::XTextCursor > & xCursor);
    SwUnoCursor* FindAny(const css::uno::Reference< css::util::XSearchDescriptor > & xDesc,
                                            css::uno::Reference< css::text::XTextCursor > & xCursor, bool bAll,
                                            sal_Int32& nResult,
                                            css::uno::Reference< css::uno::XInterface > const & xLastResult);

    SwDocShell*                 GetDocShell() {return pDocShell;}
};

class SwXLinkTargetSupplier : public cppu::WeakImplHelper
<
    css::container::XNameAccess,
    css::lang::XServiceInfo
>
{
    SwXTextDocument* pxDoc;
    OUString sTables;
    OUString sFrames;
    OUString sGraphics;
    OUString sOLEs;
    OUString sSections;
    OUString sOutlines;
    OUString sBookmarks;

public:
    SwXLinkTargetSupplier(SwXTextDocument& rxDoc);
    virtual ~SwXLinkTargetSupplier() override;

    //XNameAccess
    virtual css::uno::Any SAL_CALL getByName(const OUString& Name) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() override;
    virtual sal_Bool SAL_CALL hasByName(const OUString& Name) override;

    //XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) override;
    virtual sal_Bool SAL_CALL hasElements(  ) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    void    Invalidate() {pxDoc = nullptr;}
};

class SwXLinkNameAccessWrapper : public cppu::WeakImplHelper
<
    css::beans::XPropertySet,
    css::container::XNameAccess,
    css::lang::XServiceInfo,
    css::document::XLinkTargetSupplier
>
{
    css::uno::Reference< css::container::XNameAccess >    xRealAccess;
    const SfxItemPropertySet*                             pPropSet;
    const OUString                                        sLinkSuffix;
    const OUString                                        sLinkDisplayName;
    SwXTextDocument*                                      pxDoc;

public:
    SwXLinkNameAccessWrapper(css::uno::Reference< css::container::XNameAccess >  const & xAccess,
            const OUString& rLinkDisplayName, const OUString& sSuffix);
    SwXLinkNameAccessWrapper(SwXTextDocument& rxDoc,
            const OUString& rLinkDisplayName, const OUString& sSuffix);
    virtual ~SwXLinkNameAccessWrapper() override;

    //XNameAccess
    virtual css::uno::Any SAL_CALL getByName(const OUString& Name) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() override;
    virtual sal_Bool SAL_CALL hasByName(const OUString& Name) override;

    //XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) override;
    virtual sal_Bool SAL_CALL hasElements(  ) override;

    //XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    //XLinkTargetSupplier
    virtual css::uno::Reference< css::container::XNameAccess >  SAL_CALL getLinks() override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

};

class SwXOutlineTarget : public cppu::WeakImplHelper
<
    css::beans::XPropertySet,
    css::lang::XServiceInfo
>
{
    const SfxItemPropertySet*   pPropSet;
    OUString const              sOutlineText;

public:
    SwXOutlineTarget(const OUString& rOutlineText);
    virtual ~SwXOutlineTarget() override;

    //XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

enum class SwCreateDrawTable {
    Dash = 1, Gradient, Hatch, Bitmap, TransGradient, Marker, Defaults
};

class SwXDocumentPropertyHelper : public SvxUnoForbiddenCharsTable
{
    css::uno::Reference < css::uno::XInterface > xDashTable;
    css::uno::Reference < css::uno::XInterface > xGradientTable;
    css::uno::Reference < css::uno::XInterface > xHatchTable;
    css::uno::Reference < css::uno::XInterface > xBitmapTable;
    css::uno::Reference < css::uno::XInterface > xTransGradientTable;
    css::uno::Reference < css::uno::XInterface > xMarkerTable;
    css::uno::Reference < css::uno::XInterface > xDrawDefaults;

    SwDoc*  m_pDoc;
public:
    SwXDocumentPropertyHelper(SwDoc& rDoc);
    virtual ~SwXDocumentPropertyHelper() override;
    css::uno::Reference<css::uno::XInterface> GetDrawTable(SwCreateDrawTable nWhich);
    void Invalidate();

    virtual void onChange() override;
};

// The class SwViewOptionAdjust_Impl is used to adjust the SwViewOption of
// the current SwViewShell so that fields are not printed as commands and
// hidden characters are always invisible. Hidden text and place holders
// should be printed according to the current print options.
// After printing the view options are restored
class SwViewOptionAdjust_Impl
{
    SwViewShell *      m_pShell;
    SwViewOption const m_aOldViewOptions;
public:
    SwViewOptionAdjust_Impl( SwViewShell& rSh, const SwViewOption &rViewOptions );
    ~SwViewOptionAdjust_Impl();
    void AdjustViewOptions( SwPrintData const* const pPrtOptions, bool setShowPlaceHoldersInPDF );
    bool checkShell( const SwViewShell& rCompare ) const
    { return &rCompare == m_pShell; }
    void DontTouchThatViewShellItSmellsFunny() { m_pShell = nullptr; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
