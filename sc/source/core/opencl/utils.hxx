/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sstream>

namespace sc::opencl
{
// The way ostream handles floating point output is pretty broken for our usecase.
// The default precision (6) is low, leading to possible precision loss of constant
// float arguments in the generated code. Moreover numbers without a fractional
// part will be printed as e.g. '2', making them integers in the generated code
// (causing problems e.g. with floor() overloads being ambiguous). The std::showpoint
// manipulator forces a decimal point, but then all numbers will be printed
// at the maximum precision even with insignificant trailing 0's, making the numbers
// a pain to read.
// And these flags need to be set on every std::stringstream instance we use,
// which may be easy to omit.
// So as a solution to this:
// - provide our own wrapper class
// - prohibit direct usage of std::stringstream

class outputstream : public std::stringstream
{
private:
    typedef std::stringstream base;

public:
    outputstream();
};

// Returns a string containing the string representation of the given floating pointer number.
// Unlike printf/iostream, this tries to be as precise and representative as possible
// (it always includes a decimal point, and it uses the highest precision necessary/possible).
std::string preciseFloat(double f);
std::string preciseFloat(long double f) = delete; // we do not use these

#define stringstream do_not_use_std_stringstream
} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
