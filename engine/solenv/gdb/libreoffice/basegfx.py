# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import gdb
import six

from libreoffice.util import printing

class B2DRangePrinter(object):
    '''Prints a B2DRange object.'''

    def __init__(self, typename, value):
        self.typename = typename
        self.value = value
        # inject children() func dynamically
        if not self._isEmpty():
            self.children = self._children

    def to_string(self):
        if self._isEmpty():
            return "empty %s" % (self.typename)
        else:
            return "%s" % (self.typename)

    def _isEmpty(self):
        return (self.value['maRangeX']['mnMinimum'] > self.value['maRangeX']['mnMaximum']
                or self.value['maRangeY']['mnMinimum'] > self.value['maRangeY']['mnMaximum'])

    def _children(self):
        left = self.value['maRangeX']['mnMinimum']
        top = self.value['maRangeY']['mnMinimum']
        right = self.value['maRangeX']['mnMaximum']
        bottom = self.value['maRangeY']['mnMaximum']
        children = [('left', left), ('top', top), ('right', right), ('bottom', bottom)]
        return children.__iter__()

class B2DPolygonPrinter(object):
    '''Prints a B2DPolygon object.'''

    def __init__(self, typename, value):
        self.typename = typename
        self.value = value
        self.children = self._children

    def to_string(self):
        return self.typename

    def _children(self):
        if self.value['mpPolygon']['m_pimpl'].type.code in (gdb.TYPE_CODE_PTR, gdb.TYPE_CODE_MEMBERPTR):
            try:
                vector = self.value['mpPolygon']['m_pimpl'].dereference()['m_value']['maPoints']['maVector']
                import libstdcxx.v6.printers as std
                return std.StdVectorPrinter("std::vector", vector).children()
            except RuntimeError:
                gdb.write("Cannot access memory at address " + str(self.value['mpPolygon']['m_pimpl'].address))

class B2DPolyPolygonPrinter(object):
    '''Prints a B2DPolyPolygon object.'''

    def __init__(self, typename, value):
        self.typename = typename
        self.value = value
        self.children = self._children

    def to_string(self):
        return self.typename

    def _children(self):
        if self.value['mpPolyPolygon']['m_pimpl'].type.code in (gdb.TYPE_CODE_PTR, gdb.TYPE_CODE_MEMBERPTR):
            if self.value['mpPolyPolygon']['m_pimpl']:
                try:
                    vector = self.value['mpPolyPolygon']['m_pimpl'].dereference()['m_value']['maPolygons']
                    import libstdcxx.v6.printers as std
                    return std.StdVectorPrinter("std::vector", vector).children()
                except RuntimeError:
                    gdb.write("Cannot access memory at address " + str(self.value['mpPolyPolygon']['m_pimpl'].address))

        return None

printer = None

def build_pretty_printers():
    global printer

    printer = printing.Printer('libreoffice/basegfx')

    # basic types
    printer.add('basegfx::B2DRange', B2DRangePrinter)
    printer.add('basegfx::B2DPolygon', B2DPolygonPrinter)
    printer.add('basegfx::B2DPolyPolygon', B2DPolyPolygonPrinter)

def register_pretty_printers(obj):
    printing.register_pretty_printer(printer, obj)

build_pretty_printers()

# vim:set shiftwidth=4 softtabstop=4 expandtab:

