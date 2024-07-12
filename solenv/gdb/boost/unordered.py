# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
# GDB pretty printers for Boost.Unordered.
#
# Copyright (C) 2012 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#
# This file is part of boost-gdb-printers.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.


import gdb
import six

from boost.lib.unordered import Map, Set

import boost.util.printing as printing

class PrinterBase(object):
    '''Contains common functionality for printing Boost.Unordered types'''

    def __init__(self, typename, value, container, iterator):
        self.typename = typename
        self.impl = container(value)
        self.iterator = iterator

    def to_string(self):
        if self.impl.empty():
            return "empty " + self.typename
        else:
            return "%s with %s elements" % (self.typename, len(self.impl))

    def children(self):
        return self.iterator(iter(self.impl))

class UnorderedMapPrinter(PrinterBase):

    def __init__(self, typename, value):
        super(UnorderedMapPrinter, self).__init__(typename, value, Map, self._iterator)

    def display_hint(self):
        return 'map'

    class _iterator(six.Iterator):

        def __init__(self, impl):
            self.impl = impl
            self.value = None
            self.step = True

        def __iter__(self):
            return self

        def __next__(self):
            if self.step:
                self.value = six.advance_iterator(self.impl)
                value = self.value[0]
            else:
                value = self.value[1]
            self.step = not self.step
            return ("", value)

class UnorderedSetPrinter(PrinterBase):

    def __init__(self, typename, value):
        super(UnorderedSetPrinter, self).__init__(typename, value, Set, self._iterator)

    def display_hint(self):
        return 'array'

    class _iterator(six.Iterator):

        def __init__(self, impl):
            self.impl = impl

        def __iter__(self):
            return self

        def __next__(self):
            return ("", six.advance_iterator(self.impl)[1])

printer = None

def build_pretty_printers():
    global printer

    if printer is not None:
        return

    printer = printing.Printer("boost.unordered")

    printer.add('boost::unordered_map', UnorderedMapPrinter)
    printer.add('boost::unordered_multimap', UnorderedMapPrinter)
    printer.add('boost::unordered_multiset', UnorderedSetPrinter)
    printer.add('boost::unordered_set', UnorderedSetPrinter)

def register_pretty_printers(obj):
    printing.register_pretty_printer(printer, obj)

build_pretty_printers()

# vim:set filetype=python shiftwidth=4 softtabstop=4 expandtab:
