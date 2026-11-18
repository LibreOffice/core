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

/*
 * Unit test for fuzzing functionality.
 */

#include <config.h>

#include <iostream>
#include <random>

#include <Common.hpp>
#include <Protocol.hpp>
#include <Unit.hpp>
#include <common/Util.hpp>

#include <Poco/Net/HTTPServerRequest.h>

using namespace std::chrono_literals;

// Inside the WSD process
class UnitFuzz : public UnitWSD
{
    std::random_device _rd;
    std::mt19937 _mt;
    std::uniform_int_distribution<> _dist;
public:
    UnitFuzz() :
        UnitWSD("fuzz"),
        _mt(_rd()),
        _dist(0, 1000)
    {
        std::cerr << "\n\nYour WSD process is being randomly fuzzed\n\n\n";
        setHasKitHooks();
        setTimeout(1h);
    }

    std::string corruptString(const std::string &str)
    {
        std::string ret;
        for (auto it = str.begin(); it != str.end(); ++it)
        {
            int op = _dist(_mt);
            if (op < 10) {
                switch (op) {
                    case 0:
                        ret += 0xff;
                        break;
                case 1:
                case 3:
                    ret += *it & 0x80;
                    break;
                default:
                    ret += *it ^ _dist(_mt);
                    break;
                }
            }
        }
        return ret;
    }

    /*
     * Note: Fuzzers are fragile and their performance is rather
     * sensitive. Please avoid random code tweaking in this method.
     */
    virtual bool filterSessionInput(Session *, const char *buffer,
                                    int length,
                                    std::unique_ptr< std::vector<char> > &replace) override
    {
        // Avoid fuzzing most messages
        if (_dist(_mt) < 875)
            return false;

        std::unique_ptr<std::vector<char>> fuzzed(new std::vector<char>());
        fuzzed->assign(buffer, buffer+length);

        int resize = _dist(_mt);
        if (resize < 50) { // truncate
            size_t shrink = (fuzzed->size() * _dist(_mt))/1000;
            fuzzed->resize(shrink);

        } else if (resize < 200) {
            bool prepend = resize < 100;
            bool middle = resize < 150;
            size_t count = 1 + _dist(_mt)/100;
            for (size_t i = 0; i < count; ++i)
            {
                char c = (_dist(_mt) * 256 / 1000);
                if (prepend)
                    fuzzed->insert(fuzzed->begin(), c);
                else if (middle)
                    fuzzed->insert(fuzzed->begin() + fuzzed->size()/2, c);
                else
                    fuzzed->push_back(c);
            }
        }

        int numCorrupt = (_dist(_mt) / 100) - 75;
        for (int i = 0; i < numCorrupt; ++i)
        {
            size_t offset = (_dist(_mt) * fuzzed->size() - 1) / 1000;
            char c = (*fuzzed)[offset];
            int change = _dist(_mt);
            if (change < 256)
                c ^= change;
            else if (c >= '0' && c <= '9')
                c = '0' + (change - 256)/100;
            else
                c |= 0x80;
        }

        replace = std::move(fuzzed);

        return true;
    }
};

// Inside the forkit & kit processes
class UnitKitFuzz : public UnitKit
{
public:
    UnitKitFuzz() : UnitKit("fuzz")
    {
        std::cerr << "\n\nYour Kit process has fuzzing hooks\n\n\n";
        setTimeout(1h);
    }
    virtual bool filterKitMessage(WebSocketHandler *, std::string & /* message */) override
    {
        return false;
    }
};

UnitBase *unit_create_wsd(void)
{
    return new UnitFuzz();
}

UnitBase *unit_create_kit(void)
{
    return new UnitKitFuzz();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
