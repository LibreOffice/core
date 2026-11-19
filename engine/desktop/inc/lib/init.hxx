/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <map>
#include <memory>
#include <set>
#include <string>
#include <string_view>
#include <utility>

#include <osl/thread.h>
#include <rtl/ref.hxx>
#include <rtl/strbuf.hxx>
#include <COKit/COKit.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <tools/gen.hxx>
#include <sfx2/kit/helper.hxx>
#include <vcl/idle.hxx>

#include <desktop/dllapi.h>

class KitClipboard;
class KitInteractionHandler;
namespace vcl { class ITiledRenderable; }

namespace desktop {

    /// Represents an invalidated rectangle inside a given document part.
    struct RectangleAndPart
    {
        tools::Rectangle m_aRectangle;
        /// The part identifier: a page GUID for a presentation or drawing document, a decimal
        /// index for the other document types. "-1" stands for every part, and an empty string
        /// for a part nobody has set yet.
        OString m_aPart;
        int m_nMode;

        // This is the "EMPTY" rectangle, which somewhat confusingly actually means
        // to drop all rectangles (see COKitCallbackType::INVALIDATE_TILES documentation),
        // and so it is actually an infinite rectangle and not an empty one.
        constexpr static tools::Rectangle emptyAllRectangle = {0, 0, KitHelper::MaxTwips, KitHelper::MaxTwips};

        RectangleAndPart()
            : m_nMode(0)
        {
        }

        RectangleAndPart(const tools::Rectangle* pRect, OString aPart, int nMode)
            : m_aRectangle( pRect ? SanitizedRectangle(*pRect) : emptyAllRectangle)
            , m_aPart(std::move(aPart))
            , m_nMode(nMode)
        {
        }

        /// Whether the part stands for every part of the document.
        bool isAllParts() const
        {
            return m_aPart == "-1";
        }

        OString toString() const
        {
            // Every payload carries the part, so the part must have been set by now.
            assert(!m_aPart.isEmpty());
            return (isInfinite() ? "EMPTY"_ostr : m_aRectangle.toString())
                + ", " + m_aPart + ", " + OString::number(m_nMode);
        }

        /// Infinite Rectangle is both sides are
        /// equal or longer than KitHelper::MaxTwips.
        bool isInfinite() const
        {
            return m_aRectangle.GetWidth() >= KitHelper::MaxTwips &&
                   m_aRectangle.GetHeight() >= KitHelper::MaxTwips;
        }

        /// Empty Rectangle is when it has zero dimensions.
        bool isEmpty() const
        {
            return m_aRectangle.IsEmpty();
        }

        static RectangleAndPart Create(const OString& rPayload);
        /// Makes sure a rectangle is valid (apparently some code does not like negative coordinates for example).
        static tools::Rectangle SanitizedRectangle(tools::Long nLeft, tools::Long nTop, tools::Long nWidth, tools::Long nHeight);
        static tools::Rectangle SanitizedRectangle(const tools::Rectangle& rect);
    };

    class CallbackFlushHandler;

    struct WaitUntilIdle
    {
        WaitUntilIdle();

        Idle maIdle;
        OUString msIdleId;
        std::shared_ptr<CallbackFlushHandler> mpCallbackFlushHandler;
        int mnViewId = -1;
        // True until the idle handler has run once for the current request.
        bool mbFirstPass = true;

        DECL_LINK(IdleHdl, Timer*, void);
    };

    struct DESKTOP_DLLPUBLIC COKitDocumentImpl : public COKitDocument
    {
        cpo::uno::Reference<css::lang::XComponent> mxComponent;
        std::map<size_t, std::shared_ptr<CallbackFlushHandler>> mpCallbackFlushHandlers;
        const int mnDocumentId;
        // Cached document type, to avoid holding the SolarMutex on
        // document type queries.
        COKitDocumentType meDocumentType;
        WaitUntilIdle maIdleHelper;
        /// Family names of the requested fonts that text layout replaced by unrelated fonts.
        std::set<OUString> maFontsMissing;
        // Canonical main URL this document's original-document-URL mapping was
        // keyed by at load (empty if none); used to clear it at destroy without
        // touching the possibly-disposed model. See comphelper::COKit.
        OUString maOriginalDocumentUrlKey;

        // Area that has had tiles painted, by view render state.
        std::map<OString, std::map<OString, std::map<int, tools::Rectangle>>>
            maPaintedAreaByRenderState;

        explicit COKitDocumentImpl(cpo::uno::Reference<css::lang::XComponent> xComponent,
                                    int nDocumentId);
        ~COKitDocumentImpl();

        void updateViewsForPaintedTile(int nOrigViewId, const OString& rPart, int nMode,
                                       const tools::Rectangle& rRectangle);

        bool saveAs(const char* pUrl, const char* pFormat, const char* pFilterOptions) override;
        COKitDocumentType getDocumentType() override;
        int getParts() override;
        std::string getWriterPageRectangles() override;
        std::string getPart() override;
        void setPart(const char* pPart) override;
        std::string getPartName(int nPart) override;
        void setPartMode(COKitPartMode eMode) override;
        void paintTile(std::span<unsigned char> aBuffer, const int nCanvasWidth,
                       const int nCanvasHeight, const int nTilePosX, const int nTilePosY,
                       const int nTileWidth, const int nTileHeight) override;
        COKitTileMode getTileMode() override;
        COKitSize getDocumentSize() override;
        void initializeForRendering(const char* pArguments) override;
        void registerCallback(COKitCallback pCallback, void* pData) override;
        void postKeyEvent(COKitKeyEventType eType, int nCharCode, int nKeyCode) override;
        void postMouseEvent(COKitMouseEventType eType, int nX, int nY, int nCount, int nButtons,
                            int nModifier) override;
        void postUnoCommand(const char* pCommand, const char* pArguments,
                            bool bNotifyWhenFinished) override;
        void setTextSelection(COKitSetTextSelectionType eType, int nX, int nY) override;
        std::string getTextSelection(std::string_view aMimeType) override;
        bool paste(const char* pMimeType, const char* pData, size_t nSize) override;
        void setGraphicSelection(COKitSetGraphicSelectionType eType, int nX, int nY) override;
        void setInPlaceObjectArea(int nX, int nY, int nWidth, int nHeight) override;
        void resetSelection() override;
        std::size_t insertFiles(const std::vector<std::string>& rUrls) override;
        std::string getCommandValues(const char* pCommand) override;
        void setClientZoom(int nTilePixelWidth, int nTilePixelHeight, int nTileTwipWidth,
                           int nTileTwipHeight) override;
        void setClientVisibleArea(int nX, int nY, int nWidth, int nHeight) override;
        int createView() override;
        void destroyView(int nId) override;
        void setView(int nId) override;
        int getView() override;
        int getViewsCount() override;
        void paintPartTile(std::span<unsigned char> aBuffer, const char* pPart, const int nMode,
                           const int nCanvasWidth, const int nCanvasHeight, const int nTilePosX,
                           const int nTilePosY, const int nTileWidth, const int nTileHeight,
                           bool bIsPreview = false) override;
        std::vector<int> getViewIds() override;
        void setOutlineState(bool bColumn, int nLevel, int nIndex, bool bHidden) override;
        void paintWindow(unsigned nWindowId, std::span<unsigned char> aBuffer, const int x,
                         const int y, const int width, const int height) override;
        void postWindow(unsigned nWindowId, COKitWindowAction eAction, const char* pData) override;
        void postWindowKeyEvent(unsigned nWindowId, COKitKeyEventType eType, int nCharCode,
                                int nKeyCode) override;
        void postWindowMouseEvent(unsigned nWindowId, COKitMouseEventType eType, int nX, int nY,
                                  int nCount, int nButtons, int nModifier) override;
        void setViewLanguage(int nId, const char* language) override;
        void postWindowExtTextInputEvent(unsigned nWindowId, COKitExtTextInputType eType,
                                         const char* pText) override;
        std::string getPartInfo(int nPart) override;
        void paintWindowDPI(unsigned nWindowId, std::span<unsigned char> aBuffer, const int x,
                            const int y, const int width, const int height,
                            const double dpiscale) override;
        bool insertCertificate(std::span<const unsigned char> aCertificateBinary,
                               std::span<const unsigned char> aPrivateKeyBinary) override;
        bool addCertificate(std::span<const unsigned char> aCertificateBinary) override;
        int getSignatureState() override;
        std::vector<char> renderShapeSelection() override;
        void postWindowGestureEvent(unsigned nWindowId, const char* pType, int nX, int nY,
                                    int nOffset) override;
        int createViewWithOptions(const char* pOptions) override;
        void selectPart(const char* pPart, int nSelect) override;
        void moveSelectedParts(int nPosition, bool bDuplicate, int nIntoSection) override;
        void resizeWindow(unsigned nWindowId, const int width, const int height) override;
        std::vector<COKitClipboardItem> getClipboard(const char **pMimeTypes,
                                                     bool bSkipDuplicateRenderings = false) override;
        bool setClipboard(const size_t   nInCount, const char   **pInMimeTypes,
                          const size_t  *pInSizes, const char   **pInStreams) override;
        COKitSelectionType getSelectionType() override;
        void removeTextContext(unsigned nWindowId, int nBefore, int nAfter) override;
        void sendDialogEvent(unsigned long long int nKitWindowId, const char* pArguments) override;
        COKitBitmap renderFontOrientation(const char* pFontName, const char* pChar,
                                          int nRequestedWidth, int nRequestedHeight,
                                          int nOrientation) override;
        void paintWindowForView(unsigned nWindowId, std::span<unsigned char> aBuffer, const int x,
                                const int y, const int width, const int height,
                                const double dpiscale, int viewId) override;
        void completeFunction(const char* pFunctionName) override;
        void setWindowTextSelection(unsigned nWindowId, bool bSwap, int nX, int nY) override;
        void sendFormFieldEvent(const char* pArguments) override;
        void setBlockedCommandList(int nViewId, const char* blockedCommandList) override;
        COKitBitmap renderSearchResult(const char* pSearchResult) override;
        void sendContentControlEvent(const char* pArguments) override;
        COKitSelection getSelectionTypeAndText(const char* pMimeType) override;
        COKitDataArea getDataArea(long nPart) override;
        int getEditMode() override;
        void setViewTimezone(int nId, const char* pTimezone) override;
        void setAccessibilityState(int nId, bool nEnabled) override;
        void viewGotFirstTile(int nId) override;
        std::string getA11yFocusedParagraph() override;
        int getA11yCaretPosition() override;
        bool setA11yCaretInTableCell(int nRow, int nCol, bool bEnd) override;
        void setViewReadOnly(int nId, const bool readOnly) override;
        void setAllowChangeComments(int nId, const bool allow) override;
        std::string getPresentationInfo() override;
        std::optional<COKitPixelSize> createSlideRenderer(const char* pSlideHash, int nSlideNumber,
                                                          COKitPixelSize aMaximumSize,
                                                          bool bRenderBackground,
                                                          bool bRenderMasterPage) override;
        void postSlideshowCleanup() override;
        COKitSlideLayer renderNextSlideLayer(std::span<unsigned char> aBuffer,
                                             double fScale) override;
        void setViewOption(const char* pOption, const char* pValue) override;
        void setColorPreviewState(int nId, bool nEnabled) override;
        void setAllowManageRedlines(int nId, bool allow) override;
        void transferClipboardFromView(int nSourceViewId) override;
        void flushClipboard() override;
        std::string getPartId(int nPart, int nMode) override;
        int getPartIndex(const char* pPart, int nMode) override;
        bool insertPagesFromFile(const char* pUrl, const char* pJsonOptions) override;
        std::string getSlideLinks() override;
        int refreshSlideLink(const char* pUnit, const char* pId, const char* pSourceName,
                             const char* pUrl, const char* pLastModifiedTime, const char* pName,
                             std::string* pNotUpdated) override;
        int refreshSlideLinks(const char* pSourceName, const char* pUrl,
                              const char* pLastModifiedTime, std::string* pNotUpdated) override;
        bool breakSlideLink(const char* pUnit, const char* pId, bool bDeep) override;
        std::string getLinkedFiles() override;
        int setLinkedFileSource(const char* pId, const char* pUrl) override;
        bool exportPages(const char* pParts, const char* pUrl) override;
        void selectObjects(const char* pObjectIds) override;
        void convertObjects(const char* pObjectIds, const char* pTo) override;
        bool deleteObjects(const char* pObjectIds) override;
        void setObjectPoints(unsigned long long nObjectId, const char* pChanges,
                             const char* pStep) override;
        void splitObjectPath(unsigned long long nObjectId, int nPolygon, int nPoint,
                             unsigned long long nNewId) override;
        void setObjectPathClosed(unsigned long long nObjectId, bool bClosed, int nAtPolygon,
                                 int nAtPoint) override;
        void setObjectGluePoint(unsigned long long nObjectId, int nGlueId, double fX,
                                double fY) override;
        void insertObjectGluePoint(unsigned long long nObjectId, int nGlueId, double fX,
                                   double fY) override;
        void deleteObjectGluePoint(unsigned long long nObjectId, int nGlueId) override;
        void setObjectGluePointWayOut(unsigned long long nObjectId, int nGlueId,
                                      const char* pWayOut) override;
        void setObjectGluePointAnchor(unsigned long long nObjectId, int nGlueId,
                                      const char* pAnchor) override;
        void setObjectTransform(const char* pMappings, const char* pWhat) override;
        void setObjectControlPoint(unsigned long long nObjectId, int nAt, int nX, int nY,
                                   bool bWithTheShape) override;
        void setObjectHandle(unsigned long long nObjectId, int nKind, int nPolygon, int nPoint,
                             int nAt, int nX, int nY, bool bOrtho) override;
        void cropObject(unsigned long long nObjectId, int nUpperLeftX, int nUpperLeftY,
                        int nLowerRightX, int nLowerRightY) override;
        void formControlAction(unsigned long long nObjectId, const char* pAction,
                               const char* pValue) override;
        void selectTableCells(unsigned long long nObjectId, int nFirstColumn, int nFirstRow,
                              int nLastColumn, int nLastRow) override;
        void setConnectorEnd(unsigned long long nConnectorId, bool bStart, int nX, int nY,
                             unsigned long long nTiedTo, int nGluePoint) override;
    };

    struct DESKTOP_DLLPUBLIC COKitImpl : public COKit
    {
        OUString maLastExceptionMsg;
        oslThread maThread;
        COKitCallback mpCallback;
        void *mpCallbackData;
        COKitOptionalFeatures mOptionalFeatures;
        std::map<OString, rtl::Reference<KitInteractionHandler>> mInteractionMap;

        COKitImpl();
        ~COKitImpl();

        bool hasOptionalFeature(COKitOptionalFeatures const feature)
        {
            return (mOptionalFeatures & feature) != COKitOptionalFeatures::NONE;
        }

        void dumpState(rtl::OStringBuffer &aState);

        COKitDocument* documentLoad(const char* pURL) override;
        std::string getError() override;
        COKitDocument* documentLoadWithOptions(const char* pURL, const char* pOptions) override;
        void registerCallback(COKitCallback pCallback, void* pData) override;
        void setOptionalFeatures(COKitOptionalFeatures features) override;
        void setDocumentPassword(char const* pURL, char const* pPassword) override;
        std::string getVersionInfo() override;
        bool runMacro(const char* pURL) override;
        bool signDocument(const char* pUrl, std::span<const unsigned char> aCertificateBinary,
                          std::span<const unsigned char> aPrivateKeyBinary) override;
        void runLoop(COKitPollCallback pPollCallback, COKitWakeCallback pWakeCallback,
                     void* pData) override;
        void sendDialogEvent(unsigned long long int nKitWindowId, const char* pArguments) override;
        void setOption(const char* pOption, const char* pValue) override;
        std::string dumpState(std::string_view aOptions) override;
        std::string extractRequest(const char* pFilePath) override;
        void trimMemory(int nTarget) override;
        void* startURP(
            void* pReceiveURPFromEngineContext, void* pSendURPToEngineContext,
            int (*fnReceiveURPFromEngine)(void* pContext, const signed char* pBuffer, int nLen),
            int (*fnSendURPToEngine)(void* pContext, signed char* pBuffer, int nLen)) override;
        void stopURP(void* pSendURPToEngineContext) override;
        bool joinThreads() override;
        void startThreads() override;
        void setForkedChild(bool bIsChild) override;
        std::string extractDocumentStructureRequest(const char* pFilePath, const char* pFilter) override;
        void registerAnyInputCallback(COKitAnyInputCallback pCallback, void* pData) override;
        int getDocsCount() override;
        void registerFileSaveDialogCallback(COKitFileSaveDialogCallback pCallback) override;
        COKitScriptResult executeScript(std::string const & script,
                           std::string_view source, int line,
                           std::string const & arguments,
                           std::function<void(
                               void * data, std::string_view level, std::string_view message)>
                               consoleCallback,
                           void * consoleCallbackData,
                           void (*proxyCallback) (void * data, std::string_view payload),
                           void * proxyCallbackData) override;
        void deliverProxyResult(std::string_view callId, std::string_view jsonValue) override;
        void cancelProxyCall(std::string_view callId) override;
        void detachProxyCallback(void * proxyCallbackData) override;
        bool isExpectedReentry() override;
        bool takeLegacyUnoApiUseFlag() override;
        void
        registerRevealInFileManagerCallback(COKitRevealInFileManagerCallback pCallback) override;
        void installClipboardProvider(const COKitClipboardProvider* pProvider) override;
        void installFilePickerProvider(const COKitFilePickerProvider* pProvider) override;
        std::string getInsertableFileWildcards() override;
        bool getGlobalClipboard(const char **pMimeTypes,
                                std::vector<std::string>& rOutMimeTypes,
                                std::vector<std::vector<char>>& rOutStreams) override;
        COKitTranslateResult translateDocument(const char* pInputPath, const char* pOutputPath,
                                               const char* pBCP47Language) override;
    };

    /// The document model of pThis, or nullptr when the model renders no tiles.
    vcl::ITiledRenderable* getTiledRenderable(COKitDocument* pThis);

    /// Sets the message of the last exception of the kit, or clears it for an empty s.
    void SetLastExceptionMsg(const OUString& s = OUString());

    /// Sets the clipboard of the current view on the document model of pThis, and returns
    /// that clipboard.
    rtl::Reference<KitClipboard> forceSetClipboardForCurrentView(COKitDocument* pThis);

    /// Helper function to extract the value from parameters delimited by
    /// comma, like: Name1=Value1,Name2=Value2,Name3=Value3.
    /// @param rOptions When extracted, the Param=Value is removed from it.
    DESKTOP_DLLPUBLIC OUString extractParameter(OUString& aOptions, std::u16string_view rName);

    /// Helper function to convert JSON to a vector of PropertyValues.
    /// Public to be unit-test-able.
    DESKTOP_DLLPUBLIC std::vector<css::beans::PropertyValue> jsonToPropertyValuesVector(const char* pJSON);

    /// Returns true when rUsedFonts, the fonts that text layout used for the requested font
    /// rOriginalFont, include that font or a known stand-in for it. Each name has the form
    /// "Family" or "Family/Style". Only the families are compared, ignoring ASCII case, so a
    /// face of the same family with another style counts as the requested font. rOriginalFont can
    /// also be a list of families separated by ";" or ",", which counts as present when any of
    /// its families was used or when any of them is a generic CSS family such as sans-serif.
    /// Public to be unit-test-able.
    DESKTOP_DLLPUBLIC bool isFontPresent(std::u16string_view rOriginalFont,
                                         const std::vector<OUString>& rUsedFonts);

    /// The name of a style in the style list: its DisplayName, or rName when the style has no
    /// DisplayName property or an empty one.
    /// Public to be unit-test-able.
    DESKTOP_DLLPUBLIC OUString getStyleDisplayName(
        const cpo::uno::Reference<css::beans::XPropertySet>& xStyle, const OUString& rName);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
