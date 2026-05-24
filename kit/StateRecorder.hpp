/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <common/ContainerUtil.hpp>

/*
 * When the session is inactive, we need to record its state for a replay.
 */

struct RecordedEvent
{
private:
    int _type = 0;
    std::string _payload;

public:
    RecordedEvent() {}

    RecordedEvent(int type, const std::string& payload)
        : _type(type)
        , _payload(payload)
    {
    }

    void setType(int type) { _type = type; }

    int getType() const { return _type; }

    void setPayload(const std::string& payload) { _payload = payload; }

    const std::string& getPayload() const { return _payload; }
};

class StateRecorder
{
private:
    bool _invalidate;
    Util::UnorderedStringMap<std::string> _recordedStates;
    std::unordered_map<int, std::unordered_map<int, RecordedEvent>> _recordedViewEvents;
    std::unordered_map<int, RecordedEvent> _recordedEvents;
    std::vector<RecordedEvent> _recordedEventsVector;

public:
    StateRecorder()
        : _invalidate(false)
    {
    }

    // TODO Remember the maximal area we need to invalidate - grow it step by step.
    void recordInvalidate() { _invalidate = true; }

    bool isInvalidate() const { return _invalidate; }

    const Util::UnorderedStringMap<std::string>& getRecordedStates() const
    {
        return _recordedStates;
    }

    const std::unordered_map<int, std::unordered_map<int, RecordedEvent>>&
    getRecordedViewEvents() const
    {
        return _recordedViewEvents;
    }

    const std::unordered_map<int, RecordedEvent>& getRecordedEvents() const
    {
        return _recordedEvents;
    }

    const std::vector<RecordedEvent>& getRecordedEventsVector() const
    {
        return _recordedEventsVector;
    }

    void recordEvent(const int type, const std::string& payload)
    {
        _recordedEvents[type] = RecordedEvent(type, payload);
    }

    void recordViewEvent(const int viewId, const int type, const std::string& payload)
    {
        _recordedViewEvents[viewId][type] = { type, payload };
    }

    void recordState(const std::string& name, const std::string& value)
    {
        _recordedStates[name] = value;
    }

    /// In the case we need to remember all the events that come, not just
    /// the final state.
    void recordEventSequence(const int type, const std::string& payload)
    {
        _recordedEventsVector.emplace_back(type, payload);
    }

    void dumpState(std::ostream&)
    {
        // TODO: the rest ...
    }

    void clear()
    {
        _invalidate = false;
        _recordedEvents.clear();
        _recordedViewEvents.clear();
        _recordedStates.clear();
        _recordedEventsVector.clear();
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
