/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <map>
#include <mutex>

#include <boost/shared_ptr.hpp>

#include <osl/thread.h>
#include <vcl/idle.hxx>
#include <LibreOfficeKit/LibreOfficeKit.h>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include "../../source/inc/desktopdllapi.h"

using namespace css;
using namespace boost;

class LOKInteractionHandler;

namespace desktop {

    class CallbackFlushHandler : public Idle
    {
    public:
        explicit CallbackFlushHandler(LibreOfficeKitCallback pCallback, void* pData)
            : Idle( "lokit idle callback" ),
              m_pCallback(pCallback),
              m_pData(pData)
        {
            SetPriority(SchedulerPriority::POST_PAINT);
        }

        virtual ~CallbackFlushHandler()
        {
            Stop();

            // Wait for Invoke to finish, if called.
            std::unique_lock<std::mutex> lock(m_mutex);
        }

        virtual void Invoke() override
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            flush();
        }

        static
        void callback(const int type, const char* payload, void* data)
        {
            CallbackFlushHandler* self = reinterpret_cast<CallbackFlushHandler*>(data);
            if (self)
            {
                self->queue(type, payload);
            }
        }

        void queue(const int type, const char* payload)
        {
            std::unique_lock<std::mutex> lock(m_mutex);

            // TODO: Add more state tracking and prune superfluous notifications.
            if (type == LOK_CALLBACK_INVALIDATE_TILES || type == LOK_CALLBACK_TEXT_SELECTION)
            {
                if (m_queue.empty() || std::get<0>(m_queue.back()) != type)
                {
                    m_queue.emplace_back(type, std::string(payload ? payload : "(nil)"));

                    if (!IsActive())
                    {
                        Start();
                    }
                }
            }
            else
            {
                m_queue.emplace_back(type, std::string(payload ? payload : "(nil)"));
                flush();
            }
        }

    private:
        void flush()
        {
            if (m_pCallback)
            {
                for (auto& pair : m_queue)
                {
                    m_pCallback(std::get<0>(pair), std::get<1>(pair).c_str(), m_pData);
                }
            }

            m_queue.clear();
        }

    private:
        std::vector<std::tuple<int, std::string>> m_queue;
        LibreOfficeKitCallback m_pCallback;
        void *m_pData;
        std::mutex m_mutex;
    };

    struct DESKTOP_DLLPUBLIC LibLODocument_Impl : public _LibreOfficeKitDocument
    {
        uno::Reference<css::lang::XComponent> mxComponent;
        shared_ptr< LibreOfficeKitDocumentClass > m_pDocumentClass;
        shared_ptr<CallbackFlushHandler> mpCallbackFlushHandler;

        explicit LibLODocument_Impl(const uno::Reference <css::lang::XComponent> &xComponent);
        ~LibLODocument_Impl();
    };

    struct DESKTOP_DLLPUBLIC LibLibreOffice_Impl : public _LibreOfficeKit
    {
        OUString maLastExceptionMsg;
        boost::shared_ptr< LibreOfficeKitClass > m_pOfficeClass;
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
}
