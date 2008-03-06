#!/usr/bin/env python

import sys
import expression

def run (exp):
    tokens = exp.split()
    expparser = expression.ExpParser(tokens)
    expparser.build()
    expparser.dumpTree()

def main ():
    run("6 + 34")
    run("6 + 34 - 10")
    run("6 + 34 - 10 + 200")
    run("6 + 34 - 10 * 200")
    run("6 + 34 - 10 * 200 + 18")
    run("6 + 34 - 10 * 200 + 18 / 2")

    run("6 * ( ( 10 + 2 ) - 10 ) * 33")

if __name__ == '__main__':
    main()
