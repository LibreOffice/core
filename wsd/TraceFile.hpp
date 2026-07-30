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

/*
 * Trace file recording for debugging and analysis.
 * Classes: TraceFileWriter, TraceFileReader
 */

#pragma once

#include <common/FileUtil.hpp>
#include <common/Log.hpp>
#include <common/Protocol.hpp>
#include <common/RegexUtil.hpp>
#include <common/StringVector.hpp>
#include <common/Uri.hpp>
#include <common/Util.hpp>

#include <Poco/DateTime.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/DeflatingStream.h>
#include <Poco/InflatingStream.h>
#include <Poco/URI.h>

#include <chrono>
#include <fstream>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

/// Dumps commands and notification trace.
class TraceFileRecord
{
public:
    enum class Direction : char
    {
        Invalid = 0,
        Incoming = '>',
        Outgoing = '<',
        Event = '~'
    };

    TraceFileRecord()
        : _dir(Direction::Invalid)
        , _timestampUs(std::chrono::microseconds::zero())
        , _pid(0)
    {
    }

    std::string toString() const
    {
        if (_dir == Direction::Invalid)
        {
            return "Invalid TraceFileRecord";
        }
        else
        {
            std::ostringstream oss;
            oss << static_cast<char>(_dir) << _pid << static_cast<char>(_dir)
                << _sessionId << static_cast<char>(_dir) << _payload;
            return oss.str();
        }
    }

    void setDir(Direction dir) { _dir = dir; }

    Direction getDir() const { return _dir; }

    void setTimestampUs(std::chrono::microseconds timestampUs) { _timestampUs = timestampUs; }

    std::chrono::microseconds getTimestampUs() const { return _timestampUs; }

    void setPid(unsigned pid) { _pid = pid; }

    unsigned getPid() const { return _pid; }

    void setSessionId(const std::string& sessionId) { _sessionId = sessionId; }

    const std::string& getSessionId() const { return _sessionId; }

    void setPayload(const std::string& payload) { _payload = payload; }

    const std::string& getPayload() const { return _payload; }

private:
    Direction _dir;
    std::chrono::microseconds _timestampUs;
    unsigned _pid;
    std::string _sessionId;
    std::string _payload;
};

/// Trace-file generator class.
/// Writes records into a trace file.
class TraceFileWriter
{
public:
    TraceFileWriter(const std::string& path, const bool recordOutgoing, const bool compress,
                    const bool takeSnapshot, const std::vector<std::string>& filters)
        : _stream(processPath(path), compress ? std::ios::binary : std::ios::out)
        , _deflater(_stream, Poco::DeflatingStreamBuf::STREAM_GZIP)
        , _filter(true)
        , _path(Poco::Path(path).parent().toString())
        , _epochStart(std::chrono::duration_cast<std::chrono::microseconds>(
              std::chrono::system_clock::now().time_since_epoch()))
        , _lastTime(_epochStart)
        , _recordOutgoing(recordOutgoing)
        , _compress(compress)
        , _takeSnapshot(takeSnapshot)
    {
        for (const auto& f : filters)
        {
            _filter.deny(f);
        }
    }

    ~TraceFileWriter()
    {
        std::unique_lock<std::mutex> lock(_mutex);

        _deflater.close();
        _stream.close();
    }

    void newSession(const std::string& id, const std::string& sessionId, const std::string& uri, const std::string& localPath)
    {
        std::unique_lock<std::mutex> lock(_mutex);

        std::string snapshot = uri;

        if (_takeSnapshot)
        {
            const std::string url = Poco::URI(Uri::decode(uri)).getPath();
            const auto it = _urlToSnapshot.find(url);
            if (it != _urlToSnapshot.end())
            {
                snapshot = it->second.getSnapshot();
                it->second.getSessionCount()++;
            }
            else
            {
                // Create a snapshot file.
                const Poco::Path origPath(localPath);
                std::string filename = origPath.getBaseName();
                filename += '_' + Poco::DateTimeFormatter::format(Poco::DateTime(), "%Y%m%d_%H-%M-%S");
                filename += '.' + origPath.getExtension();
                snapshot = Poco::Path(_path, filename).toString();

                FileUtil::copyFileTo(localPath, snapshot);
                snapshot = Poco::URI(Poco::URI("file://"), snapshot).toString();

                LOG_TRC("TraceFile: Mapped URL " << url << " to " << snapshot);
                _urlToSnapshot.emplace(url, SnapshotData(snapshot));
            }
        }

        const auto data = "NewSession: " + snapshot;
        writeLocked(id, sessionId, data, static_cast<char>(TraceFileRecord::Direction::Event));
        flushLocked();
    }

    void endSession(const std::string& id, const std::string& sessionId, const std::string& uri)
    {
        std::unique_lock<std::mutex> lock(_mutex);

        std::string snapshot = uri;

        const std::string url = Poco::URI(uri).getPath();
        const auto it = _urlToSnapshot.find(url);
        if (it != _urlToSnapshot.end())
        {
            snapshot = it->second.getSnapshot();
            if (it->second.getSessionCount() == 1)
            {
                // Last session, remove the mapping.
                _urlToSnapshot.erase(it);
            }
            else
            {
                it->second.getSessionCount()--;
            }
        }

        const auto data = "EndSession: " + snapshot;
        writeLocked(id, sessionId, data, static_cast<char>(TraceFileRecord::Direction::Event));
        flushLocked();
    }

    void writeEvent(const std::string& id, const std::string& sessionId, const std::string& data)
    {
        std::unique_lock<std::mutex> lock(_mutex);

        writeLocked(id, sessionId, data, static_cast<char>(TraceFileRecord::Direction::Event));
        flushLocked();
    }

    void writeIncoming(const std::string& id, const std::string& sessionId, const std::string& data)
    {
        std::unique_lock<std::mutex> lock(_mutex);

        if (_filter.match(data))
        {
            // Remap the URL to the snapshot.
            if (COOLProtocol::matchPrefix("load", data))
            {
                StringVector tokens = StringVector::tokenize(data);
                if (tokens.size() >= 2)
                {
                    std::string url;
                    if (COOLProtocol::getTokenString(tokens[1], "url", url))
                    {
                        Poco::URI uriPublic(Uri::decode(url));
                        if (uriPublic.isRelative() || uriPublic.getScheme() == "file")
                        {
                            uriPublic.normalize();
                        }

                        url = uriPublic.getPath();
                        const auto it = _urlToSnapshot.find(url);
                        if (it != _urlToSnapshot.end())
                        {
                            LOG_TRC("TraceFile: Mapped URL: " << url << " to " << it->second.getSnapshot());
                            tokens[1] = "url=" + it->second.getSnapshot();
                            std::string newData;
                            for (const auto& token : tokens)
                            {
                                newData += tokens.getParam(token) + ' ';
                            }

                            writeLocked(id, sessionId, newData, static_cast<char>(TraceFileRecord::Direction::Incoming));
                            return;
                        }
                    }
                }
            }

            if (!COOLProtocol::matchPrefix("tileprocessed ", data))
                writeLocked(id, sessionId, data, static_cast<char>(TraceFileRecord::Direction::Incoming));
        }
    }

    void writeOutgoing(const std::string& id, const std::string& sessionId, const std::string& data)
    {
        std::unique_lock<std::mutex> lock(_mutex);

        if (_recordOutgoing && _filter.match(data))
        {
            writeLocked(id, sessionId, data, static_cast<char>(TraceFileRecord::Direction::Outgoing));
        }
    }

private:
    void flushLocked()
    {
        Util::assertIsLocked(_mutex);

        _deflater.flush();
        _stream.flush();
    }

    void writeLocked(const std::string& id, const std::string& sessionId, const std::string& data, const char delim)
    {
        Util::assertIsLocked(_mutex);

        const std::chrono::microseconds usec =
            std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::system_clock::now().time_since_epoch());
        const std::chrono::microseconds deltaT = usec - _lastTime;
        _lastTime = usec;
        if (_compress)
        {
            _deflater.write(&delim, 1);
            _deflater << '+' << deltaT;
            _deflater.write(&delim, 1);
            _deflater << id;
            _deflater.write(&delim, 1);
            _deflater << sessionId;
            _deflater.write(&delim, 1);
            _deflater.write(data.c_str(), data.size());
            _deflater.write("\n", 1);
        }
        else
        {
            _stream.write(&delim, 1);
            _stream << '+' << deltaT;
            _stream.write(&delim, 1);
            _stream << id;
            _stream.write(&delim, 1);
            _stream << sessionId;
            _stream.write(&delim, 1);
            _stream.write(data.c_str(), data.size());
            _stream.write("\n", 1);
        }
    }

    static std::string processPath(const std::string& path)
    {
        const size_t pos = path.find('%');
        if (pos == std::string::npos)
        {
            return path;
        }

        std::string res = path.substr(0, pos);
        res += Poco::DateTimeFormatter::format(Poco::DateTime(), "%Y%m%d_%H-%M-%S");
        res += path.substr(pos + 1);
        LOG_INF("Command trace dumping enabled to file: " << res);
        return res;
    }

private:
    struct SnapshotData
    {
        SnapshotData(const std::string& snapshot) :
            _snapshot(snapshot)
        {
            _sessionCount = 1;
        }

        SnapshotData(const SnapshotData& other) :
            _snapshot(other.getSnapshot())
        {
            _sessionCount = other.getSessionCount().load();
        }

        const std::string& getSnapshot() const { return _snapshot; }

        std::atomic<size_t>& getSessionCount() { return _sessionCount; }

        const std::atomic<size_t>& getSessionCount() const { return _sessionCount; }

    private:
        std::string _snapshot;
        std::atomic<size_t> _sessionCount;
    };

private:
    std::ofstream _stream;
    Poco::DeflatingOutputStream _deflater;
    RegexUtil::RegexListMatcher _filter;
    std::map<std::string, SnapshotData> _urlToSnapshot;
    std::mutex _mutex;
    const std::string _path;
    const std::chrono::microseconds _epochStart;
    std::chrono::microseconds _lastTime;
    const bool _recordOutgoing;
    const bool _compress;
    const bool _takeSnapshot;
};

/// Trace-file parser class.
/// Reads records from a trace file.
class TraceFileReader final
{
public:
    TraceFileReader(const std::string& path, float latencyFactor = 1)
        : TraceFileReader(path, (path.size() > 2 && path.substr(path.size() - 2) == "gz"),
                          latencyFactor)
    {
    }

    ~TraceFileReader()
    {
        _stream.close();
    }

    std::chrono::microseconds getEpochStart() const { return _epochStart; }
    std::chrono::microseconds getEpochEnd() const { return _epochEnd; }

    TraceFileRecord getNextRecord()
    {
        if (_index < _records.size())
        {
            return _records[_index++];
        }

        // Invalid.
        return TraceFileRecord();
    }

    TraceFileRecord getNextRecord(const TraceFileRecord::Direction dir)
    {
        if (dir == TraceFileRecord::Direction::Incoming)
        {
            if (_indexIn < _records.size())
            {
                TraceFileRecord rec = _records[_indexIn];
                _indexIn = advance(_indexIn, dir);
                return rec;
            }
        }
        else
        {
            if (_indexOut < _records.size())
            {
                TraceFileRecord rec = _records[_indexOut];
                _indexOut = advance(_indexOut, dir);
                return rec;
            }
        }

        // Invalid.
        return TraceFileRecord();
    }

private:
    TraceFileReader(const std::string& path, bool compressed, float latencyFactor)
        : _stream(path, compressed ? std::ios::binary : std::ios::in)
        , _inflater(_stream, Poco::InflatingStreamBuf::STREAM_GZIP)
        , _epochStart(std::chrono::microseconds::zero())
        , _epochEnd(std::chrono::microseconds::zero())
        , _index(0)
        , _indexIn(-1)
        , _indexOut(-1)
        , _latencyFactor(latencyFactor)
        , _compressed(compressed)
    {
        readFile();
    }

    void readFile()
    {
        _records.clear();

        std::string line;
        std::chrono::microseconds lastTime = std::chrono::microseconds::zero();
        for (;;)
        {
            if (_compressed)
            {
                std::getline(_inflater, line);
            }
            else
            {
                std::getline(_stream, line);
            }

            if (line.empty())
            {
                break;
            }

            TraceFileRecord rec;
            if (extractRecord(line, lastTime, rec, _latencyFactor))
                _records.push_back(std::move(rec));
            else
                fprintf(stderr, "Invalid trace file record, expected 4 tokens. [%s]\n", line.c_str());
        }

        if (_records.empty() ||
            _records[0].getDir() != TraceFileRecord::Direction::Event ||
            _records[0].getPayload().find("NewSession") != 0)
        {
            fprintf(stderr, "Invalid trace file with %ld records. First record: %s\n", static_cast<long>(_records.size()),
                    _records.empty() ? "<empty>" : _records[0].getPayload().c_str());
            throw std::runtime_error("Invalid trace file.");
        }

        _indexIn = advance(-1, TraceFileRecord::Direction::Incoming);
        _indexOut = advance(-1, TraceFileRecord::Direction::Outgoing);

        _epochStart = _records[0].getTimestampUs();
        _epochEnd = _records[_records.size() - 1].getTimestampUs();
    }

    static bool extractRecord(const std::string& s, std::chrono::microseconds& lastTime,
                              TraceFileRecord& rec, float latencyFactor)
    {
        if (s.length() < 1)
            return false;

        char delimiter = s[0];
        rec.setDir(static_cast<TraceFileRecord::Direction>(delimiter));

        size_t pos = 1;

        for (int record = 0; record < 4 && pos < s.length(); ++record)
        {
            size_t next = s.find(delimiter, pos);

            switch (record)
            {
                case 0:
                    if (s[pos] == '+') { // incremental timestamps
                        auto time =
                            std::chrono::microseconds(std::atol(s.substr(pos, next - pos).c_str()));
                        time = std::chrono::duration_cast<std::chrono::microseconds>(time *
                                                                                     latencyFactor);
                        rec.setTimestampUs(lastTime + time);
                        lastTime += time;
                    }
                    else
                        rec.setTimestampUs(std::chrono::microseconds(
                            std::atol(s.substr(pos, next - pos).c_str())));
                    break;
                case 1:
                    rec.setPid(std::atoi(s.substr(pos, next - pos).c_str()));
                    break;
                case 2:
                    rec.setSessionId(s.substr(pos, next - pos));
                    break;
                case 3:
                    rec.setPayload(s.substr(pos));
                    return true;
            }

            if (next == std::string::npos)
                break;

            pos = next + 1;
        }

        return false;
    }

    unsigned advance(unsigned index, const TraceFileRecord::Direction dir)
    {
        while (++index < _records.size())
        {
            if (_records[index].getDir() == dir)
            {
                break;
            }
        }

        return index;
    }

private:
    std::ifstream _stream;
    Poco::InflatingInputStream _inflater;
    std::vector<TraceFileRecord> _records;
    std::chrono::microseconds _epochStart;
    std::chrono::microseconds _epochEnd;
    unsigned _index;
    unsigned _indexIn;
    unsigned _indexOut;
    float _latencyFactor;
    const bool _compressed;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
