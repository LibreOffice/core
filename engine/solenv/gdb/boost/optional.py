# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
# GDB pretty printers for Boost.Optional.
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

import boost.util.printing as printing

class OptionalPrinter:

    def __init__(self, typename, value):
        self.typename = typename
        self.value = value

    def to_string(self):
        if self.value['m_initialized']:
            data = self.value['m_storage']['dummy_']['data']
            ptr_type = self.value.type.template_argument(0).pointer()
            return "%s %s" % (self.typename, data.cast(ptr_type).dereference())
        else:
            return "empty " + self.typename

printer = None

def build_pretty_printers():
    global printer

    if printer is not None:
        return

    printer = printing.Printer("boost.optional")

    printer.add('boost::optional', OptionalPrinter)

def register_pretty_printers(obj):
    printing.register_pretty_printer(printer, obj)

build_pretty_printers()

# vim:set filetype=python shiftwidth=4 softtabstop=4 expandtab:
