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
#include <memory>
#include <mutex>

#include <osl/thread.h>
#include <vcl/idle.hxx>
#include <LibreOfficeKit/LibreOfficeKit.h>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/lang/XComponent.hpp>

#include "../../source/inc/desktopdllapi.h"

class LOKInteractionHandler;

namespace desktop {

    class CallbackFlushHandler : public Idle
    {
    public:
        explicit CallbackFlushHandler(LibreOfficeKitCallback pCallback, void* pData)
            : Idle( "lokit timer callback" ),
              m_pCallback(pCallback),
              m_pData(pData)
        {
            SetPriority(SchedulerPriority::POST_PAINT);

            // Add the states that are safe to skip duplicates on,
            // even when not consequent.
            m_states.emplace(LOK_CALLBACK_TEXT_SELECTION_START, "NIL");
            m_states.emplace(LOK_CALLBACK_TEXT_SELECTION_END, "NIL");
            m_states.emplace(LOK_CALLBACK_TEXT_SELECTION, "NIL");
            m_states.emplace(LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR, "NIL");
            m_states.emplace(LOK_CALLBACK_STATE_CHANGED, "NIL");
            m_states.emplace(LOK_CALLBACK_MOUSE_POINTER, "NIL");
            m_states.emplace(LOK_CALLBACK_CELL_CURSOR, "NIL");
            m_states.emplace(LOK_CALLBACK_CELL_FORMULA, "NIL");
            m_states.emplace(LOK_CALLBACK_CURSOR_VISIBLE, "NIL");
            m_states.emplace(LOK_CALLBACK_SET_PART, "NIL");

            Start();
        }

        virtual ~CallbackFlushHandler()
        {
            Stop();

            // We might have important notification (.uno:save?).
            flush();
        }

        virtual void Invoke() override
        {
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

        void queue(const int type, const char* data)
        {
            const std::string payload(data ? data : "(nil)");
            std::unique_lock<std::mutex> lock(m_mutex);

            const auto stateIt = m_states.find(type);
            if (stateIt != m_states.end())
            {
                // If the state didn't change, it's safe to ignore.
                if (stateIt->second == payload)
                {
                    return;
                }

                stateIt->second = payload;
            }

            if (type == LOK_CALLBACK_INVALIDATE_TILES &&
                !m_queue.empty() && m_queue.back().first == type && m_queue.back().second == payload)
            {
                // Supress duplicate invalidation only when they are in sequence.
                return;
            }

            if (type == LOK_CALLBACK_TEXT_SELECTION && payload.empty())
            {
                // Removing text selection invalidates the start and end as well.
                m_states[LOK_CALLBACK_TEXT_SELECTION_START] = "";
                m_states[LOK_CALLBACK_TEXT_SELECTION_END] = "";
            }

            m_queue.emplace_back(type, payload);

            lock.unlock();
            if (!IsActive())
            {
                Start();
            }
        }

    private:
        void flush()
        {
            if (m_pCallback)
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                for (auto& pair : m_queue)
                {
                    m_pCallback(pair.first, pair.second.c_str(), m_pData);
                }

                m_queue.clear();
            }
        }

    private:
        std::vector<std::pair<int, std::string>> m_queue;
        std::map<int, std::string> m_states;
        LibreOfficeKitCallback m_pCallback;
        void *m_pData;
        std::mutex m_mutex;
    };

    struct DESKTOP_DLLPUBLIC LibLODocument_Impl : public _LibreOfficeKitDocument
    {
        css::uno::Reference<css::lang::XComponent> mxComponent;
        std::shared_ptr< LibreOfficeKitDocumentClass > m_pDocumentClass;
        std::shared_ptr<CallbackFlushHandler> mpCallbackFlushHandler;

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
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
