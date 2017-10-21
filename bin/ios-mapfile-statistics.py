#!/usr/bin/env python3
# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import argparse
import re
import sys


def parse_args(argv):
    description = """This program parses a linker map file, especially one produced when linking an iOS executable.
Input is read from a map file provided as command-line argument
By default a list of libraries used and the size of code and data
linked in from each library is printed, in reverse order of size."""

    parser = argparse.ArgumentParser(formatter_class=argparse.RawDescriptionHelpFormatter,
                                     description=description)
    parser.add_argument('-f', dest='filter', metavar='filter',
                        help="Filter which libraries are handled. The filter can be "
                             "a regular expression, typically several library names "
                             "combined with the '|' operator. Makes sense only when "
                             "-s is used too.")
    parser.add_argument('-s', dest='get_symbols', action='store_true',
                        help="Print a list of symbols instead.")
    parser.add_argument('input', help='Map file name')

    args = parser.parse_args(argv)

    if args.filter and not args.s:
        sys.exit('The -f switch makes sense only if -s is also used')

    return args


def get_sizes(filename, filtr, get_symbols):
    with open(filename, 'r') as fh:
        states = {0: re.compile(r'^# Object files:'),
                  1: re.compile(r'^# Sections:'),
                  2: re.compile(r'^# Address\s+Size\s+File\s+Name')}
        lib_num_re = re.compile(r'^\[ *([0-9]+)\] .*/([-_a-z0-9]+\.a)\(.*', re.IGNORECASE)
        size_re = re.compile(r'^0x[0-9A-F]+\s+(0x[0-9A-F]+)\s+\[ *([0-9]+)\] (.*)')

        libs = {}
        sizes = {}
        state = 0

        for line in fh:
            if state == 1:
                match = lib_num_re.match(line)
                if match:
                    libs[match.group(1)] = match.group(2)
            elif state == 3:
                match = size_re.match(line)
                if match:
                    size, libnum, symbol = match.groups()
                    if libnum not in libs:
                        continue

                    lib = libs[libnum]
                    if get_symbols:
                        if not filtr or filtr.search(lib):
                            sizes[symbol] = int(size, 16)
                    else:
                        sizes[lib] += int(size, 16)
                continue

            if state in states and states[state].match(line):
                state += 1

    return sizes


def main():
    args = parse_args(sys.argv[1:])
    filter_re = re.compile(args.filter) if args.filter else None
    sizes = get_sizes(args.input, filter_re, args.get_symbols)

    # Print items in reverse size order
    items = sorted(sizes.keys(), key=lambda x: sizes[x], reverse=True)
    for item in items:
        print('%s: %d' % (item, sizes[item]))


if __name__ == '__main__':
    main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
