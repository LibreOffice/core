# -*- coding: utf-8 -*-

'''
This file is part of the LibreOffice project.

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

This file incorporates work covered by the following license notice:

   Licensed to the Apache Software Foundation (ASF) under one or more
   contributor license agreements. See the NOTICE file distributed
   with this work for additional information regarding copyright
   ownership. The ASF licenses this file to you under the Apache
   License, Version 2.0 (the "License"); you may not use this file
   except in compliance with the License. You may obtain a copy of
   the License at http://www.apache.org/licenses/LICENSE-2.0 .
'''
import uno
import unittest
import os.path
from org.libreoffice.unotest import UnoInProcess, mkPropertyValue
from tempfile import TemporaryDirectory
from com.sun.star.uno import RuntimeException
from com.sun.star.lang import IllegalArgumentException, NoSupportException
from com.sun.star.beans import PropertyValue, StringPair
from com.sun.star.rdf.URIs import ODF_PREFIX, ODF_SUFFIX
from com.sun.star.i18n.NumberFormatIndex import NUMBER_INT
from com.sun.star.text.ControlCharacter import PARAGRAPH_BREAK, HARD_HYPHEN
from com.sun.star.text.TextContentAnchorType import (
                AT_CHARACTER, AS_CHARACTER, AT_PARAGRAPH, AT_PAGE, AT_FRAME)


class TreeNode():
    '''base class for tree nodes. only instance: root of tree.'''

    def __init__(self, content=None):
        self.content = content
        self._children = []
        self.nodetype = "__ROOT__"
        self.isnesting = False

    def __str__(self):
        return "<{}>".format(self.nodetype)

    def __eq__(self, other):
        return type(self) == type(other)

    def _dup(self, nodetype, *args):
        try:
            return nodetype(*args)
        except Exception as e:
            raise RuntimeError("TreeNode.dup") from e

    def createenumeration(self):
        return iter(self._children)

    def appendchild(self, child):
        self._children.append(child)
        return self


class ContentNode(TreeNode):

    def __init__(self, content):
        super().__init__(content)

    def __str__(self):
        return "{}\tcontent: {}".format(super().__str__(), self.content)

    def __eq__(self, other):
        try:
            return other.content == self.content and super().__eq__(other)
        except AttributeError:
            return False

    def appendchild(self, child):
        try:
            self._children.append(child)
            return self
        except Exception as e:
            raise RuntimeError("ContentNode.appendchild") from e


class TextNode(ContentNode):

    def __init__(self, content):
        super().__init__(content)
        self.nodetype = "Text"

    def dup(self):
        return self._dup(TextNode, self.content)


class TextFieldNode(ContentNode):

    def __init__(self, content):
        super().__init__(content)
        self.nodetype = "TextField"

    def dup(self):
        return self._dup(TextFieldNode, self.content)


class ControlCharacterNode(TreeNode):
    def __init__(self, char):
        super().__init__()
        self.char = char
        self.nodetype = "ControlCharacter"

    def __str__(self):
        return "{}\tcontent: {}".format(super().__str__(), self.char)

    def __eq__(self, other):
        try:
            return other.char == self.char and super().__eq__(other)
        except AttributeError:
            return False

    def dup(self):
        return self._dup(ControlCharacterNode, self.char)


class Inserter():

    def __init__(self, xDoc):
        self.xDoc = xDoc
        self.xText = xDoc.getText()
        self.xCursor = self.xText.createTextCursor()

    def initparagraph(self):
        ## we split the first (empty) paragraph, and then insert into the
        ## second (empty) paragraph; this ensures first is always empty!
        self.xCursor.gotoStartOfParagraph(False)
        self.xText.insertControlCharacter(self.xCursor, PARAGRAPH_BREAK, False)

    def inserttext(self, xCursor, text):
        xCursor.setString(text)

    def inserttextfield(self, xCursor, content):
        xContent = self.maketextfield(content)
        xContent.attach(xCursor)

    def maketextfield(self, content):
        xField = self.xDoc.createInstance("com.sun.star.text.textfield.Author")
        xField.IsFixed = True
        xField.FullName = False
        xField.Content = content
        return xField

    def insertcontrolcharacter(self, xCursor, cchar):
        self.xText.insertControlCharacter(xCursor, cchar, False)


class TreeInserter(Inserter):

    def __init__(self, xDoc):
        super().__init__(xDoc)
        self._bookmarkstarts = {}
        self._referencemarkstarts = {}
        self._documentindexmarkstarts = {}
        self._framehints = []

    def inserttree(self, tree):
        if tree.nodetype != "__ROOT__":
            raise RuntimeError("insertTree: test error: no root")
        self.initparagraph()
        self.insertchildren(tree.createenumeration())
        for p in self._framehints:
            self.insertframe(p[0], p[1].name, p[1].anchor)

    def insertchildren(self, children):
        xCursor = self.xCursor
        for node in children:
            xCursor.gotoEndOfParagraph(False)
            type_ = node.nodetype
            if type_ == "Text":
                self.inserttext(xCursor, node.content)
            elif type_ == "TextField":
                self.inserttextfield(xCursor, node.content)
            elif type_ == "ControlCharacter":
                self.insertcontrolcharacter(xCursor, node.char)
            elif type_ == "SoftPageBreak":
                raise RuntimeError("sorry, cannot test SoftPageBreak")
            else:
                raise RuntimeError("unexpected type: {}".format(type_))

    def mkcursor(self, xRange):
        xCursor = self.xText.createTextCursorByRange(xRange)
        xCursor.gotoEndOfParagraph(True)
        return xCursor


class EnumConverter():

    def __init__(self):
        self._stack = []

    def convert(self, xEnum):
        root = TreeNode()
        self._stack.append(root)
        ret = self.convertchildren(xEnum)
        assert (len(self._stack)==0), "EnumConverter.convert: stack is not empty"
        return ret

    def convertchildren(self, xEnum):
        for xPortion in xEnum:
            type_ = xPortion.TextPortionType
            if type_ == "Text":
                text = xPortion.getString()
                node = TextNode(text)
                url = xPortion.HyperLinkURL
                if len(url) > 0:
                    temp = node
                    node = HyperlinkNode(url)
                    node.appendchild(temp)
            elif type_ == "TextField":
                xField = xPortion.TextField
                if xField.supportsService("com.sun.star.text.textfield.MetadataField"):
                    xmlid = xField.MetadataReference
                    node = MetaFieldNode(xmlid)
                    self._stack.append(node)
                    xEnumChildren = xField.createEnumeration()
                    node2 = self.convertchildren(xEnumChildren)
                    print(node)
                    print(node2)
                    assert (node2 is node), "stack error: meta-field"
                else:
                    content = xField.Content
                    isFixed = xField.IsFixed
                    assert isFixed, "field not fixed?"
                    node = TextFieldNode(content)
            elif type_ == "ControlCharacter":
                c = xPortion.ControlCharacter
                node = ControlCharacterNode(c)
            elif type_ == "SoftPageBreak":
                node = SoftPageBreakNode()
            else:
                raise RuntimeError("unexpected type: {}".format(type_))
            self._stack[-1].appendchild(node)
        ret = self._stack.pop()
        return ret


class FuzzyTester():
    '''this is where we nail the pudding to the wall'''
    def __init__(self):
        self.diffcontent = 0
        self.diffmissing = 0
        self.diffnesting = 0
        self.diffspuriousemptytext = 0
        self.diffsequence = 0 # ignored?
        self.stackexpected = []
        self.stackactual = []
        self.bufferexpected = []
        self.bufferactual = []

    def dotest(self, expected, actual):
        '''idea: traverse both trees, enumerate nodes, stopping at content nodes.
        then compare buffers.'''
        assert "__ROOT__" == expected.nodetype
        assert "__ROOT__" == actual.nodetype
        self.stackexpected.append((expected, expected.createenumeration()))
        self.stackactual.append((actual, actual.createenumeration()))
        while self.stackexpected or self.stackactual:
            self.traverse(self.stackexpected, self.bufferexpected)
            self.traverse(self.stackactual, self.bufferactual)
            self.testbuffer()
        if self.diffsequence:
            print("warning: {} differences in sequence".format(
                                                    self.diffsequence))
        if self.diffspuriousemptytext:
            print("warning: {} spurious empty text nodes".format(
                                                    self.diffspuriousemptytext))
        if self.diffnesting:
            print("WARNING: {} differences in nesting".format(
                                                    self.diffnesting))
        assert self.diffcontent == 0
        assert self.diffmissing == 0

    def traverse(self, stack, buffer):
        while stack:
            topenum = stack[-1][1]
            try:
                node = next(topenum)
                buffer.append(node)
                if node._children:
                    node_enum = node.createenumeration()
                    stack.append((node, node_enum))
                if node.content:
                    if not (isinstance(node, TextNode) and # spurious empty text?
                            len(node.content) == 0):
                        return # break here
            except StopIteration:
                buffer.append(stack[-1][0])
                stack.pop()

    def testterminatingnode(self):
        lenexpected = len(self.bufferexpected)
        lenactual = len(self.bufferactual)
        if lenexpected == 0 or lenactual == 0:
            return
        expected = self.bufferexpected[-1]
        actual = self.bufferactual[-1]
        eroot = expected.nodetype == "__ROOT__"
        aroot = actual.nodetype == "__ROOT__"
        if eroot or aroot:
            if not (eroot and aroot):
                if aroot:
                    self.printmissing(expected)
                else:
                    self.printunexpected(actual)
                self.diffmissing += 1
            return
        self.testcontentnode(expected, actual)
        self.bufferexpected[-1] = None
        self.bufferactual[-1] = None

    def testcontentnode(self, expected, actual):
        contentexpected = expected.content
        contentactual = actual.content
        if expected != actual:
            self.printdiff("text content differs", contentexpected, contentactual)
            self.diffcontent += 1

    def testbuffer(self):
        lenactual = len(self.bufferactual)
        tmp_bufferactual = self.bufferactual[:]
        for i, node in enumerate(self.bufferexpected):
            try:
                j = tmp_bufferactual.index(node)
                if j != i:
                    # FIXME how bad is this?
                    self.printdiff("position differs", i, j)
                    # a hacky hack
                    min_ = min(i,j)
                    max_ = max(min(lenactual-1, i),j)
                    for k in range(min_, max_):
                        tmp = tmp_bufferactual[k]
                        if tmp and tmp.isnesting:
                            self.printnesting(node, tmp)
                            self.diffnesting += 1
                    self.diffsequence += 1
                tmp_bufferactual[j] = None
            except ValueError:
                print('perdrix')
                self.printmissing(node)
                self.diffmissing += 1
        for j, node in enumerate(tmp_bufferactual):
            if node:
                self.printunexpected(node)
                if isinstance(node, TextNode) and len(node.content) == 0:
                    self.diffspuriousemptytext += 1
                else:
                    print('renard')
                    self.diffmissing += 1
        self.testterminatingnode()
        self.bufferexpected[:] = []
        self.bufferactual[:] = []

    def printdiff(self, prefix, expected, actual):
        print("{}:\texpected: {}\tactual: {}".format(prefix, expected, actual))

    def printnesting(self, node, nesting):
        print("node: {} possibly moved across nesting {}".format(
                                                    str(node), str(nesting)))

    def printmissing(self, node):
        print("   missing node: {}".format(str(node)))

    def printunexpected(self, node):
        print("unexpected node: {}".format(str(node)))


class TextPortionEnumerationTest(unittest.TestCase):

    xMSF = None
    xContext = None
    tempdir = None

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()
        cls.xDoc = cls._uno.openEmptyWriterDoc()
        cls.count = 0

    @classmethod
    def tearDownClass(cls):
        cls.xDoc.close(True)

    def test_text(self):
        root = TreeNode()
        text = TextNode("abc")
        root.appendchild(text)
        self.dotest(root)

    def test_text_field(self):
        self.mkname("ruby")
        root = TreeNode()
        txtf = TextFieldNode("abc")
        root.appendchild(txtf)
        self.dotest(root)

    ## FIXME this is converted to a text portion: ControlCharacter is obsolete
    # def test_control_char(self):
        # root = TreeNode()
        # cchr = ControlCharacterNode(HARD_HYPHEN)
        # root.appendchild(cchr)
        # self.dotest(root)

    ## FIXME: insert a soft page break: not done
    # def test_soft_page_break(self):
        # root = TreeNode()
        # spbk =SoftPageBreakNode()
        # text = TextNode("abc")
        # root.appendchild(spbk)
        # root.appendchild(text)
        # self.dotest(root)

    def dotest(self, intree, insert=True):
        xDoc = self.__class__.xDoc
        self._dotest(xDoc, intree, insert)

    def _dotest(self, xDoc, intree, insert):
        self._dumptree(intree, "I: ")
        if insert:
            TreeInserter(xDoc).inserttree(intree)
        xText = xDoc.getText()
        xTextEnum = xText.createEnumeration()
        ## skip to right paragraph
        xTextEnum.nextElement(); # skip first -- always empty!
        xElement = xTextEnum.nextElement() # second contains test case
        xEnum = xElement.createEnumeration()
        outtree = EnumConverter().convert(xEnum)
        self._dumptree(outtree, "O: ")
        FuzzyTester().dotest(intree, outtree)

    def _dumptree(self, tree, prefix):
        print('{}{}'.format(prefix, str(tree)))
        children = tree.createenumeration()
        for node in children:
            self._dumptree(node, "{}  ".format(prefix))

    def mkname(self, prefix):
        self.__class__.count += 1
        return "{}{}".format(prefix, self.__class__.count)

    def mkid(self, prefix):
        id = self.mkname(prefix)
        return StringPair("content.xml", self.mkname(prefix))

    def mkid_(self, id):
        return StringPair("content.xml", id)

    def eq(self, left, right):
        return (left.First == right.First and
                left.Second == right.Second)


if __name__ == '__main__':
    unittest.main()