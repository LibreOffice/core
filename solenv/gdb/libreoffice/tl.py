# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import gdb

from libreoffice.util import printing

class BigIntPrinter(object):
    '''Prints big integer'''

    def __init__(self, typename, val):
        self.val = val

    def to_string(self):
        if self.val['bIsSet']:
            if self.val['bIsBig']:
                return self._value()
            else:
                return self.val['nVal']
        else:
            return "unset %s" % self.typename

    def _value(self):
        len = self.val['nLen']
        digits = self.val['nNum']
        dsize = digits.dereference().type.sizeof * 8
        num = 0
        # The least significant byte is on index 0
        for i in reversed(range(0, len)):
            num <<= dsize
            num += digits[i]
        return num

class ColorPrinter(object):
    '''Prints color as rgb(r, g, b) or rgba(r, g, b, a)'''

    def __init__(self, typename, val):
        self.val = val

    def to_string(self):
        r = self.val['R']
        g = self.val['G']
        b = self.val['B']
        t = self.val['T']
        if t:
            return "rgba(%d, %d, %d, %d)" % (r, g, b, 255 - t)
        else:
            return "rgb(%d, %d, %d)" % (r, g, b)

class DateTimeImpl(object):

    def __init__(self, date, time):
        self.date = date
        self.time = time

    def __str__(self):
        result = ''
        if self.date:
            result += str(self.date)
            if self.time:
                result += ' '
        if self.time:
            result += str(self.time)
        return result

    @staticmethod
    def parse(val):
        return DateTimeImpl(DateImpl.parse(val), TimeImpl.parse(val))

class DateTimePrinter(object):
    '''Prints date and time'''

    def __init__(self, typename, val):
        self.val = val

    def to_string(self):
        return str(DateTimeImpl.parse(self.val))

class DateImpl(DateTimeImpl):

    def __init__(self, year, month, day):
        super(DateImpl, self).__init__(self, None)
        self.year = year
        self.month = month
        self.day = day

    def __str__(self):
        return "%d-%d-%d" % (self.year, self.month, self.day)

    @staticmethod
    def parse(val):
        date = val['mnDate']
        y = date / 10000
        if date < 0:
            date = -date
        m = (date / 100) % 100
        d = date % 100
        return DateImpl(y, m, d)

class DatePrinter(object):
    '''Prints date'''

    def __init__(self, typename, val):
        self.val = val

    def to_string(self):
        return str(DateImpl.parse(self.val))

class TimeImpl(DateTimeImpl):

    def __init__(self, hour, minute, second, nanosecond = 0):
        super(TimeImpl, self).__init__(None, self)
        self.hour = hour
        self.minute = minute
        self.second = second
        self.nanosecond = nanosecond

    def __str__(self):
        decimal = ''
        if self.nanosecond != 0:
            decimal = '.%09d' % self.nanosecond
        return "%02d:%02d:%02d%s" % (self.hour, self.minute, self.second, decimal)

    @staticmethod
    def parse(val):
        time = val['nTime']
        h = time / 10000000000000
        m = (time / 100000000000) % 100
        s = (time / 1000000000) % 100
        ns = time % 1000000000
        return TimeImpl(h, m, s, ns)

class TimePrinter(object):
    '''Prints time'''

    def __init__(self, typename, val):
        self.val = val

    def to_string(self):
        return str(TimeImpl.parse(self.val))

class PointPrinter(object):
    '''Prints a Point.'''

    def __init__(self, typename, value):
        self.typename = typename
        self.value = value

    def to_string(self):
        return "%s" % (self.typename)

    def children(self):
        x = self.value['nA']
        y = self.value['nB']
        children = [('x', x), ('y', y)]
        return children.__iter__()

class SizePrinter(object):
    '''Prints a Size.'''

    def __init__(self, typename, value):
        self.typename = typename
        self.value = value

    def to_string(self):
        return "%s" % (self.typename)

    def children(self):
        width = self.value['nA']
        height = self.value['nB']
        children = [('width', width), ('height', height)]
        return children.__iter__()

class RectanglePrinter(object):
    '''Prints a Rectangle.'''

    def __init__(self, typename, value):
        self.typename = typename
        self.value = value

    def to_string(self):
        return "%s" % (self.typename)

    def children(self):
        left = self.value['nLeft']
        top = self.value['nTop']
        right = self.value['nRight']
        bottom = self.value['nBottom']
        children = [('left', left), ('top', top), ('right', right), ('bottom', bottom)]
        return children.__iter__()

printer = None

def build_pretty_printers():
    global printer

    printer = printing.Printer('libreoffice/tl')

    # various types
    printer.add('BigInt', BigIntPrinter)
    printer.add('Color', ColorPrinter)
    printer.add('DateTime', DateTimePrinter)
    printer.add('Date', DatePrinter)
    printer.add('Time', TimePrinter)
    printer.add('Point', PointPrinter)
    printer.add('Size', SizePrinter)
    printer.add('Rectangle', RectanglePrinter)

def register_pretty_printers(obj):
    printing.register_pretty_printer(printer, obj)

build_pretty_printers()

# vim:set shiftwidth=4 softtabstop=4 expandtab:
