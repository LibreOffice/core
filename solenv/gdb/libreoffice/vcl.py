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
          p *ImplGetSVData()->mpFirstSchedulerData
    '''

    def __init__(self, typename, value):
        self.typename = typename
        self.value = value
        self.timer_type_ptr = gdb.lookup_type("Timer").pointer()
        self.idle_type_ptr = gdb.lookup_type("Idle").pointer()

    def as_string(self, gdbobj):
        if gdbobj['mpScheduler']:
            sched = gdbobj['mpScheduler'].dereference()
            if gdbobj['mpScheduler'].dynamic_cast( self.timer_type_ptr ):
                sched_type = "Timer"
            elif gdbobj['mpScheduler'].dynamic_cast( self.idle_type_ptr ):
                sched_type = "Idle"
            else:
                assert sched_type, "Scheduler object neither Timer nor Idle"
            res = "{:7s}{:10s}".format( sched_type, str(sched['mePriority']) )
            name = sched['mpDebugName']
            if not name:
                res = "{}   (scheduler debug name not set) ({})".format(res, str(sched.dynamic_type))
            else:
                res = "{} '{}' ({})".format(res, str(name.string()), str(sched.dynamic_type))
            return res
        else:
            return "(no scheduler - to be deleted)"

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

printer = None

def build_pretty_printers():
    global printer

    printer = printing.Printer("libreoffice/vcl")
    printer.add('ImplSchedulerData', ImplSchedulerDataPrinter)

def register_pretty_printers(obj):
    printing.register_pretty_printer(printer, obj)

build_pretty_printers()

# vim:set shiftwidth=4 softtabstop=4 expandtab:
