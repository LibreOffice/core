# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
# GDB pretty printers for Boost.Pointer Container.
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

std = None

class PtrStdPrinterBase(object):

    def __init__(self, typename, value, seq_tag):
        self._import_std()
        self.typename = typename
        self.value = value

        # (try to) init printer of underlying std sequence and get elements
        printer = self._get_sequence_printer(seq_tag)
        if printer:
            seq = value['c_']
            if str(seq.type.strip_typedefs()).startswith('std::__debug::'):
                seq_typename = 'std::__debug::%s' % seq_tag
            else:
                seq_typename = 'std::%s' % seq_tag
            self.sequence = list(printer(seq_typename, seq).children())
        else:
            self.sequence = None

    def to_string(self):
        if self.sequence is not None:
            length = len(self.sequence)
            if length:
                return "%s %s" % (self.typename, self.print_size(length))
            else:
                return "empty %s" % self.typename
        else:
            return "opaque %s" % self.typename

    def children(self):
        return self._iterator(self.sequence, self.value.type.template_argument(0))

    class _iterator(six.Iterator):

        def __init__(self, sequence, type):
            self.impl = iter(sequence)
            self.type = type.pointer()

        def __iter__(self):
            return self

        def __next__(self):
            (index, value) = six.advance_iterator(self.impl)
            return (index, value.cast(self.type).dereference())

    def _import_std(self):
        global std
        if not std:
            try:
                import libstdcxx.v6.printers
                std = libstdcxx.v6.printers
            except:
                pass

    def _get_sequence_printer(self, typename):
        if typename == "deque":
            return std.StdDequePrinter
        if typename == "list":
            return std.StdListPrinter
        if typename == "map":
            return std.StdMapPrinter
        if typename == "set":
            return std.StdSetPrinter
        if typename == "vector":
            return std.StdVectorPrinter

class PtrSequencePrinter(PtrStdPrinterBase):

    def __init__(self, typename, value, seq_tag):
        super(PtrSequencePrinter, self).__init__(typename, value, seq_tag)

    def print_size(self, size):
        return "of length %s" % size

    def display_hint(self):
        return 'array'

class PtrSetPrinter(PtrStdPrinterBase):

    def __init__(self, typename, value):
        super(PtrSetPrinter, self).__init__(typename, value, 'set')

    def print_size(self, size):
        return "with %s elements" % size

    def display_hint(self):
        return 'array'

class PtrMapPrinter(PtrStdPrinterBase):

    def __init__(self, typename, value):
        super(PtrMapPrinter, self).__init__(typename, value, 'map')

    def children(self):
        type = self.value.type
        return self._iterator(self.sequence, type.template_argument(0), type.template_argument(1))

    class _iterator(six.Iterator):

        def __init__(self, sequence, key_type, value_type):
            self.impl = iter(sequence)
            self.key_type = key_type
            self.value_type = value_type.pointer()
            self.key = True

        def __iter__(self):
            return self

        def __next__(self):
            (index, value) = six.advance_iterator(self.impl)
            if self.key:
                value = value.cast(self.key_type)
            else:
                value = value.cast(self.value_type).dereference()
            self.key = not self.key
            return (index, value)

    def display_hint(self):
        return 'map'

    def print_size(self, size):
        return "with %s elements" % (size / 2)

class PtrBoostPrinterBase(object):

    def __init__(self, typename, value, container, iterator, value_type):
        self.typename = typename
        self.impl = container(value['c_'])
        self.iterator = iterator
        self.value_type = value_type.pointer()

    def to_string(self):
        if self.impl.empty():
            return "empty " + self.typename
        else:
            return "%s with %s elements" % (self.typename, len(self.impl))

    def children(self):
        return self.iterator(iter(self.impl), self.value_type)

class PtrUnorderedMapPrinter(PtrBoostPrinterBase):

    def __init__(self, typename, value):
        super(PtrUnorderedMapPrinter, self).__init__(typename, value, Map, self._iterator,
                value.type.template_argument(1))

    def display_hint(self):
        return 'map'

    class _iterator(six.Iterator):

        def __init__(self, impl, value_type):
            self.impl = impl
            self.step = True
            self.value = None
            self.value_type = value_type

        def __iter__(self):
            return self

        def __next__(self):
            if self.step:
                self.value = six.advance_iterator(self.impl)
                value = self.value[0]
            else:
                value = self.value[1].cast(self.value_type).dereference()
            self.step = not self.step
            return ("", value)

class PtrUnorderedSetPrinter(PtrBoostPrinterBase):

    def __init__(self, typename, value):
        super(PtrUnorderedSetPrinter, self).__init__(typename, value, Set, self._iterator,
                value.type.template_argument(0))

    def display_hint(self):
        return 'array'

    class _iterator(six.Iterator):

        def __init__(self, impl, value_type):
            self.impl = impl
            self.value_type = value_type

        def __iter__(self):
            return self

        def __next__(self):
            return ("", six.advance_iterator(self.impl)[1].cast(self.value_type).dereference())

printer = None

def build_pretty_printers():
    global printer

    if printer is not None:
        return

    printer = printing.Printer("boost.ptr_container")

    printer.add('boost::ptr_deque', (lambda t, v: PtrSequencePrinter(t, v, "deque")))
    printer.add('boost::ptr_list', (lambda t, v: PtrSequencePrinter(t, v, "list")))
    printer.add('boost::ptr_map', PtrMapPrinter)
    printer.add('boost::ptr_multimap', PtrMapPrinter)
    printer.add('boost::ptr_multiset', PtrSetPrinter)
    printer.add('boost::ptr_set', PtrSetPrinter)
    printer.add('boost::ptr_unordered_map', PtrUnorderedMapPrinter)
    printer.add('boost::ptr_unordered_multimap', PtrUnorderedMapPrinter)
    printer.add('boost::ptr_unordered_multiset', PtrUnorderedSetPrinter)
    printer.add('boost::ptr_unordered_set', PtrUnorderedSetPrinter)
    printer.add('boost::ptr_vector', (lambda t, v: PtrSequencePrinter(t, v, "vector")))

def register_pretty_printers(obj):
    printing.register_pretty_printer(printer, obj)

build_pretty_printers()

# vim:set filetype=python shiftwidth=4 softtabstop=4 expandtab:
