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

def flatten(nlist):
    '''
    utility function to flatten an arbitrarily nested list into a flat list
    example input: [['a', 'b', ['d', 'e', ['f']], ['g', 'h']], 'i', ['j']]
    example output: ['a', 'b', 'd', 'e', 'f', 'g', 'h', 'i', 'j']
    '''
    for i in nlist:
        if isinstance(i, (list,tuple)):
            for j in flatten(i): yield j
        else: yield i

class DepManager:
    '''
    this class stores information about the project dependencies
    '''
    # a fake name being used to be at the top of the dependency tree
    # see the comments inside parseInput()
    MYROOT = 'LO'

    def __init__(self):
        self._tree = {}

    def parseInput(self, file):
        depDict = {}
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
                if t not in depDict:
                    depDict[t] = []
                depDict[t].append(p)
        '''
        any object which is a target but not a prerequisite for another object
        would at the top (or bottom?) of the dependency tree
        Since there may be more than one such object and we want to have a single
        dependency tree, a new fake root is added at the top and these objects will
        be at the first level of the tree
        care should be taken to exclue the fake root object from the results/lists
        by the member functions
        '''
        hList = []
        for x in allT:
            if x not in allP:
                hList.append(x)
        mr = DepManager.MYROOT
        if mr in allP or mr in allT:
            print ('please change the value of the fake root: DepManager.MYROOT')
            sys.exit()

        depDict[mr] = hList
        self._tree = self._makeTree(depDict, mr)

    def loadTree(self, t):
        self._tree = t

    def jsonDump(self):
        return json.dumps(self._tree)

    def targetList(self, item):
        ''' returns a list of objects impacted by the specified item '''
        l = [self._tree['name']]
        self._targetList(item, self._tree, l)
        l.remove(DepManager.MYROOT)
        return l

    def _targetList(self, item, dtree, l):
        n = dtree['name']
        if n==item:
            l.remove(n)
            return True
        preqs = dtree['preqs']
        found = False
        if isinstance(preqs, list):
            for i in preqs:
                iname = i['name']
                l.append(iname)
                if self._targetList(item, i, l) == True:
                    found = True
                else:
                    l.remove(iname)
            return found
        else:
            return False

    def allObjects(self):
        ''' returns the full list all objects in the tree '''
        # just get the preqList of the root
        return self.preqList(DepManager.MYROOT)

    def preqList(self, item):
        ''' returns a dependent list for the specified item '''
        dtree = self._tree
        dt0 = self._depTree(item, dtree)
        dt = {'name':item, 'preqs':dt0}
        l0 = self._getListFromTree(dt)
        l = list(flatten(l0))
        l.remove(item)
        return l

    def _getListFromTree(self, dtree):
        n = dtree['name']
        preqs = dtree['preqs']
        if isinstance(preqs, list):
            l = [self._getListFromTree(i) for i in preqs]
            l.append(n)
            return l
        else:
            return [n]

    def _depTree(self, n, dtree):
        if dtree['name'] == n:
            return dtree['preqs']
        preqs = dtree['preqs']
        if isinstance(preqs, list):
            for i in preqs:
                s = self._depTree(n, i)
                if s is not None:
                    return s

    def printTree(self):
        print ('the dependency tree looks like this:')
        self._printTree(self._tree)

    def _printTree(self, t, n=0):
        ts = ''
        for x in range(n): ts += '  '
        if t['name'] != DepManager.MYROOT:
            print (ts+t['name'])
        preqs = t['preqs']
        if isinstance(preqs, list):
            n+=1
            for i in preqs: self._printTree(i, n)

    def _makeTree(self, depDict, node):
        tree = {'name': node}
        if node in depDict:
            tree['preqs'] = [self._makeTree(depDict, child) for child in depDict[node]]
        else:
            tree['preqs'] = None
        return tree

def main():
    parser = argparse.ArgumentParser(description='Project dependency helper for various ides')
    parser.add_argument('-j','--jsonfile', help='load dependencies from this json file', required=False)
    parser.add_argument('-d','--dumpjson', help='create json file of project dependencies', required=False)
    parser.add_argument('-i','--inputmake', help='input from make -ndp', required=False)
    parser.add_argument('--print', dest='print', action='store_true')
    parser.add_argument('--debug', dest='debug', action='store_true')
    args = vars(parser.parse_args())
    dm = DepManager()
    if args['inputmake'] is not None:
        dm.parseInput(args['inputmake'])
        if args['dumpjson'] is not None:
            with open(args['dumpjson'], 'w') as f:
                f.write(dm.jsonDump())
        elif args['print']: dm.printTree()
        else:
            print ('please use -d to save the results or -p to print')
    elif args['jsonfile'] is not None:
        with open(args['jsonfile'], 'r') as f:
            data = json.load(f)
            dm.loadTree(data)
        if args['print']: dm.printTree()
        if args['debug']:
            dm.printTree()
            olist = dm.allObjects()
            print ('objects:', olist)
            for o in olist:
                plist = dm.preqList(o)
                tlist = dm.targetList(o)
                print (o, '\n\tpre-requisites:', plist, ' \n\ttargets:', tlist)
    else:
        print ('no action specified, please provide input using -i or -j')

if __name__ == '__main__':
    main()

# Local Variables:
# indent-tabs-mode: nil
# End:
#
# vim: set et sw=4 ts=4:
