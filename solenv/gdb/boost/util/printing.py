# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
# Printer interface adaptor.
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


from collections.abc import Mapping
import gdb
import re
import six

from boost.util.compatibility import use_gdb_printing

class SimplePrinter(object):

    def __init__(self, name, function):
        self.name = name
        self.function = function
        self.enabled = True

    def invoke(self, val):
        if not self.enabled:
            return None
        return self.function(self.name, val)

class NameLookup(Mapping):

    def __init__(self):
        self.map = {}
        self.name_regex = re.compile(r'^([\w:]+)(<.*>)?')

    def add(self, name, printer):
        self.map[name] = printer

    def __len__(self):
        return len(self.map)

    def __getitem__(self, type):
        typename = self._basic_type(type)
        if typename and typename in self.map:
            return self.map[typename]
        return None

    def __iter__(self):
        return self.map

    def _basic_type(self, type):
        basic_type = self.basic_type(type)
        if basic_type:
            match = self.name_regex.match(basic_type)
            if match:
                return match.group(1)
        return None

    @staticmethod
    def basic_type(type):
        if type.code == gdb.TYPE_CODE_REF:
            type = type.target()
        type = type.unqualified().strip_typedefs()
        return type.tag

class FunctionLookup(Mapping):

    def __init__(self):
        self.map = {}

    def add(self, test, printer):
        self.map[test] = printer

    def __len__(self):
        return len(self.map)

    def __getitem__(self, type):
        for (test, printer) in six.iteritems(self.map):
            if test(type):
                return printer
        return None

    def __iter__(self):
        return self.map

class Printer(object):

    def __init__(self, name):
        self.name = name
        self.subprinters = []
        self.name_lookup = NameLookup()
        self.func_lookup = FunctionLookup()
        self.enabled = True

    def add(self, name, function, lookup = None):
        printer = SimplePrinter(name, function)
        self.subprinters.append(printer)
        if not lookup:
            self.name_lookup.add(name, printer)
        else:
            self.func_lookup.add(lookup, printer)


    def __call__(self, val):
        printer = self.name_lookup[val.type]
        if not printer:
            printer = self.func_lookup[val.type]

        if printer:
            return printer.invoke(val)
        return None

def register_pretty_printer(printer, obj):
    '''Registers printer with objfile'''

    if use_gdb_printing:
        gdb.printing.register_pretty_printer(obj, printer)
    else:
        if obj is None:
            obj = gdb
        obj.pretty_printers.append(printer)

# vim:set filetype=python shiftwidth=4 softtabstop=4 expandtab:
