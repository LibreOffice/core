/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <functional>
#include <map>
#include <mutex>
#include <set>
#include <unordered_map>
#include <utility>
#include <vector>

#include <boost/container/flat_map.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/variant.hpp>

#include <COKit/COKit.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/string.hxx>
#include <sfx2/kit/callback.hxx>
#include <tools/gen.hxx>
#include <tools/link.hxx>

#include <desktop/dllapi.h>
#include <lib/init.hxx>

class SfxViewShell;
struct ImplSVEvent;

namespace desktop {

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
        // painting bitmap tiles. A view asks in each mode it renders.
        void setVectorRendering(int nMode)
        {
            m_bVectorRendering = true;
            m_aVectorRenderingModes.insert(nMode);
        }
        bool isVectorRendering() const { return m_bVectorRendering; }
        // The view paints bitmap tiles again, in no mode from vector primitives.
        void stopVectorRendering()
        {
            m_bVectorRendering = false;
            m_aVectorRenderingModes.clear();
        }

        /// The next flush sends the caret of the view, also when it is the caret sent last.
        void resendVisibleCursor();

        DESKTOP_DLLPUBLIC void tilePainted(const OString& rPart, int nMode,
                                           const tools::Rectangle& rRectangle);
        const OString& getViewRenderState() const { return m_aViewRenderState; }
        const std::map<OString, std::map<int, tools::Rectangle>>& getPaintedTiles() const
        {
            return m_aPaintedTiles;
        }
        void setPaintedTiles(const std::map<OString, std::map<int, tools::Rectangle>>& rPaintedTiles)
        {
            m_aPaintedTiles = rPaintedTiles;
        }

        // SfxLockCallbackInterface
        virtual void viewCallback(COKitCallbackType eType, const OString& pPayload) override;
        virtual void viewCallbackWithViewId(COKitCallbackType eType, const OString& pPayload, int nViewId) override;
        DESKTOP_DLLPUBLIC virtual void viewInvalidateTilesCallback(const tools::Rectangle* pRect, int nPart, int nMode) override;
        virtual void viewUpdatedCallback(COKitCallbackType eType) override;
        virtual void viewUpdatedCallbackPerViewId(COKitCallbackType eType, int nViewId, int nSourceViewId) override;
        /// Records that a part changed, so the next flush pushes that part's vector-primitives
        /// delta to the client. The part is named by its id, the page GUID.
        DESKTOP_DLLPUBLIC virtual void viewVectorPartChanged(const OString& rPartId,
                                                             int nMode) override;
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

            CallbackData(const tools::Rectangle* pRect, OString part, int mode)
                : PayloadObject(RectangleAndPart(pRect, std::move(part), mode))
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

        /// BBox of already painted tiles: part identifier -> part mode -> rectangle.
        std::map<OString, std::map<int, tools::Rectangle>> m_aPaintedTiles;

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
        /// Modes the view asked for primitives in. A change in another mode
        /// pushes nothing.
        std::set<int> m_aVectorRenderingModes;
        /// Parts whose vector-primitives delta is still to be pushed, collected between two
        /// flushes, each as its part id and the mode it is served in. The set is read and changed
        /// only on the main thread under the SolarMutex, and m_mutex does not guard it.
        std::set<std::pair<OString, int>> m_vectorDeltaParts;
        COKitCallback m_pCallback;
        ImplSVEvent* m_pFlushEvent;
        void *m_pData;
        int m_nDisableCallbacks;
        std::recursive_mutex m_mutex;

        DECL_LINK(FlushQueue, void*, void);
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
