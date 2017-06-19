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

    def __ne__(self, other):
        return not self == other

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


class FootnoteNode(TreeNode):

    def __init__(self, label):
        super().__init__()
        self.label = label
        self.nodetype = "Footnote"

    def __str__(self):
        return "{}\tlabel: {}".format(super().__str__(), self.label)

    def __eq__(self, other):
        try:
            return other.label == self.label and super().__eq__(other)
        except AttributeError:
            return False

    def dup(self):
        return self._dup(FootnoteNode, self.label)


class FrameNode(TreeNode):
    def __init__(self, name, anchor):
        super().__init__()
        self.name = name
        self.anchor = anchor
        self.nodetype = "Frame"

    def __str__(self):
        return "{}\tname: {}\tanchor: {}".format(
                super().__str__(),self.name, self.str_anchor(self.anchor))

    def __eq__(self, other):
        try:
            return (other.name == self.name and
                    other.anchor == self.anchor and
                    super().__eq__(other))
        except AttributeError:
            return False

    def dup(self):
        return self._dup(FrameNode, self.name, self.anchor)

    def str_anchor(self, anchor):
        anchors = {str(AS_CHARACTER): "AS_CHARACTER",
                   str(AT_CHARACTER): "AT_CHARACTER",
                   str(AT_PARAGRAPH): "AT_PARAGRAPH",
                   str(AT_PAGE): "AT_PAGE",
                   str(AT_FRAME): "AT_FRAME"}
        try:
            return anchors[str(anchor)]
        except KeyError:
            raise RuntimeError("unknown anchor")


class MetaNode(TreeNode):
    def __init__(self, xmlid):
        super().__init__()
        self.xmlid = xmlid
        self.nodetype = "InContentMetadata"
        self.isnesting = True

    def __str__(self):
        return "{}\txmlid: {}#{}".format(
            super().__str__(), self.xmlid.First, self.xmlid.Second)

    def __eq__(self, other):
        try:
            return (type(other) == type(self) and
                    MetaNode.eq(other.xmlid, self.xmlid))
        except AttributeError:
            return False

    @classmethod
    def eq(cls, left, right):
        return left.First == right.First and left.Second == right.Second

    def dup(self):
        return self._dup(MetaNode, self.xmlid)


class MarkNode(TreeNode):
    def __init__(self, name, ispoint=True):
        super().__init__()
        self.name = name
        self.ispoint = ispoint
        self.isstart = False

    def __str__(self):
        return "{}\tisPoint: {}\tisStart: {}".format(
                super().__str__(), self.ispoint, self.isstart)

    def __eq__(self, other):
        try:
            return (other.name == self.name and
                    other.ispoint == self.ispoint and
                    other.isstart == self.isstart)
        except AttributeError:
            return False


class BookmarkNode(MarkNode):
    def __init__(self, name, xmlid=StringPair()):
        super().__init__(name)
        self.xmlid = xmlid
        self.nodetype = "Bookmark"

    def __str__(self):
        return "{}\txmlid: {}#{}".format(
            super().__str__(), self.xmlid.First, self.xmlid.Second)

    def __eq__(self, other):
        try:
            return (type(other) == type(self) and
                    super().__eq__(other) and
                    MetaNode.eq(other.xmlid, self.xmlid))
        except AttributeError:
            return False

    def dup(self):
        return self._dup(BookmarkNode, self.name, self.xmlid)


class BookmarkStartNode(BookmarkNode):

    def __init__(self, name, xmlid=StringPair()):
        super().__init__(name, xmlid)
        self.ispoint = False
        self.isstart = True

    def dup(self):
        return self._dup(BookmarkStartNode, self.name)


class BookmarkEndNode(BookmarkNode):

    def __init__(self, name, xmlid=StringPair()):
        super().__init__(name, xmlid)
        self.ispoint = False
        self.isstart = False

    def dup(self):
        return self._dup(BookmarkEndNode, self.name)


class ReferenceMarkNode(MarkNode):
    def __init__(self, name):
        super().__init__(name)
        self.nodetype = "ReferenceMark"

    def __eq__(self, other):
        return (type(other) == type(self) and super().__eq__(other))

    def dup(self):
        return self._dup(ReferenceMarkNode, self.name)


class ReferenceMarkStartNode(ReferenceMarkNode):
    def __init__(self, name):
        super().__init__(name)
        self.ispoint = False
        self.isstart = True

    def dup(self):
        return self._dup(ReferenceMarkStartNode, self.name)


class ReferenceMarkEndNode(ReferenceMarkNode):
    def __init__(self, name):
        super().__init__(name)
        self.ispoint = False
        self.isstart = False

    def dup(self):
        return self._dup(ReferenceMarkEndNode, self.name)


class DocumentIndexMarkNode(MarkNode):
    def __init__(self, name):
        super().__init__(name)
        self.nodetype = "DocumentIndexMark"

    def __eq__(self, other):
        return (type(other) == type(self) and super().__eq__(other))

    def dup(self):
        return self._dup(DocumentIndexMarkNode, self.name)


class DocumentIndexMarkStartNode(DocumentIndexMarkNode):
    def __init__(self, name):
        super().__init__(name)
        self.ispoint = False
        self.isstart = True

    def dup(self):
        return self._dup(DocumentIndexMarkStartNode, self.name)


class DocumentIndexMarkEndNode(DocumentIndexMarkNode):
    def __init__(self, name):
        super().__init__(name)
        self.ispoint = False
        self.isstart = False

    def dup(self):
        return self._dup(DocumentIndexMarkEndNode, self.name)


class HyperlinkNode(TreeNode):
    def __init__(self, url):
        super().__init__()
        self.nodetype = "Hyperlink"
        self.isnesting = True
        if url:
            self.url = url
        else:
            raise RuntimeError("HyperlinkNode")

    def __str__(self):
        return "{}\turl: {}".format(super().__str__(), self.url)

    def __eq__(self, other):
        try:
            return other.url == self.url and super().__eq__(other)
        except AttributeError:
            return False

    def dup(self):
        return self._dup(HyperlinkNode, self.url)


class RubyNode(TreeNode):
    def __init__(self, ruby):
        super().__init__()
        self.nodetype = "Ruby"
        self.isnesting = True
        if ruby:
            self.ruby = ruby
        else:
            raise RuntimeError("RubyNode")

    def __str__(self):
        return "{}\trubytext: {}".format(super().__str__(), self.ruby)

    def __eq__(self, other):
        try:
            return other.ruby == self.ruby and super().__eq__(other)
        except AttributeError:
            return False

    def dup(self):
        return self._dup(RubyNode, self.ruby)


class MetaFieldNode(MetaNode):
    def __init__(self, xmlid):
        super().__init__(xmlid)
        self.nodetype = "MetadataField"

    def dup(self):
        return self._dup(MetaFieldNode, self.xmlid)


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

    def insertframe(self, xCursor, name, anchor):
        xContent = self.makeframe(name, anchor)
        xContent.attach(xCursor)

    def makeframe(self, name, anchor):
        xFrame = self.xDoc.createInstance("com.sun.star.text.TextFrame")
        xFrame.AnchorType = anchor
        xFrame.setName(name)
        return xFrame

    def insertfootnote(self, xCursor, label):
        xContent = self.makefootnote(label)
        xContent.attach(xCursor)

    def makefootnote(self, label):
        xFootNote = self.xDoc.createInstance("com.sun.star.text.Footnote")
        xFootNote.setLabel(label)
        return xFootNote

    def insertbookmark(self, xCursor, name, xmlid):
        xContent = self.makebookmark(name)
        xContent.attach(xCursor)
        if xmlid.First != "":
            xContent.MetadataReference = xmlid

    def makebookmark(self, name):
        xBookmark = self.xDoc.createInstance("com.sun.star.text.Bookmark")
        xBookmark.setName(name)
        return xBookmark

    def insertreferencemark(self, xCursor, name):
        xContent = self.makereferencemark(name)
        xContent.attach(xCursor)

    def makereferencemark(self, name):
        xMark = self.xDoc.createInstance("com.sun.star.text.ReferenceMark")
        xMark.setName(name)
        return xMark

    def insertdocumentindexmark(self, xCursor, key):
        xContent = self.makedocumentindexmark(key)
        xContent.attach(xCursor)

    def makedocumentindexmark(self, key):
        xMark = self.xDoc.createInstance("com.sun.star.text.DocumentIndexMark")
        xMark.PrimaryKey = key
        return xMark

    def inserthyperlink(self, xCursor, url):
        xCursor.HyperLinkURL = url

    def insertruby(self, xCursor, rubytext):
        xCursor.RubyText = rubytext


    def insertmeta(self, xCursor, xmlid):
        xContent = self.makemeta()
        xContent.attach(xCursor)
        xContent.MetadataReference = xmlid
        return xContent

    def makemeta(self):
        xMeta = self.xDoc.createInstance("com.sun.star.text.InContentMetadata")
        return xMeta

    def insertmetafield(self, xCursor, xmlid):
        xContent = self.makemetafield()
        xContent.attach(xCursor)
        xContent.MetadataReference = xmlid
        return xContent

    def makemetafield(self):
        xMeta = self.xDoc.createInstance(
                                "com.sun.star.text.textfield.MetadataField")
        return xMeta


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
            elif type_ == "Footnote":
                self.insertfootnote(xCursor, node.label)
            elif type_ == "Frame":
                if node.anchor == AT_CHARACTER:
                    self._framehints.append((xCursor.getStart(), node))
                else:
                    self.insertframe(xCursor, node.name, node.anchor)
            elif type_ == "Bookmark":
                name = node.name
                id_ = node.xmlid
                if node.ispoint:
                    self.insertbookmark(xCursor, name, id_)
                elif node.isstart:
                    self._bookmarkstarts[name] = xCursor.getStart()
                else:
                    xRange = self._bookmarkstarts[name]
                    xParaCursor = self.mkcursor(xRange)
                    self.insertbookmark(xParaCursor, name, id_)
            elif type_ == "ReferenceMark":
                name = node.name
                if node.ispoint:
                    self.insertreferencemark(xCursor, name)
                elif node.isstart:
                    self._referencemarkstarts[name] = xCursor.getStart()
                else:
                    xRange = self._referencemarkstarts[name]
                    xParaCursor = self.mkcursor(xRange)
                    self.insertreferencemark(xParaCursor, name)
            elif type_ == "DocumentIndexMark":
                name = node.name
                if node.ispoint:
                    self.insertdocumentindexmark(xCursor, name)
                elif node.isstart:
                    self._documentindexmarkstarts[name] = xCursor.getStart()
                else:
                    xRange = self._documentindexmarkstarts[name]
                    xParaCursor = self.mkcursor(xRange)
                    self.insertdocumentindexmark(xParaCursor, name)
            elif type_ == "Hyperlink":
                xRange = xCursor.getStart()
                self.insertchildren(node.createenumeration())
                xParaCursor = self.mkcursor(xRange)
                self.inserthyperlink(xParaCursor, node.url)
            elif type_ == "Ruby":
                xRange = xCursor.getStart()
                self.insertchildren(node.createenumeration())
                xParaCursor = self.mkcursor(xRange)
                self.insertruby(xParaCursor, node.ruby)
            elif type_ == "InContentMetadata":
                xRange = xCursor.getStart()
                self.insertchildren(node.createenumeration())
                xParaCursor = self.mkcursor(xRange)
                self.insertmeta(xParaCursor, node.xmlid)
            elif type_ == "MetadataField":
                xRange = xCursor.getStart()
                self.insertchildren(node.createenumeration())
                xParaCursor = self.mkcursor(xRange)
                self.insertmetafield(xParaCursor, node.xmlid)
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
            elif type_ == "Footnote":
                xFootnote = xPortion.Footnote
                label = xFootnote.getLabel()
                node = FootnoteNode(label)
            elif type_ == "Frame":
                xCEA = xPortion.createContentEnumeration('')
                while xCEA.hasMoreElements():
                    xFrame = xCEA.nextElement()
                    anchor = xFrame.AnchorType
                    name = xFrame.getName()
                    node = FrameNode(name, anchor)
                    self._stack[-1].appendchild(node)
                continue
            elif type_ == "Bookmark":
                xMark = xPortion.Bookmark
                name = xMark.getName()
                xmlid = xMark.MetadataReference
                isCollapsed = xPortion.IsCollapsed
                if isCollapsed:
                    node = BookmarkNode(name, xmlid)
                else:
                    isStart = xPortion.IsStart
                    if isStart:
                        node = BookmarkStartNode(name, xmlid)
                    else:
                        node = BookmarkEndNode(name, xmlid)
            elif type_ == "ReferenceMark":
                xMark = xPortion.ReferenceMark
                name = xMark.getName()
                isCollapsed = xPortion.IsCollapsed
                if isCollapsed:
                    node = ReferenceMarkNode(name)
                else:
                    isStart = xPortion.IsStart
                    if isStart:
                        node = ReferenceMarkStartNode(name)
                    else:
                        node = ReferenceMarkEndNode(name)
            elif type_ == "DocumentIndexMark":
                xMark = xPortion.DocumentIndexMark
                name = xMark.PrimaryKey
                isCollapsed = xPortion.IsCollapsed
                if isCollapsed:
                    node = DocumentIndexMarkNode(name)
                else:
                    isStart = xPortion.IsStart
                    if isStart:
                        node = DocumentIndexMarkStartNode(name)
                    else:
                        node = DocumentIndexMarkEndNode(name)
            elif type_ == "Ruby":
                isStart = xPortion.IsStart
                if isStart:
                    # ARRGH!!! stupid api...
                    # the text is ONLY at the start!
                    ruby = xPortion.RubyText
                    node = RubyNode(ruby)
                    self._stack.append(node)
                    continue
                else:
                    node = self._stack.pop()
                    assert (isinstance(node, RubyNode),
                        "stack error: Ruby expected; is: {}".format(str(node)))
            elif type_ == "InContentMetadata":
                xMeta = xPortion.InContentMetadata
                xmlid = xMeta.MetadataReference
                node = MetaNode(xmlid)
                self._stack.append(node)
                xEnumChildren = xMeta.createEnumeration()
                node2 = self.convertchildren(xEnumChildren)
                assert (node2 is node), "stack error: meta"
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

    @unittest.skip("FIXME this is converted to a text portion: ControlCharacter is obsolete")
    def test_control_char(self):
        root = TreeNode()
        cchr = ControlCharacterNode(HARD_HYPHEN)
        root.appendchild(cchr)
        self.dotest(root)

    @unittest.skip("FIXME: insert a soft page break: not done")
    def test_soft_page_break(self):
        root = TreeNode()
        spbk =SoftPageBreakNode()
        text = TextNode("abc")
        root.appendchild(spbk)
        root.appendchild(text)
        self.dotest(root)

    def test_footnote(self):
        name = self.mkname("ftn")
        root = TreeNode()
        ftnd = FootnoteNode(name)
        root.appendchild(ftnd)
        self.dotest(root)

    def test_frame_as(self):
        name = self.mkname("frame")
        root = TreeNode()
        fram = FrameNode(name, AS_CHARACTER)
        root.appendchild(fram)
        self.dotest(root)

    def test_frame_at(self):
        name = self.mkname("frame")
        root = TreeNode()
        fram = FrameNode(name, AT_CHARACTER)
        root.appendchild(fram)
        self.dotest(root)

    def test_bookmark_point(self):
        name = self.mkname("mark")
        root = TreeNode()
        bkmk = BookmarkNode(name)
        text = TextNode("abc")
        root.appendchild(bkmk)
        root.appendchild(text)
        self.dotest(root)

    def test_bookmark(self):
        name = self.mkname("mark")
        root = TreeNode()
        bkm1 = BookmarkStartNode(name)
        text = TextNode("abc")
        bkm2 = BookmarkEndNode(name)
        root.appendchild(bkm1)
        root.appendchild(text)
        root.appendchild(bkm2)
        self.dotest(root)

    def test_bookmark_point_xmlid(self):
        name = self.mkname("mark")
        id = self.mkid("id")
        root = TreeNode()
        bkmk = BookmarkNode(name, id)
        text = TextNode("abc")
        root.appendchild(bkmk)
        root.appendchild(text)
        self.dotest(root)

    def test_bookmark_xmlid(self):
        name = self.mkname("mark")
        id = self.mkid("id")
        root = TreeNode()
        bkm1 = BookmarkStartNode(name, id)
        text = TextNode("abc")
        bkm2 = BookmarkEndNode(name, id)
        root.appendchild(bkm1)
        root.appendchild(text)
        root.appendchild(bkm2)
        self.dotest(root)

    def test_refmark_point(self):
        name = self.mkname("refmark")
        root = TreeNode()
        rfmk = ReferenceMarkNode(name)
        text = TextNode("abc")
        root.appendchild(rfmk)
        root.appendchild(text)
        self.dotest(root)

    def test_refmark(self):
        name = self.mkname("refmark")
        root = TreeNode()
        rfm1 = ReferenceMarkStartNode(name)
        text = TextNode("abc")
        rfm2 = ReferenceMarkEndNode(name)
        root.appendchild(rfm1)
        root.appendchild(text)
        root.appendchild(rfm2)
        self.dotest(root)

    def test_toxmark_point(self):
        name = self.mkname("toxmark")
        root = TreeNode()
        txmk = DocumentIndexMarkNode(name)
        text = TextNode("abc")
        root.appendchild(txmk)
        root.appendchild(text)
        self.dotest(root)

    def test_toxmark(self):
        name = self.mkname("toxmark")
        root = TreeNode()
        txm1 = DocumentIndexMarkStartNode(name)
        text = TextNode("abc")
        txm2 = DocumentIndexMarkEndNode(name)
        root.appendchild(txm1)
        root.appendchild(text)
        root.appendchild(txm2)
        self.dotest(root)

    def test_hyperlink(self):
        name = self.mkname("url")
        root = TreeNode()
        href = HyperlinkNode(name)
        text = TextNode("abc")
        href.appendchild(text)
        root.appendchild(href)
        self.dotest(root)

    def test_hyperlink_empty(self):
        name = self.mkname("url")
        root = TreeNode()
        href = HyperlinkNode(name)
        text = TextNode("")
        href.appendchild(text)
        root.appendchild(href)
        self.dotest(root)

    def test_ruby(self):
        name = self.mkname("ruby")
        root = TreeNode()
        ruby = RubyNode(name)
        text = TextNode("abc")
        ruby.appendchild(text)
        root.appendchild(ruby)
        self.dotest(root)

    def test_ruby_empty(self):
        # BUG: #i91534#
        name = self.mkname("ruby")
        root = TreeNode()
        ruby = RubyNode(name)
        root.appendchild(ruby)
        self.dotest(root)

    def test_meta(self):
        id = StringPair("content.xml", self.mkname("id"))
        root = TreeNode()
        meta = MetaNode(id)
        text = TextNode("abc")
        root.appendchild(TextNode("123"))
        meta.appendchild(text)
        root.appendchild(meta)
        self.dotest(root)

    def test_meta_empty(self):
        id = StringPair("content.xml", self.mkname("id"))
        root = TreeNode()
        meta = MetaNode(id)
        root.appendchild(meta)
        self.dotest(root)

    def test_meta_field(self):
        id = StringPair("content.xml", self.mkname("id"))
        root = TreeNode()
        meta = MetaFieldNode(id)
        text = TextNode("abc")
        root.appendchild(TextNode("123"))
        meta.appendchild(text)
        root.appendchild(meta)
        self.dotest(root)

    def test_meta_field_empty(self):
        id = StringPair("content.xml", self.mkname("id"))
        root = TreeNode()
        meta = MetaFieldNode(id)
        root.appendchild(meta)
        self.dotest(root)

    def test_bookmark1(self):
        name1 = self.mkname("mark")
        name2 = self.mkname("mark")
        name3 = self.mkname("mark")
        root = TreeNode()
        root.appendchild(BookmarkStartNode(name1))
        root.appendchild(BookmarkNode(name2))
        root.appendchild(BookmarkStartNode(name3))
        root.appendchild(TextNode("abc"))
        root.appendchild(BookmarkEndNode(name1))
        root.appendchild(TextNode("de"))
        root.appendchild(BookmarkEndNode(name3))
        self.dotest(root)

    def test_bookmark2(self):
        name1 = self.mkname("mark")
        name2 = self.mkname("mark")
        name3 = self.mkname("mark")
        root = TreeNode()
        root.appendchild(BookmarkStartNode(name1))
        root.appendchild(TextNode("abc"))
        root.appendchild(BookmarkNode(name2))
        root.appendchild(BookmarkStartNode(name3))
        root.appendchild(BookmarkEndNode(name1))
        root.appendchild(TextNode("de"))
        root.appendchild(BookmarkEndNode(name3))
        self.dotest(root)

    def test_refmark2(self):
        name1 = self.mkname("refmark")
        root = TreeNode()
        root.appendchild(ReferenceMarkStartNode(name1))
        root.appendchild(TextNode("abc"))
        # BUG: #i102541# (this is actually not unoportenum's fault)
        root.appendchild(ReferenceMarkEndNode(name1))
        root.appendchild(TextNode("de"))
        self.dotest(root)

    def test_refmark3(self):
        # BUG: #i107672# (non-deterministic; depends on pointer ordering)
        name1 = self.mkname("refmark")
        name2 = self.mkname("refmark")
        name3 = self.mkname("refmark")
        name4 = self.mkname("refmark")
        name5 = self.mkname("refmark")
        name6 = self.mkname("refmark")
        name7 = self.mkname("refmark")
        root = TreeNode()
        root.appendchild(ReferenceMarkStartNode(name1))
        root.appendchild(ReferenceMarkStartNode(name2))
        root.appendchild(ReferenceMarkStartNode(name3))
        root.appendchild(ReferenceMarkStartNode(name4))
        root.appendchild(ReferenceMarkStartNode(name5))
        root.appendchild(ReferenceMarkStartNode(name6))
        root.appendchild(ReferenceMarkStartNode(name7))
        root.appendchild(TextNode("abc"))
        root.appendchild(ReferenceMarkEndNode(name7))
        root.appendchild(ReferenceMarkEndNode(name6))
        root.appendchild(ReferenceMarkEndNode(name5))
        root.appendchild(ReferenceMarkEndNode(name4))
        root.appendchild(ReferenceMarkEndNode(name3))
        root.appendchild(ReferenceMarkEndNode(name2))
        root.appendchild(ReferenceMarkEndNode(name1))
        root.appendchild(TextNode("de"))
        self.dotest(root)

    def test_toxmark2(self):
        name1 = self.mkname("toxmark")
        root = TreeNode()
        root.appendchild(DocumentIndexMarkStartNode(name1))
        root.appendchild(TextNode("abc"))
        root.appendchild(DocumentIndexMarkEndNode(name1))
        root.appendchild(TextNode("de"))
        self.dotest(root)

    def test_toxmark3(self):
        # BUG: #i107672# (non-deterministic; depends on pointer ordering)
        name1 = self.mkname("toxmark")
        name2 = self.mkname("toxmark")
        name3 = self.mkname("toxmark")
        name4 = self.mkname("toxmark")
        name5 = self.mkname("toxmark")
        name6 = self.mkname("toxmark")
        name7 = self.mkname("toxmark")
        root = TreeNode()
        root.appendchild(DocumentIndexMarkStartNode(name1))
        root.appendchild(DocumentIndexMarkStartNode(name2))
        root.appendchild(DocumentIndexMarkStartNode(name3))
        root.appendchild(DocumentIndexMarkStartNode(name4))
        root.appendchild(DocumentIndexMarkStartNode(name5))
        root.appendchild(DocumentIndexMarkStartNode(name6))
        root.appendchild(DocumentIndexMarkStartNode(name7))
        root.appendchild(TextNode("abc"))
        root.appendchild(DocumentIndexMarkEndNode(name7))
        root.appendchild(DocumentIndexMarkEndNode(name6))
        root.appendchild(DocumentIndexMarkEndNode(name5))
        root.appendchild(DocumentIndexMarkEndNode(name4))
        root.appendchild(DocumentIndexMarkEndNode(name3))
        root.appendchild(DocumentIndexMarkEndNode(name2))
        root.appendchild(DocumentIndexMarkEndNode(name1))
        root.appendchild(TextNode("de"))
        self.dotest(root)

    def test_marks1(self):
        name1 = self.mkname("bookmark")
        name2 = self.mkname("toxmark")
        name3 = self.mkname("refmark")
        name4 = self.mkname("toxmark")
        root = TreeNode()
        root.appendchild(BookmarkStartNode(name1))
        root.appendchild(DocumentIndexMarkNode(name2))
        root.appendchild(ReferenceMarkStartNode(name3))
        root.appendchild(TextNode("abc"))
        root.appendchild(BookmarkEndNode(name1))
        root.appendchild(DocumentIndexMarkStartNode(name4))
        root.appendchild(TextNode("de"))
        root.appendchild(DocumentIndexMarkEndNode(name4))
        root.appendchild(ReferenceMarkEndNode(name3))
        self.dotest(root)

    def test_marks2(self):
        name1 = self.mkname("bookmark")
        name2 = self.mkname("refmark")
        name3 = self.mkname("refmark")
        name4 = self.mkname("toxmark")
        name5 = self.mkname("refmark")
        root = TreeNode()
        root.appendchild(BookmarkStartNode(name1))
        root.appendchild(ReferenceMarkNode(name2))
        root.appendchild(ReferenceMarkStartNode(name3))
        root.appendchild(TextNode("abc"))
        root.appendchild(DocumentIndexMarkStartNode(name4))
        root.appendchild(ReferenceMarkStartNode(name5))
        # BUG: #i102541# (this is actually not unoportenum's fault)
        root.appendchild(ReferenceMarkEndNode(name3))
        root.appendchild(TextNode("de"))
        root.appendchild(DocumentIndexMarkEndNode(name4))
        root.appendchild(BookmarkEndNode(name1))
        root.appendchild(ReferenceMarkEndNode(name5))
        self.dotest(root)

    def test_marks3(self):
        name1 = self.mkname("bookmark")
        name2 = self.mkname("refmark")
        name3 = self.mkname("refmark")
        name4 = self.mkname("toxmark")
        name5 = self.mkname("refmark")
        root = TreeNode()
        root.appendchild(BookmarkStartNode(name1))
        root.appendchild(DocumentIndexMarkNode(name2))
        root.appendchild(DocumentIndexMarkStartNode(name3))
        root.appendchild(TextNode("abc"))
        root.appendchild(ReferenceMarkStartNode(name4))
        root.appendchild(DocumentIndexMarkStartNode(name5))
        root.appendchild(DocumentIndexMarkEndNode(name3))
        root.appendchild(TextNode("de"))
        root.appendchild(ReferenceMarkEndNode(name4))
        root.appendchild(BookmarkEndNode(name1))
        root.appendchild(DocumentIndexMarkEndNode(name5))
        self.dotest(root)

    def test_frame_mark1(self):
        name1 = self.mkname("bookmark")
        name2 = self.mkname("frame")
        root = TreeNode()
        root.appendchild(TextNode("abc"))
        root.appendchild(BookmarkNode(name1))
        root.appendchild(TextNode("de"))
        root.appendchild(FrameNode(name2, AS_CHARACTER))
        self.dotest(root)

    def test_frame_mark2(self):
        # BUG: #i98530#
        name1 = self.mkname("bookmark")
        name2 = self.mkname("frame")
        root = TreeNode()
        root.appendchild(TextNode("abc"))
        root.appendchild(BookmarkNode(name1))
        root.appendchild(TextNode("de"))
        root.appendchild(FrameNode(name2, AT_CHARACTER))
        self.dotest(root)

    def test_frame_mark3(self):
        name1 = self.mkname("frame")
        name2 = self.mkname("bookmark")
        root = TreeNode()
        root.appendchild(TextNode("abc"))
        root.appendchild(FrameNode(name1, AS_CHARACTER))
        root.appendchild(TextNode("de"))
        root.appendchild(BookmarkNode(name2))
        self.dotest(root)

    def test_frame_mark4(self):
        name1 = self.mkname("frame")
        name2 = self.mkname("bookmark")
        root = TreeNode()
        root.appendchild(TextNode("abc"))
        root.appendchild(FrameNode(name1, AT_CHARACTER))
        root.appendchild(TextNode("de"))
        root.appendchild(BookmarkNode(name2))
        self.dotest(root)

    def test_frames1(self):
        name1 = self.mkname("frame")
        name2 = self.mkname("frame")
        name3 = self.mkname("frame")
        root = TreeNode()
        root.appendchild(FrameNode(name1, AT_CHARACTER))
        root.appendchild(FrameNode(name2, AT_CHARACTER))
        root.appendchild(FrameNode(name3, AT_CHARACTER))
        self.dotest(root)

    def test_frames2(self):
        name1 = self.mkname("frame")
        name2 = self.mkname("frame")
        name3 = self.mkname("frame")
        root = TreeNode()
        root.appendchild(FrameNode(name1, AS_CHARACTER))
        root.appendchild(FrameNode(name2, AS_CHARACTER))
        root.appendchild(FrameNode(name3, AS_CHARACTER))
        self.dotest(root)

    def test_frames3(self):
        name1 = self.mkname("frame")
        name2 = self.mkname("frame")
        name3 = self.mkname("frame")
        root = TreeNode()
        root.appendchild(FrameNode(name1, AT_CHARACTER))
        root.appendchild(FrameNode(name2, AS_CHARACTER))
        root.appendchild(FrameNode(name3, AT_CHARACTER))
        self.dotest(root)

    def test_frames4(self):
        name1 = self.mkname("frame")
        name2 = self.mkname("frame")
        name3 = self.mkname("frame")
        root = TreeNode()
        root.appendchild(FrameNode(name1, AT_CHARACTER))
        root.appendchild(FrameNode(name2, AT_CHARACTER))
        root.appendchild(FrameNode(name3, AS_CHARACTER))
        self.dotest(root)

    def test_frames5(self):
        name1 = self.mkname("frame")
        name2 = self.mkname("frame")
        name3 = self.mkname("frame")
        root = TreeNode()
        root.appendchild(FrameNode(name1, AS_CHARACTER))
        root.appendchild(FrameNode(name2, AT_CHARACTER))
        root.appendchild(FrameNode(name3, AT_CHARACTER))
        self.dotest(root)

    def test_ruby_hyperlink1(self):
        name1 = self.mkname("ruby")
        name2 = self.mkname("url")
        root = TreeNode()
        ruby = RubyNode(name1)
        href = HyperlinkNode(name2)
        href.appendchild(TextNode("abc"))
        ruby.appendchild(href)
        root.appendchild(ruby)
        self.dotest(root)

    def test_ruby_hyperlink2(self):
        name1 = self.mkname("url")
        name2 = self.mkname("ruby")
        root = TreeNode()
        href = HyperlinkNode(name1)
        ruby = RubyNode(name2)
        ruby.appendchild(TextNode("abc"))
        href.appendchild(ruby)
        root.appendchild(href)
        self.dotest(root)

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
