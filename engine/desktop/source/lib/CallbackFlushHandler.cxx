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

#include <lib/CallbackFlushHandler.hxx>

#include <lib/init.hxx>
#include "kitclipboard.hxx"
#include "kitinteractionhandler.hxx"

#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <comphelper/kit.hxx>
#include <comphelper/profilezone.hxx>
#include <o3tl/string_view.hxx>
#include <sal/log.hxx>
#include <sfx2/kit/componenthelpers.hxx>
#include <sfx2/kit/helper.hxx>
#include <sfx2/viewsh.hxx>
#include <tools/json_writer.hxx>
#include <vcl/ITiledRenderable.hxx>
#include <vcl/IVectorParts.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>

#include <boost/algorithm/string/trim.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <algorithm>
#include <sstream>
#include <string>
#include <string_view>

using namespace css;
using namespace vcl;
using namespace desktop;
using namespace ::cpo;
using namespace ::cpo::uno;

static int lcl_getViewId(std::string_view payload);

namespace desktop {

const OString& CallbackFlushHandler::CallbackData::getPayload() const
{
    if(PayloadString.isEmpty())
    {
        // Do to-string conversion on demand, as many calls will get dropped without
        // needing the string.
        if(PayloadObject.which() == 1)
            PayloadString = getRectangleAndPart().toString();
    }
    return PayloadString;
}

void CallbackFlushHandler::CallbackData::updateRectangleAndPart(const RectangleAndPart& rRectAndPart)
{
    PayloadObject = rRectAndPart;
    PayloadString.clear(); // will be set on demand if needed
}

const RectangleAndPart& CallbackFlushHandler::CallbackData::getRectangleAndPart() const
{
    // TODO: In case of unittests, they do not pass invalidations in binary but as text messages.
    // LO core should preferably always pass binary for performance.
    if(PayloadObject.which() != 1)
        PayloadObject = RectangleAndPart::Create(PayloadString);
    return boost::get<RectangleAndPart>(PayloadObject);
}

boost::property_tree::ptree& CallbackFlushHandler::CallbackData::setJson(const std::string& payload)
{
    boost::property_tree::ptree aTree;
    std::stringstream aStream(payload);
    boost::property_tree::read_json(aStream, aTree);

    // Let boost normalize the payload so it always matches the cache.
    setJson(aTree);

    // Return reference to the cached object.
    return boost::get<boost::property_tree::ptree>(PayloadObject);
}

void CallbackFlushHandler::CallbackData::setJson(const boost::property_tree::ptree& rTree)
{
    std::stringstream aJSONStream;
    constexpr bool bPretty = false; // Don't waste time and bloat logs.
    boost::property_tree::write_json(aJSONStream, rTree, bPretty);
    PayloadString = OString(o3tl::trim(aJSONStream.str()));

    PayloadObject = rTree;
}

const boost::property_tree::ptree& CallbackFlushHandler::CallbackData::getJson() const
{
    assert(PayloadObject.which() == 2);
    return boost::get<boost::property_tree::ptree>(PayloadObject);
}

int CallbackFlushHandler::CallbackData::getViewId() const
{
    if (isCached())
    {
        assert(PayloadObject.which() == 3);
        return boost::get<int>(PayloadObject);
    }
    return lcl_getViewId(getPayload());
}

bool CallbackFlushHandler::CallbackData::validate() const
{
    switch (PayloadObject.which())
    {
        // Not cached.
        case 0:
            return true;

        // RectangleAndPart.
        case 1:
            return getRectangleAndPart().toString().getStr() == getPayload();

        // Json.
        case 2:
        {
            std::stringstream aJSONStream;
            boost::property_tree::write_json(aJSONStream, getJson(), false);
            const std::string aExpected = boost::trim_copy(aJSONStream.str());
            return getPayload() == std::string_view(aExpected);
        }

        // View id.
        case 3:
            return getViewId() == lcl_getViewId( getPayload());

        default:
            assert(!"Unknown variant type; please add an entry to validate.");
    }

    return false;
}

} // namespace desktop

static bool lcl_isViewCallbackType(const COKitCallbackType type)
{
    switch (type)
    {
        case COKitCallbackType::CELL_VIEW_CURSOR:
        case COKitCallbackType::GRAPHIC_VIEW_SELECTION:
        case COKitCallbackType::INVALIDATE_VIEW_CURSOR:
        case COKitCallbackType::TEXT_VIEW_SELECTION:
        case COKitCallbackType::CELL_VIEW_SELECTION:
        case COKitCallbackType::VIEW_CURSOR_VISIBLE:
            return true;

        default:
            return false;
    }
}

static bool isUpdatedType(COKitCallbackType type)
{
    switch (type)
    {
        case COKitCallbackType::TEXT_SELECTION:
        case COKitCallbackType::TEXT_SELECTION_START:
        case COKitCallbackType::TEXT_SELECTION_END:
            return true;
        default:
            return false;
    }
}

static bool isUpdatedTypePerViewId(COKitCallbackType type)
{
    switch (type)
    {
        case COKitCallbackType::INVALIDATE_VISIBLE_CURSOR:
        case COKitCallbackType::INVALIDATE_VIEW_CURSOR:
        case COKitCallbackType::TEXT_VIEW_SELECTION:
            return true;
        default:
            return false;
    }
}

static int lcl_getViewId(std::string_view payload)
{
    // this is a cheap way how to get the viewId from a JSON message; proper
    // parsing is terribly expensive, and we just need the viewId here
    size_t viewIdPos = payload.find("viewId");
    if (viewIdPos == std::string::npos)
        return 0;

    size_t numberPos = payload.find(":", viewIdPos + 6);
    if (numberPos == std::string::npos)
        return 0;

    for (++numberPos; numberPos < payload.length(); ++numberPos)
    {
        if (payload[numberPos] == ',' || payload[numberPos] == '}' || (payload[numberPos] >= '0' && payload[numberPos] <= '9'))
            break;
    }

    if (numberPos < payload.length() && payload[numberPos] >= '0' && payload[numberPos] <= '9')
        return o3tl::toInt32(payload.substr(numberPos));

    return 0;
}

// One of these is created per view to handle events cf. doc_registerCallback
CallbackFlushHandler::CallbackFlushHandler(COKitDocument* pDocument, COKitCallback pCallback, void* pData)
    : m_pDocument(pDocument),
      m_pCallback(pCallback),
      m_pFlushEvent(nullptr),
      m_pData(pData),
      m_nDisableCallbacks(0)
{
    // Add the states that are safe to skip duplicates on, even when
    // not consequent (i.e. do no emit them if unchanged from last).
    m_states.emplace(COKitCallbackType::TEXT_SELECTION, "NIL"_ostr);
    m_states.emplace(COKitCallbackType::GRAPHIC_SELECTION, "NIL"_ostr);
    m_states.emplace(COKitCallbackType::INVALIDATE_VISIBLE_CURSOR, "NIL"_ostr);
    m_states.emplace(COKitCallbackType::STATE_CHANGED, "NIL"_ostr);
    m_states.emplace(COKitCallbackType::MOUSE_POINTER, "NIL"_ostr);
    m_states.emplace(COKitCallbackType::CELL_CURSOR, "NIL"_ostr);
    m_states.emplace(COKitCallbackType::CELL_FORMULA, "NIL"_ostr);
    m_states.emplace(COKitCallbackType::CELL_ADDRESS, "NIL"_ostr);
    m_states.emplace(COKitCallbackType::CURSOR_VISIBLE, "NIL"_ostr);
    m_states.emplace(COKitCallbackType::SET_PART, "NIL"_ostr);
    m_states.emplace(COKitCallbackType::TABLE_SELECTED, "NIL"_ostr);
    m_states.emplace(COKitCallbackType::TAB_STOP_LIST, "NIL"_ostr);
    m_states.emplace(COKitCallbackType::RULER_UPDATE, "NIL"_ostr);
    m_states.emplace(COKitCallbackType::VERTICAL_RULER_UPDATE, "NIL"_ostr);
    m_states.emplace(COKitCallbackType::STATUS_INDICATOR_SET_VALUE, "NIL"_ostr);

    std::string pViewRenderState = pDocument->getCommandValues(".uno:ViewRenderState");
    if (!pViewRenderState.empty())
    {
        m_aViewRenderState = OString(pViewRenderState);
    }
}

void CallbackFlushHandler::stop()
{
    if (m_pFlushEvent)
    {
        Application::RemoveUserEvent(m_pFlushEvent);
        m_pFlushEvent = nullptr;
    }
}

CallbackFlushHandler::~CallbackFlushHandler()
{
    stop();
}

CallbackFlushHandler::queue_type2::iterator CallbackFlushHandler::toQueue2(CallbackFlushHandler::queue_type1::iterator pos)
{
    int delta = std::distance(m_queue1.begin(), pos);
    return m_queue2.begin() + delta;
}

CallbackFlushHandler::queue_type2::reverse_iterator CallbackFlushHandler::toQueue2(CallbackFlushHandler::queue_type1::reverse_iterator pos)
{
    int delta = std::distance(m_queue1.rbegin(), pos);
    return m_queue2.rbegin() + delta;
}

void CallbackFlushHandler::setUpdatedType( COKitCallbackType eType, bool value )
{
    assert(isUpdatedType(eType));
    const size_t nIndex = static_cast<size_t>(eType);
    if( m_updatedTypes.size() <= nIndex )
        m_updatedTypes.resize( nIndex + 1 ); // new are default-constructed, i.e. false
    m_updatedTypes[ nIndex ] = value;
    if(value)
        scheduleFlush();
}

void CallbackFlushHandler::resetUpdatedType( COKitCallbackType eType )
{
    setUpdatedType( eType, false );
}

void CallbackFlushHandler::setUpdatedTypePerViewId( COKitCallbackType eType, int nViewId, int nSourceViewId, bool value )
{
    assert(isUpdatedTypePerViewId(eType));
    std::vector<PerViewIdData>& types = m_updatedTypesPerViewId[ nViewId ];
    const size_t nIndex = static_cast<size_t>(eType);
    if( types.size() <= nIndex )
        types.resize( nIndex + 1 ); // new are default-constructed, i.e. 'set' is false
    types[ nIndex ] = PerViewIdData{ value, nSourceViewId };
    if(value)
        scheduleFlush();
}

void CallbackFlushHandler::resetUpdatedTypePerViewId( COKitCallbackType eType, int nViewId )
{
    assert(isUpdatedTypePerViewId(eType));
    setUpdatedTypePerViewId( eType, nViewId, -1, false );
}

void CallbackFlushHandler::viewCallback(COKitCallbackType eType, const OString& pPayload)
{
    // A SET_PART payload carries the page-list index of the page the view switched to, while
    // the boundary names parts of a presentation or drawing document by their stable page
    // identifiers. Translate when queueing, while the page still holds that index.
    if (eType == COKitCallbackType::SET_PART)
    {
        if (ITiledRenderable* pDoc = getTiledRenderable(m_pDocument))
        {
            const OString aPartId = pDoc->getPartId(pPayload.toInt32(), pDoc->getEditMode());
            if (aPartId.isEmpty())
            {
                SAL_INFO("kit", "Skipping SET_PART for the gone part " << pPayload);
                return;
            }
            CallbackData callbackData(aPartId);
            queue(eType, callbackData);
            return;
        }
    }

    CallbackData callbackData(pPayload);
    queue(eType, callbackData);
}

void CallbackFlushHandler::viewCallbackWithViewId(COKitCallbackType eType, const OString& pPayload, int nViewId)
{
    CallbackData callbackData(pPayload, nViewId);
    queue(eType, callbackData);
}

void CallbackFlushHandler::viewVectorPartChanged(const OString& rPartId, int nMode)
{
    // Only a vector-rendering view consumes deltas, and only in a mode it
    // asked in.
    if (!m_bVectorRendering || rPartId.isEmpty())
        return;
    if (m_aVectorRenderingModes.find(nMode) == m_aVectorRenderingModes.end())
        return;

    // Repeated changes of the same part between two flushes collapse
    // into a single delta, computed at flush time.
    m_vectorDeltaParts.insert({ rPartId, nMode });
    scheduleFlush();
}

void CallbackFlushHandler::flushVectorPrimitivesDeltas()
{
    if (m_vectorDeltaParts.empty())
        return;

    std::set<std::pair<OString, int>> aParts;
    aParts.swap(m_vectorDeltaParts);

    // A background save sends no content. The parent process pushes the
    // deltas for the same changes, so the parts taken above are dropped.
    if (comphelper::COKit::isForkedChild())
    {
        SAL_INFO("kit", "Elide vector primitives delta in background save mode");
        return;
    }

    ITiledRenderable* pDocument = getTiledRenderable(m_pDocument);
    vcl::IVectorParts* pVectorParts = pDocument ? pDocument->getVectorPartsInterface() : nullptr;
    if (!pVectorParts)
        return;

    // The handlers of the other views of the document. A view can close while a delta is computed,
    // so each handler is held until the last part is done.
    std::vector<std::shared_ptr<CallbackFlushHandler>> aOtherHandlers;
    for (const auto& rEntry : static_cast<COKitDocumentImpl*>(m_pDocument)->mpCallbackFlushHandlers)
    {
        if (rEntry.second && rEntry.second.get() != this)
            aOtherHandlers.push_back(rEntry.second);
    }

    for (const std::pair<OString, int>& rPart : aParts)
    {
        const OString& rPartId = rPart.first;
        const int nMode = rPart.second;

        // One delta is written for a change of the part, so the part is taken from every other
        // handler that still waits to push it. A change that lands while the delta is computed
        // records the part on each handler again.
        for (const std::shared_ptr<CallbackFlushHandler>& pOther : aOtherHandlers)
            pOther->m_vectorDeltaParts.erase(rPart);

        // Computing the delta at delivery time reads the document after the change that triggered
        // the invalidation has fully landed. The document tracks the version the part was last
        // pushed at and writes the delta since it, so the mark advances only for a delta that is
        // handed out.
        //
        // The delta describes the part, not this view. So the first handler to flush a part writes
        // the one delta for the change.
        tools::JsonWriter aJsonWriter;
        pVectorParts->pushVectorPrimitivesDelta(
            aJsonWriter, std::string_view(rPartId.getStr(), rPartId.getLength()), nMode);
        const OString aDelta = aJsonWriter.finishAndGetAsOString();

        // Nothing is written when the part is unchanged since it was last pushed, or when no
        // client holds it. The delta does not depend on the view, so no other handler would get
        // more, and the empty document is not worth a frame of its own.
        if (aDelta == tools::JsonWriter::EMPTY_DOCUMENT)
            continue;

        // The part id goes on its own line ahead of the JSON, so a reader can tell which part
        // the delta is for without parsing it.
        const OString aPayload = rPartId + "\n" + aDelta;
        m_pCallback(COKitCallbackType::VECTOR_PRIMITIVES_DELTA, aPayload.getStr(), m_pData);
    }
}

void CallbackFlushHandler::viewInvalidateTilesCallback(const tools::Rectangle* pRect, int nPart, int nMode)
{
    // The invalidation carries the page-list index of the invalidated page, while the boundary
    // names parts of a presentation or drawing document by their stable page identifiers.
    // Translate when queueing, while the page still holds that index, so the queued rectangles
    // merge under a key that survives page renumbering. A negative part stands for every part
    // and passes through as "-1".
    OString aPart;
    if (nPart >= 0)
    {
        ITiledRenderable* pDoc = getTiledRenderable(m_pDocument);
        aPart = pDoc ? pDoc->getPartId(nPart, nMode) : OString::number(nPart);
        if (aPart.isEmpty())
            return;
    }
    else
        aPart = "-1"_ostr;

    tools::Rectangle& rPaintedTiles = m_aPaintedTiles[aPart][nMode];

    tools::Rectangle aRect;
    if (m_bVectorRendering)
    {
        // A vector-rendered view paints no bitmap tiles, so there is no
        // painted-tile bounding box to crop against. Forward the invalidation
        // as-is, or invalidate everything when the rect is null. The delta
        // push is not scheduled here: an invalidation only reaches views
        // whose paint pipeline is active, while viewVectorPartChanged is
        // driven from the model change and reaches every view.
        aRect = pRect ? *pRect : RectangleAndPart::emptyAllRectangle;
    }
    else if (rPaintedTiles.IsEmpty())
    {
        // We have not sent any tiles: don't send invalidations.
        return;
    }
    else if (pRect)
    {
        // We got an invalidate: crop it against the bbox.
        aRect = *pRect;
        aRect.Intersection(rPaintedTiles);
        if (aRect.IsEmpty())
        {
            return;
        }
    }
    else
    {
        // EMPTY invalidation: reset the bbox.
        rPaintedTiles = tools::Rectangle();
        // nullptr pRect means: invalidate everything.
        aRect = RectangleAndPart::emptyAllRectangle;
    }

    // RectangleAndPart ctor doesn't store &aRect, so this is OK.
    CallbackData callbackData(&aRect, aPart, nMode);
    queue(COKitCallbackType::INVALIDATE_TILES, callbackData);
}

void CallbackFlushHandler::viewUpdatedCallback(COKitCallbackType eType)
{
    assert(isUpdatedType( eType ));
    std::unique_lock<std::recursive_mutex> lock(m_mutex);
    SAL_INFO("kit", "Updated: [" << eType << "]");
    setUpdatedType(eType, true);
}

void CallbackFlushHandler::viewUpdatedCallbackPerViewId(COKitCallbackType eType, int nViewId, int nSourceViewId)
{
    assert(isUpdatedTypePerViewId( eType ));
    std::unique_lock<std::recursive_mutex> lock(m_mutex);
    SAL_INFO("kit", "Updated: [" << eType << "]");
    setUpdatedTypePerViewId(eType, nViewId, nSourceViewId, true);
}

void CallbackFlushHandler::dumpState(rtl::OStringBuffer &rState)
{
    // NB. no locking
    rState.append("\n    View:\t");
    rState.append(static_cast<sal_Int32>(m_viewId));
    rState.append("\n\tDisableCallbacks:\t");
    rState.append(static_cast<sal_Int32>(m_nDisableCallbacks));
    rState.append("\n\tStates:\n");
    for (const auto &i : m_states)
    {
        rState.append("\n\t\t");
        rState.append(static_cast<sal_Int32>(i.first));
        rState.append("\t");
        rState.append(i.second);
    }
}

void CallbackFlushHandler::resendVisibleCursor()
{
    std::unique_lock<std::recursive_mutex> lock(m_mutex);
    // A caret equal to the one sent last is dropped at the flush, so the record of that one is
    // reset to the value it starts with.
    m_states[COKitCallbackType::INVALIDATE_VISIBLE_CURSOR] = "NIL"_ostr;
}

void CallbackFlushHandler::viewAddPendingInvalidateTiles()
{
    // Invoke() will call flushPendingKitInvalidateTiles(), so just make sure the timer is active.
    scheduleFlush();
}

void CallbackFlushHandler::queue(const COKitCallbackType type, const OString& data)
{
    CallbackData callbackData(data);
    queue(type, callbackData);
}

void CallbackFlushHandler::queue(const COKitCallbackType type, CallbackData& aCallbackData)
{
    comphelper::ProfileZone aZone("CallbackFlushHandler::queue");

    SAL_INFO("kit", "Queue: [" << type << "]: [" << aCallbackData.getPayload() << "] on " << m_queue1.size() << " entries.");

    if (comphelper::COKit::isForkedChild())
    {
        // In background mode - avoid any extraneous or confusing messages
        switch (type)
        {
        case COKitCallbackType::INVALIDATE_TILES:
        case COKitCallbackType::INVALIDATE_VISIBLE_CURSOR:
        case COKitCallbackType::TEXT_SELECTION:
        case COKitCallbackType::CURSOR_VISIBLE:
        case COKitCallbackType::GRAPHIC_SELECTION:
        case COKitCallbackType::TABLE_SELECTED:
        case COKitCallbackType::SET_PART:
        case COKitCallbackType::DOCUMENT_SIZE_CHANGED:
        case COKitCallbackType::MOUSE_POINTER:
        case COKitCallbackType::INVALIDATE_HEADER:
        case COKitCallbackType::INVALIDATE_SHEET_GEOMETRY:
        case COKitCallbackType::VECTOR_PART_IDS:
        case COKitCallbackType::INVALIDATE_CELL_RANGE_INVENTORY:
        case COKitCallbackType::CELL_SELECTION:
        case COKitCallbackType::CELL_VIEW_SELECTION:
        case COKitCallbackType::CELL_CURSOR_STATE:
            SAL_INFO("kit", "Elide event in background save mode");
            return;
        default:
            break;
        }
    }

    bool bIsChartActive = false;
    bool bIsComment = false;
    if (type == COKitCallbackType::GRAPHIC_SELECTION)
    {
        KitChartHelper aChartHelper(SfxViewShell::Current());
        bIsChartActive = aChartHelper.GetWindow() != nullptr;
    }
    else if (type == COKitCallbackType::COMMENT)
    {
        bIsComment = true;
    }
    else if (type == COKitCallbackType::VIEW_RENDER_STATE)
    {
        m_aViewRenderState = aCallbackData.getPayload();
    }

    if (callbacksDisabled() && !bIsChartActive && !bIsComment)
    {
        // We drop notifications when this is set, except for important ones.
        // When we issue a complex command (such as .uno:InsertAnnotation)
        // there will be multiple notifications. On the first invalidation
        // we will start painting, but other events will get fired
        // while the complex command in question executes.
        // We don't want to suppress everything here on the wrong assumption
        // that no new events are fired during painting.
        //
        // Painting the tile of another part switches the view to that part and
        // switches it back. The text selection reported in between belongs to
        // that transient state and is empty, which says that the selection is
        // gone. So the three selection types are not in the list below, and a
        // paint reports nothing about the selection.
        if (type != COKitCallbackType::STATE_CHANGED &&
            type != COKitCallbackType::INVALIDATE_TILES &&
            type != COKitCallbackType::INVALIDATE_VISIBLE_CURSOR &&
            type != COKitCallbackType::CURSOR_VISIBLE &&
            type != COKitCallbackType::VIEW_CURSOR_VISIBLE &&
            type != COKitCallbackType::MEDIA_SHAPE &&
            type != COKitCallbackType::REFERENCE_MARKS)
        {
            SAL_INFO("kit", "Skipping while painting [" << type << "]: [" << aCallbackData.getPayload() << "].");
            return;
        }

        // In Writer we drop all notifications during painting.
        if (static_cast<COKitDocumentImpl*>(m_pDocument)->getDocumentType() == COKitDocumentType::TEXT)
            return;
    }

    // Suppress invalid payloads.
    if (type == COKitCallbackType::INVALIDATE_VISIBLE_CURSOR &&
        aCallbackData.getPayload().indexOf(", 0, 0, ") != -1 &&
        aCallbackData.getPayload().indexOf("\"hyperlink\":\"\"") == -1 &&
        aCallbackData.getPayload().indexOf("\"hyperlink\": {}") == -1)
    {
        // The cursor position is often the relative coordinates of the widget
        // issuing it, instead of the absolute one that we expect.
        // This is temporary however, and, once the control is created and initialized
        // correctly, it eventually emits the correct absolute coordinates.
        SAL_INFO("kit", "Skipping invalid event [" << type << "]: [" << aCallbackData.getPayload() << "].");
        return;
    }

    std::unique_lock<std::recursive_mutex> lock(m_mutex);

    // Update types should be received via the updated callbacks for performance,
    // getting them as normal callbacks is technically not wrong, but probably should be avoided.
    // Reset the updated flag if we get a normal message.
    if(isUpdatedType(type))
    {
        SAL_INFO("kit", "Received event with updated type [" << type << "] as normal callback");
        resetUpdatedType(type);
    }
    if(isUpdatedTypePerViewId(type))
    {
        SAL_INFO("kit", "Received event with updated type [" << type << "] as normal callback");
        resetUpdatedTypePerViewId(type, aCallbackData.getViewId());
    }

    // drop duplicate callbacks for the listed types
    switch (type)
    {
        case COKitCallbackType::TEXT_SELECTION_START:
        case COKitCallbackType::TEXT_SELECTION_END:
        case COKitCallbackType::TEXT_SELECTION:
        case COKitCallbackType::GRAPHIC_SELECTION:
        case COKitCallbackType::GRAPHIC_VIEW_SELECTION:
        case COKitCallbackType::INVALIDATE_VISIBLE_CURSOR:
        case COKitCallbackType::INVALIDATE_VIEW_CURSOR:
        case COKitCallbackType::STATE_CHANGED:
        case COKitCallbackType::MOUSE_POINTER:
        case COKitCallbackType::CELL_CURSOR:
        case COKitCallbackType::CELL_VIEW_CURSOR:
        case COKitCallbackType::CELL_FORMULA:
        case COKitCallbackType::CELL_ADDRESS:
        case COKitCallbackType::CELL_SELECTION_AREA:
        case COKitCallbackType::CELL_SELECTION:
        case COKitCallbackType::CELL_CURSOR_STATE:
        case COKitCallbackType::CURSOR_VISIBLE:
        case COKitCallbackType::VIEW_CURSOR_VISIBLE:
        case COKitCallbackType::SET_PART:
        case COKitCallbackType::TEXT_VIEW_SELECTION:
        case COKitCallbackType::CELL_VIEW_SELECTION:
        case COKitCallbackType::INVALIDATE_HEADER:
        case COKitCallbackType::WINDOW:
        case COKitCallbackType::CALC_FUNCTION_LIST:
        case COKitCallbackType::INVALIDATE_SHEET_GEOMETRY:
        case COKitCallbackType::INVALIDATE_CELL_RANGE_INVENTORY:
        case COKitCallbackType::REFERENCE_MARKS:
        case COKitCallbackType::CELL_AUTO_FILL_AREA:
        case COKitCallbackType::A11Y_FOCUS_CHANGED:
        case COKitCallbackType::A11Y_CARET_CHANGED:
        case COKitCallbackType::A11Y_TEXT_SELECTION_CHANGED:
        case COKitCallbackType::A11Y_FOCUSED_CELL_CHANGED:
        case COKitCallbackType::COLOR_PALETTES:
        case COKitCallbackType::A11Y_EDITING_IN_SELECTION_STATE:
        case COKitCallbackType::A11Y_SELECTION_CHANGED:
        {
            const auto pos = std::find(m_queue1.rbegin(), m_queue1.rend(), type);
            auto pos2 = toQueue2(pos);
            if (pos != m_queue1.rend() && pos2->getPayload() == aCallbackData.getPayload())
            {
                SAL_INFO("kit", "Skipping queue duplicate [" << type << + "]: [" << aCallbackData.getPayload() << "].");
                return;
            }
        }
        break;

        default:
        break;
    }

    if (type == COKitCallbackType::TEXT_SELECTION && aCallbackData.isEmpty())
    {
        const auto posStart = std::find(m_queue1.rbegin(), m_queue1.rend(), COKitCallbackType::TEXT_SELECTION_START);
        auto posStart2 = toQueue2(posStart);
        if (posStart != m_queue1.rend())
            posStart2->clear();

        const auto posEnd = std::find(m_queue1.rbegin(), m_queue1.rend(), COKitCallbackType::TEXT_SELECTION_END);
        auto posEnd2 = toQueue2(posEnd);
        if (posEnd != m_queue1.rend())
            posEnd2->clear();
    }

    // When payload is empty discards any previous state.
    if (aCallbackData.isEmpty())
    {
        switch (type)
        {
            case COKitCallbackType::TEXT_SELECTION_START:
            case COKitCallbackType::TEXT_SELECTION_END:
            case COKitCallbackType::TEXT_SELECTION:
            case COKitCallbackType::GRAPHIC_SELECTION:
            case COKitCallbackType::INVALIDATE_VISIBLE_CURSOR:
            case COKitCallbackType::INVALIDATE_TILES:
            case COKitCallbackType::TOOLTIP:
            // The callback carries no payload, so the last queued copy says what every earlier
            // copy says.
            case COKitCallbackType::SLIDE_LINKS_CHANGED:
                if (removeAll(type))
                    SAL_INFO("kit", "Removed dups of [" << type << "]: [" << aCallbackData.getPayload() << "].");
                break;

            default:
                break;
        }
    }
    else
    {
        switch (type)
        {
            // These are safe to use the latest state and ignore previous
            // ones (if any) since the last overrides previous ones.
            case COKitCallbackType::TEXT_SELECTION_START:
            case COKitCallbackType::TEXT_SELECTION_END:
            case COKitCallbackType::TEXT_SELECTION:
            case COKitCallbackType::MOUSE_POINTER:
            case COKitCallbackType::CELL_CURSOR:
            case COKitCallbackType::CELL_SELECTION:
            case COKitCallbackType::CELL_CURSOR_STATE:
            case COKitCallbackType::CELL_FORMULA:
            case COKitCallbackType::CELL_ADDRESS:
            case COKitCallbackType::CURSOR_VISIBLE:
            case COKitCallbackType::SET_PART:
            case COKitCallbackType::STATUS_INDICATOR_SET_VALUE:
            case COKitCallbackType::RULER_UPDATE:
            case COKitCallbackType::VERTICAL_RULER_UPDATE:
            case COKitCallbackType::A11Y_FOCUS_CHANGED:
            case COKitCallbackType::A11Y_CARET_CHANGED:
            case COKitCallbackType::A11Y_TEXT_SELECTION_CHANGED:
            case COKitCallbackType::A11Y_FOCUSED_CELL_CHANGED:
            case COKitCallbackType::COLOR_PALETTES:
            case COKitCallbackType::TOOLTIP:
            case COKitCallbackType::SHAPE_INNER_TEXT:
            case COKitCallbackType::SHAPE_DRAG_PREVIEW:
            case COKitCallbackType::VECTOR_PART_IDS:
            {
                if (removeAll(type))
                    SAL_INFO("kit", "Removed dups of [" << type << "]: [" << aCallbackData.getPayload() << "].");
            }
            break;

            // These are safe to use the latest state and ignore previous
            // ones (if any) since the last overrides previous ones,
            // but only if the view is the same.
            case COKitCallbackType::INVALIDATE_VISIBLE_CURSOR:
                // deleting the duplicate of visible cursor message can cause hyperlink popup not to show up on second/or more click on the same place.
                // If the hyperlink is not empty we can bypass that to show the popup
                if (aCallbackData.getPayload().indexOf("\"hyperlink\":\"\"") == -1
                    && aCallbackData.getPayload().indexOf("\"hyperlink\": {}") == -1)
                    break;
                [[fallthrough]];
            case COKitCallbackType::CELL_VIEW_CURSOR:
            case COKitCallbackType::GRAPHIC_VIEW_SELECTION:
            case COKitCallbackType::INVALIDATE_VIEW_CURSOR:
            case COKitCallbackType::TEXT_VIEW_SELECTION:
            case COKitCallbackType::CELL_VIEW_SELECTION:
            case COKitCallbackType::VIEW_CURSOR_VISIBLE:
            case COKitCallbackType::CALC_FUNCTION_LIST:
            case COKitCallbackType::FORM_FIELD_BUTTON:
            {
                const int nViewId = aCallbackData.getViewId();
                removeAll(type, [nViewId] (const CallbackData& elemData) {
                        return (nViewId == elemData.getViewId());
                    }
                );
            }
            break;

            case COKitCallbackType::INVALIDATE_TILES:
                if (processInvalidateTilesEvent(type, aCallbackData))
                    return;
            break;

            // State changes with same name override previous ones with a different value.
            // Ex. ".uno:PageStatus=Slide 20 of 83" overwrites any previous PageStatus.
            case COKitCallbackType::STATE_CHANGED:
            {
                // Compare the state name=value and overwrite earlier entries with same name.
                const auto pos = aCallbackData.getPayload().indexOf('=');
                if (pos != -1)
                {
                    const std::string_view name = aCallbackData.getPayload().subView(0, pos + 1);
                    // This is needed because otherwise it creates some problems when
                    // a save occurs while a cell is still edited in Calc.
                    if (name != ".uno:ModifiedStatus=")
                    {
                        removeAll(type, [&name] (const CallbackData& elemData) {
                                return elemData.getPayload().startsWith(name);
                            }
                        );
                    }
                }
            }
            break;

            case COKitCallbackType::WINDOW:
                if (processWindowEvent(type, aCallbackData))
                    return;
            break;

            case COKitCallbackType::GRAPHIC_SELECTION:
            {
                // remove only selection ranges and 'EMPTY' messages
                // always send 'INPLACE' and 'INPLACE EXIT' messages
                removeAll(type, [] (const CallbackData& elemData)
                    { return (elemData.getPayload().indexOf("INPLACE") == -1); });
            }
            break;

            default:
            break;
        }
    }

    // Validate that the cached data and the payload string are identical.
    assert(aCallbackData.validate() && "Cached callback payload object and string mismatch!");
    m_queue1.emplace_back(type);
    m_queue2.emplace_back(aCallbackData);
    SAL_INFO("kit", "Queued #" << (m_queue1.size() - 1) <<
             " [" << type << "]: [" << aCallbackData.getPayload() << "] to have " << m_queue1.size() << " entries.");

#ifdef DBG_UTIL
    // Validate the cached data of the whole queue.
    assert(
        std::all_of(
            m_queue2.begin(), m_queue2.end(),
            [](const CallbackData& c) { return c.validate(); }));
#endif

    lock.unlock();
    scheduleFlush();
}

bool CallbackFlushHandler::processInvalidateTilesEvent(COKitCallbackType type, CallbackData& aCallbackData)
{
    RectangleAndPart rcNew = aCallbackData.getRectangleAndPart();
    if (rcNew.isEmpty())
    {
        SAL_INFO("kit", "Skipping invalid event [" << type << "]: [" << aCallbackData.getPayload() << "].");
        return true;
    }

    // If we have to invalidate all tiles, we can skip any new tile invalidation.
    // Find the last INVALIDATE_TILES entry, if any to see if it's invalidate-all.
    const auto pos
        = std::find(m_queue1.rbegin(), m_queue1.rend(), COKitCallbackType::INVALIDATE_TILES);
    if (pos != m_queue1.rend())
    {
        auto pos2 = toQueue2(pos);
        const RectangleAndPart& rcOld = pos2->getRectangleAndPart();
        if (rcOld.isInfinite() && (rcOld.isAllParts() || rcOld.m_aPart == rcNew.m_aPart) &&
            (rcOld.m_nMode == rcNew.m_nMode))
        {
            SAL_INFO("kit", "Skipping queue [" << type << "]: [" << aCallbackData.getPayload()
                                               << "] since all tiles need to be invalidated.");
            return true;
        }

        if ((rcOld.isAllParts() || rcOld.m_aPart == rcNew.m_aPart) && (rcOld.m_nMode == rcNew.m_nMode))
        {
            // If fully overlapping.
            if (rcOld.m_aRectangle.Contains(rcNew.m_aRectangle))
            {
                SAL_INFO("kit", "Skipping queue [" << type << "]: [" << aCallbackData.getPayload()
                                                   << "] since overlaps existing all-parts.");
                return true;
            }
        }
    }

    if (rcNew.isInfinite())
    {
        SAL_INFO("kit", "Have Empty [" << type << "]: [" << aCallbackData.getPayload()
                                       << "] so removing all with part " << rcNew.m_aPart << ".");
        removeAll(COKitCallbackType::INVALIDATE_TILES, [&rcNew](const CallbackData& elemData) {
            // Remove exiting if new is all-encompassing, or if of the same part.
            return ((rcNew.isAllParts() || rcNew.m_aPart == elemData.getRectangleAndPart().m_aPart)
                && (rcNew.m_nMode == elemData.getRectangleAndPart().m_nMode));
        });
    }
    else
    {
        const auto rcOrig = rcNew;

        SAL_INFO("kit", "Have [" << type << "]: [" << aCallbackData.getPayload() << "] so merging overlapping.");
        removeAll(COKitCallbackType::INVALIDATE_TILES,[&rcNew](const CallbackData& elemData) {
            const RectangleAndPart& rcOld = elemData.getRectangleAndPart();
            if (!rcNew.isAllParts() && !rcOld.isAllParts() &&
                (rcOld.m_aPart != rcNew.m_aPart || rcOld.m_nMode != rcNew.m_nMode))
            {
                SAL_INFO("kit", "Nothing to merge between new: "
                                    << rcNew.toString() << ", and old: " << rcOld.toString());
                return false;
            }

            if (rcNew.isAllParts())
            {
                // Don't merge unless fully overlapped.
                SAL_INFO("kit", "New " << rcNew.toString() << " has " << rcOld.toString()
                                       << "?");
                if (rcNew.m_aRectangle.Contains(rcOld.m_aRectangle) && rcOld.m_nMode == rcNew.m_nMode)
                {
                    SAL_INFO("kit", "New " << rcNew.toString() << " engulfs old "
                                           << rcOld.toString() << ".");
                    return true;
                }
            }
            else if (rcOld.isAllParts())
            {
                // Don't merge unless fully overlapped.
                SAL_INFO("kit", "Old " << rcOld.toString() << " has " << rcNew.toString()
                                       << "?");
                if (rcOld.m_aRectangle.Contains(rcNew.m_aRectangle) && rcOld.m_nMode == rcNew.m_nMode)
                {
                    SAL_INFO("kit", "New " << rcNew.toString() << " engulfs old "
                                           << rcOld.toString() << ".");
                    return true;
                }
            }
            else
            {
                const tools::Rectangle rcOverlap
                    = rcNew.m_aRectangle.GetIntersection(rcOld.m_aRectangle);
                const bool bOverlap = !rcOverlap.IsEmpty() && rcOld.m_nMode == rcNew.m_nMode;
                SAL_INFO("kit", "Merging " << rcNew.toString() << " & " << rcOld.toString()
                                           << " => " << rcOverlap.toString()
                                           << " Overlap: " << bOverlap);
                if (bOverlap)
                {
                    rcNew.m_aRectangle.Union(rcOld.m_aRectangle);
                    SAL_INFO("kit", "Merged: " << rcNew.toString());
                    return true;
                }
            }

            // Keep others.
            return false;
        });

        if (rcNew.m_aRectangle != rcOrig.m_aRectangle)
        {
            SAL_INFO("kit", "Replacing: " << rcOrig.toString() << " by " << rcNew.toString());
            if (rcNew.m_aRectangle.GetWidth() < rcOrig.m_aRectangle.GetWidth()
                || rcNew.m_aRectangle.GetHeight() < rcOrig.m_aRectangle.GetHeight())
            {
                SAL_WARN("kit", "Error: merged rect smaller.");
            }
        }
    }

    aCallbackData.updateRectangleAndPart(rcNew);
    // Queue this one.
    return false;
}

bool CallbackFlushHandler::processWindowEvent(COKitCallbackType type, CallbackData& aCallbackData)
{
    const OString& payload = aCallbackData.getPayload();

    boost::property_tree::ptree& aTree = aCallbackData.setJson(std::string(payload));
    const unsigned nKitWindowId = aTree.get<unsigned>("id", 0);
    const std::string aAction = aTree.get<std::string>("action", "");
    if (aAction == "invalidate")
    {
        std::string aRectStr = aTree.get<std::string>("rectangle", "");
        // no 'rectangle' field => invalidate all of the window =>
        // remove all previous window part invalidations
        if (aRectStr.empty())
        {
            removeAll(COKitCallbackType::WINDOW,[&nKitWindowId](const CallbackData& elemData) {
                const boost::property_tree::ptree& aOldTree = elemData.getJson();
                if (nKitWindowId == aOldTree.get<unsigned>("id", 0)
                    && aOldTree.get<std::string>("action", "") == "invalidate")
                {
                    return true;
                }
                return false;
            });
        }
        else
        {
            // if we have to invalidate all of the window, ignore
            // any part invalidation message
            bool invAllExist = false;
            auto it1 = m_queue1.rbegin();
            auto it2 = m_queue2.rbegin();
            for (;it1 != m_queue1.rend(); ++it1, ++it2)
            {
                if (*it1 != COKitCallbackType::WINDOW)
                    continue;
                const boost::property_tree::ptree& aOldTree = it2->getJson();
                if (nKitWindowId == aOldTree.get<unsigned>("id", 0)
                     && aOldTree.get<std::string>("action", "") == "invalidate"
                     && aOldTree.get<std::string>("rectangle", "").empty())
                {
                    invAllExist = true;
                    break;
                }
            }

            // we found a invalidate-all window callback
            if (invAllExist)
            {
                SAL_INFO("kit.dialog", "Skipping queue ["
                                           << type << "]: [" << payload
                                           << "] since whole window needs to be invalidated.");
                return true;
            }

            std::istringstream aRectStream(aRectStr);
            tools::Long nLeft, nTop, nWidth, nHeight;
            char nComma;
            aRectStream >> nLeft >> nComma >> nTop >> nComma >> nWidth >> nComma >> nHeight;
            tools::Rectangle aNewRect(nLeft, nTop, nLeft + nWidth, nTop + nHeight);
            bool currentIsRedundant = false;
            removeAll(COKitCallbackType::WINDOW, [&aNewRect, &nKitWindowId,
                       &currentIsRedundant](const CallbackData& elemData) {
                const boost::property_tree::ptree& aOldTree = elemData.getJson();
                if (aOldTree.get<std::string>("action", "") == "invalidate")
                {
                    // Not possible that we encounter an empty rectangle here; we already handled this case above.
                    std::istringstream aOldRectStream(aOldTree.get<std::string>("rectangle", ""));
                    tools::Long nOldLeft, nOldTop, nOldWidth, nOldHeight;
                    char nOldComma;
                    aOldRectStream >> nOldLeft >> nOldComma >> nOldTop >> nOldComma >> nOldWidth
                        >> nOldComma >> nOldHeight;
                    const tools::Rectangle aOldRect = tools::Rectangle(
                        nOldLeft, nOldTop, nOldLeft + nOldWidth, nOldTop + nOldHeight);

                    if (nKitWindowId == aOldTree.get<unsigned>("id", 0))
                    {
                        if (aNewRect == aOldRect)
                        {
                            SAL_INFO("kit.dialog", "Duplicate rect [" << aNewRect.toString()
                                                                      << "]. Skipping new.");
                            // We have a rectangle in the queue already that makes the current Callback useless.
                            currentIsRedundant = true;
                            return false;
                        }
                        // new one engulfs the old one?
                        else if (aNewRect.Contains(aOldRect))
                        {
                            SAL_INFO("kit.dialog",
                                     "New rect [" << aNewRect.toString() << "] engulfs old ["
                                                  << aOldRect.toString() << "]. Replacing old.");
                            return true;
                        }
                        // old one engulfs the new one?
                        else if (aOldRect.Contains(aNewRect))
                        {
                            SAL_INFO("kit.dialog",
                                     "Old rect [" << aOldRect.toString() << "] engulfs new ["
                                                  << aNewRect.toString() << "]. Skipping new.");
                            // We have a rectangle in the queue already that makes the current Callback useless.
                            currentIsRedundant = true;
                            return false;
                        }
                        else
                        {
                            // Overlapping rects.
                            const tools::Rectangle aPreMergeRect = aNewRect;
                            aNewRect.Union(aOldRect);
                            SAL_INFO("kit.dialog", "Merging rects ["
                                                       << aPreMergeRect.toString() << "] & ["
                                                       << aOldRect.toString() << "] = ["
                                                       << aNewRect.toString()
                                                       << "]. Replacing old.");
                            return true;
                        }
                    }
                }

                // keep rest
                return false;
            });

            // Do not enqueue if redundant.
            if (currentIsRedundant)
                return true;

            aTree.put("rectangle", aNewRect.toString().getStr());
            aCallbackData.setJson(aTree);
            assert(aCallbackData.validate() && "Validation after setJson failed!");
        }
    }
    else if (aAction == "created")
    {
        // Remove all previous actions on same dialog, if we are creating it anew.
        removeAll(COKitCallbackType::WINDOW,[&nKitWindowId](const CallbackData& elemData) {
            const boost::property_tree::ptree& aOldTree = elemData.getJson();
            if (nKitWindowId == aOldTree.get<unsigned>("id", 0))
                return true;
            return false;
        });

        VclPtr<Window> pWindow = vcl::Window::FindKitWindow(nKitWindowId);
        if (!pWindow)
        {
            SetLastExceptionMsg(u"Document doesn't support dialog rendering, or window not found."_ustr);
            return false;
        }

        auto xClip = forceSetClipboardForCurrentView(m_pDocument);

        pWindow->SetClipboard(uno::Reference<datatransfer::clipboard::XClipboard>(xClip));
    }
    else if (aAction == "size_changed")
    {
        // A size change is practically re-creation of the window.
        // But at a minimum it's a full invalidation.
        removeAll(COKitCallbackType::WINDOW, [&nKitWindowId](const CallbackData& elemData) {
            const boost::property_tree::ptree& aOldTree = elemData.getJson();
            if (nKitWindowId == aOldTree.get<unsigned>("id", 0))
            {
                const std::string aOldAction = aOldTree.get<std::string>("action", "");
                if (aOldAction == "invalidate")
                    return true;
            }
            return false;
        });
    }

    // Queue this one.
    return false;
}

void CallbackFlushHandler::enqueueUpdatedTypes()
{
    if( m_updatedTypes.empty() && m_updatedTypesPerViewId.empty())
        return;
    assert(m_viewId >= 0);
    SfxViewShell* viewShell = SfxViewShell::GetFirst( false,
        [this](const SfxViewShell& shell) { return shell.GetViewShellId().get() == m_viewId; } );

    // First move data to local structures, so that callbacks don't possibly modify it.
    std::vector<bool> updatedTypes;
    std::swap(updatedTypes, m_updatedTypes);
    boost::container::flat_map<int, std::vector<PerViewIdData>> updatedTypesPerViewId;
    std::swap(updatedTypesPerViewId, m_updatedTypesPerViewId);

    // Some types must always precede other types, for example
    // COKitCallbackType::TEXT_SELECTION_START and COKitCallbackType::TEXT_SELECTION_END
    // must always precede COKitCallbackType::TEXT_SELECTION if present.
    // Only these types should be present (see isUpdatedType()) and should be processed in this order.
    static const COKitCallbackType orderedUpdatedTypes[] = {
        COKitCallbackType::TEXT_SELECTION_START, COKitCallbackType::TEXT_SELECTION_END,
        COKitCallbackType::TEXT_SELECTION };
    // Only these types should be present (see isUpdatedTypePerViewId()) and (as of now)
    // the order doesn't matter.
    static const COKitCallbackType orderedUpdatedTypesPerViewId[] = {
        COKitCallbackType::INVALIDATE_VISIBLE_CURSOR,
        COKitCallbackType::INVALIDATE_VIEW_CURSOR,
        COKitCallbackType::TEXT_VIEW_SELECTION };

    if (viewShell)
    {
        for( COKitCallbackType type : orderedUpdatedTypes )
        {
            const size_t nIndex = static_cast<size_t>(type);
            if(nIndex < updatedTypes.size() && updatedTypes[ nIndex ])
            {
                enqueueUpdatedType( type, viewShell, m_viewId );
            }
        }
    }
    else
    {
        // View removed, probably cleaning up.
        SAL_INFO("kit", "View #" << m_viewId << " no longer found for updated events");
    }

    for( const auto& it : updatedTypesPerViewId )
    {
        int viewId = it.first;
        const std::vector<PerViewIdData>& types = it.second;
        for( COKitCallbackType type : orderedUpdatedTypesPerViewId )
        {
            const size_t nIndex = static_cast<size_t>(type);
            if(nIndex < types.size() && types[ nIndex ].set)
            {
                SfxViewShell* sourceViewShell = viewShell;
                const int sourceViewId = types[ nIndex ].sourceViewId;
                if( sourceViewId != m_viewId )
                {
                    assert(sourceViewId >= 0);
                    sourceViewShell = SfxViewShell::GetFirst( false,
                    [sourceViewId](const SfxViewShell& shell) { return shell.GetViewShellId().get() == sourceViewId; } );
                }
                if(sourceViewShell == nullptr)
                {
                    SAL_INFO("kit", "View #" << sourceViewId << " no longer found for updated event [" << type << "]");
                    continue; // View removed, probably cleaning up.
                }
                enqueueUpdatedType( type, sourceViewShell, viewId );
            }
        }
    }
}

void CallbackFlushHandler::enqueueUpdatedType( COKitCallbackType type, const SfxViewShell* viewShell, int viewId )
{
    if (type == COKitCallbackType::INVALIDATE_VISIBLE_CURSOR)
    {
        if (const SfxViewShell* viewShell2 = KitStarMathHelper(viewShell).GetSmViewShell())
            viewShell = viewShell2;
    }
    std::optional<OString> payload = viewShell->getKitPayload( type, viewId );
    if(!payload)
        return; // No actual payload to send.
    CallbackData callbackData = isUpdatedTypePerViewId(type)
        ? CallbackData(*payload, viewId) : CallbackData(*payload);
    m_queue1.emplace_back(type);
    m_queue2.emplace_back(callbackData);
    SAL_INFO("kit", "Queued updated [" << type << "]: [" << callbackData.getPayload()
        << "] to have " << m_queue1.size() << " entries.");
}

void CallbackFlushHandler::invoke()
{
    comphelper::ProfileZone aZone("CallbackFlushHandler::Invoke");

    if (!m_pCallback)
        return;

    // Get any pending invalidate tile events. This will call our callbacks,
    // so it must be done before taking the mutex.
    assert(m_viewId >= 0);
    if(SfxViewShell* viewShell = SfxViewShell::GetFirst( false,
        [this](const SfxViewShell& shell) { return shell.GetViewShellId().get() == m_viewId; } ))
    {
        viewShell->flushPendingKitInvalidateTiles();
    }

    std::unique_lock<std::recursive_mutex> lock(m_mutex);

    // Append messages for updated types, fetch them only now.
    enqueueUpdatedTypes();

    SAL_INFO("kit", "Flushing " << m_queue1.size() << " elements.");
    auto it1 = m_queue1.begin();
    auto it2 = m_queue2.begin();
    for (; it1 != m_queue1.end(); ++it1, ++it2)
    {
        const COKitCallbackType type = *it1;
        const auto& payload = it2->getPayload();
        const int viewId = lcl_isViewCallbackType(type) ? it2->getViewId() : -1;

        SAL_INFO("kit", "processing event: [" << type << ',' << viewId << "]: [" << payload << "].");

        // common code-path for events on this view:
        if (viewId == -1)
        {
            sal_Int32 idx;
            // key-value pairs
            if (type == COKitCallbackType::STATE_CHANGED &&
                (idx = payload.indexOf('=')) != -1)
            {
                OString key = payload.copy(0, idx);
                OString value = payload.copy(idx+1);
                const auto stateIt = m_lastStateChange.find(key);
                if (stateIt != m_lastStateChange.end())
                {
                    // If the value didn't change, it's safe to ignore; but this one reports a
                    // move to a tracked change, also to the same one again.
                    if (stateIt->second == value && key != ".uno:CurrentTrackedChangeId")
                    {
                        SAL_INFO("kit", "Skipping new state duplicate: [" << type << "]: [" << payload << "].");
                        continue;
                    }
                    SAL_INFO("kit", "Replacing a state element [" << type << "]: [" << payload << "].");
                    stateIt->second = value;
                }
                else
                {
                    SAL_INFO("kit", "Inserted a new state element: [" << type << "]: [" << payload << "]");
                    m_lastStateChange.emplace(key, value);
                }
            }
            else
            {
                const auto stateIt = m_states.find(type);
                if (stateIt != m_states.end())
                {
                    // If the state didn't change, it's safe to ignore.
                    if (stateIt->second == payload)
                    {
                        SAL_INFO("kit", "Skipping duplicate [" << type << "]: [" << payload << "].");
                        continue;
                    }
                    stateIt->second = payload;
                }
            }
        }
        else // less common path for events relating to other views
        {
            const auto statesIt = m_viewStates.find(viewId);
            if (statesIt != m_viewStates.end())
            {
                auto& states = statesIt->second;
                const auto stateIt = states.find(type);
                if (stateIt != states.end())
                {
                    // If the state didn't change, it's safe to ignore.
                    if (stateIt->second == payload)
                    {
                        SAL_INFO("kit", "Skipping view duplicate [" << type << ',' << viewId << "]: [" << payload << "].");
                        continue;
                    }

                    SAL_INFO("kit", "Replacing an element in view states [" << type << ',' << viewId << "]: [" << payload << "].");
                    stateIt->second = payload;
                }
                else
                {
                    SAL_INFO("kit", "Inserted a new element in view states: [" << type << ',' << viewId << "]: [" << payload << "]");
                    states.emplace(type, payload);

                }
            }
        }

        m_pCallback(type, payload.getStr(), m_pData);
    }

    // Every queued message is sent by now, so the queue is emptied and the flush that was scheduled
    // while sending is dropped.
    m_queue1.clear();
    m_queue2.clear();
    stop();

    // The deltas of the changed vector parts follow the queued messages, so the client sees each
    // invalidation before the delta that answers it. A message that computing a delta queues
    // stays in the queue with a flush of its own.
    flushVectorPrimitivesDeltas();
}

void CallbackFlushHandler::scheduleFlush()
{
    if (!m_pFlushEvent)
        m_pFlushEvent = Application::PostUserEvent(LINK(this, CallbackFlushHandler, FlushQueue));
}

IMPL_LINK_NOARG(CallbackFlushHandler, FlushQueue, void*, void)
{
    m_pFlushEvent = nullptr;
    invoke();
}

bool CallbackFlushHandler::removeAll(COKitCallbackType type)
{
    bool bErased = false;
    auto it1 = m_queue1.begin();
    for(;;)
    {
        it1 = std::find(it1, m_queue1.end(), type);
        if(it1 == m_queue1.end())
            break;
        m_queue2.erase(toQueue2(it1));
        it1 = m_queue1.erase(it1);
        bErased = true;
    }
    return bErased;
}

bool CallbackFlushHandler::removeAll(COKitCallbackType type,
                                     const std::function<bool (const CallbackData&)>& rTestFunc)
{
    bool bErased = false;
    auto it1 = m_queue1.begin();
    for(;;)
    {
        it1 = std::find(it1, m_queue1.end(), type);
        if(it1 == m_queue1.end())
            break;
        auto it2 = toQueue2(it1);
        if (rTestFunc(*it2))
        {
            m_queue2.erase(it2);
            it1 = m_queue1.erase(it1);
            bErased = true;
        }
        else
            ++it1;
    }
    return bErased;
}

void CallbackFlushHandler::addViewStates(int viewId)
{
    const auto result = m_viewStates.emplace(viewId, decltype(m_viewStates)::mapped_type());
    if (!result.second && result.first != m_viewStates.end())
    {
        result.first->second.clear();
    }
}

void CallbackFlushHandler::removeViewStates(int viewId)
{
    m_viewStates.erase(viewId);
}

void CallbackFlushHandler::tilePainted(const OString& rPart, int nMode,
                                       const tools::Rectangle& rRectangle)
{
    // Painted a new tile: grow the bbox.
    tools::Rectangle& rPaintedTiles = m_aPaintedTiles[rPart][nMode];
    rPaintedTiles.Union(rRectangle);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
