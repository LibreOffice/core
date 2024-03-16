# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
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
        # inject children() func dynamically
        if not self._isEmpty():
            self.children = self._children

    def to_string(self):
        if self._isEmpty():
            return "empty %s" % (self.typename)
        else:
            return "%s %s" % ('bezier curve' if self._hasCurves() else 'straight line',
                              self.typename)

    def _count(self):
        # It's a call into the inferior (being debugged) process.
        # Will not work with core dumps and can cause a deadlock.
        return int(gdb.parse_and_eval(
                "(('basegfx::B2DPolygon' *) {})->count()".format(self.value.address)))

    def _isEmpty(self):
        return self._count() == 0

    def _hasCurves(self):
        # It's a call into the inferior (being debugged) process.
        # Will not work with core dumps and can cause a deadlock.
        return int(gdb.parse_and_eval(
                "(('basegfx::B2DPolygon' *) {})->areControlPointsUsed()".format(self.value.address))) != 0

    def _children(self):
        if self._hasCurves():
            return self._bezierIterator(self._count(), self.value)
        else:
            return self._plainIterator(self._count(), self.value)

    class _plainIterator(six.Iterator):
        def __init__(self, count, value):
            self.count = count
            self.value = value
            self.index = 0

        def __iter__(self):
            return self

        def __next__(self):
            if self.index >= self.count:
                raise StopIteration()
            points = self.value['mpPolygon']['m_pimpl'].dereference()['m_value']['maPoints']['maVector']
            currPoint = (points['_M_impl']['_M_start'] + self.index).dereference()
            # doesn't work?
            #currPoint = gdb.parse_and_eval(
            #        '((basegfx::B2DPolygon*)%d)->getB2DPoint(%d)' % (
            #          self.value.address, self.index))
            self.index += 1
            return ('point %d' % (self.index-1),
                    '(%15f, %15f)' % (currPoint['mnX'], currPoint['mnY']))

    class _bezierIterator(six.Iterator):
        def __init__(self, count, value):
            self.count = count
            self.value = value
            self.index = 0

        def __iter__(self):
            return self

        def __next__(self):
            if self.index >= self.count:
                raise StopIteration()
            points = self.value['mpPolygon']['m_pimpl'].dereference()['m_value']['maPoints']['maVector']
            currPoint = (points['_M_impl']['_M_start'] + self.index).dereference()
            #currPoint = gdb.parse_and_eval(
            #        '((basegfx::B2DPolygon*)%d)->getB2DPoint(%d)' % (
            #          self.value.address, self.index))

            # It's a call into the inferior (being debugged) process.
            # Will not work with core dumps and can cause a deadlock.
            prevControl = gdb.parse_and_eval(
                    "(('basegfx::B2DPolygon' *) {})->getPrevControlPoint({:d})".format(self.value.address, self.index))
            # It's a call into the inferior (being debugged) process.
            # Will not work with core dumps and can cause a deadlock.
            nextControl = gdb.parse_and_eval(
                    "(('basegfx::B2DPolygon' *) {})->getNextControlPoint({:d})".format(self.value.address, self.index))
            self.index += 1
            return ('point %d' % (self.index-1),
                    'p: (%15f, %15f) c-1: (%15f, %15f) c1: (%15f, %15f)' %
                    (currPoint['mnX'],   currPoint['mnY'],
                     prevControl['mnX'], prevControl['mnY'],
                     nextControl['mnX'], nextControl['mnY']))

class B2DPolyPolygonPrinter(object):
    '''Prints a B2DPolyPolygon object.'''

    def __init__(self, typename, value):
        self.typename = typename
        self.value = value

    def to_string(self):
        if self._isEmpty():
            return "empty %s" % (self.typename)
        else:
            return "%s %s with %d sub-polygon(s)" % ('closed' if self._isClosed() else 'open',
                                                     self.typename,
                                                     self._count())

    def _count(self):
        # It's a call into the inferior (being debugged) process.
        # Will not work with core dumps and can cause a deadlock.
        return int(gdb.parse_and_eval(
                "(('basegfx::B2DPolyPolygon' *) {})->count()".format(self.value.address)))

    def _isClosed(self):
        # It's a call into the inferior (being debugged) process.
        # Will not work with core dumps and can cause a deadlock.
        return int(gdb.parse_and_eval(
                "(('basegfx::B2DPolyPolygon' *) {})->isClosed()".format(self.value.address))) != 0

    def _isEmpty(self):
        return self._count() == 0

    def children(self):
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

