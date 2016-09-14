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

       This doesn't include currently invoked tasks AKA the stack.

       To dump the scheduler stack of invoked tasks use:
          p *ImplGetSVData()->mpSchedulerStack
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
            res = "{:7s}{:10s} active: {:6s}".format( task_type, str(task['mePriority']), str(task['mbActive']) )
            name = task['mpDebugName']
            if not name:
                res = res + "   (task debug name not set)"
            else:
                res = "{} '{}' ({})".format(res, str(name.string()), str(task.dynamic_type))
            val_type = gdb.lookup_type(str( task.dynamic_type )).pointer()
            timer = gdbobj['mpTask'].cast( val_type )
            if (task_type == "Timer"):
                res = "{}: {}ms".format(res, timer['mnTimeout'])
            else:
                assert 0 == timer['mnTimeout'], "Idle with timeout == {}".format( timer['mnTimeout'] )
            return res
        else:
            assert gdbobj['mbDelete'], "No task set and not marked for deletion!"
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
            if not self.value['mpNext']:
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
