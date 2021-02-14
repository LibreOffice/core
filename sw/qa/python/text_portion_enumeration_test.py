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


class Range():
    def __init__(self, start, end, node):
        self.start = start
        self.end = end
        self.node = node
        self.extent = end - start


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


# FIXME: this does not account for inserted dummy characters!
class RangeInserter(Inserter):
    def __init__(self, xDoc):
        super().__init__(xDoc)
        self.initparagraph()

    # def inserttext(self, pos, text):
        # self.xCursor.gotoStartOfParagraph(False)
        # self.xCursor.goRight(pos, False)
        # self.inserttext(self.xCursor, text)

    def insertrange(self, range):
        self.xCursor.gotoStartOfParagraph(False)
        self.xCursor.goRight(range.start, False)
        self.xCursor.goRight(range.extent, True)
        return self.insertnode(self.xCursor, range.node)

    def insertnode(self, xParaCursor, node):
        nodetype = node.nodetype
        if nodetype == "Text":
            text = node
            self.inserttext(xParaCursor, text.content)
        elif nodetype == "Hyperlink":
            href = node
            self.inserthyperlink(xParaCursor, href.url)
        elif nodetype == "Ruby":
            ruby = node
            self.insertruby(xParaCursor, ruby.ruby)
        elif nodetype == "InContentMetadata":
            meta = node
            return self.insertmeta(xParaCursor, meta.xmlid)
        elif nodetype == "MetadataField":
            meta = node
            return self.insertmetafield(xParaCursor, meta.xmlid)
        elif nodetype == "Bookmark":
            bkmk = node
            if bkmk.ispoint:
                raise RuntimeError("range only")
            self.insertbookmark(xParaCursor, bkmk.name, bkmk.xmlid)
        elif nodetype == "ReferenceMark":
            mark = node
            if mark.ispoint:
                raise RuntimeError("range only")
            self.insertreferencemark(xParaCursor, mark.name)
        elif nodetype == "DocumentIndexMark":
            mark = node
            if mark.ispoint:
                raise RuntimeError("range only")
            self.insertdocumentindexmark(xParaCursor, mark.name)
        elif nodetype == "TextField":
            field = node
            self.inserttextfield(self.xCursor, field.content)
        elif nodetype == "Footnote":
            note = node
            self.insertfootnote(self.xCursor, note.label)
        elif nodetype == "Frame":
            frame = node
            self.insertframe(xParaCursor, frame.name, frame.anchor)
        elif nodetype == "ControlCharacter":
            cchar = node
            self.insertcontrolcharacter(self.xCursor, cchar.char)
        elif nodetype == "SoftPageBreak":
            raise RuntimeError("sorry, cannot test SoftPageBreak")
        else:
            raise RuntimeError("unexpected nodetype: {}".format(nodetype))
        return None


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
                    assert (isinstance(node, RubyNode)), "stack error: Ruby expected; is: {}".format(str(node))
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
        cls._uno.tearDown()
        # HACK in case cls.xDoc holds a UNO proxy to an SwXTextDocument (whose dtor calls
        # Application::GetSolarMutex via sw::UnoImplPtrDeleter), which would potentially only be
        # garbage-collected after VCL has already been deinitialized:
        cls.xDoc = None

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

    def test_end1(self):
        name1 = self.mkname("bookmark")
        name2 = self.mkname("toxmark")
        name3 = self.mkname("refmark")
        root = TreeNode()
        root.appendchild(TextNode("abc"))
        root.appendchild(BookmarkNode(name1))
        root.appendchild(DocumentIndexMarkNode(name2))
        root.appendchild(ReferenceMarkNode(name3))
        self.dotest(root)

    def test_end2(self):
        name1 = self.mkname("bookmark")
        name2 = self.mkname("frame")
        name3 = self.mkname("refmark")
        name4 = self.mkname("frame")
        name5 = self.mkname("frame")
        root = TreeNode()
        root.appendchild(TextNode("abc"))
        root.appendchild(BookmarkStartNode(name1))
        root.appendchild(FrameNode(name2, AT_CHARACTER))
        root.appendchild(BookmarkEndNode(name1))
        root.appendchild(ReferenceMarkNode(name3))
        root.appendchild(FrameNode(name4, AT_CHARACTER))
        root.appendchild(FrameNode(name5, AT_CHARACTER))
        self.dotest(root)

    def test_end3(self):
        name1 = self.mkname("ftn")
        name2 = self.mkname("toxmark")
        root = TreeNode()
        root.appendchild(TextNode("abc"))
        root.appendchild(FootnoteNode(name1))
        root.appendchild(DocumentIndexMarkNode(name2))
        self.dotest(root)

    def test_end4(self):
        name1 = self.mkname("bookmark")
        name2 = self.mkname("frame")
        root = TreeNode()
        root.appendchild(BookmarkStartNode(name1))
        root.appendchild(TextNode("abc"))
        root.appendchild(FrameNode(name2, AS_CHARACTER))
        root.appendchild(BookmarkEndNode(name1))
        self.dotest(root)

    def test_end5(self):
        name1 = self.mkname("refmark")
        name2 = self.mkname("ruby")
        root = TreeNode()
        root.appendchild(ReferenceMarkStartNode(name1))
        root.appendchild(TextNode("abc"))
        ruby = RubyNode(name2)
        ruby.appendchild(TextFieldNode("de"))
        root.appendchild(ruby)
        root.appendchild(ReferenceMarkEndNode(name1))
        self.dotest(root)

    def test_empty1(self):
        name1 = self.mkname("refmark")
        name2 = self.mkname("toxmark")
        name3 = self.mkname("bookmark")
        name4 = self.mkname("frame")
        name7 = self.mkname("refmark")
        name8 = self.mkname("toxmark")
        name9 = self.mkname("bookmark")
        nameA = self.mkname("frame")
        root = TreeNode()
        root.appendchild(ReferenceMarkNode(name1))
        root.appendchild(DocumentIndexMarkNode(name2))
        root.appendchild(BookmarkStartNode(name3))
        root.appendchild(FrameNode(name4, AT_CHARACTER))
        root.appendchild(BookmarkEndNode(name3))
        root.appendchild(ReferenceMarkNode(name7))
        root.appendchild(DocumentIndexMarkNode(name8))
        root.appendchild(BookmarkStartNode(name9))
        root.appendchild(FrameNode(nameA, AT_CHARACTER))
        root.appendchild(BookmarkEndNode(name9))
        self.dotest(root)

    def test_empty2(self):
        name3 = self.mkname("bookmark")
        name4 = self.mkname("frame")
        name9 = self.mkname("bookmark")
        nameA = self.mkname("frame")
        root = TreeNode()
        root.appendchild(BookmarkStartNode(name3))
        root.appendchild(FrameNode(name4, AT_CHARACTER))
        root.appendchild(BookmarkEndNode(name3))
        root.appendchild(BookmarkStartNode(name9))
        root.appendchild(FrameNode(nameA, AT_CHARACTER))
        root.appendchild(BookmarkEndNode(name9))
        self.dotest(root)

    def test_empty3(self):
        name1 = self.mkname("refmark")
        name2 = self.mkname("toxmark")
        name3 = self.mkname("bookmark")
        name4 = self.mkname("frame")
        name5 = self.mkname("url")
        name6 = self.mkname("ruby")
        name7 = self.mkname("refmark")
        name8 = self.mkname("toxmark")
        name9 = self.mkname("bookmark")
        nameA = self.mkname("frame")
        root = TreeNode()
        root.appendchild(ReferenceMarkNode(name1))
        root.appendchild(DocumentIndexMarkNode(name2))
        root.appendchild(BookmarkStartNode(name3))
        root.appendchild(FrameNode(name4, AT_CHARACTER))
        root.appendchild(BookmarkEndNode(name3))
        ## currently empty hyperlinks may get eaten...
        # href = HyperlinkNode(name5)
        # href.appendchild(TextNode(""))
        # root.appendchild(href)
        ruby = RubyNode(name6)
        root.appendchild(ruby)
        root.appendchild(ReferenceMarkNode(name7))
        root.appendchild(DocumentIndexMarkNode(name8))
        root.appendchild(BookmarkStartNode(name9))
        root.appendchild(FrameNode(nameA, AT_CHARACTER))
        root.appendchild(BookmarkEndNode(name9))
        self.dotest(root)

    def test1(self):
        name1 = self.mkname("frame")
        name2 = self.mkname("bookmark")
        name3 = self.mkname("ruby")
        name4 = self.mkname("ftn")
        name5 = self.mkname("frame")
        root = TreeNode()
        root.appendchild(FrameNode(name1, AT_CHARACTER))
        root.appendchild(BookmarkStartNode(name2))
        root.appendchild(TextNode("abc"))
        ruby = RubyNode(name3)
        ruby.appendchild(TextNode("de"))
        ruby.appendchild(FootnoteNode(name4))
        ruby.appendchild(BookmarkEndNode(name2))
        root.appendchild(ruby)
        root.appendchild(TextNode("fg"))
        root.appendchild(FrameNode(name5, AT_CHARACTER))
        root.appendchild(TextFieldNode("h"))
        self.dotest(root)

    # some range tests for the insertion: these are for the current
    # API which treats hyperlinks and rubys not as entities, but as formatting
    # attributes; if these ever become entities, they should not be split!'''

    def test_range1(self):
        name1 = self.mkname("url")
        inserter = RangeInserter(self.__class__.xDoc)
        text = TextNode("12345")
        inserter.insertrange(Range(0, 0, text))
        url1 = HyperlinkNode(name1)
        range1 = Range(0, 5, url1)
        inserter.insertrange(range1)
        root = TreeNode()
        root.appendchild(url1)
        url1.appendchild(text)
        self.dotest(root, False)

    def test_range_hyperlink_hyperlink(self):
        inserter = RangeInserter(self.__class__.xDoc)
        text = TextNode("123456789")
        inserter.insertrange(Range(0, 0, text))
        url1 = HyperlinkNode(self.mkname("url"))
        inserter.insertrange(Range(1, 4, url1))
        ## overlap left
        url2 = HyperlinkNode(self.mkname("url"))
        inserter.insertrange(Range(0, 2, url2))
        root = TreeNode()
        root.appendchild(url2.dup().appendchild(TextNode("12")))
        root.appendchild(url1.dup().appendchild(TextNode("34")))
        root.appendchild(TextNode("56789"))
        self.dotest(root, False)
        ## overlap right
        url3 = HyperlinkNode(self.mkname("url"))
        inserter.insertrange(Range(3, 7, url3))
        root = TreeNode()
        root.appendchild(url2.dup().appendchild(TextNode("12")))
        root.appendchild(url1.dup().appendchild(TextNode("3")))
        root.appendchild(url3.dup().appendchild(TextNode("4567")))
        root.appendchild(TextNode("89"))
        self.dotest(root, False)
        ## around
        url4 = HyperlinkNode(self.mkname("url"))
        inserter.insertrange(Range(3, 7, url4))
        root = TreeNode()
        root.appendchild(url2.dup().appendchild(TextNode("12")))
        root.appendchild(url1.dup().appendchild(TextNode("3")))
        root.appendchild(url4.dup().appendchild(TextNode("4567")))
        root.appendchild(TextNode("89"))
        self.dotest(root, False)
        ## inside
        url5 = HyperlinkNode(self.mkname("url"))
        inserter.insertrange(Range(4, 6, url5))
        root = TreeNode()
        root.appendchild(url2.dup().appendchild(TextNode("12")))
        root.appendchild(url1.dup().appendchild(TextNode("3")))
        root.appendchild(url4.dup().appendchild(TextNode("4")))
        root.appendchild(url5.dup().appendchild(TextNode("56")))
        root.appendchild(url4.dup().appendchild(TextNode("7")))
        root.appendchild(TextNode("89"))
        self.dotest(root, False)
        ## empty
        url6 = HyperlinkNode(self.mkname("url"))
        inserter.insertrange(Range(7, 7, url6))
        root = TreeNode()
        root.appendchild(url2.dup().appendchild(TextNode("12")))
        root.appendchild(url1.dup().appendchild(TextNode("3")))
        root.appendchild(url4.dup().appendchild(TextNode("4")))
        root.appendchild(url5.dup().appendchild(TextNode("56")))
        root.appendchild(url4.dup().appendchild(TextNode("7")))
        ##  this one gets eaten, but we still need to test inserting it (#i106930#)
        # root.appendchild(url6.dup().appendchild(TextNode("")))
        root.appendchild(TextNode("89"))
        ## inside (left-edge)
        url7 = HyperlinkNode(self.mkname("url"))
        inserter.insertrange(Range(0, 1, url7))
        root = TreeNode()
        root.appendchild(url7.dup().appendchild(TextNode("1")))
        root.appendchild(url2.dup().appendchild(TextNode("2")))
        root.appendchild(url1.dup().appendchild(TextNode("3")))
        root.appendchild(url4.dup().appendchild(TextNode("4")))
        root.appendchild(url5.dup().appendchild(TextNode("56")))
        root.appendchild(url4.dup().appendchild(TextNode("7")))
        root.appendchild(TextNode("89"))
        ## inside (right-edge)
        url8 = HyperlinkNode(self.mkname("url"))
        inserter.insertrange(Range(5, 6, url8))
        root = TreeNode()
        root.appendchild(url7.dup().appendchild(TextNode("1")))
        root.appendchild(url2.dup().appendchild(TextNode("2")))
        root.appendchild(url1.dup().appendchild(TextNode("3")))
        root.appendchild(url4.dup().appendchild(TextNode("4")))
        root.appendchild(url5.dup().appendchild(TextNode("5")))
        root.appendchild(url8.dup().appendchild(TextNode("6")))
        root.appendchild(url4.dup().appendchild(TextNode("7")))
        root.appendchild(TextNode("89"))
        self.dotest(root, False)

    def test_range_hyperlink_ruby(self):
        inserter = RangeInserter(self.__class__.xDoc)
        text = TextNode("123456789")
        inserter.insertrange(Range(0, 0, text))
        url1 = HyperlinkNode(self.mkname("url"))
        inserter.insertrange(Range(1, 4, url1))
        # overlap left
        rby2 = RubyNode(self.mkname("ruby"))
        inserter.insertrange(Range(0, 2, rby2))
        root = TreeNode()
        root.appendchild(rby2.dup()
            .appendchild(TextNode("1"))
            .appendchild(url1.dup().appendchild(TextNode("2"))))
        root.appendchild(url1.dup().appendchild(TextNode("34")))
        root.appendchild(TextNode("56789"))
        self.dotest(root, False)
        # overlap right
        rby3 = RubyNode(self.mkname("ruby"))
        inserter.insertrange(Range(3, 5, rby3))
        root = TreeNode()
        root.appendchild(rby2.dup()
                .appendchild(TextNode("1"))
                .appendchild(url1.dup().appendchild(TextNode("2"))))
        root.appendchild(url1.dup().appendchild(TextNode("3")))
        root.appendchild(rby3.dup()
                .appendchild(url1.dup().appendchild(TextNode("4")))
                .appendchild(TextNode("5")))
        root.appendchild(TextNode("6789"))
        self.dotest(root, False)
        # around
        rby4 = RubyNode(self.mkname("ruby"))
        inserter.insertrange(Range(2, 3, rby4))
        root = TreeNode()
        root.appendchild(rby2.dup()
                .appendchild(TextNode("1"))
                .appendchild(url1.dup().appendchild(TextNode("2"))))
        root.appendchild(rby4.dup()
                .appendchild(url1.dup().appendchild(TextNode("3"))))
        root.appendchild(rby3.dup()
                .appendchild(url1.dup().appendchild(TextNode("4")))
                .appendchild(TextNode("5")))
        root.appendchild(TextNode("6789"))
        self.dotest(root, False)
        # inside
        url5 = HyperlinkNode(self.mkname("url"))
        inserter.insertrange(Range(6, 9, url5))
        rby6 = RubyNode(self.mkname("ruby"))
        inserter.insertrange(Range(7, 8, rby6))
        root = TreeNode()
        root.appendchild(rby2.dup()
                .appendchild(TextNode("1"))
                .appendchild(url1.dup().appendchild(TextNode("2"))))
        root.appendchild(rby4.dup()
                .appendchild(url1.dup().appendchild(TextNode("3"))))
        root.appendchild(rby3.dup()
                .appendchild(url1.dup().appendchild(TextNode("4")))
                .appendchild(TextNode("5")))
        root.appendchild(TextNode("6"))
        root.appendchild(url5.dup().appendchild(TextNode("7")))
        root.appendchild(rby6.dup()
                .appendchild(url5.dup().appendchild(TextNode("8"))))
        root.appendchild(url5.dup().appendchild(TextNode("9")))
        self.dotest(root, False)

    def test_range_ruby_hyperlink(self):
        inserter = RangeInserter(self.__class__.xDoc)
        text = TextNode("123456789")
        inserter.insertrange(Range(0, 0, text))
        rby1 = RubyNode(self.mkname("ruby"))
        inserter.insertrange(Range(1, 6, rby1))
        ## overlap left
        url2 = HyperlinkNode(self.mkname("url"))
        inserter.insertrange(Range(0, 3, url2))
        root = TreeNode()
        root.appendchild(url2.dup().appendchild(TextNode("1")))
        root.appendchild(rby1.dup()
                .appendchild(url2.dup().appendchild(TextNode("23")))
                .appendchild(TextNode("456")))
        root.appendchild(TextNode("789"))
        self.dotest(root, False)
        ## overlap right
        url3 = HyperlinkNode(self.mkname("url"))
        inserter.insertrange(Range(5, 7, url3))
        root = TreeNode()
        root.appendchild(url2.dup().appendchild(TextNode("1")))
        root.appendchild(rby1.dup()
                .appendchild(url2.dup().appendchild(TextNode("23")))
                .appendchild(TextNode("45"))
                .appendchild(url3.dup().appendchild(TextNode("6"))))
        root.appendchild(url3.dup().appendchild(TextNode("7")))
        root.appendchild(TextNode("89"))
        self.dotest(root, False)
        ## around (not quite, due to API)
        url4 = HyperlinkNode(self.mkname("url"))
        inserter.insertrange(Range(1, 8, url4))
        root = TreeNode()
        root.appendchild(url2.dup().appendchild(TextNode("1")))
        root.appendchild(rby1.dup()
                .appendchild(url4.dup()
                    .appendchild(TextNode("23456"))))
        root.appendchild(url4.dup().appendchild(TextNode("78")))
        root.appendchild(TextNode("9"))
        self.dotest(root, False)
        ## inside
        url5 = HyperlinkNode(self.mkname("url"))
        inserter.insertrange(Range(3, 5, url5))
        root = TreeNode()
        root.appendchild(url2.dup().appendchild(TextNode("1")))
        root.appendchild(rby1.dup()
                .appendchild(url4.dup()
                    .appendchild(TextNode("23")))
                .appendchild(url5.dup()
                    .appendchild(TextNode("45")))
                .appendchild(url4.dup()
                    .appendchild(TextNode("6"))))
        root.appendchild(url4.dup().appendchild(TextNode("78")))
        root.appendchild(TextNode("9"))
        self.dotest(root, False)

    def test_range_ruby_ruby(self):
        inserter = RangeInserter(self.__class__.xDoc)
        text = TextNode("123456789")
        inserter.insertrange(Range(0, 0, text))
        rby1 = RubyNode(self.mkname("ruby"))
        inserter.insertrange(Range(1, 4, rby1))
        ## overlap left
        rby2 = RubyNode(self.mkname("ruby"))
        inserter.insertrange(Range(0, 2, rby2))
        root = TreeNode()
        root.appendchild(rby2.dup().appendchild(TextNode("12")))
        root.appendchild(rby1.dup().appendchild(TextNode("34")))
        root.appendchild(TextNode("56789"))
        self.dotest(root, False)
        ## overlap right
        rby3 = RubyNode(self.mkname("ruby"))
        inserter.insertrange(Range(3, 7, rby3))
        root = TreeNode()
        root.appendchild(rby2.dup().appendchild(TextNode("12")))
        root.appendchild(rby1.dup().appendchild(TextNode("3")))
        root.appendchild(rby3.dup().appendchild(TextNode("4567")))
        root.appendchild(TextNode("89"))
        self.dotest(root, False)
        ## around
        rby4 = RubyNode(self.mkname("ruby"))
        inserter.insertrange(Range(3, 7, rby4))
        root = TreeNode()
        root.appendchild(rby2.dup().appendchild(TextNode("12")))
        root.appendchild(rby1.dup().appendchild(TextNode("3")))
        root.appendchild(rby4.dup().appendchild(TextNode("4567")))
        root.appendchild(TextNode("89"))
        self.dotest(root, False)
        ## inside
        rby5 = RubyNode(self.mkname("ruby"))
        inserter.insertrange(Range(4, 6, rby5))
        root = TreeNode()
        root.appendchild(rby2.dup().appendchild(TextNode("12")))
        root.appendchild(rby1.dup().appendchild(TextNode("3")))
        root.appendchild(rby4.dup().appendchild(TextNode("4")))
        root.appendchild(rby5.dup().appendchild(TextNode("56")))
        root.appendchild(rby4.dup().appendchild(TextNode("7")))
        root.appendchild(TextNode("89"))
        self.dotest(root, False)

    def test_range_hyperlink_meta(self):
        inserter = RangeInserter(self.__class__.xDoc)
        text = TextNode("123456789")
        inserter.insertrange(Range(0, 0, text))
        url1 = HyperlinkNode(self.mkname("url"))
        inserter.insertrange(Range(1, 4, url1))
        ## overlap left
        met2 = MetaNode(self.mkid("id"))
        inserter.insertrange(Range(0, 2, met2))
        root = TreeNode()
        root.appendchild(met2.dup()
                .appendchild(TextNode("1"))
                .appendchild(url1.dup().appendchild(TextNode("2"))))
        root.appendchild(url1.dup().appendchild(TextNode("34")))
        root.appendchild(TextNode("56789"))
        self.dotest(root, False)
        ## overlap right
        met3 = MetaNode(self.mkid("id"))
        # inserter.insertrange(Range(4-1, 6-1, met3))
        inserter.insertrange(Range(4, 6, met3))
        root = TreeNode()
        root.appendchild(met2.dup()
                .appendchild(TextNode("1"))
                .appendchild(url1.dup().appendchild(TextNode("2"))))
        root.appendchild(url1.dup().appendchild(TextNode("3")))
        root.appendchild(met3.dup()
                .appendchild(url1.dup().appendchild(TextNode("4")))
                .appendchild(TextNode("5")))
        root.appendchild(TextNode("6789"))
        self.dotest(root, False)
        ## around
        met4 = MetaNode(self.mkid("id"))
        # inserter.insertrange(Range(3-1, 4-1, met4))
        inserter.insertrange(Range(3, 4, met4))
        root = TreeNode()
        root.appendchild(met2.dup()
                .appendchild(TextNode("1"))
                .appendchild(url1.dup().appendchild(TextNode("2"))))
        root.appendchild(met4.dup()
                .appendchild(url1.dup().appendchild(TextNode("3"))))
        root.appendchild(met3.dup()
                .appendchild(url1.dup().appendchild(TextNode("4")))
                .appendchild(TextNode("5")))
        root.appendchild(TextNode("6789"))
        self.dotest(root, False)
        ## inside
        url5 = HyperlinkNode(self.mkname("url"))
        # inserter.insertrange(Range(9-3, 12-3, url5))
        inserter.insertrange(Range(9, 12, url5))
        met6 = MetaNode(self.mkid("id"))
        # inserter.insertrange(Range(10-3, 11-3, met6))
        inserter.insertrange(Range(10, 11, met6))
        root = TreeNode()
        root.appendchild(met2.dup()
                .appendchild(TextNode("1"))
                .appendchild(url1.dup().appendchild(TextNode("2"))))
        root.appendchild(met4.dup()
                .appendchild(url1.dup().appendchild(TextNode("3"))))
        root.appendchild(met3.dup()
                .appendchild(url1.dup().appendchild(TextNode("4")))
                .appendchild(TextNode("5")))
        root.appendchild(TextNode("6"))
        root.appendchild(url5.dup().appendchild(TextNode("7")))
        root.appendchild(met6.dup()
                .appendchild(url5.dup().appendchild(TextNode("8"))))
        root.appendchild(url5.dup().appendchild(TextNode("9")))
        self.dotest(root, False)

    def test_range_ruby_meta(self):
        inserter = RangeInserter(self.__class__.xDoc)
        text = TextNode("123456789")
        inserter.insertrange(Range(0, 0, text))
        rby1 = RubyNode(self.mkname("ruby"))
        inserter.insertrange(Range(1, 4, rby1))
        ## overlap left
        met2 = MetaNode(self.mkid("id"))
        inserter.insertrange(Range(0, 2, met2))
        root = TreeNode()
        root.appendchild(met2.dup()
                .appendchild(TextNode("1"))
                .appendchild(rby1.dup().appendchild(TextNode("2"))))
        root.appendchild(rby1.dup().appendchild(TextNode("34")))
        root.appendchild(TextNode("56789"))
        self.dotest(root, False)
        ## overlap right
        met3 = MetaNode(self.mkid("id"))
        # inserter.insertrange(Range(4-1, 6-1, met3))
        inserter.insertrange(Range(4, 6, met3))
        root = TreeNode()
        root.appendchild(met2.dup()
                .appendchild(TextNode("1"))
                .appendchild(rby1.dup().appendchild(TextNode("2"))))
        root.appendchild(rby1.dup().appendchild(TextNode("3")))
        root.appendchild(met3.dup()
                .appendchild(rby1.dup().appendchild(TextNode("4")))
                .appendchild(TextNode("5")))
        root.appendchild(TextNode("6789"))
        self.dotest(root, False)
        ## around
        met4 = MetaNode(self.mkid("id"))
        # inserter.insertrange(Range(3-1, 4-1, met4))
        inserter.insertrange(Range(3, 4, met4))
        root = TreeNode()
        root.appendchild(met2.dup()
                .appendchild(TextNode("1"))
                .appendchild(rby1.dup().appendchild(TextNode("2"))))
        root.appendchild(met4.dup()
                .appendchild(rby1.dup().appendchild(TextNode("3"))))
        root.appendchild(met3.dup()
                .appendchild(rby1.dup().appendchild(TextNode("4")))
                .appendchild(TextNode("5")))
        root.appendchild(TextNode("6789"))
        self.dotest(root, False)
        ## inside
        rby5 = RubyNode(self.mkname("ruby"))
        # inserter.insertrange(Range(9-3, 12-3, rby5))
        inserter.insertrange(Range(9, 12, rby5))
        met6 = MetaNode(self.mkid("id"))
        # inserter.insertrange(Range(10-3, 11-3, met6))
        inserter.insertrange(Range(10, 11, met6))
        root = TreeNode()
        root.appendchild(met2.dup()
                .appendchild(TextNode("1"))
                .appendchild(rby1.dup().appendchild(TextNode("2"))))
        root.appendchild(met4.dup()
                .appendchild(rby1.dup().appendchild(TextNode("3"))))
        root.appendchild(met3.dup()
                .appendchild(rby1.dup().appendchild(TextNode("4")))
                .appendchild(TextNode("5")))
        root.appendchild(TextNode("6"))
        root.appendchild(rby5.dup()
                .appendchild(TextNode("7"))
                .appendchild(met6.dup()
                    .appendchild(TextNode("8")))
                .appendchild(TextNode("9")))
        self.dotest(root, False)

    def test_range_meta_hyperlink(self):
        inserter = RangeInserter(self.__class__.xDoc)
        text = TextNode("123456789")
        inserter.insertrange(Range(0, 0, text))
        met1 = MetaNode(self.mkid("id"))
        inserter.insertrange(Range(1, 6, met1))
        ## overlap left
        url2 = HyperlinkNode(self.mkname("url"))
        # inserter.insertrange(Range(0, 4-1, url2))
        inserter.insertrange(Range(0, 4, url2))
        root = TreeNode()
        root.appendchild(url2.dup().appendchild(TextNode("1")))
        root.appendchild(met1.dup()
                .appendchild(url2.dup().appendchild(TextNode("23")))
                .appendchild(TextNode("456")))
        root.appendchild(TextNode("789"))
        self.dotest(root, False)
        ## overlap right
        url3 = HyperlinkNode(self.mkname("url"))
        # inserter.insertrange(Range(6-1, 8-1, url3))
        inserter.insertrange(Range(6, 8, url3))
        root = TreeNode()
        root.appendchild(url2.dup().appendchild(TextNode("1")))
        root.appendchild(met1.dup()
                .appendchild(url2.dup().appendchild(TextNode("23")))
                .appendchild(TextNode("45"))
                .appendchild(url3.dup().appendchild(TextNode("6"))))
        root.appendchild(url3.dup().appendchild(TextNode("7")))
        root.appendchild(TextNode("89"))
        self.dotest(root, False)
        ## around (not quite, due to API)
        url4 = HyperlinkNode(self.mkname("url"))
        # inserter.insertrange(Range(1, 9-1, url4))
        inserter.insertrange(Range(1, 9, url4))
        root = TreeNode()
        root.appendchild(url2.dup().appendchild(TextNode("1")))
        root.appendchild(met1.dup()
                .appendchild(url4.dup()
                    .appendchild(TextNode("23456"))))
        root.appendchild(url4.dup().appendchild(TextNode("78")))
        root.appendchild(TextNode("9"))
        self.dotest(root, False)
        ## inside
        url5 = HyperlinkNode(self.mkname("url"))
        # inserter.insertrange(Range(4-1, 6-1, url5))
        inserter.insertrange(Range(4, 6, url5))
        root = TreeNode()
        root.appendchild(url2.dup().appendchild(TextNode("1")))
        root.appendchild(met1.dup()
                .appendchild(url4.dup()
                    .appendchild(TextNode("23")))
                .appendchild(url5.dup()
                    .appendchild(TextNode("45")))
                .appendchild(url4.dup()
                    .appendchild(TextNode("6"))))
        root.appendchild(url4.dup().appendchild(TextNode("78")))
        root.appendchild(TextNode("9"))
        self.dotest(root, False)

    def test_range_meta_ruby(self):
        inserter = RangeInserter(self.__class__.xDoc)
        text = TextNode("123456789")
        inserter.insertrange(Range(0, 0, text))
        met1 = MetaNode(self.mkid("id"))
        inserter.insertrange(Range(1, 5, met1))
        ## overlap left
        rby2 = RubyNode(self.mkname("ruby"))
        # inserter.insertrange(Range(0, 3-1, rby2))
        inserter.insertrange(Range(0, 3, rby2))
        root = TreeNode()
        root.appendchild(rby2.dup().appendchild(TextNode("1")))
        root.appendchild(met1.dup()
                .appendchild(rby2.dup().appendchild(TextNode("2")))
                .appendchild(TextNode("345")))
        root.appendchild(TextNode("6789"))
        self.dotest(root, False)
        ## overlap right
        rby3 = RubyNode(self.mkname("ruby"))
        # inserter.insertrange(Range(5-1, 7-1, rby3))
        inserter.insertrange(Range(5, 7, rby3))
        root = TreeNode()
        root.appendchild(rby2.dup().appendchild(TextNode("1")))
        root.appendchild(met1.dup()
                .appendchild(rby2.dup().appendchild(TextNode("2")))
                .appendchild(TextNode("34"))
                .appendchild(rby3.dup().appendchild(TextNode("5"))))
        root.appendchild(rby3.dup().appendchild(TextNode("6")))
        root.appendchild(TextNode("789"))
        self.dotest(root, False)
        ## // around
        rby4 = RubyNode(self.mkname("ruby"))
        # inserter.insertrange(Range(1, 7-1, rby4))
        inserter.insertrange(Range(1, 7, rby4))
        root = TreeNode()
        root.appendchild(rby2.dup().appendchild(TextNode("1")))
        root.appendchild(rby4.dup()
                .appendchild(met1.dup()
                    .appendchild(TextNode("2345")))
                .appendchild(TextNode("6")))
        root.appendchild(TextNode("789"))
        self.dotest(root, False)
        ## inside
        met5 = MetaNode(self.mkid("id"))
        # inserter.insertrange(Range(7-1, 9-1, met5))
        inserter.insertrange(Range(7, 9, met5))
        rby6 = RubyNode(self.mkname("ruby"))
        # inserter.insertrange(Range(9-2, 10/-2, rby6))
        inserter.insertrange(Range(9, 10, rby6))
        root = TreeNode()
        root.appendchild(rby2.dup().appendchild(TextNode("1")))
        root.appendchild(rby4.dup()
                .appendchild(met1.dup()
                    .appendchild(TextNode("2345")))
                .appendchild(TextNode("6")))
        root.appendchild(met5.dup()
                .appendchild(TextNode("7"))
                .appendchild(rby6.dup()
                    .appendchild(TextNode("8"))))
        root.appendchild(TextNode("9"))
        self.dotest(root, False)
        ## inside, with invalid range that includes the dummy char
        rby7 = RubyNode(self.mkname("ruby"))
        # inserter.insertrange(Range(7-1, 9-2, rby7))
        inserter.insertrange(Range(7, 9, rby7))
        root = TreeNode()
        root.appendchild(rby2.dup().appendchild(TextNode("1")))
        root.appendchild(rby4.dup()
                .appendchild(met1.dup()
                    .appendchild(TextNode("2345")))
                .appendchild(TextNode("6")))
        root.appendchild(met5.dup()
                .appendchild(rby7.dup()
                    .appendchild(TextNode("7")))
                .appendchild(rby6.dup()
                    .appendchild(TextNode("8"))))
        root.appendchild(TextNode("9"))
        self.dotest(root, False)
        ## around, at same position as meta
        rby8 = RubyNode(self.mkname("ruby"))
        # inserter.insertrange(Range(7-1, 10-2, rby8))
        inserter.insertrange(Range(7, 10, rby8))
        root = TreeNode()
        root.appendchild(rby2.dup().appendchild(TextNode("1")))
        root.appendchild(rby4.dup()
                .appendchild(met1.dup()
                    .appendchild(TextNode("2345")))
                .appendchild(TextNode("6")))
        root.appendchild(rby8.dup()
                .appendchild(met5.dup()
                    .appendchild(TextNode("78"))))
        root.appendchild(TextNode("9"))
        self.dotest(root, False)

    def test_range_meta_meta(self):
        inserter = RangeInserter(self.__class__.xDoc)
        text = TextNode("123456789")
        inserter.insertrange(Range(0, 0, text))
        met1 = MetaNode(self.mkid("id"))
        inserter.insertrange(Range(3, 6, met1))
        ## overlap left
        met2 = MetaNode(self.mkid("id"))
        try:
            inserter.insertrange(Range(0, 4, met2))
            fail("testRangeMetaMeta: overlap left allowed")
        except IllegalArgumentException:
            pass
        root = TreeNode()
        root.appendchild(TextNode("123"))
        root.appendchild(met1.dup().appendchild(TextNode("456")))
        root.appendchild(TextNode("789"))
        self.dotest(root, False)
        ## overlap right
        met3 = MetaNode(self.mkid("id"))

        try:
            # inserter.insertrange(Range(5-1, 8-1, met3))
            inserter.insertrange(Range(5, 8, met3))
            self.fail("testRangeMetaMeta: overlap right allowed")
        except IllegalArgumentException:
            pass

        root = TreeNode()
        root.appendchild(TextNode("123"))
        root.appendchild(met1.dup().appendchild(TextNode("456")))
        root.appendchild(TextNode("789"))
        self.dotest(root, False)
        ## around
        met4 = MetaNode(self.mkid("id"))
        # inserter.insertrange(Range(3, 7-1, met4))
        inserter.insertrange(Range(3, 7, met4))
        root = TreeNode()
        root.appendchild(TextNode("123"))
        root.appendchild(met4.dup()
                .appendchild(met1.dup().appendchild(TextNode("456"))))
        root.appendchild(TextNode("789"))
        self.dotest(root, False)
        ## inside
        met5 = MetaNode(self.mkid("id"))
        # inserter.insertrange(Range(6-2, 8-2, met5))
        inserter.insertrange(Range(6, 8, met5))
        root = TreeNode()
        root.appendchild(TextNode("123"))
        root.appendchild(met4.dup()
                .appendchild(met1.dup()
                    .appendchild(TextNode("4"))
                    .appendchild(met5.dup()
                        .appendchild(TextNode("56")))))
        root.appendchild(TextNode("789"))
        self.dotest(root, False)

    def test_range2(self):
        inserter = RangeInserter(self.__class__.xDoc)
        text = TextNode("123456789")
        inserter.insertrange(Range(0, 0, text))
        met1 = MetaNode(self.mkid("id"))
        inserter.insertrange(Range(1, 8, met1))
        met2 = MetaNode(self.mkid("id"))
        # inserter.insertrange(Range(3-1, 8-1, met2))
        inserter.insertrange(Range(3, 8, met2))
        met3 = MetaNode(self.mkid("id"))
        # inserter.insertrange(Range(5-2, 8-2, met3))
        inserter.insertrange(Range(5, 8, met3))
        root = TreeNode()
        root.appendchild(TextNode("1"))
        root.appendchild(met1.dup()
                .appendchild(TextNode("2"))
                .appendchild(met2.dup()
                    .appendchild(TextNode("3"))
                    .appendchild(met3.dup()
                        .appendchild(TextNode("456")))
                    .appendchild(TextNode("7")))
                .appendchild(TextNode("8")))
        root.appendchild(TextNode("9"))
        self.dotest(root, False)
        ## split ruby at every meta start!
        rby4 = RubyNode(self.mkname("ruby"))
        # inserter.insertrange(Range(0, 7-3, rby4))
        inserter.insertrange(Range(0, 7, rby4))
        root = TreeNode()
        root.appendchild(rby4.dup()
                .appendchild(TextNode("1")))
        root.appendchild(met1.dup()
                .appendchild(rby4.dup()
                    .appendchild(TextNode("2")))
                .appendchild(met2.dup()
                    .appendchild(rby4.dup()
                        .appendchild(TextNode("3")))
                    .appendchild(met3.dup()
                        .appendchild(rby4.dup()
                            .appendchild(TextNode("4")))
                        .appendchild(TextNode("56")))
                    .appendchild(TextNode("7")))
                .appendchild(TextNode("8")))
        root.appendchild(TextNode("9"))
        self.dotest(root, False)
        ## split ruby at every meta end!
        rby5 = RubyNode(self.mkname("ruby"))
        # inserter.insertrange(Range(8-3, 12-3, rby5))
        inserter.insertrange(Range(8, 12, rby5))
        root = TreeNode()
        root.appendchild(rby4.dup()
                .appendchild(TextNode("1")))
        root.appendchild(met1.dup()
                .appendchild(rby4.dup()
                    .appendchild(TextNode("2")))
                .appendchild(met2.dup()
                    .appendchild(rby4.dup()
                        .appendchild(TextNode("3")))
                    .appendchild(met3.dup()
                        .appendchild(rby4.dup()
                            .appendchild(TextNode("4")))
                        .appendchild(TextNode("5"))
                        .appendchild(rby5.dup()
                            .appendchild(TextNode("6"))))
                    .appendchild(rby5.dup()
                        .appendchild(TextNode("7"))))
                .appendchild(rby5.dup()
                    .appendchild(TextNode("8"))))
        root.appendchild(rby5.dup()
                .appendchild(TextNode("9")))
        self.dotest(root, False)

    def test_range3(self):
        inserter = RangeInserter(self.__class__.xDoc)
        text = TextNode("123456789")
        inserter.insertrange(Range(0, 0, text))
        rby1 = RubyNode(self.mkname("ruby"))
        inserter.insertrange(Range(0, 9, rby1))
        met2 = MetaNode(self.mkid("id"))
        inserter.insertrange(Range(2, 7, met2))
        root = TreeNode()
        root.appendchild(rby1.dup()
                .appendchild(TextNode("12"))
                .appendchild(met2.dup()
                    .appendchild(TextNode("34567")))
                .appendchild(TextNode("89")))
        self.dotest(root, False)
        ## overwrite outer ruby, split remains at inner meta!
        rby3 = RubyNode(self.mkname("ruby"))
        # inserter.insertrange(Range(5-1, 6-1, rby3))
        inserter.insertrange(Range(5, 6, rby3))
        root = TreeNode()
        root.appendchild(rby1.dup()
                .appendchild(TextNode("12")))
        root.appendchild(met2.dup()
                .appendchild(rby1.dup()
                    .appendchild(TextNode("34")))
                .appendchild(rby3.dup()
                    .appendchild(TextNode("5")))
                .appendchild(rby1.dup()
                    .appendchild(TextNode("67"))))
        root.appendchild(rby1.dup()
                .appendchild(TextNode("89")))
        self.dotest(root, False)

    def test_range4(self):
        inserter = RangeInserter(self.__class__.xDoc)
        text = TextNode("123456789")
        inserter.insertrange(Range(0, 0, text))
        rby1 = RubyNode(self.mkname("ruby"))
        inserter.insertrange(Range(0, 9, rby1))
        met2 = MetaNode(self.mkid("id"))
        inserter.insertrange(Range(1, 8, met2))
        met3 = MetaNode(self.mkid("id"))
        # inserter.insertrange(Range(3-1, 8-1, met3))
        inserter.insertrange(Range(3, 8, met3))
        met4 = MetaNode(self.mkid("id"))
        # inserter.insertrange(Range(5-2, 8-2, met4))
        inserter.insertrange(Range(5, 8, met4))
        root = TreeNode()
        root.appendchild(rby1.dup()
                .appendchild(TextNode("1"))
                .appendchild(met2.dup()
                    .appendchild(TextNode("2"))
                    .appendchild(met3.dup()
                        .appendchild(TextNode("3"))
                        .appendchild(met4.dup()
                            .appendchild(TextNode("456")))
                        .appendchild(TextNode("7")))
                    .appendchild(TextNode("8")))
                .appendchild(TextNode("9")))
        self.dotest(root, False)
        ## overwrite outer ruby, split remains at every inner meta!
        rby5 = RubyNode(self.mkname("ruby"))
        # inserter.insertrange(Range(7-3, 8-3, rby5))
        inserter.insertrange(Range(7, 8, rby5))
        root = TreeNode()
        root.appendchild(rby1.dup()
                .appendchild(TextNode("1")))
        root.appendchild(met2.dup()
                .appendchild(rby1.dup()
                    .appendchild(TextNode("2")))
                .appendchild(met3.dup()
                    .appendchild(rby1.dup()
                        .appendchild(TextNode("3")))
                    .appendchild(met4.dup()
                        .appendchild(rby1.dup()
                            .appendchild(TextNode("4")))
                        .appendchild(rby5.dup()
                            .appendchild(TextNode("5")))
                        .appendchild(rby1.dup()
                            .appendchild(TextNode("6"))))
                    .appendchild(rby1.dup()
                        .appendchild(TextNode("7"))))
                .appendchild(rby1.dup()
                    .appendchild(TextNode("8"))))
        root.appendchild(rby1.dup()
                .appendchild(TextNode("9")))
        self.dotest(root, False)

    def test_range5(self):
        inserter = RangeInserter(self.__class__.xDoc)
        text = TextNode("123456789")
        inserter.insertrange(Range(0, 0, text))
        rby1 = RubyNode(self.mkname("ruby"))
        inserter.insertrange(Range(0, 9, rby1))
        met2 = MetaNode(self.mkid("id"))
        inserter.insertrange(Range(1, 3, met2))
        met3 = MetaNode(self.mkid("id"))
        # inserter.insertrange(Range(5-1, 6-1, met3))
        inserter.insertrange(Range(5, 6, met3))
        met4 = MetaNode(self.mkid("id"))
        # inserter.insertrange(Range(8-2, 10-2, met4))
        inserter.insertrange(Range(8, 10, met4))
        root = TreeNode()
        root.appendchild(rby1.dup()
                .appendchild(TextNode("1"))
                .appendchild(met2.dup().appendchild(TextNode("23")))
                .appendchild(TextNode("4"))
                .appendchild(met3.dup().appendchild(TextNode("5")))
                .appendchild(TextNode("6"))
                .appendchild(met4.dup().appendchild(TextNode("78")))
                .appendchild(TextNode("9")))
        self.dotest(root, False)
        ## overwrite outer ruby, but split at inner metas!
        rby5 = RubyNode(self.mkname("ruby"))
        # inserter.insertrange(Range(3-1, 10-3, rby5))
        inserter.insertrange(Range(3, 10, rby5))
        root = TreeNode()
        root.appendchild(rby1.dup()
                .appendchild(TextNode("1")))
        root.appendchild(met2.dup()
                .appendchild(rby1.dup()
                    .appendchild(TextNode("2")))
                .appendchild(rby5.dup()
                    .appendchild(TextNode("3"))))
        root.appendchild(rby5.dup()
                .appendchild(TextNode("4"))
                .appendchild(met3.dup()
                    .appendchild(TextNode("5")))
                .appendchild(TextNode("6")))
        root.appendchild(met4.dup()
                .appendchild(rby5.dup()
                    .appendchild(TextNode("7")))
                .appendchild(rby1.dup()
                    .appendchild(TextNode("8"))))
        root.appendchild(rby1.dup()
                .appendchild(TextNode("9")))
        self.dotest(root, False)

    def test_range6(self):
        inserter = RangeInserter(self.__class__.xDoc)
        text = TextNode("123456789")
        inserter.insertrange(Range(0, 0, text))
        met1 = MetaNode(self.mkid("id"))
        inserter.insertrange(Range(1, 5, met1))
        met2 = MetaNode(self.mkid("id"))
        # inserter.insertrange(Range(3-1, 6-1, met2))
        inserter.insertrange(Range(3, 6, met2))
        met3 = MetaNode(self.mkid("id"))
        # inserter.insertrange(Range(5-2, 7-2, met3))
        inserter.insertrange(Range(5, 7, met3))
        root = TreeNode()
        root.appendchild(TextNode("1"))
        root.appendchild(met1.dup()
                .appendchild(TextNode("2"))
                .appendchild(met2.dup()
                    .appendchild(TextNode("3"))
                    .appendchild(met3.dup()
                        .appendchild(TextNode("45")))))
        root.appendchild(TextNode("6789"))
        self.dotest(root, False)
        ## split at 3 metas, all at same position
        rby4 = RubyNode(self.mkname("ruby"))
        # inserter.insertrange(Range(7-3, 10-3, rby4))
        inserter.insertrange(Range(7, 10, rby4))
        root = TreeNode()
        root.appendchild(TextNode("1"))
        root.appendchild(met1.dup()
                .appendchild(TextNode("2"))
                .appendchild(met2.dup()
                    .appendchild(TextNode("3"))
                    .appendchild(met3.dup()
                        .appendchild(TextNode("4"))
                        .appendchild(rby4.dup()
                            .appendchild(TextNode("5"))))))
        root.appendchild(rby4.dup()
                .appendchild(TextNode("67")))
        root.appendchild(TextNode("89"))
        self.dotest(root, False)

    def test_range7(self):
        inserter = RangeInserter(self.__class__.xDoc)
        text = TextNode("123456789")
        inserter.insertrange(Range(0, 0, text))
        url1 = HyperlinkNode(self.mkname("url"))
        inserter.insertrange(Range(1, 5, url1))
        met2 = MetaNode(self.mkid("id"))
        inserter.insertrange(Range(3, 5, met2))
        root = TreeNode()
        root.appendchild(TextNode("1"))
        root.appendchild(url1.dup()
                .appendchild(TextNode("23")))
        root.appendchild(met2.dup()
                .appendchild(url1.dup()
                    .appendchild(TextNode("45"))))
        root.appendchild(TextNode("6789"))
        self.dotest(root, False)
        ## this should result in not splitting the hyperlink, but due to API
        ## we can't tell :(
        rby3 = RubyNode(self.mkname("ruby"))
        # inserter.insertrange(Range(5-1, 8-1, rby3))
        inserter.insertrange(Range(5, 8, rby3))
        root = TreeNode()
        root.appendchild(TextNode("1"))
        root.appendchild(url1.dup()
                .appendchild(TextNode("23")))
        root.appendchild(met2.dup()
                .appendchild(url1.dup()
                    .appendchild(TextNode("4")))
                .appendchild(rby3.dup()
                    .appendchild(url1.dup()
                        .appendchild(TextNode("5")))))
        root.appendchild(rby3.dup()
                .appendchild(TextNode("67")))
        root.appendchild(TextNode("89"))
        self.dotest(root, False)

    # TODO: test partial selection, test UNDO/REDO

    ##i109601# NestedTextContent and XChild
    def test_meta_xchild(self):
        xDoc = self.__class__.xDoc
        id1 = StringPair("content.xml", self.mkname("id"))
        id2 = StringPair("content.xml", self.mkname("id"))
        id3 = StringPair("content.xml", self.mkname("id"))
        id4 = StringPair("content.xml", self.mkname("id"))
        id5 = StringPair("content.xml", self.mkname("id"))
        id6 = StringPair("content.xml", self.mkname("id"))
        meta1 = MetaNode(id1)
        meta2 = MetaNode(id2)
        meta3 = MetaFieldNode(id3)
        meta4 = MetaNode(id4)
        meta5 = MetaNode(id5)
        meta6 = MetaFieldNode(id6)
        root = TreeNode()
        root.appendchild(meta1.dup()
                .appendchild(TextNode("1")))
        root.appendchild(TextNode("2"))
        root.appendchild(meta2.dup()
                .appendchild(meta3.dup()
                    .appendchild(TextNode("34"))
                    .appendchild(meta4.dup()
                        .appendchild(TextNode("56")))
                    .appendchild(meta5.dup())
                    .appendchild(TextNode("7"))))
        root.appendchild(TextNode("8"))
        root.appendchild(meta6.dup()
                .appendchild(TextNode("9")))

        inserter = RangeInserter(xDoc)
        text = TextNode("123456789")
        inserter.insertrange(Range(0, 0, text))
        xMeta1 = inserter.insertrange(Range(0, 1, meta1))
        xMeta2 = inserter.insertrange(Range(3, 8, meta2))
        xMeta3 = inserter.insertrange(Range(4, 9, meta3))
        xMeta4 = inserter.insertrange(Range(7, 9, meta4))
        xMeta5 = inserter.insertrange(Range(10, 10, meta5))
        xMeta6 = inserter.insertrange(Range(13, 14, meta6))

        self.dotest(root, False)

        xDocText = xDoc.getText()
        xDocTextCursor = xDocText.createTextCursor()
        xDocTextCursor.gotoNextParagraph(False) # second paragraph
        #  X12XX34X56X78X9
        #  1  23  4  5  6
        #   1       452  6
        #             3
        nestedTextContent = (
            None,
            id1,
            id1,
            None,
            id2,
            id3,
            id3,
            id3,
            id4,
            id4,
            id4,
            id5,
            id3,
            None,
            id6,
            id6)
        for i, ntc in enumerate(nestedTextContent):
            oNTC = xDocTextCursor.NestedTextContent
            if ntc is None:
                self.assertIsNone(oNTC,
                            "unexpected NestedTextContent at: {}".format(i))
            else:
                xmlid = oNTC.MetadataReference
                self.assertTrue(MetaNode.eq(ntc, xmlid),
                            "wrong NestedTextContent at: {}".format(i))
            xDocTextCursor.goRight(1, False)

        try:
            xMeta1.setParent(xMeta4)
            fail("setParent(): allowed?")
        except NoSupportException:
            pass
        self.assertIsNone(xMeta1.getParent(), "getParent(): not None")
        self.assertIsNone(xMeta2.getParent(), "getParent(): not None")
        self.assertIsNone(xMeta6.getParent(), "getParent(): not None")

        xParent3 = xMeta3.getParent()
        self.assertIsNotNone(xParent3, "getParent(): None")
        xmlid = xParent3.MetadataReference
        self.assertTrue(MetaNode.eq(xmlid, id2), "getParent(): wrong")

        xParent4 = xMeta4.getParent()
        self.assertIsNotNone(xParent4, "getParent(): None")
        xmlid = xParent4.MetadataReference
        self. assertTrue(MetaNode.eq(xmlid, id3), "getParent(): wrong")

        xParent5 = xMeta5.getParent()
        self.assertIsNotNone(xParent5, "getParent(): None")
        xmlid = xParent5.MetadataReference
        self.assertTrue(MetaNode.eq(xmlid, id3), "getParent(): wrong")

    # test SwXMeta XText interface
    def test_meta_xtext(self):
        xDoc = self.__class__.xDoc
        inserter = RangeInserter(xDoc)
        text = TextNode("12AB6789")
        inserter.insertrange(Range(0, 0, text))
        meta = MetaNode(self.mkid("id"))
        xMeta = inserter.makemeta()

        xDocText = xDoc.getText()
        xDocTextCursor = xDocText.createTextCursor()
        xDocTextCursor.goRight(3, False)
        xDocTextCursor.goRight(2, True)
        xDocText.insertTextContent(xDocTextCursor, xMeta, True)

        xMeta.MetadataReference = meta.xmlid
        xParentText = xMeta.getText()
        self.assertIsNotNone(xParentText, "getText(): no parent")

        xStart = xMeta.getStart()
        self.assertIsNotNone(xStart, "getStart(): no start")

        xEnd = xMeta.getEnd()
        self.assertIsNotNone(xEnd, "getEnd(): no end")

        xMeta.setString("45")

        string = xMeta.getString()
        self.assertEqual("45", string, "getString(): invalid string returned")

        xTextCursor = xMeta.createTextCursor()
        self.assertIsNotNone(xTextCursor, "createTextCursor(): failed")

        try:
            xMeta.createTextCursorByRange(None)
            fail("createTextCursorByRange(): None allowed?")
        except RuntimeException:
            pass

        xTextCursorStart = xMeta.createTextCursorByRange(xStart)
        self.assertIsNotNone(xTextCursorStart,
                    "createTextCursorByRange(): failed for start")

        xTextCursorEnd = xMeta.createTextCursorByRange(xEnd)
        self.assertIsNotNone(xTextCursorEnd,
                             "createTextCursorByRange(): failed for end")

        ## move outside meta
        xDocTextCursor.gotoStart(False)

        try:
            xMeta.insertString(None, "foo", False)
            fail("insertString(): None allowed?")
        except RuntimeException:
            pass

        try:
            xMeta.insertString(xDocTextCursor, "foo", False)
            fail("insertString(): cursor outside allowed?")
        except RuntimeException:
            pass

        xStart = xMeta.getStart()
        xMeta.insertString(xStart, "A", False)
        string = xMeta.getString()
        self.assertEqual("A45", string, "getString(): invalid string returned")

        xMeta.insertString(xEnd, "B", False)
        string = xMeta.getString()
        self.assertEqual("A45B", string, "getString(): invalid string returned")

        try:
            xMeta.insertControlCharacter(None, HARD_HYPHEN, False)
            fail("insertControlCharacter(): None allowed?")
        except IllegalArgumentException:
            pass

        xStart = xMeta.getStart()
        try:
            xMeta.insertControlCharacter(xDocTextCursor, HARD_HYPHEN, False)
            fail("insertControlCharacter(): cursor outside allowed?")
        except IllegalArgumentException:
            pass

        xMeta.insertControlCharacter(xStart, HARD_HYPHEN, False)
        string = xMeta.getString()
        self.assertEqual('\u2011' + 'A45B', string,
                                "getString(): invalid string returned")

        xMeta.insertControlCharacter(xEnd, HARD_HYPHEN, False)
        string = xMeta.getString()
        self.assertEqual('\u2011' + 'A45B' + '\u2011', string,
                                "getString(): invalid string returned")

        xMeta.setString("45")
        try:
            xMeta.insertTextContent(None, xMeta, False)
            fail("insertTextContent(): None range allowed?")
        except IllegalArgumentException:
            pass

        try:
            xMeta.insertTextContent(xStart, None, False)
            fail("insertTextContent(): None content allowed?")
        except IllegalArgumentException:
            pass

        try:
            xMeta.insertTextContent(xDocTextCursor, xMeta, False)
            fail("insertTextContent(): cursor outside allowed?")
        except IllegalArgumentException:
            pass

        field1 = TextFieldNode("f1")
        field2 = TextFieldNode("f2")
        xField1 = inserter.maketextfield(field1.content)
        xField2 = inserter.maketextfield(field2.content)

        xStart = xMeta.getStart()
        xMeta.insertTextContent(xStart, xField1, False)

        root = TreeNode()
        root.appendchild(TextNode("12"))
        root.appendchild(meta.dup()
                .appendchild(field1.dup())
                .appendchild(TextNode("45")))
        root.appendchild(TextNode("6789"))
        self.dotest(root, False)

        xMeta.insertTextContent(xEnd, xField2, False)

        root = TreeNode()
        root.appendchild(TextNode("12"))
        root.appendchild(meta.dup()
                .appendchild(field1.dup())
                .appendchild(TextNode("45"))
                .appendchild(field2.dup()))
        root.appendchild(TextNode("6789"))
        self.dotest(root, False)

        try:
            xMeta.removeTextContent(None)
            fail("removeTextContent(): None content allowed?")
        except RuntimeException:
            pass

        xMeta.removeTextContent(xField1)

        xAnchor = xMeta.getAnchor()
        self.assertIsNotNone(xAnchor, "getAnchor(): None")

        ## evil test case: insert ruby around meta
        ruby = RubyNode(self.mkname("ruby"))
        inserter.insertrange(Range(2, 6, ruby))

        ## prevent caching...
        # root = TreeNode()
        # root.appendchild(TextNode("12"))
        # root.appendchild(ruby.dup()
                # .appendchild(meta.dup()
                    # .appendchild(TextNode("45"))
                    # .appendchild(field2.dup())))
        # root.appendchild(TextNode("6789"))
        # self.dotest(root, False)

        xEnum = xMeta.createEnumeration()
        self.assertIsNotNone("createEnumeration(): returns None", xEnum)

        self.assertTrue(xEnum.hasMoreElements(),"hasNext(): first missing")
        xPortion = xEnum.nextElement()
        type_ = xPortion.TextPortionType
        self.assertEqual("Text", type_, "first: not text")
        txt = xPortion.getString()
        self.assertEqual("45", txt, "first: text differs")

        self.assertTrue(xEnum.hasMoreElements(),"hasNext(): second missing")
        xPortion = xEnum.nextElement()
        type_ = xPortion.TextPortionType
        self.assertEqual("TextField", type_, "second: not text")

        ## no ruby end here!!!
        self.assertFalse(xEnum.hasMoreElements(), "hasNext(): more elements?")

        xMeta.dispose()

        try:
            xCursor = xMeta.createTextCursor()
            self.assertIsNone(xCursor,
                        "createTextCursor(): succeeds on disposed object?")
        except RuntimeException:
            pass

    # check that cursor move methods move to positions in the meta,
    # but do not move to positions outside the meta.
    def test_meta_xtextcursor(self):
        xDoc = self.__class__.xDoc
        inserter = RangeInserter(xDoc)
        text = TextNode("Text. 12 More text here.")
        inserter.insertrange(Range(0, 0, text))
        met1 = MetaNode(self.mkid("id"))
        xMeta = inserter.makemeta()

        xDocText = xDoc.getText()
        xDocTextCursor = xDocText.createTextCursor()
        xDocTextCursor.goRight(7, False)
        xDocTextCursor.goRight(2, True)
        xDocText.insertTextContent(xDocTextCursor, xMeta, True)
        xDocTextCursor.gotoStart(True)

        xMeta.MetadataReference = met1.xmlid
        xStart = xMeta.getStart()
        self.assertIsNotNone(xStart, "getStart(): no start")
        xEnd = xMeta.getEnd()
        self.assertIsNotNone(xEnd, "getEnd(): no end")

        ## XTextCursor
        xMetaCursor = xMeta.createTextCursor()
        self.assertIsNotNone(xMetaCursor, "createTextCursor(): no cursor")
        bSuccess = False
        xMetaCursor.gotoStart(False)
        xMetaCursor.gotoEnd(False)
        bSuccess = xMetaCursor.goLeft(1, False)
        self.assertTrue(bSuccess, "goLeft(): failed")
        bSuccess = xMetaCursor.goLeft(1000, False)
        self.assertFalse(bSuccess, "goLeft(): succeeded")
        bSuccess = xMetaCursor.goRight(1, False)
        self.assertTrue(bSuccess, "goRight(): failed")
        bSuccess = xMetaCursor.goRight(1000, False)
        self.assertFalse(bSuccess, "goRight(): succeeded")
        xMetaCursor.gotoRange(xStart, False)
        xMetaCursor.gotoRange(xEnd, False)
        try:
            xMetaCursor.gotoRange(xDocTextCursor, False)
            fail("gotoRange(): succeeded")
        except RuntimeException:
            pass

        ## XWordCursor
        xMeta.setString("Two words")
        xMetaCursor.gotoStart(False)

        bSuccess = xMetaCursor.gotoNextWord(True)  # at start of "words"
        self.assertTrue(bSuccess, "gotoNextWord(): failed")

        string = xMetaCursor.getString()
        self.assertEqual("Two ", string, "gotoNextWord(): wrong string")

        bSuccess = xMetaCursor.gotoNextWord(False)  # at end of "words", cannot leave metafield
        self.assertFalse(bSuccess,"gotoNextWord(): succeeded")
        xMetaCursor.collapseToEnd()
        bSuccess = xMetaCursor.gotoPreviousWord(True)  # at start of "words"
        self.assertTrue(bSuccess, "gotoPreviousWord(): failed")

        string = xMetaCursor.getString()
        self.assertEqual("words", string, "gotoPreviousWord(): wrong string")

        bSuccess = xMetaCursor.gotoPreviousWord(False)  # at start of "Two"
        self.assertTrue(bSuccess, "gotoPreviousWord(): failed")

        bSuccess = xMetaCursor.gotoPreviousWord(False)  # cannot leave metafield
        self.assertFalse(bSuccess, "gotoPreviousWord(): succeeded")

        bSuccess = xMetaCursor.gotoEndOfWord(True)  # at end of "Two"
        self.assertTrue(bSuccess, "gotoEndOfWord(): failed")

        string = xMetaCursor.getString()
        self.assertEqual("Two", string, "gotoEndOfWord(): wrong string")

        xMetaCursor.gotoEnd(False)
        bSuccess = xMetaCursor.gotoStartOfWord(True)
        self.assertTrue(bSuccess, "gotoStartOfWord(): failed")

        string = xMetaCursor.getString()
        self.assertEqual("words", string, "gotoStartOfWord(): wrong string")

        xMeta.setString("")
        bSuccess = xMetaCursor.gotoEndOfWord(False)
        self.assertFalse(bSuccess, "gotoEndOfWord(): succeeded")
        bSuccess = xMetaCursor.gotoStartOfWord(False)
        self.assertFalse(bSuccess, "gotoStartOfWord(): succeeded")

        ## XSentenceCursor
        xMeta.setString("This is a sentence. Another sentence.")
        xMetaCursor.gotoStart(False)

        bSuccess = xMetaCursor.gotoNextSentence(True)
        self.assertTrue(bSuccess,"gotoNextSentence(): failed")

        string = xMetaCursor.getString()
        self.assertEqual("This is a sentence. ", string,
                            "gotoNextSentence(): wrong string")

        bSuccess = xMetaCursor.gotoNextSentence(False)
        self.assertFalse(bSuccess, "gotoNextSentence(): succeeded")
        ## FIXME:
        ## the sentence cursor seems to work differently than the word cursor
        xMeta.setString("This is a sentence. Another sentence. Sentence 3.")
        xMetaCursor.gotoEnd(False)
        bSuccess = xMetaCursor.gotoPreviousSentence(True)
        self.assertTrue(bSuccess, "gotoPreviousSentence(): failed")

        string = xMetaCursor.getString()
        self.assertEqual("Another sentence. Sentence 3.", string,
                                "gotoPreviousSentence(): wrong string")

        bSuccess = xMetaCursor.gotoPreviousSentence(False)
        self.assertFalse(bSuccess, "gotoPreviousSentence(): succeeded")
        bSuccess = xMetaCursor.gotoEndOfSentence(True)
        self.assertTrue(bSuccess, "gotoEndOfSentence(): failed")

        string = xMetaCursor.getString()
        self.assertEqual("This is a sentence.", string,
                            "gotoEndOfSentence(): wrong string")

        xMetaCursor.gotoEnd(False)
        bSuccess = xMetaCursor.gotoStartOfSentence(True)
        self.assertTrue(bSuccess,"gotoStartOfSentence(): failed")

        string = xMetaCursor.getString()
        self.assertEqual("Sentence 3.", string,
                         "gotoStartOfSentence(): wrong string")

        xMeta.setString("")
        bSuccess = xMetaCursor.gotoEndOfSentence(False)
        self.assertFalse(bSuccess, "gotoEndOfSentence(): succeeded")
        bSuccess = xMetaCursor.gotoStartOfSentence(False)
        self.assertFalse(bSuccess, "gotoStartOfSentence(): succeeded")

        ## XParagraphCursor (does not make sense)
        bSuccess = xMetaCursor.gotoNextParagraph(False)
        self.assertFalse(bSuccess, "gotoNextParagraph(): succeeded")
        bSuccess = xMetaCursor.gotoPreviousParagraph(False)
        self.assertFalse(bSuccess, "gotoPreviousParagraph(): succeeded")
        bSuccess = xMetaCursor.gotoStartOfParagraph(False)
        self.assertFalse(bSuccess, "gotoStartOfParagraph(): succeeded")
        bSuccess = xMetaCursor.gotoEndOfParagraph(False)
        self.assertFalse(bSuccess, "gotoEndOfParagraph(): succeeded")

    # See https://bugs.libreoffice.org/show_bug.cgi?id=49629
    # ensure that gotoEndOfWord does not fail when footnote is at word end
    def test_xtextcursor(self):
        xDoc = self.__class__.xDoc
        inserter = RangeInserter(xDoc)
        xDocText = xDoc.getText()
        xDocTextCursor = xDocText.createTextCursor()
        xDocTextCursor.gotoNextParagraph(False)
        inserter.inserttext(xDocTextCursor, "Text")
        xDocTextCursor.gotoEndOfWord(False)
        inserter.insertfootnote(xDocTextCursor, "footnote")
        xDocTextCursor.gotoStartOfParagraph(False)
        bSuccess = xDocTextCursor.gotoEndOfWord(True)
        self.assertTrue(bSuccess, "gotoEndOfWord(): failed")
        string = xDocTextCursor.getString()
        self.assertEqual("Text", string, "gotoEndOfWord(): wrong string")
        self.assertNotEqual("a","b")

    class AttachHelper():
        def isattribute(self): pass
        def mktreenode(self): pass
        def mktextcontent(self, inserter, node): pass
        def postinserted(self, node, xContent): pass

    def test_meta_xtextattach_toxmark(self):
        class Helper(self.AttachHelper):
            def isattribute(_):
                return True
            def mktreenode(_):
                return DocumentIndexMarkNode(self.mkname("toxmark"))
            def mktextcontent(_, inserter, node):
                return inserter.makedocumentindexmark(node.name)
        self.do_meta_xtextattach(Helper())

    def test_meta_xtextattach_refmark(self):
        class Helper(self.AttachHelper):
            def isattribute(_):
                return True
            def mktreenode(_):
                return ReferenceMarkNode(self.mkname("refmark"))
            def mktextcontent(_, inserter, node):
                return inserter.makereferencemark(node.name)
        self.do_meta_xtextattach(Helper())

    def test_meta_xtextattach_textfield(self):
        class Helper(self.AttachHelper):
            def isattribute(_):
                return False
            def mktreenode(_):
                return TextFieldNode(self.mkname("field"))
            def mktextcontent(_, inserter, node):
                return inserter.maketextfield(node.content)
        self.do_meta_xtextattach(Helper())

    def test_meta_xtextattach_footnote(self):
        class Helper(self.AttachHelper):
            def isattribute(_):
                return False
            def mktreenode(_):
                return FootnoteNode(self.mkname("ftn"))
            def mktextcontent(_, inserter, node):
                return inserter.makefootnote(node.label)
        self.do_meta_xtextattach(Helper())

    def test_meta_xtextattach_meta(self):
        class Helper(self.AttachHelper):
            def isattribute(_):
                return True
            def mktreenode(_):
                return MetaNode(self.mkid("id"))
            def mktextcontent(_, inserter, node):
                return inserter.makemeta()
            def postinserted(_, node, xContent):
                xContent.MetadataReference = node.xmlid
        self.do_meta_xtextattach(Helper())

    def do_meta_xtextattach(self, helper):
        xDoc = self.__class__.xDoc
        inserter = RangeInserter(xDoc)
        text = TextNode("12AB6789")
        inserter.insertrange(Range(0, 0, text))
        met1 = MetaNode(self.mkid("id"))
        xMeta = inserter.makemeta()

        xDocText = xDoc.getText()
        xDocTextCursor = xDocText.createTextCursor()
        xDocTextCursor.goRight(3, False)
        xDocTextCursor.goRight(2, True)
        xDocText.insertTextContent(xDocTextCursor, xMeta, True)

        xMeta.MetadataReference = met1.xmlid
        xStart = None
        xEnd = None

        xStart = xMeta.getStart()
        xEnd = xMeta.getEnd()

        nod1 = helper.mktreenode()
        nod2 = helper.mktreenode()
        xContent1 = helper.mktextcontent(inserter, nod1)
        xContent2 = helper.mktextcontent(inserter, nod2)

        ## insertTextContent with meta getStart()/getEnd()
        xMeta.insertTextContent(xStart, xContent1, False)
        xMeta.insertTextContent(xEnd, xContent2, False)

        helper.postinserted(nod1, xContent1)
        helper.postinserted(nod2, xContent2)

        root = TreeNode()
        root.appendchild(TextNode("12"))
        root.appendchild(met1.dup()
                .appendchild(nod1.dup())
                .appendchild(TextNode("AB"))
                .appendchild(nod2.dup()))
        root.appendchild(TextNode("6789"))
        self.dotest(root, False)

        xMeta.setString("AB")
        xStart = xMeta.getStart()
        xEnd = xMeta.getEnd()

        nod1 = helper.mktreenode()
        nod2 = helper.mktreenode()
        xContent1 = helper.mktextcontent(inserter, nod1)
        xContent2 = helper.mktextcontent(inserter, nod2)

        xTextCursor = xMeta.createTextCursor()
        xTextCursor.gotoStart(False)

        ## insertTextContent with meta cursor
        xMeta.insertTextContent(xTextCursor, xContent1, False)
        xTextCursor.gotoEnd(False)
        xMeta.insertTextContent(xTextCursor, xContent2, False)

        helper.postinserted(nod1, xContent1)
        helper.postinserted(nod2, xContent2)

        root = TreeNode()
        root.appendchild(TextNode("12"))
        root.appendchild(met1.dup()
                .appendchild(nod1.dup())
                .appendchild(TextNode("AB"))
                .appendchild(nod2.dup()))
        root.appendchild(TextNode("6789"))
        self.dotest(root, False)

        if not helper.isattribute():
            # xMeta.setString("AB")
            xStart = xMeta.getStart()
            xEnd = xMeta.getEnd()

            nod1 = helper.mktreenode()
            nod2 = helper.mktreenode()
            xContent1 = helper.mktextcontent(inserter, nod1)
            xContent2 = helper.mktextcontent(inserter, nod2)

            xTextCursor = xMeta.createTextCursor()
            xTextCursor.gotoStart(False)
            xTextCursor.goRight(1, True)

            ## insertTextContent with meta cursor and absorb
            xMeta.insertTextContent(xTextCursor, xContent1, True)
            xTextCursor.gotoEnd(False)
            xTextCursor.goLeft(1, True)
            xMeta.insertTextContent(xTextCursor, xContent2, True)

            helper.postinserted(nod1, xContent1)
            helper.postinserted(nod2, xContent2)

            root = TreeNode()
            root.appendchild(TextNode("12"))
            root.appendchild(met1.dup()
                    .appendchild(nod1.dup())
                    .appendchild(TextNode("AB"))
                    .appendchild(nod2.dup()))
            root.appendchild(TextNode("6789"))
            self.dotest(root, False)

            xMeta.setString("AB")
            xStart = xMeta.getStart()
            xEnd = xMeta.getEnd()

            nod1 = helper.mktreenode()
            nod2 = helper.mktreenode()
            xContent1 = helper.mktextcontent(inserter, nod1)
            xContent2 = helper.mktextcontent(inserter, nod2)

            xDocTextCursor.gotoRange(xStart, False)

            ## insertTextContent with document cursor
            xMeta.insertTextContent(xDocTextCursor, xContent1, False)
            xDocTextCursor.gotoRange(xEnd, False)
            xMeta.insertTextContent(xDocTextCursor, xContent2, False)

            helper.postinserted(nod1, xContent1)
            helper.postinserted(nod2, xContent2)

            root = TreeNode()
            root.appendchild(TextNode("12"))
            root.appendchild(met1.dup()
                    .appendchild(nod1.dup())
                    .appendchild(TextNode("AB"))
                    .appendchild(nod2.dup()))
            root.appendchild(TextNode("6789"))
            self.dotest(root, False)

        if not helper.isattribute():
            xStart = xMeta.getStart()
            xEnd = xMeta.getEnd()

            nod1 = helper.mktreenode()
            nod2 = helper.mktreenode()
            xContent1 = helper.mktextcontent(inserter, nod1)
            xContent2 = helper.mktextcontent(inserter, nod2)

            xDocTextCursor.gotoRange(xStart, False)
            xDocTextCursor.goRight(1, True)

            ## insertTextContent with document cursor and absorb
            xMeta.insertTextContent(xDocTextCursor, xContent1, True)
            xDocTextCursor.gotoRange(xEnd, False)
            xDocTextCursor.goLeft(1, True)
            xMeta.insertTextContent(xDocTextCursor, xContent2, True)

            helper.postinserted(nod1, xContent1)
            helper.postinserted(nod2, xContent2)

            root = TreeNode()
            root.appendchild(TextNode("12"))
            root.appendchild(met1.dup()
                    .appendchild(nod1.dup())
                    .appendchild(TextNode("AB"))
                    .appendchild(nod2.dup()))
            root.appendchild(TextNode("6789"))
            self.dotest(root, False)

            xMeta.setString("AB")
            xStart = xMeta.getStart()
            xEnd = xMeta.getEnd()

            nod1 = helper.mktreenode()
            nod2 = helper.mktreenode()
            xContent1 = helper.mktextcontent(inserter, nod1)
            xContent2 = helper.mktextcontent(inserter, nod2)

            ## attach to range from meta getStart()/getEnd()
            xContent1.attach(xStart)
            xContent2.attach(xEnd)

            helper.postinserted(nod1, xContent1)
            helper.postinserted(nod2, xContent2)

            root = TreeNode()
            root.appendchild(TextNode("12"))
            root.appendchild(met1.dup()
                    .appendchild(nod1.dup())
                    .appendchild(TextNode("AB"))
                    .appendchild(nod2.dup()))
            root.appendchild(TextNode("6789"))
            self.dotest(root, False)

            xMeta.setString("AB")
            xStart = xMeta.getStart()
            xEnd = xMeta.getEnd()

            nod1 = helper.mktreenode()
            nod2 = helper.mktreenode()
            xContent1 = helper.mktextcontent(inserter, nod1)
            xContent2 = helper.mktextcontent(inserter, nod2)

            xTextCursor = xMeta.createTextCursor()
            xTextCursor.gotoStart(False)

            ## attach to cursor from meta XText
            xContent1.attach(xTextCursor)
            xTextCursor.gotoEnd(False)
            xContent2.attach(xTextCursor)

            helper.postinserted(nod1, xContent1)
            helper.postinserted(nod2, xContent2)

            root = TreeNode()
            root.appendchild(TextNode("12"))
            root.appendchild(met1.dup()
                    .appendchild(nod1.dup())
                    .appendchild(TextNode("AB"))
                    .appendchild(nod2.dup()))
            root.appendchild(TextNode("6789"))
            self.dotest(root, False)

    def test_metafield_xtextfield(self):
        xDoc = self.__class__.xDoc
        smgr = self.__class__._uno.xContext.ServiceManager
        xRepo = xDoc.getRDFRepository()
        ## for testing just add it to the first graph
        Graphs = xRepo.getGraphNames()
        xGraph = xRepo.getGraph(Graphs[0])
        xOdfPrefix = smgr.createInstance("com.sun.star.rdf.URI")
        xOdfPrefix.initialize((ODF_PREFIX,))
        xOdfSuffix = smgr.createInstance("com.sun.star.rdf.URI")
        xOdfSuffix.initialize((ODF_SUFFIX,))

        xPrefix = smgr.createInstance("com.sun.star.rdf.Literal")
        xPrefix.initialize(("foo",))
        xSuffix = smgr.createInstance("com.sun.star.rdf.Literal")
        xSuffix.initialize(("bar",))

        inserter = RangeInserter(xDoc)
        text = TextNode("abc")
        inserter.insertrange(Range(0, 0, text))
        xDocText = xDoc.getText()
        xDocTextCursor = xDocText.createTextCursor()
        xDocTextCursor.goRight(1, False)
        xDocTextCursor.goRight(3, True)

        xMetaField = inserter.makemetafield()

        xDocText.insertTextContent(xDocTextCursor, xMetaField, True)

        xMetaField.ensureMetadataReference

        xGraph.addStatement(xMetaField, xOdfPrefix, xPrefix)
        xGraph.addStatement(xMetaField, xOdfSuffix, xSuffix)
        self.assertEqual("fooabcbar", xMetaField.getPresentation(False),
                         "getPresentation(): wrong")
        inserter.insertrange(Range(0, 0, text))

    def test_metafield_xpropertyset(self):
        xDoc = self.__class__.xDoc
        inserter = RangeInserter(xDoc)
        text = TextNode("123")
        inserter.insertrange(Range(0, 0, text))
        xDocText = xDoc.getText()
        xDocTextCursor = xDocText.createTextCursor()
        xDocTextCursor.goRight(1, False)
        xDocTextCursor.goRight(3, True)

        xMetaField = inserter.makemetafield()

        xDocText.insertTextContent(xDocTextCursor, xMetaField, True)

        self.assertIsNotNone(xMetaField, "PropertySet: not supported?")
        xPropertySetInfo = xMetaField.getPropertySetInfo()
        self.assertTrue(xPropertySetInfo.hasPropertyByName("NumberFormat"),
                        'hasPropertyByName("NumberFormat"):')
        self.assertTrue(xPropertySetInfo.hasPropertyByName("IsFixedLanguage"),
                        'hasPropertyByName("IsFixedLanguage"):')

        def_ = xMetaField.NumberFormat
        print("NumberFormat: default is {}".format(def_))
        xMetaField.NumberFormat = NUMBER_INT
        xMetaField.IsFixedLanguage = True
        format = xMetaField.NumberFormat
        self.assertEqual(NUMBER_INT, format, "NumberFormat: failed")
        isfixed = xMetaField.IsFixedLanguage
        self.assertTrue(isfixed, "IsFixedLanguage: failed")

    def dostore(self, xComp, file):
        print("Storing test document...")
        file = uno.systemPathToFileUrl(file)
        xComp.storeToURL(file, ())
        print("...done")

    def doload(self, file):
        xComp = None
        print("Loading test document...")
        xComp = self.__class__._uno.openDocFromAbsolutePath(file)
        self.assertIsNotNone(xComp, "cannot load: {}".format(file))
        print("...done")
        return xComp

    def close(self, i_comp):
        try:
            if i_comp:
                i_comp.close(True)
        except Exception as e:
            pass

    def test_load_store(self):
        xComp = None
        filename = "TESTMETA.odt"
        try:
            xComp = self.__class__._uno.openDocFromTDOC(filename)
            if xComp:
                self.checkloadmeta(xComp)
                with TemporaryDirectory() as tempdir:
                    file = os.path.join(tempdir, filename)
                    self.dostore(xComp, file)
                    self.close(xComp)
                    xComp2 = None
                    try:
                        xComp2 = self.doload(file)
                        self.checkloadmeta(xComp2)
                    finally:
                        self.close(xComp2)
        finally:
            self.close(xComp)

    def checkloadmeta(self, xTextDoc):
        xText = xTextDoc.getText()
        print("Checking meta(-field)s in loaded test document...")
        root = TreeNode()
        root.appendchild(RubyNode("ruby1")
                .appendchild(TextNode("1")))
        root.appendchild(MetaNode(self.mkid_("id1"))
                .appendchild(TextNode("2")))
        root.appendchild(MetaFieldNode(self.mkid_("id2"))
                .appendchild(TextNode("3")))
        root.appendchild(RubyNode("ruby2")
                .appendchild(MetaNode(self.mkid_("id3"))
                    .appendchild(TextNode("4"))))
        root.appendchild(RubyNode("ruby3")
                .appendchild(MetaFieldNode(self.mkid_("id4"))
                    .appendchild(TextNode("5"))))
        root.appendchild(MetaNode(self.mkid_("id5"))
                .appendchild(RubyNode("ruby4")
                    .appendchild(TextNode("6"))))
        root.appendchild(MetaFieldNode(self.mkid_("id6"))
                .appendchild(RubyNode("ruby5")
                    .appendchild(TextNode("7"))))
        root.appendchild(MetaNode(self.mkid_("id7"))
                .appendchild(MetaNode(self.mkid_("id8"))
                    .appendchild(TextNode("8"))))
        root.appendchild(MetaNode(self.mkid_("id9"))
                .appendchild(MetaFieldNode(self.mkid_("id10"))
                    .appendchild(TextNode("9"))))
        root.appendchild(MetaFieldNode(self.mkid_("id11"))
                .appendchild(MetaNode(self.mkid_("id12"))
                    .appendchild(TextNode("10"))))
        root.appendchild(MetaFieldNode(self.mkid_("id13"))
                .appendchild(MetaFieldNode(self.mkid_("id14"))
                    .appendchild(TextNode("11"))))
        root.appendchild(MetaNode(self.mkid_("id15"))
                .appendchild(RubyNode("ruby6")
                    .appendchild(MetaFieldNode(self.mkid_("id16"))
                        .appendchild(TextNode("12")))))

        class MetaNode_(MetaNode):
            def __init__(self, id):
                super().__init__(id)
            def __eq__(self, other):
                return isinstance(other, MetaNode)
        root.appendchild(MetaNode_(self.mkid_(""))
                .appendchild(TextNode("13")))
        root.appendchild(TextNode(" X X "))
        self._dotest(xTextDoc, root, False)
        print("...done")

    def test_load_store_xmlid(self):
        xComp = None
        filename = "TESTXMLID.odt"
        try:
            xComp = self.__class__._uno.openDocFromTDOC(filename)
            if xComp:
                self.checkloadxmlid(xComp)
                with TemporaryDirectory() as tempdir:
                    file = os.path.join(tempdir, filename)
                    self.dostore(xComp, file)
                    self.close(xComp)
                    xComp2 = None
                    try:
                        xComp2 = self.doload(file)
                        self.checkloadxmlid(xComp2)
                    finally:
                        self.close(xComp2)
        finally:
            self.close(xComp)

    def checkloadxmlid(self, xTextDoc):
        xText = xTextDoc.getText()
        xRepo = xTextDoc.getRDFRepository()

        print("Checking bookmarks in loaded test document...")
        xBookmarks = xTextDoc.getBookmarks()
        xMark1 = xBookmarks["mk1"]
        self.assertTrue(self.eq(xMark1.MetadataReference,
                                StringPair("content.xml", "id90")), "mark1")
        xMark2 = xBookmarks["mk2"]
        result = xRepo.getStatementRDFa(xMark2)
        self.assertTrue(len(result.First) == 1 and
                        result.First[0].Subject.StringValue == "uri:foo" and
                        result.First[0].Predicate.StringValue == "uri:bar" and
                        result.First[0].Object.Value == "a fooish bar",
                        "mark2")
        xMark3 = xBookmarks["mk3"]
        self.assertTrue(self.eq(xMark3.MetadataReference,
                        StringPair("content.xml", "id91")), "mark3")
        print("...done")

        print("Checking sections in loaded test document...")
        xSections = xTextDoc.getTextSections()
        xSection1 = xSections["Section 1"]
        self.assertTrue(self.eq(xSection1.MetadataReference,
                        StringPair("content.xml", "idSection1")), "idsection1")
        xSection2 = xSections["Section 2"]
        self.assertTrue(self.eq(xSection2.MetadataReference,
                        StringPair("content.xml", "idSection2")),"idSection2")
        xSection3 = xSections["Table of Contents1_Head"]
        self.assertTrue(self.eq(xSection3.MetadataReference,
                        StringPair("content.xml", "idTOCTitle")), "idTOCTitle")
        xSection4 = xSections["Alphabetical Index1_Head"]
        self.assertTrue(self.eq(xSection4.MetadataReference,
                        StringPair("content.xml", "idAITitle")), "idAITitle")
        xSection5 = xSections["Illustration Index1_Head"]
        self.assertTrue(self.eq(xSection5.MetadataReference,
                        StringPair("content.xml", "idIITitle")), "idIITitle")
        xSection6 = xSections["Index of Tables1_Head"]
        self.assertTrue(self.eq(xSection6.MetadataReference,
                        StringPair("content.xml", "idIOTTitle")), "idIOTTitle")
        xSection7 = xSections["User-Defined1_Head"]
        self.assertTrue(self.eq(xSection7.MetadataReference,
                        StringPair("content.xml", "idUDTitle")), "idUDTitle")
        xSection8 = xSections["Table of Objects1_Head"]
        self.assertTrue(self.eq(xSection8.MetadataReference,
                        StringPair("content.xml", "idTOOTitle")), "idTOOTitle")
        xSection9 = xSections["Bibliography1_Head"]
        self.assertTrue(self.eq(xSection9.MetadataReference,
                        StringPair("content.xml", "idBibTitle")), "idBibTitle")
        print("...done")

        print("Checking indexes in loaded test document...")
        xIndexes = xTextDoc.getDocumentIndexes()
        xIndex1 = xIndexes["Table of Contents1"]
        self.assertTrue(self.eq(xIndex1.MetadataReference,
                        StringPair("content.xml", "idTOC")), "idTOC")
        xIndex1s = xSections["Table of Contents1"]
        self.assertTrue(self.eq(xIndex1s.MetadataReference,
                        StringPair("content.xml", "idTOC")), "idTOC")
        xIndex2 = xIndexes["Alphabetical Index1"]
        self.assertTrue(self.eq(xIndex2.MetadataReference,
                        StringPair("content.xml", "idAI")), "idAI")
        xIndex2s = xSections["Alphabetical Index1"]
        self.assertTrue(self.eq(xIndex2s.MetadataReference,
                        StringPair("content.xml", "idAI")), "idAI")
        xIndex3 = xIndexes["Illustration Index1"]
        self.assertTrue(self.eq(xIndex3.MetadataReference,
                        StringPair("content.xml", "idII")), "idII")
        xIndex3s = xSections["Table of Figures1"]
        self.assertTrue(self.eq(xIndex3s.MetadataReference,
                        StringPair("content.xml", "idII")), "idII")
        xIndex4 = xIndexes["Index of Tables1"]
        self.assertTrue(self.eq(xIndex4.MetadataReference,
                        StringPair("content.xml", "idIOT")), "idIOT")
        xIndex4s = xSections["Index of Tables1"]
        self.assertTrue(self.eq(xIndex4s.MetadataReference,
                        StringPair("content.xml", "idIOT")), "idIOT")
        xIndex5 = xIndexes["User-Defined1"]
        self.assertTrue(self.eq(xIndex5.MetadataReference,
                        StringPair("content.xml", "idUD")), "idUD")
        xIndex5s = xSections["User-Defined1"]
        self.assertTrue(self.eq(xIndex5s.MetadataReference,
                        StringPair("content.xml", "idUD")), "idUD")
        xIndex6 = xIndexes["Table of Objects1"]
        self.assertTrue(self.eq(xIndex6.MetadataReference,
                        StringPair("content.xml", "idTOO")), "idTOO")
        xIndex6s = xSections["Table of Objects1"]
        self.assertTrue(self.eq(xIndex6s.MetadataReference,
                        StringPair("content.xml", "idTOO")), "idTOO")
        xIndex7 = xIndexes["Bibliography1"]
        self.assertTrue(self.eq(xIndex7.MetadataReference,
                        StringPair("content.xml", "idBib")), "idBib")
        xIndex7s = xSections["Bibliography1"]
        self.assertTrue(self.eq(xIndex7s.MetadataReference,
                        StringPair("content.xml", "idBib")), "idBib")
        print("...done")

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
