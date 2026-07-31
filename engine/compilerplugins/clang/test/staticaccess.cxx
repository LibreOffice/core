/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <chrono>
#include <sstream>
#include <string>

// A static member of a std type reached through an id-expression is an accepted idiom and stays
// unflagged. A non-std type, or a std member reached through anything other than an id-expression,
// still has to be qualified.

struct WithStatic
{
    static int number;
};

struct WithStdMember
{
    std::chrono::milliseconds interval;

    // The member named without an explicit this-> is an id-expression, so this is allowed:
    bool implicitThis() const { return interval > interval.zero(); }

    bool explicitThis() const
    {
        // expected-error@+1 {{accessing static class member through class member access syntax, use a qualified name}}
        return interval > this->interval.zero();
    }
};

std::string const& getString();

void test(std::string uri, std::chrono::milliseconds interval, std::istringstream& stream,
          WithStatic withStatic)
{
    (void)(uri.find('/') != uri.npos);
    (void)(interval > interval.zero());
    stream.seekg(0, stream.beg);

    // expected-error@+1 {{accessing static class member through class member access syntax, use a qualified name}}
    (void)withStatic.number;

    // expected-error@+1 {{accessing static class member through class member access syntax, use a qualified name}}
    (void)(getString().npos != 0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
