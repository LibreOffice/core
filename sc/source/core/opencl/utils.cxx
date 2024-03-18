/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "utils.hxx"

#include <cassert>
#include <cfloat>
#include <limits>
#include <sstream>

namespace sc
{
namespace opencl
{
namespace
{
#ifdef SAL_LOG_INFO
class outputstream_num_put : public std::num_put<char>
{
protected:
    virtual iter_type do_put(iter_type s, std::ios_base&, char_type, double v) const override
    {
        std::string str = preciseFloat(v);
        return std::copy(str.begin(), str.end(), s);
    }
    virtual iter_type do_put(iter_type, std::ios_base&, char_type, long double) const override
    {
        abort(); // we do not use these
    }
    using std::num_put<char>::do_put;
};
#endif
} // namespace

outputstream::outputstream()
{
    precision(DECIMAL_DIG);
    setf(std::ios::showpoint);
#ifdef SAL_LOG_INFO
    // Calling precision() makes the output have trailing insignificant zeroes, which
    // makes reading the source annoying. So override this stream's floating output
    // handling to force usage of our preciseFloat(), which has a saner output.
    imbue(std::locale(std::locale("C"), new outputstream_num_put));
#endif
}

#undef stringstream
std::string preciseFloat(double f)
{
    std::stringstream stream;
    stream.precision(std::numeric_limits<double>::digits10 + 1);
    stream.setf(std::ios::showpoint);
    stream << f;
    std::string str = stream.str();
    size_t end = str.find('e');
    if (end == std::string::npos)
        end = str.size();
    for (size_t pos = end - 1; pos > 0; --pos)
    {
        if (str[pos] != '0')
        {
            if (str[pos] == '.')
            {
                ++pos;
                if (pos == end) // 10. without trailing 0
                {
                    return str + '0';
                }
            }
            ++pos;
            assert(pos <= end);
            str.resize(std::copy(str.begin() + end, str.end(), str.begin() + pos) - str.begin());
            return str;
        }
    }
    abort();
}
}
} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
