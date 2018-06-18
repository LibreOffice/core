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

#include <osl/thread.h>
#include <rtl/ref.hxx>
#include <vcl/idle.hxx>
#include <LibreOfficeKit/LibreOfficeKit.h>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/lang/XComponent.hpp>

#include <desktop/dllapi.h>

class LOKInteractionHandler;

namespace desktop {

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

        typedef std::vector<std::pair<int, std::string>> queue_type;

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
    /// @param rOptions When exctacted, the Param=Value is removed from it.
    DESKTOP_DLLPUBLIC OUString extractParameter(OUString& aOptions, const OUString& rName);

    /// Helper function to convert JSON to a vector of PropertyValues.
    /// Public to be unit-test-able.
    DESKTOP_DLLPUBLIC std::vector<com::sun::star::beans::PropertyValue> jsonToPropertyValuesVector(const char* pJSON);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
