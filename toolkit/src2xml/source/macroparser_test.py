#!/usr/bin/env python

import macroparser

def runParser (buf):
    mparser = macroparser.MacroParser(buf)
    mparser.debug = True
    mparser.parse()

def main ():
    buf = 'FOO   (asdfsdaf)'
    runParser(buf)
    buf = 'FOO (x, y)  (x) + (y)'
    runParser(buf)
    buf = 'FOO(x, y)  (x) + (y)'
    runParser(buf)


if __name__ == '__main__':
    main()
