# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import six
from libreoffice.util import printing

class SwPositionPrinter(object):
    '''Prints SwPosition.'''

    def __init__(self, typename, value):
        self.typename = typename
        self.value = value

    def to_string(self):
        node = self.value['nNode']['m_pNode'].dereference();
        block = node['m_pBlock'].dereference();
        nodeindex = block['nStart'] + node['m_nOffset']
        offset = self.value['nContent']['m_nIndex']
        return "%s (node %d, offset %d)" % (self.typename, nodeindex, offset)

class SwNodeIndexPrinter(object):
    '''Prints SwNodeIndex.'''

    def __init__(self, typename, value):
        self.typename = typename
        self.value = value

    def to_string(self):
        node = self.value['m_pNode'].dereference();
        block = node['m_pBlock'].dereference();
        nodeindex = block['nStart'] + node['m_nOffset']
        return "%s (node %d)" % (self.typename, nodeindex)

class SwIndexPrinter(object):
    '''Prints SwIndex.'''

    def __init__(self, typename, value):
        self.typename = typename
        self.value = value

    def to_string(self):
        offset = self.value['m_nIndex']
        return "%s (offset %d)" % (self.typename, offset)

class SwPaMPrinter(object):
    '''Prints SwPaM.'''

    def __init__(self, typename, value):
        self.typename = typename
        self.value = value

    def to_string(self):
        return "%s" % (self.typename)

    def children(self):
        next_ = self.value['m_pNext']
        prev  = self.value['m_pPrev']
        point = self.value['m_pPoint'].dereference()
        mark = self.value['m_pMark'].dereference()
        children = [ ( 'point', point), ( 'mark', mark ) ]
        if next_ != self.value.address:
            children.append(("next", next_))
        if prev != self.value.address:
            children.append(("prev", prev))
        return children.__iter__()

# apparently the purpose of this is to suppress printing all the extra members
# that SwCursor and SwUnoCursor add
class SwUnoCursorPrinter(SwPaMPrinter):
    '''Prints SwUnoCursor.'''

class SwRectPrinter(object):
    '''Prints SwRect.'''

    def __init__(self, typename, value):
        self.typename = typename
        self.value = value

    def to_string(self):
        return "%s" % (self.typename)

    def children(self):
        point = self.value['m_Point']
        size = self.value['m_Size']
        children = [ ( 'point', point), ( 'size', size ) ]
        return children.__iter__()

class MarkBasePrinter(object):
    '''Prints sw::mark::MarkBase.'''

    def __init__(self, typename, value):
        self.typename = typename
        self.value = value

    def to_string(self):
        return "%s" % (self.typename)

    def children(self):
        m = self.value.cast(self.value.dynamic_type)
        return [ ( v, m[ v ] )
            for v in ( 'm_aName', 'm_pPos1', 'm_pPos2' ) ].__iter__()

class SwXTextRangeImplPrinter(object):
    '''Prints SwXTextRange::Impl.'''

    def __init__(self, typename, value):
        self.typename = typename
        self.value = value

    def to_string(self):
        return "%s" % (self.typename)

    def children(self):
        mark = self.value['m_pMark'].dereference()
        children = [('mark', mark)]
        return children.__iter__()

class SwXTextCursorImplPrinter(object):
    '''Prints SwXTextCursor::Impl.'''

    def __init__(self, typename, value):
        self.typename = typename
        self.value = value

    def to_string(self):
        return "%s" % (self.typename)

    def children(self):
        cursor = self.value['m_pUnoCursor']["m_pCursor"]["_M_ptr"]
        registeredIn = cursor.dereference()
        children = [('m_pUnoCursor', registeredIn)]
        return children.__iter__()

class SwUnoImplPtrPrinter(object):
    """Prints sw::UnoImplPtr"""

    def __init__(self, typename, value):
        self.typename = typename
        self.value = value

    def to_string(self):
        if self.value['m_p']:
            return "%s %s" % (self.typename, self.value['m_p'].dereference())
        else:
            return "empty %s" % (self.typename,)

class SwXTextRangePrinter(object):
    '''Prints SwXTextRange.'''

    def __init__(self, typename, value):
        self.typename = typename
        self.value = value

    def to_string(self):
        return "%s %s" % (self.typename, self.value['m_pImpl'])

class SwXTextCursorPrinter(object):
    '''Prints SwXTextCursor.'''

    def __init__(self, typename, value):
        self.typename = typename
        self.value = value

    def to_string(self):
        return "%s %s" % (self.typename, self.value['m_pImpl'])

class BigPtrArrayPrinter(object):
    '''Prints BigPtrArray.'''

    def __init__(self, typename, value):
        self.typename = typename
        self.value = value

    def to_string(self):
        length = self.value['m_nSize']
        if length > 0:
            return "%s of length %d" % (self.typename, length)
        else:
            return "empty %s" % self.typename

    def children(self):
        return self._iterator(self.value)

    def display_hint(self):
        return 'array'


    class _iterator(six.Iterator):

        def __init__(self, array):
            self.blocks = array['m_ppInf']
            self.count = array['m_nSize']
            self.pos = 0
            self.block_count = array['m_nBlock']
            self.block_pos = 0
            self.block = None
            self.indent = ""
            self.max_indent = "        "
            self._next_block(False)
            self._check_invariant()

        def __iter__(self):
            return self

        def _node_value(self, node):
            cur_indent = self.indent
            if str(node.dynamic_type.target()) == "SwTextNode":
                # accessing this is completely non-obvious...
                # also, node.dynamic_cast(node.dynamic_type) is null?
                value = "    TextNode " + \
                  six.text_type(node.cast(node.dynamic_type).dereference()['m_Text'])
            elif str(node.dynamic_type.target()) == "SwOLENode":
                value = "     OLENode "
            elif str(node.dynamic_type.target()) == "SwGrfNode":
                value = "     GrfNode "
            elif str(node.dynamic_type.target()) == "SwSectionNode":
                value = " SectionNode "
                self.indent += " "
            elif str(node.dynamic_type.target()) == "SwTableNode":
                value = "   TableNode "
                self.indent += " "
            elif str(node.dynamic_type.target()) == "SwStartNode":
                value = "   StartNode "
                self.indent += " "
            elif str(node.dynamic_type.target()) == "SwEndNode":
                value = "     EndNode "
                self.indent = self.indent[:-1]
                cur_indent = self.indent
            elif str(node.dynamic_type.target()) == "SwDummySectionNode":
                value = "DummySctNode "
            else: # must be currently being deleted, so has some abstract type
                value = "~DeletedNode "
#            return "\n[%s%4d%s] %s %s" % (cur_indent, self.pos, \
#                                self.max_indent[len(cur_indent):], node, value)
            return "\n[%4d] %s%s%s %s" % (self.pos, cur_indent, \
                                node, self.max_indent[len(cur_indent):], value)

        def __next__(self):
            if self.pos == self.count:
                raise StopIteration()

            name = str(self.pos)
            node = self.block['mvData']['_M_elems'][self.pos - self.block['nStart']]
            value =  self._node_value(node)
            if self.pos == self.block['nEnd']:
                self._next_block()
            self.pos += 1

            self._check_invariant()
            return (name, value)

        def _next_block(self, advance = True):
            if advance:
                self.block_pos += 1

            if self.block_pos == self.block_count:
                return

            pblock = self.blocks[self.block_pos]
            assert pblock
            block = pblock.dereference()
            start = block['nStart']
            end = block['nEnd']
            assert end - start + 1 == block['nElem']
            if self.block:
                assert start == self.block['nEnd'] + 1
                assert end <= self.count
            else:
                assert start == 0
            self.block = block

        def _check_invariant(self):
            assert self.pos <= self.count
            assert self.block_pos <= self.block_count
            if self.pos == 0 and self.pos < self.count:
                assert self.block != None

printer = None

def build_pretty_printers():
    global printer

    printer = printing.Printer("libreoffice/sw")
    printer.add('BigPtrArray', BigPtrArrayPrinter)
    printer.add('SwPosition', SwPositionPrinter)
    printer.add('SwNodeIndex', SwNodeIndexPrinter)
    printer.add('SwIndex', SwIndexPrinter)
    printer.add('SwPaM', SwPaMPrinter)
    printer.add('SwUnoCursor', SwUnoCursorPrinter)
    printer.add('SwRect', SwRectPrinter)
    printer.add('sw::mark::Bookmark', MarkBasePrinter)
    printer.add('sw::mark::MarkBase', MarkBasePrinter)
    printer.add('sw::mark::UnoMark', MarkBasePrinter)
    printer.add('sw::mark::IMark', MarkBasePrinter)
    printer.add('SwXTextRange::Impl', SwXTextRangeImplPrinter)
    printer.add('sw::UnoImplPtr', SwUnoImplPtrPrinter)
    printer.add('SwXTextRange', SwXTextRangePrinter)
    printer.add('SwXTextCursor::Impl', SwXTextCursorImplPrinter)
    printer.add('SwXTextCursor', SwXTextCursorPrinter)

def register_pretty_printers(obj):
    printing.register_pretty_printer(printer, obj)

build_pretty_printers()

# vim:set shiftwidth=4 softtabstop=4 expandtab:
