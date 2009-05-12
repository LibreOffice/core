#!/usr/bin/env python

import sys

class PageSize(object):
    def __init__ (self):
        self.index = 0
        self.list = []

    def parseLine (self, line):
        lhs, rhs = line.split("=")
        name, size = rhs.split("(")
        index = int(lhs)
        name = name.strip()
        size = size[:size.rfind(")")]
        words = size.split()
        width, height = float(words[0]), float(words[3])
        if words[1] != words[4]:
            print "unit mismatch", words[1], words[4]
            sys.exit(1)
        unit = ''
        if words[1] == 'mm':
            unit = 'mm'
        elif words[1] == 'in.':
            unit = 'inches'
        else:
            print "unknown unit:", words[1]
            sys.exit(1)

        item = {'index': index, 'name': name, 'width': width, 'height': height, 'unit': unit}
        self.list.append(item)

    def output (self):
        print "static const XmlPaperSize PaperSizeTable[] ="
        print "{"
        line = "    { 0, 0 },"
        while len(line) <= 60:
            line += ' '
        line += "//  0 - (undefined)"
        print line
        n = len(self.list)
        for i in xrange(0, n):
            if self.list[i]['unit'] == 'mm':
                macro = 'MM2MM100'
            elif self.list[i]['unit'] == 'inches':
                macro = 'IN2MM100'
            else:
                print "unknown unit:", self.list[i]['unit']
                sys.exit(1)
            line  = "    "
            line += '{ ' + macro + "( %g )"%self.list[i]['width'] + ','
            while len(line) <= 28:
                line += ' '
            line += macro + "( %g )"%self.list[i]['height']
            while len(line) <= 48:
                line += ' '
            line += '}'
            if i != n - 1:
                line += ","
            while len(line) <= 60:
                line += ' '
            if self.list[i]['index'] < 10:
                line += "//  %d - "%self.list[i]['index'] + self.list[i]['name']
            else:
                line += "// %d - "%self.list[i]['index'] + self.list[i]['name']

            print line
        print "};"
        return


if len(sys.argv) < 2:
    sys.exit(1)

obj = PageSize()
for line in open(sys.argv[1], 'r').readlines():
    obj.parseLine(line)
obj.output()
