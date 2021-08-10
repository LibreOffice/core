/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Unlike other precompiled_* files, this one is (semi-)manually created.
// It is based on the output of
// (for file in */inc/pch/precompiled_*.hxx; do cat "$file" | grep -A 1000 -F "#if PCH_LEVEL >= 1" | grep -B 1000 -F "#endif // PCH_LEVEL >= 1"; done) | grep "#include" | sort -u
// which is then examined and used here.

#if PCH_LEVEL >= 1
#include <algorithm>
#include <array>
#include <assert.h>
#include <atomic>
#include <boost/algorithm/string.hpp>
#include <boost/cast.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/functional/hash.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/math/special_functions/sinc.hpp>
#include <boost/multi_array.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/operators.hpp>
#include <optional>
#include <boost/property_tree/json_parser.hpp>
#include <boost/rational.hpp>
#include <boost/version.hpp>
#include <cassert>
#include <climits>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <errno.h>
#include <exception>
#include <float.h>
#include <functional>
#include <initializer_list>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <limits.h>
#include <list>
#include <locale>
#include <map>
#include <math.h>
#include <memory>
#include <new>
#include <numeric>
#include <ostream>
#include <queue>
#include <set>
#include <sstream>
#include <stack>
#include <stdarg.h>
#include <stddef.h>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <string_view>
#include <time.h>
#include <typeinfo>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#endif // PCH_LEVEL >= 1

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
