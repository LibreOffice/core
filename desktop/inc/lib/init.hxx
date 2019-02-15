/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_DESKTOP_INC_LIB_INIT_HXX
#define INCLUDED_DESKTOP_INC_LIB_INIT_HXX

#include <map>
#include <unordered_map>
#include <memory>
#include <mutex>

#include <boost/variant.hpp>

#include <osl/thread.h>
#include <rtl/ref.hxx>
#include <vcl/idle.hxx>
#include <LibreOfficeKit/LibreOfficeKit.h>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <tools/gen.hxx>
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

        OString toString() const
        {
            std::stringstream ss;
            ss << m_aRectangle.toString();
            if (m_nPart >= -1)
                ss << ", " << m_nPart;
            return ss.str().c_str();
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

    class DESKTOP_DLLPUBLIC CallbackFlushHandler : public Idle
    {
    public:
        explicit CallbackFlushHandler(LibreOfficeKitDocument* pDocument, LibreOfficeKitCallback pCallback, void* pData);
        virtual ~CallbackFlushHandler() override;
        virtual void Invoke() override;
        static void callback(const int type, const char* payload, void* data);
        void queue(const int type, const char* data);

        /// When enabled events are queued but callback not invoked.
        void setEventLatch(const bool bEventLatch)
        {
            m_bEventLatch = bEventLatch;
        }

        bool isEventLatchOn() const { return m_bEventLatch; }
        void setPartTilePainting(const bool bPartPainting) { m_bPartTilePainting = bPartPainting; }
        bool isPartTilePainting() const { return m_bPartTilePainting; }

        void addViewStates(int viewId);
        void removeViewStates(int viewId);

        struct CallbackData
        {
            CallbackData(int type, const std::string& payload)
                : Type(type)
                , PayloadString(payload)
            {
            }

            /// Parse and set the RectangleAndPart object and return it. Clobbers PayloadString.
            RectangleAndPart& setRectangleAndPart(const std::string& payload);
            /// Set a RectangleAndPart object and update PayloadString.
            void setRectangleAndPart(const RectangleAndPart& rRectAndPart);
            /// Return the parsed RectangleAndPart instance.
            const RectangleAndPart& getRectangleAndPart() const;
            /// Parse and set the JSON object and return it. Clobbers PayloadString.
            boost::property_tree::ptree& setJson(const std::string& payload);
            /// Set a Json object and update PayloadString.
            void setJson(const boost::property_tree::ptree& rTree);
            /// Return the parsed JSON instance.
            const boost::property_tree::ptree& getJson() const;

            /// Validate that the payload and parsed object match.
            bool validate() const;

            /// Returns true iff there is cached data.
            bool isCached() const { return PayloadObject.which() != 0; }

            int Type;
            std::string PayloadString;

        private:
            /// The parsed payload cache. Update validate() when changing this.
            boost::variant<boost::blank, RectangleAndPart, boost::property_tree::ptree> PayloadObject;
        };

        typedef std::vector<CallbackData> queue_type;

    private:
        void removeAll(const std::function<bool (const queue_type::value_type&)>& rTestFunc);

        queue_type m_queue;
        std::map<int, std::string> m_states;
        std::unordered_map<int, std::unordered_map<int, std::string>> m_viewStates;
        LibreOfficeKitDocument* m_pDocument;
        LibreOfficeKitCallback m_pCallback;
        void *m_pData;
        bool m_bPartTilePainting;
        bool m_bEventLatch;
        std::mutex m_mutex;
    };

    struct DESKTOP_DLLPUBLIC LibLODocument_Impl : public _LibreOfficeKitDocument
    {
        css::uno::Reference<css::lang::XComponent> mxComponent;
        std::shared_ptr< LibreOfficeKitDocumentClass > m_pDocumentClass;
        std::map<size_t, std::shared_ptr<CallbackFlushHandler>> mpCallbackFlushHandlers;

        explicit LibLODocument_Impl(const css::uno::Reference <css::lang::XComponent> &xComponent);
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

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
