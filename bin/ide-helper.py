#! /usr/bin/env python3
# -*- Mode: python; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
# 

'''
This program is used to analyze the project dependency information
and help set up various ide project files (?)

The program currently runs in two modes:
1) generate and save dependency information into a json file
The input to this program comes from 'make -npd'.  Example:
% make -npdf Makefile.gbuild all > make.out
% ide-helper.py -i make.out -d dep.json

2) load the dependency information previously saved from json file
(from the above step) and serve (still need to find out how/what)
% ide-helper.py -j dep.json
'''
import re
import sys
import json
from functools import reduce
import argparse
import time

def flatten(nlist):
    '''
    utility function to flatten an arbitrarily nested list into a flat list
    example input: [['a', 'b', ['d', 'e', ['f']], ['g', 'h']], 'i', ['j']]
    example output: ['a', 'b', 'd', 'e', 'f', 'g', 'h', 'i', 'j']
    '''
    for i in nlist:
        if isinstance(i, (list,tuple,set)):
            for j in flatten(i): yield j
        else: yield i

class DepManager:
    '''
    this class stores information about the project dependencies
    '''
    # a fake name being used to be at the top (bottom?) of the dependency tree
    MYROOT = 'LO'
    def __init__(self):
        self._data = {}

    def parseInput(self, file):
        # FIXME  for now, duplicating the data by storing two way dictionaries..
        # key: target name, value: set of prerequisites
        pDict = {}
        # key: object name, value: set of dependents/targets
        tDict = {}
        allP = set()
        allT = set()
        with open(file, 'r') as f:
            for line in f:
                line = line.strip()
                # currently, only the following type of lines are parsed to build dependency
                # this will be gradually extended to parse other information from the output
                # of 'make -npd'
                if 'Prerequisite' not in line or 'is older than target' not in line:
                    continue
                line = re.sub("'", "", line)
                v = line.split()
                p = v[1]
                t = v[-1]
                # removing the trailing '.'
                t = re.sub(r'\.$', '', t)
                allP.add(p)
                allT.add(t)
                # FIXME set would be more efficient but JSON complains
                # set is not serializable, will come back to this
                if t not in pDict:
                    pDict[t] = [] #set()
                if p not in pDict[t]:
                    pDict[t].append(p)
                if p not in tDict:
                    tDict[p] = [] #set()
                if t not in tDict[p]:
                    tDict[p].append(t)
        #hList = set()
        hList = []
        for x in allT:
            if x not in allP:
                hList.append(x)
        mr = DepManager.MYROOT
        pDict[mr] = hList
        self._data['PDICT'] = pDict
        self._data['TDICT'] = tDict

    def preqList(self, item):
        l0 = [self._preqList(item)]
        l = list(flatten(l0))
        # don't leave the item as the prerequisite of itself
        l.remove(item)
        return (l)

    def _preqList(self, o):
        if o in self._data['PDICT']:
            # list is very inefficient here
            #l = [self._preqList(i) for i in self._data['PDICT'][o]]
            l = set()
            for i in self._data['PDICT'][o]:
                li = self._preqList(i)
                for x in li: l.add(x)
                l.add(o)
            return l
        else:
            return {o}

    def targetList(self, item):
        l0 = [self._targetList(item)]
        l = list(flatten(l0))
        l.remove(item)
        return (l)

    def _targetList(self, o):
        if o in self._data['TDICT']:
            l = set()
            for i in self._data['TDICT'][o]:
                li = self._targetList(i)
                for x in li: l.add(x)
                l.add(o)
            return l
        else:
            return {o}

    def loadData(self, d):
        self._data = d

    def jsonDump(self):
        return json.dumps(self._data)

    def allObjects(self):
        ''' returns the full list all objects in the tree '''
        # FIXME  this currently takes forever on libreoffice data
        # just get the preqList of the root
        return self.preqList(DepManager.MYROOT)

def mydebug(dm):
    olist = dm.allObjects()
    print ('objects:', olist)
    for o in olist:
        plist = dm.preqList(o)
        tlist = dm.targetList(o)
        print (o, '\n\tpre-requisites:', plist, ' \n\ttargets:', tlist)

def main():
    parser = argparse.ArgumentParser(description='Project dependency helper for various ides')
    parser.add_argument('-j','--jsonfile', help='load dependencies from this json file', required=False)
    parser.add_argument('-d','--dumpjson', help='create json file of project dependencies', required=False)
    parser.add_argument('-i','--inputmake', help='input from make -ndp', required=False)
    args = vars(parser.parse_args())
    dm = DepManager()
    if args['inputmake'] is not None:
        dm.parseInput(args['inputmake'])
        if args['dumpjson'] is not None:
            with open(args['dumpjson'], 'w') as f:
                f.write(dm.jsonDump())
    elif args['jsonfile'] is not None:
        with open(args['jsonfile'], 'r') as f:
            data = json.load(f)
            dm.loadData(data)
    else:
        print ('no input given, please provide input using -i or -j')
        sys.exit()
    # only for small mock input, not libreoffice make data
    # mydebug(dm)

if __name__ == '__main__':
    main()

# Local Variables:
# indent-tabs-mode: nil
# End:
#
# vim: set et sw=4 ts=4:
