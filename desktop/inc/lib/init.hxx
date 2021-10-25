/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
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

#include <boost/property_tree/ptree.hpp>
#include <boost/variant.hpp>
#include <boost/container/flat_map.hpp>

#include <osl/thread.h>
#include <rtl/ref.hxx>
#include <vcl/idle.hxx>
#include <LibreOfficeKit/LibreOfficeKit.h>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <tools/gen.hxx>
#include <sfx2/lokcallback.hxx>
#include <sfx2/lokhelper.hxx>

#include <desktop/dllapi.h>

class LOKInteractionHandler;

namespace desktop {

    /// Represents an invalidated rectangle inside a given document part.
    struct RectangleAndPart
    {
        tools::Rectangle m_aRectangle;
        int m_nPart;

        RectangleAndPart()
            : m_nPart(INT_MIN)  // -1 is reserved to mean "all parts".
        {
        }

        RectangleAndPart(const tools::Rectangle* pRect, int nPart)
            : m_aRectangle( pRect ? *pRect : tools::Rectangle(0, 0, SfxLokHelper::MaxTwips, SfxLokHelper::MaxTwips))
            , m_nPart(nPart)
        {
        }

        OString toString() const
        {
            if (m_nPart >= -1)
                return m_aRectangle.toString() + ", " + OString::number(m_nPart);
            else
                return m_aRectangle.toString();
        }

        /// Infinite Rectangle is both sides are
        /// equal or longer than SfxLokHelper::MaxTwips.
        bool isInfinite() const
        {
            return m_aRectangle.GetWidth() >= SfxLokHelper::MaxTwips &&
                   m_aRectangle.GetHeight() >= SfxLokHelper::MaxTwips;
        }

        /// Empty Rectangle is when it has zero dimensions.
        bool isEmpty() const
        {
            return m_aRectangle.IsEmpty();
        }

        static RectangleAndPart Create(const std::string& rPayload);
    };

    class DESKTOP_DLLPUBLIC CallbackFlushHandler final : public Idle, public SfxLokCallbackInterface
    {
    public:
        explicit CallbackFlushHandler(LibreOfficeKitDocument* pDocument, LibreOfficeKitCallback pCallback, void* pData);
        virtual ~CallbackFlushHandler() override;
        virtual void Invoke() override;
        // TODO This should be dropped and the binary libreOfficeKitViewCallback() variants should be called?
        void queue(const int type, const char* data);

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

        // SfxLockCallbackInterface
        virtual void libreOfficeKitViewCallback(int nType, const char* pPayload) override;
        virtual void libreOfficeKitViewCallbackWithViewId(int nType, const char* pPayload, int nViewId) override;
        virtual void libreOfficeKitViewInvalidateTilesCallback(const tools::Rectangle* pRect, int nPart) override;
        virtual void libreOfficeKitViewUpdatedCallback(int nType) override;
        virtual void libreOfficeKitViewUpdatedCallbackPerViewId(int nType, int nViewId, int nSourceViewId) override;

    private:
        struct CallbackData
        {
            CallbackData(const char* payload)
                : PayloadString(payload ? payload : "(nil)")
            {
            }

            CallbackData(const char* payload, int viewId)
                : PayloadString(payload ? payload : "(nil)")
                , PayloadObject(viewId)
            {
            }

            CallbackData(const tools::Rectangle* pRect, int viewId)
                : PayloadObject(RectangleAndPart(pRect, viewId))
            { // PayloadString will be done on demand
            }

            const std::string& getPayload() const;
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
                return PayloadString.empty() && PayloadObject.which() == 0;
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
            mutable std::string PayloadString;

            /// The parsed payload cache. Update validate() when changing this.
            mutable boost::variant<boost::blank, RectangleAndPart, boost::property_tree::ptree, int> PayloadObject;
        };

        typedef std::vector<int> queue_type1;
        typedef std::vector<CallbackData> queue_type2;

        bool removeAll(int type);
        bool removeAll(int type, const std::function<bool (const CallbackData&)>& rTestFunc);
        bool removeAll(const std::function<bool (int, const CallbackData&)>& rTestFunc);
        bool processInvalidateTilesEvent(int type, CallbackData& aCallbackData);
        bool processWindowEvent(int type, CallbackData& aCallbackData);
        queue_type2::iterator toQueue2(queue_type1::iterator);
        queue_type2::reverse_iterator toQueue2(queue_type1::reverse_iterator);
        void queue(const int type, CallbackData& data);
        void enqueueUpdatedTypes();
        void enqueueUpdatedType( int type, SfxViewShell* sourceViewShell, int viewId );

        /** we frequently want to scan the queue, and mostly when we do so, we only care about the element type
            so we split the queue in 2 to make the scanning cache friendly. */
        queue_type1 m_queue1;
        queue_type2 m_queue2;
        std::map<int, std::string> m_states;
        std::unordered_map<int, std::unordered_map<int, std::string>> m_viewStates;

        // For some types only the last message matters (see isUpdatedType()) or only the last message
        // per each viewId value matters (see isUpdatedTypePerViewId()), so instead of using push model
        // where we'd get flooded by repeated messages (which might be costly to generate and process),
        // the preferred way is that libreOfficeKitViewUpdatedCallback()
        // or libreOfficeKitViewUpdatedCallbackPerViewId() get called to notify about such a message being
        // needed, and we'll set a flag here to fetch the actual message before flushing.
        void setUpdatedType( int nType, bool value );
        void setUpdatedTypePerViewId( int nType, int nViewId, int nSourceViewId, bool value );
        void resetUpdatedType( int nType);
        void resetUpdatedTypePerViewId( int nType, int nViewId );
        std::vector<bool> m_updatedTypes; // index is type, value is if set
        struct PerViewIdData
        {
            bool set = false; // value is if set
            int sourceViewId;
        };
        // Flat_map is used in preference to unordered_map because the map is accessed very often.
        boost::container::flat_map<int, std::vector<PerViewIdData>> m_updatedTypesPerViewId; // key is view, index is type

        LibreOfficeKitDocument* m_pDocument;
        int m_viewId = -1; // view id of the associated SfxViewShell
        LibreOfficeKitCallback m_pCallback;
        void *m_pData;
        int m_nDisableCallbacks;
        std::mutex m_mutex;
        class TimeoutIdle : public Timer
        {
        public:
            TimeoutIdle( CallbackFlushHandler* handler );
            virtual void Invoke() override;
        private:
            CallbackFlushHandler* mHandler;
        };
        TimeoutIdle m_TimeoutIdle;
    };

    struct DESKTOP_DLLPUBLIC LibLODocument_Impl : public _LibreOfficeKitDocument
    {
        css::uno::Reference<css::lang::XComponent> mxComponent;
        std::shared_ptr< LibreOfficeKitDocumentClass > m_pDocumentClass;
        std::map<size_t, std::shared_ptr<CallbackFlushHandler>> mpCallbackFlushHandlers;
        const int mnDocumentId;

        explicit LibLODocument_Impl(const css::uno::Reference<css::lang::XComponent>& xComponent,
                                    int nDocumentId);
        ~LibLODocument_Impl();
    };

    struct DESKTOP_DLLPUBLIC LibLibreOffice_Impl : public _LibreOfficeKit
    {
        OUString maLastExceptionMsg;
        std::shared_ptr< LibreOfficeKitClass > m_pOfficeClass;
        oslThread maThread;
        LibreOfficeKitCallback mpCallback;
        void *mpCallbackData;
        int64_t mOptionalFeatures;
        std::map<OString, rtl::Reference<LOKInteractionHandler>> mInteractionMap;

        LibLibreOffice_Impl();
        ~LibLibreOffice_Impl();

        bool hasOptionalFeature(LibreOfficeKitOptionalFeatures const feature)
        {
            return (mOptionalFeatures & feature) != 0;
        }
    };

    /// Helper function to extract the value from parameters delimited by
    /// comma, like: Name1=Value1,Name2=Value2,Name3=Value3.
    /// @param rOptions When extracted, the Param=Value is removed from it.
    DESKTOP_DLLPUBLIC OUString extractParameter(OUString& aOptions, const OUString& rName);

    /// Helper function to convert JSON to a vector of PropertyValues.
    /// Public to be unit-test-able.
    DESKTOP_DLLPUBLIC std::vector<com::sun::star::beans::PropertyValue> jsonToPropertyValuesVector(const char* pJSON);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
