# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original Code is
#       David Tardon, Red Hat Inc. <dtardon@redhat.com>
# Portions created by the Initial Developer are Copyright (C) 2010 the
# Initial Developer. All Rights Reserved.
#
# Major Contributor(s):
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

from collections import Mapping
import gdb
import re

from libreoffice.util.compatibility import use_gdb_printing

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
        self.name_regex = re.compile('^([\w:]+)(<.*>)?')

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
        for (test, printer) in self.map.iteritems():
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

# vim:set shiftwidth=4 softtabstop=4 expandtab:
