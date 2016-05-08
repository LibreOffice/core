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

#include <desktop/dllapi.h>

class LOKInteractionHandler;

namespace desktop {

    class CallbackFlushHandler : public Idle
    {
    public:
        explicit CallbackFlushHandler(LibreOfficeKitCallback pCallback, void* pData)
            : Idle( "lokit timer callback" ),
              m_pCallback(pCallback),
              m_pData(pData),
              m_bPartTilePainting(false)
        {
            SetPriority(SchedulerPriority::POST_PAINT);

            // Add the states that are safe to skip duplicates on,
            // even when not consequent.
            m_states.emplace(LOK_CALLBACK_TEXT_SELECTION_START, "NIL");
            m_states.emplace(LOK_CALLBACK_TEXT_SELECTION_END, "NIL");
            m_states.emplace(LOK_CALLBACK_TEXT_SELECTION, "NIL");
            m_states.emplace(LOK_CALLBACK_GRAPHIC_SELECTION, "NIL");
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
            CallbackFlushHandler* self = static_cast<CallbackFlushHandler*>(data);
            if (self)
            {
                self->queue(type, payload);
            }
        }

        void queue(const int type, const char* data)
        {
            if (m_bPartTilePainting)
            {
                // We drop notifications when this is set.
                return;
            }

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

            // These are safe to use the latest state and ignore previous
            // ones (if any) since the last overrides previous ones.
            switch (type)
            {
                case LOK_CALLBACK_TEXT_SELECTION_START:
                case LOK_CALLBACK_TEXT_SELECTION_END:
                case LOK_CALLBACK_TEXT_SELECTION:
                case LOK_CALLBACK_GRAPHIC_SELECTION:
                case LOK_CALLBACK_MOUSE_POINTER:
                case LOK_CALLBACK_CELL_CURSOR:
                case LOK_CALLBACK_CELL_FORMULA:
                case LOK_CALLBACK_CURSOR_VISIBLE:
                case LOK_CALLBACK_SET_PART:
                case LOK_CALLBACK_STATUS_INDICATOR_SET_VALUE:
                    removeAllButLast(type, false);
                break;

                // These come with rects, so drop earlier
                // only when the latter includes former ones.
                case LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR:
                case LOK_CALLBACK_INVALIDATE_TILES:
                    if (payload.empty())
                    {
                        // Invalidating everything means previously
                        // invalidated tiles can be dropped.
                        removeAllButLast(type, false);
                    }
                    else
                    {
                        removeAllButLast(type, true);
                    }

                break;
            }

            lock.unlock();
            if (!IsActive())
            {
                Start();
            }
        }

        void setPartTilePainting(const bool bPartPainting) { m_bPartTilePainting = bPartPainting; }
        bool isPartTilePainting() const { return m_bPartTilePainting; }

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

        void removeAllButLast(const int type, const bool identical)
        {
            int i = m_queue.size() - 1;
            std::string payload;
            for (; i >= 0; --i)
            {
                if (m_queue[i].first == type)
                {
                    payload = m_queue[i].second;
                    //SAL_WARN("idle", "Found [" + std::to_string(type) + "] at " + std::to_string(i) + ": [" + payload + "].");
                    break;
                }
            }

            for (--i; i >= 0; --i)
            {
                if (m_queue[i].first == type &&
                    (!identical || m_queue[i].second == payload))
                {
                    //SAL_WARN("idle", "Removing [" + std::to_string(type) + "] at " + std::to_string(i) + ": " + m_queue[i].second + "].");
                    m_queue.erase(m_queue.begin() + i);
                }
            }
        }

    private:
        std::vector<std::pair<int, std::string>> m_queue;
        std::map<int, std::string> m_states;
        LibreOfficeKitCallback m_pCallback;
        void *m_pData;
        bool m_bPartTilePainting;
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
