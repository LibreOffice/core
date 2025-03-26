# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import six
import gdb
from libreoffice.util import printing

class ImplSchedulerDataPrinter(object):
    '''Prints the ImplSchedulerData linked list.

       This can be used to dump the current state of the scheduler via:
          p *ImplGetSVData()->maSchedCtx.mpFirstSchedulerData

       This doesn't include currently invoked tasks AKA the stack.

       To dump the scheduler stack of invoked tasks use:
          p *ImplGetSVData()->maSchedCtx.mpSchedulerStack
    '''

    def __init__(self, typename, value):
        self.typename = typename
        self.value = value
        self.timer_type_ptr = gdb.lookup_type("Timer").pointer()
        self.idle_type_ptr = gdb.lookup_type("Idle").pointer()

    def as_string(self, gdbobj):
        if gdbobj['mpTask']:
            task  = gdbobj['mpTask'].dereference()
            timer = gdbobj['mpTask'].dynamic_cast( self.timer_type_ptr )
            idle  = gdbobj['mpTask'].dynamic_cast( self.idle_type_ptr )
            if idle:
                task_type = "Idle"
            elif timer:
                task_type = "Timer"
            else:
                task_type = "Task"
            res = "{:7s}{:10s} active: {:6s}".format( task_type, str(task['mePriority']).replace('TaskPriority::',''), str(task['mbActive']) )
            name = task['mpDebugName']
            if not name:
                res = res + "   (task debug name not set)"
            else:
                res = "{} '{}' ({})".format(res, str(name.string()), str(task.dynamic_type))
            val_type = gdb.lookup_type(str( task.dynamic_type )).pointer()
            timer = gdbobj['mpTask'].cast( val_type )
            if task_type == "Timer":
                res = "{}: {}ms".format(res, timer['mnTimeout'])
            elif task_type == "Idle":
                assert 0 == timer['mnTimeout'], "Idle with timeout == {}".format( timer['mnTimeout'] )
            return res
        else:
            return "(no task)"

    def to_string(self):
        return self.typename

    def children(self):
        return self._iterator(self)

    def display_hint(self):
        return 'array'

    class _iterator(six.Iterator):

        def __init__(self, printer):
            self.pos = 0
            self.printer = printer
            self.value = printer.value

        def __iter__(self):
            return self

        def __next__(self):
            if not self.value:
                raise StopIteration()

            pos = str(self.pos)
            name = "\n  " + self.printer.as_string(self.value)
            self.value = self.value['mpNext']
            self.pos += 1

            return (pos, name)

class ImplSchedulerContextPrinter(object):

    def __init__(self, typename, value):
        self.typename = typename
        self.value = value
        self.prio = gdb.lookup_type('TaskPriority')

    def to_string(self):
        res = "{\n"
        if self.value['mnTimerPeriod']:
            res = res + "mnTimerPeriod = " + str(self.value['mnTimerPeriod']) + "\n"
        if self.value['mpSchedulerStack']:
            res = res + "STACK, " + str(self.value['mpSchedulerStack'].dereference())
        if self.value['mpFirstSchedulerData']:
            for key, value in self.prio.items():
                first = self.value['mpFirstSchedulerData'][value.enumval]
                if first:
                    res = res + key.replace('TaskPriority::', '') + ", " + str(first.dereference())
        return res + "}"

class ImplRegionBandPrinter(object):

    class iterator(six.Iterator):
        '''RegionBand iterator'''

        def __init__(self, first):
            self.pimplband = first
            if self.pimplband:
                self.sep = self.pimplband.dereference()['mpFirstSep']
            else:
                self.sep = None

        def __iter__(self):
            return self

        def __next__(self):
            if not(self.pimplband):
                raise StopIteration
            implband = self.pimplband.dereference()
            top = implband['mnYTop']
            bot = implband['mnYBottom']
            touched = implband['mbTouched']
            if self.sep:
                sep = self.sep.dereference()
                left = sep['mnXLeft']
                right = sep['mnXRight']
                removed = sep['mbRemoved']
                self.sep = self.sep.dereference()['mpNextSep']
                return ('Y [%d,%d] %s' % (top,bot,touched), 'X (%d,%d) %s' % (left,right,removed))
            else:
                self.pimplband = self.pimplband.dereference()['mpNextBand']
                if self.pimplband:
                    self.sep = self.pimplband.dereference()['mpFirstSep']
                return ('Y [%d,%d] %s' % (top,bot,touched), 'END')

    def __init__(self, typename, value):
        self.typename = typename
        self.value = value

    def children(self):
        first = self.value['mpFirstBand']
        return self.iterator(first)

class ImplRegionPrinter(object):

    def __init__(self, typename, value):
        self.typename = typename
        self.value = value

    def children(self):
        b2dpp = self.value['mpB2DPolyPolygon']
        polypoly = self.value['mpPolyPolygon']
        regionband = self.value['mpRegionBand']
        children = [('mbIsNull', self.value['mbIsNull'])]
        if b2dpp:
            # TODO: dereference() doesn't work on std::optional
            children.append(('mpB2DPolyPolygon', b2dpp))
        if polypoly:
            children.append(('mpPolyPolygon', polypoly))
        if regionband['_M_ptr']:
            children.append(('mpRegionBand', regionband['_M_ptr'].dereference()))
        return children.__iter__()


printer = None

def build_pretty_printers():
    global printer

    printer = printing.Printer("libreoffice/vcl")
    printer.add('ImplSchedulerData', ImplSchedulerDataPrinter)
    printer.add('ImplSchedulerContext', ImplSchedulerContextPrinter)
    printer.add('RegionBand', ImplRegionBandPrinter)
    printer.add('vcl::Region', ImplRegionPrinter)

def register_pretty_printers(obj):
    printing.register_pretty_printer(printer, obj)

build_pretty_printers()

# vim:set shiftwidth=4 softtabstop=4 expandtab:
