#!/usr/bin/env python3
# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# Copyright the Collabora Office contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# Call $0 <input .js file> <output .hxx file>

import os
import sys

source, target = sys.argv[1:]
with open(source, 'rb') as f:
    data = f.read()
lines = ['// Generated from ' + os.path.basename(source) + ':', '#pragma once',
         '#include <string_view>', 'namespace detail {',
         'inline constexpr char8_t prelude[] = {', '#if defined __cpp_pp_embed //TODO: C++26',
         '#embed "' + os.path.abspath(source).replace('\\', '/') + '" suffix(,)', '#else']
for i in range(0, len(data), 12):
    lines.append('    ' + ' '.join('0x%02x,' % b for b in data[i:i + 12]))
lines += ['#endif', '    0};', '}',
          'inline constexpr std::u8string_view prelude(detail::prelude, sizeof detail::prelude);']
with open(target, 'w') as f:
    f.write('\n'.join(lines) + '\n')

# vim: set shiftwidth=4 softtabstop=4 expandtab:
