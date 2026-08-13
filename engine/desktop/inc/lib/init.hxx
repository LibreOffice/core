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
#include <unordered_map>
#include <memory>
#include <mutex>
#include <set>
#include <string_view>

#include <boost/property_tree/ptree.hpp>
#include <boost/variant.hpp>
#include <boost/container/flat_map.hpp>

#include <osl/thread.h>
#include <rtl/ref.hxx>
#include <rtl/strbuf.hxx>
#include <COKit/COKit.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <tools/gen.hxx>
#include <sfx2/kit/callback.hxx>
#include <sfx2/kit/helper.hxx>
#include <vcl/idle.hxx>

#include <desktop/dllapi.h>

class KitInteractionHandler;

namespace desktop {

    /// Represents an invalidated rectangle inside a given document part.
    struct RectangleAndPart
    {
        tools::Rectangle m_aRectangle;
        int m_nPart;
        int m_nMode;

        // This is the "EMPTY" rectangle, which somewhat confusingly actually means
        // to drop all rectangles (see COKitCallbackType::INVALIDATE_TILES documentation),
        // and so it is actually an infinite rectangle and not an empty one.
        constexpr static tools::Rectangle emptyAllRectangle = {0, 0, KitHelper::MaxTwips, KitHelper::MaxTwips};

        RectangleAndPart()
            : m_nPart(INT_MIN)  // -1 is reserved to mean "all parts".
            , m_nMode(0)
        {
        }

        RectangleAndPart(const tools::Rectangle* pRect, int nPart, int nMode)
            : m_aRectangle( pRect ? SanitizedRectangle(*pRect) : emptyAllRectangle)
            , m_nPart(nPart)
            , m_nMode(nMode)
        {
        }

        OString toString() const
        {
            if (m_nPart >= -1)
                return (isInfinite() ? "EMPTY"_ostr : m_aRectangle.toString())
                    + ", " + OString::number(m_nPart) + ", " + OString::number(m_nMode);
            else
                return (isInfinite() ? "EMPTY"_ostr : m_aRectangle.toString());
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

    /// One instance of this per view, handles flushing callbacks
    class SAL_DLLPUBLIC_RTTI CallbackFlushHandler final : public CoKitCallbackInterface
    {
    public:
        DESKTOP_DLLPUBLIC explicit CallbackFlushHandler(COKitDocument* pDocument, COKitCallback pCallback, void* pData);
        DESKTOP_DLLPUBLIC virtual ~CallbackFlushHandler() override;
        // TODO This should be dropped and the binary viewCallback() variants should be called?
        DESKTOP_DLLPUBLIC void queue(const COKitCallbackType type, const OString& data);

        /// Disables callbacks on this handler. Must match with identical count
        /// of enableCallbacks. Used during painting and changing views.
        void disableCallbacks() { ++m_nDisableCallbacks; }
        /// Enables callbacks on this handler. Must match with identical count
        /// of disableCallbacks. Used during painting and changing views.
        void enableCallbacks() { --m_nDisableCallbacks; }
        /// Returns true iff callbacks are disabled.
        bool callbacksDisabled() const { return m_nDisableCallbacks != 0; }

        void addViewStates(int viewId);
        void removeViewStates(int viewId);

        void setViewId( int viewId ) { m_viewId = viewId; }

        // True for a view that renders from vector primitives rather than
        // painting bitmap tiles.
        void setVectorRendering() { m_bVectorRendering = true; }
        bool isVectorRendering() const { return m_bVectorRendering; }

        DESKTOP_DLLPUBLIC void tilePainted(int nPart, int nMode, const tools::Rectangle& rRectangle);
        const OString& getViewRenderState() const { return m_aViewRenderState; }
        const std::map<int, std::map<int, tools::Rectangle>>& getPaintedTiles() const
        {
            return m_aPaintedTiles;
        }
        void setPaintedTiles(const std::map<int, std::map<int, tools::Rectangle>>& rPaintedTiles)
        {
            m_aPaintedTiles = rPaintedTiles;
        }

        // SfxLockCallbackInterface
        virtual void viewCallback(COKitCallbackType eType, const OString& pPayload) override;
        virtual void viewCallbackWithViewId(COKitCallbackType eType, const OString& pPayload, int nViewId) override;
        DESKTOP_DLLPUBLIC virtual void viewInvalidateTilesCallback(const tools::Rectangle* pRect, int nPart, int nMode) override;
        virtual void viewUpdatedCallback(COKitCallbackType eType) override;
        virtual void viewUpdatedCallbackPerViewId(COKitCallbackType eType, int nViewId, int nSourceViewId) override;
        /// Records that a slide part changed, so the next flush pushes
        /// that part's vector-primitives delta to the client.
        DESKTOP_DLLPUBLIC virtual void viewVectorPartChanged(int nPart) override;
        virtual void viewAddPendingInvalidateTiles() override;
        virtual void dumpState(rtl::OStringBuffer &rState) override;

    private:
        struct CallbackData
        {
            CallbackData(OString payload)
                : PayloadString(std::move(payload))
            {
            }

            CallbackData(OString payload, int viewId)
                : PayloadString(std::move(payload))
                , PayloadObject(viewId)
            {
            }

            CallbackData(const tools::Rectangle* pRect, int viewId)
                : PayloadObject(RectangleAndPart(pRect, viewId, 0))
            { // PayloadString will be done on demand
            }

            CallbackData(const tools::Rectangle* pRect, int part, int mode)
                : PayloadObject(RectangleAndPart(pRect, part, mode))
            { // PayloadString will be done on demand
            }

            const OString& getPayload() const;
            /// Update a RectangleAndPart object and update PayloadString if necessary.
            void updateRectangleAndPart(const RectangleAndPart& rRectAndPart);
            /// Return the parsed RectangleAndPart instance.
            const RectangleAndPart& getRectangleAndPart() const;
            /// Parse and set the JSON object and return it. Clobbers PayloadString.
            boost::property_tree::ptree& setJson(const std::string& payload);
            /// Set a Json object and update PayloadString.
            void setJson(const boost::property_tree::ptree& rTree);
            /// Return the parsed JSON instance.
            const boost::property_tree::ptree& getJson() const;

            int getViewId() const;

            bool isEmpty() const
            {
                return PayloadString.isEmpty() && PayloadObject.which() == 0;
            }
            void clear()
            {
                PayloadString.clear();
                PayloadObject = boost::blank();
            }

            /// Validate that the payload and parsed object match.
            bool validate() const;

            /// Returns true iff there is cached data.
            bool isCached() const { return PayloadObject.which() != 0; }

        private:
            mutable OString PayloadString;

            /// The parsed payload cache. Update validate() when changing this.
            mutable boost::variant<boost::blank, RectangleAndPart, boost::property_tree::ptree, int> PayloadObject;
        };

        typedef std::vector<COKitCallbackType> queue_type1;
        typedef std::vector<CallbackData> queue_type2;

        void scheduleFlush();
        void invoke();
        bool removeAll(COKitCallbackType type);
        bool removeAll(COKitCallbackType type,
                       const std::function<bool (const CallbackData&)>& rTestFunc);
        bool processInvalidateTilesEvent(COKitCallbackType type, CallbackData& aCallbackData);
        bool processWindowEvent(COKitCallbackType type, CallbackData& aCallbackData);
        queue_type2::iterator toQueue2(queue_type1::iterator);
        queue_type2::reverse_iterator toQueue2(queue_type1::reverse_iterator);
        void queue(const COKitCallbackType type, CallbackData& data);
        void enqueueUpdatedTypes();
        void enqueueUpdatedType( COKitCallbackType type, const SfxViewShell* sourceViewShell, int viewId );
        /// Compute and send the delta of every part recorded since the
        /// last flush.
        void flushVectorPrimitivesDeltas();

        void stop();

        /** we frequently want to scan the queue, and mostly when we do so, we only care about the element type
            so we split the queue in 2 to make the scanning cache friendly. */
        queue_type1 m_queue1;
        queue_type2 m_queue2;
        std::map<COKitCallbackType, OString> m_states;
        std::unordered_map<OString, OString> m_lastStateChange;
        std::unordered_map<int, std::unordered_map<COKitCallbackType, OString>> m_viewStates;

        /// BBox of already painted tiles: part number -> part mode -> rectangle.
        std::map<int, std::map<int, tools::Rectangle>> m_aPaintedTiles;

        // For some types only the last message matters (see isUpdatedType()) or only the last message
        // per each viewId value matters (see isUpdatedTypePerViewId()), so instead of using push model
        // where we'd get flooded by repeated messages (which might be costly to generate and process),
        // the preferred way is that viewUpdatedCallback()
        // or viewUpdatedCallbackPerViewId() get called to notify about such a message being
        // needed, and we'll set a flag here to fetch the actual message before flushing.
        void setUpdatedType( COKitCallbackType eType, bool value );
        void setUpdatedTypePerViewId( COKitCallbackType eType, int nViewId, int nSourceViewId, bool value );
        void resetUpdatedType( COKitCallbackType eType);
        void resetUpdatedTypePerViewId( COKitCallbackType eType, int nViewId );
        std::vector<bool> m_updatedTypes; // index is type, value is if set
        struct PerViewIdData
        {
            bool set = false; // value is if set
            int sourceViewId;
        };
        // Flat_map is used in preference to unordered_map because the map is accessed very often.
        boost::container::flat_map<int, std::vector<PerViewIdData>> m_updatedTypesPerViewId; // key is view, index is type

        COKitDocument* m_pDocument;
        OString m_aViewRenderState;
        int m_viewId = -1; // view id of the associated SfxViewShell
        bool m_bVectorRendering = false;
        /// Slide parts whose vector-primitives delta is still to be
        /// pushed, collected between two flushes.
        std::set<int> m_vectorDeltaParts;
        COKitCallback m_pCallback;
        ImplSVEvent* m_pFlushEvent;
        void *m_pData;
        int m_nDisableCallbacks;
        std::recursive_mutex m_mutex;

        DECL_LINK(FlushQueue, void*, void);
    };

    struct WaitUntilIdle
    {
        WaitUntilIdle();

        Idle maIdle;
        OUString msIdleId;
        std::shared_ptr<CallbackFlushHandler> mpCallbackFlushHandler;
        int mnViewId = -1;

        DECL_LINK(IdleHdl, Timer*, void);
    };

    struct DESKTOP_DLLPUBLIC COKitDocumentImpl : public COKitDocument
    {
        css::uno::Reference<css::lang::XComponent> mxComponent;
        std::map<size_t, std::shared_ptr<CallbackFlushHandler>> mpCallbackFlushHandlers;
        const int mnDocumentId;
        WaitUntilIdle maIdleHelper;
        std::set<OUString> maFontsMissing;
        // Canonical main URL this document's original-document-URL mapping was
        // keyed by at load (empty if none); used to clear it at destroy without
        // touching the possibly-disposed model. See comphelper::COKit.
        OUString maOriginalDocumentUrlKey;

        explicit COKitDocumentImpl(css::uno::Reference<css::lang::XComponent> xComponent,
                                    int nDocumentId);
        ~COKitDocumentImpl();

        void updateViewsForPaintedTile(int nOrigViewId, int nPart, int nMode, const tools::Rectangle& rRectangle);

        bool saveAs(const char* pUrl, const char* pFormat, const char* pFilterOptions) override;
        COKitDocumentType getDocumentType() override;
        int getParts() override;
        char* getPartPageRectangles() override;
        int getPart() override;
        void setPart(int nPart) override;
        char* getPartName(int nPart) override;
        void setPartMode(COKitPartMode eMode) override;
        void paintTile(unsigned char* pBuffer, const int nCanvasWidth, const int nCanvasHeight,
                       const int nTilePosX, const int nTilePosY, const int nTileWidth,
                       const int nTileHeight) override;
        COKitTileMode getTileMode() override;
        void getDocumentSize(long* pWidth, long* pHeight) override;
        void initializeForRendering(const char* pArguments) override;
        void registerCallback(COKitCallback pCallback, void* pData) override;
        void postKeyEvent(COKitKeyEventType eType, int nCharCode, int nKeyCode) override;
        void postMouseEvent(COKitMouseEventType eType, int nX, int nY, int nCount, int nButtons,
                            int nModifier) override;
        void postUnoCommand(const char* pCommand, const char* pArguments,
                            bool bNotifyWhenFinished) override;
        void setTextSelection(COKitSetTextSelectionType eType, int nX, int nY) override;
        char* getTextSelection(const char* pMimeType, char** pUsedMimeType) override;
        bool paste(const char* pMimeType, const char* pData, size_t nSize) override;
        void setGraphicSelection(COKitSetGraphicSelectionType eType, int nX, int nY) override;
        void resetSelection() override;
        char* getCommandValues(const char* pCommand) override;
        void setClientZoom(int nTilePixelWidth, int nTilePixelHeight, int nTileTwipWidth,
                           int nTileTwipHeight) override;
        void setClientVisibleArea(int nX, int nY, int nWidth, int nHeight) override;
        int createView() override;
        void destroyView(int nId) override;
        void setView(int nId) override;
        int getView() override;
        int getViewsCount() override;
        char* getPartHash(int nPart) override;
        void paintPartTile(unsigned char* pBuffer, const int nPart, const int nMode,
                           const int nCanvasWidth, const int nCanvasHeight, const int nTilePosX,
                           const int nTilePosY, const int nTileWidth,
                           const int nTileHeight) override;
        bool getViewIds(int* pArray, size_t nSize) override;
        void setOutlineState(bool bColumn, int nLevel, int nIndex, bool bHidden) override;
        void paintWindow(unsigned nWindowId, unsigned char* pBuffer, const int x, const int y,
                         const int width, const int height) override;
        void postWindow(unsigned nWindowId, COKitWindowAction eAction, const char* pData) override;
        void postWindowKeyEvent(unsigned nWindowId, COKitKeyEventType eType, int nCharCode,
                                int nKeyCode) override;
        void postWindowMouseEvent(unsigned nWindowId, COKitMouseEventType eType, int nX, int nY,
                                  int nCount, int nButtons, int nModifier) override;
        void setViewLanguage(int nId, const char* language) override;
        void postWindowExtTextInputEvent(unsigned nWindowId, COKitExtTextInputType eType,
                                         const char* pText) override;
        char* getPartInfo(int nPart) override;
        void paintWindowDPI(unsigned nWindowId, unsigned char* pBuffer, const int x, const int y,
                            const int width, const int height, const double dpiscale) override;
        bool insertCertificate(const unsigned char* pCertificateBinary,
                               const int nCertificateBinarySize,
                               const unsigned char* pPrivateKeyBinary,
                               const int nPrivateKeyBinarySize) override;
        bool addCertificate(const unsigned char* pCertificateBinary,
                            const int nCertificateBinarySize) override;
        int getSignatureState() override;
        size_t renderShapeSelection(char** pOutput) override;
        void postWindowGestureEvent(unsigned nWindowId, const char* pType, int nX, int nY,
                                    int nOffset) override;
        int createViewWithOptions(const char* pOptions) override;
        void selectPart(int nPart, int nSelect) override;
        void moveSelectedParts(int nPosition, bool bDuplicate, int nIntoSection) override;
        void resizeWindow(unsigned nWindowId, const int width, const int height) override;
        bool getClipboard(const char **pMimeTypes, size_t      *pOutCount,
                          char      ***pOutMimeTypes, size_t     **pOutSizes,
                          char      ***pOutStreams) override;
        bool setClipboard(const size_t   nInCount, const char   **pInMimeTypes,
                          const size_t  *pInSizes, const char   **pInStreams) override;
        COKitSelectionType getSelectionType() override;
        void removeTextContext(unsigned nWindowId, int nBefore, int nAfter) override;
        void sendDialogEvent(unsigned long long int nKitWindowId, const char* pArguments) override;
        unsigned char* renderFontOrientation(const char* pFontName, const char* pChar,
                                             int* pFontWidth, int* pFontHeight,
                                             int pOrientation) override;
        void paintWindowForView(unsigned nWindowId, unsigned char* pBuffer, const int x,
                                const int y, const int width, const int height,
                                const double dpiscale, int viewId) override;
        void completeFunction(const char* pFunctionName) override;
        void setWindowTextSelection(unsigned nWindowId, bool bSwap, int nX, int nY) override;
        void sendFormFieldEvent(const char* pArguments) override;
        void setBlockedCommandList(int nViewId, const char* blockedCommandList) override;
        bool renderSearchResult(const char* pSearchResult, unsigned char** pBitmapBuffer,
                                int* pWidth, int* pHeight, size_t* pByteSize) override;
        void sendContentControlEvent(const char* pArguments) override;
        COKitSelectionType getSelectionTypeAndText(const char* pMimeType, char** pText,
                                                   char** pUsedMimeType) override;
        void getDataArea(long nPart, long* pCol, long* pRow) override;
        int getEditMode() override;
        void setViewTimezone(int nId, const char* pTimezone) override;
        void setAccessibilityState(int nId, bool nEnabled) override;
        char* getA11yFocusedParagraph() override;
        int getA11yCaretPosition() override;
        void setViewReadOnly(int nId, const bool readOnly) override;
        void setAllowChangeComments(int nId, const bool allow) override;
        char* getPresentationInfo() override;
        bool createSlideRenderer(const char* pSlideHash, int nSlideNumber, unsigned* nViewWidth,
                                 unsigned* nViewHeight, bool bRenderBackground,
                                 bool bRenderMasterPage) override;
        void postSlideshowCleanup() override;
        bool renderNextSlideLayer(unsigned char* pBuffer, bool* bIsBitmapLayer, double* pScale,
                                  char** pJsonMessage) override;
        void setViewOption(const char* pOption, const char* pValue) override;
        void setColorPreviewState(int nId, bool nEnabled) override;
        void setAllowManageRedlines(int nId, bool allow) override;
        void transferClipboardFromView(int nSourceViewId) override;
        void flushClipboard() override;
        unsigned long long getPartUniqueId(int nPart, int nMode) override;
        int getPartIndex(int nPart, int nMode) override;
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
        char* getFilterTypes() override;
        void setOptionalFeatures(COKitOptionalFeatures features) override;
        void setDocumentPassword(char const* pURL, char const* pPassword) override;
        char* getVersionInfo() override;
        bool runMacro(const char* pURL) override;
        bool signDocument(const char* pUrl, const unsigned char* pCertificateBinary,
                          const int nCertificateBinarySize, const unsigned char* pPrivateKeyBinary,
                          const int nPrivateKeyBinarySize) override;
        void runLoop(COKitPollCallback pPollCallback, COKitWakeCallback pWakeCallback,
                     void* pData) override;
        void sendDialogEvent(unsigned long long int nKitWindowId, const char* pArguments) override;
        void setOption(const char* pOption, const char* pValue) override;
        void dumpState(const char* pOptions, char** pState) override;
        char* extractRequest(const char* pFilePath) override;
        void trimMemory(int nTarget) override;
        void* startURP(
            void* pReceiveURPFromEngineContext, void* pSendURPToEngineContext,
            int (*fnReceiveURPFromEngine)(void* pContext, const signed char* pBuffer, int nLen),
            int (*fnSendURPToEngine)(void* pContext, signed char* pBuffer, int nLen)) override;
        void stopURP(void* pSendURPToEngineContext) override;
        bool joinThreads() override;
        void startThreads() override;
        void setForkedChild(bool bIsChild) override;
        char* extractDocumentStructureRequest(const char* pFilePath, const char* pFilter) override;
        void registerAnyInputCallback(COKitAnyInputCallback pCallback, void* pData) override;
        int getDocsCount() override;
        void registerFileSaveDialogCallback(COKitFileSaveDialogCallback pCallback) override;
        void executeScript(char const * script, char ** result, char ** error,
                           void (*proxyCallback) (void * data, char const * payload),
                           void * proxyCallbackData, bool * usedLegacyUnoApi) override;
        void deliverProxyResult(char const * callId, char const * jsonValue) override;
        void cancelProxyCalls() override;
        bool isExpectedReentry() override;
        bool takeLegacyUnoApiUseFlag() override;
        void
        registerRevealInFileManagerCallback(COKitRevealInFileManagerCallback pCallback) override;
        void installClipboardProvider(const COKitClipboardProvider* pProvider) override;
        bool getGlobalClipboard(const char **pMimeTypes, size_t      *pOutCount,
                                char      ***pOutMimeTypes, size_t     **pOutSizes,
                                char      ***pOutStreams) override;
    };

    /// Helper function to extract the value from parameters delimited by
    /// comma, like: Name1=Value1,Name2=Value2,Name3=Value3.
    /// @param rOptions When extracted, the Param=Value is removed from it.
    DESKTOP_DLLPUBLIC OUString extractParameter(OUString& aOptions, std::u16string_view rName);

    /// Helper function to convert JSON to a vector of PropertyValues.
    /// Public to be unit-test-able.
    DESKTOP_DLLPUBLIC std::vector<css::beans::PropertyValue> jsonToPropertyValuesVector(const char* pJSON);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
