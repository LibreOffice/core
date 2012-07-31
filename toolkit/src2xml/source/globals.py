# *************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
# *************************************************************

import sys

class ParseError (Exception):
    pass

def error (msg, exit=0):
    sys.stderr.write (msg)
    if exit:
        sys.exit (exit)

def progress (msg):
    sys.stderr.write(msg)


def removeQuote (text):
    """Remove quotes from a literal.
"""
    if len(text) >= 2 and text[0] == text[len(text)-1] == '"':
        text = text[1:-1]
    return text


class Macro(object):
    def __init__ (self, name):
        self.name = name
        self.tokens = []
        self.vars = {}


class Node(object):
    def __init__ (self):
        self.children = []

    def appendChild (self, childnode):
        self.children.append(childnode)
        childnode.parent = self

    def getChildren (self):
        return self.children


class RootNode(Node):
    def __init__ (self):
        Node.__init__(self)

    def dump (self):
        chars = ''
        for child in self.getChildren():
            chars += child.dump()
        return chars


class Element(Node):

    INDENT = "    "

    def __init__ (self, name, rid = None):
        Node.__init__(self)
        self.name = name
        self.parent = None
#        print "name: " + self.name - stats ...

        # The following attributes are copied when 'clone'ed.
        self.rid = rid
        self.attrs = {}

    def dump (self, level = 0):
        chars = ''
        chars += "\n" + Element.INDENT*level
        chars += '<%s'%self.name

        if self.rid != None:
            self.setAttr("rid", self.rid)

        chars += self.__dumpAttrs()

        if len(self.children) == 0:
            chars += '/>'
        else:
            chars += '>'
            for child in self.getChildren():
                chars += child.dump(level+1)

            chars += "\n"+Element.INDENT*level
            chars += "</%s>"%self.name

        return chars

    def hasAttr (self, name):
        return self.attrs.has_key(name)

    def getAttr (self, name):
        return self.attrs[name]

    def setAttr (self, name, value):
        if type(value) == type(0):
            value = "%d"%value
        self.attrs[name] = removeQuote(value)
#        print "attr: " + self.name + "." + name - stats ...
        return

    def clone (self, elem):
        keys = elem.attrs.keys()
        for key in keys:
            self.attrs[key] = elem.attrs[key]
        self.rid = elem.rid

    def __dumpAttrs (self):
        text = ''
        keys = sorted(self.attrs.keys())
        for key in keys:
            value = self.attrs[key]
            text += ' %s="%s"'%(key, value)
        return text
