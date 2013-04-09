# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2010 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#  (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

from libreoffice.util import printing
from libreoffice.util.uno import TypeClass, make_uno_type, uno_cast

class UnoAnyPrinter(object):
    '''Prints UNO any'''

    def __init__(self, typename, value):
        self.value = value
        self.typename = typename.replace('com::sun::star::', '')

    def to_string(self):
        if self._is_set():
            return ('%s %s' % (self.typename, self._make_string()))
        else:
            return "empty %s" % self.typename

    def _is_set(self):
        return self.value['pType'].dereference()['eTypeClass'] != TypeClass.VOID

    def _make_string(self):
        ptr = self.value['pData']
        assert ptr
        type_desc = self.value['pType']
        assert type_desc
        type = make_uno_type(type_desc.dereference())
        assert type
        return str(uno_cast(type, ptr).dereference())

class UnoReferencePrinter(object):
    '''Prints reference to a UNO interface'''

    def __init__(self, typename, value):
        self.value = value
        self.typename = typename.replace('com::sun::star::', '')

    def to_string(self):
        iface = self.value['_pInterface']
        if iface:
            try:
                impl = iface.cast(iface.dynamic_type).dereference()
                return '%s to %s' % (self.typename, str(impl))
            except:
                # fallback for potential problem:
                # base class 'com::sun::star::uno::XInterface' is ambiguous
                return '%s to (XInterface) %s' % (self.typename, str(iface))

        else:
            return "empty %s" % self.typename

class UnoSequencePrinter(object):
    '''Prints UNO Sequence'''

    class iterator(object):
        '''Sequence iterator'''

        def __init__(self, first, size):
            self.item = first
            self.size = size
            self.count = 0

        def __iter__(self):
            return self

        def next(self):
            if self.count == self.size:
                raise StopIteration
            count = self.count
            self.count = self.count + 1
            elem = self.item.dereference()
            self.item = self.item + 1
            return ('[%d]' % count, elem)


    def __init__(self, typename, value):
        self.value = value
        self.typename = typename.replace('com::sun::star::', '')

    def to_string(self):
        pimpl = self.value['_pSequence']
        if pimpl:
            impl = pimpl.dereference()
            elems = impl['nElements']
            if elems == 0:
                return "empty %s" % self.typename
            else:
                return "%s of length %d" % (self.typename, elems)
        else:
            return "uninitialized %s" % self.typename

    def children(self):
        pimpl = self.value['_pSequence']
        if pimpl:
            impl = pimpl.dereference()
            elemtype = self.value.type.template_argument(0)
            elements = impl['elements'].cast(elemtype.pointer())
            return self.iterator(elements, int(impl['nElements']))
        else:
            # TODO is that the best thing to do here?
            return None

    def display_hint(self):
        if self.value['_pSequence']:
            return 'array'
        else:
            return None

class UnoTypePrinter(object):
    '''Prints UNO Type'''

    def __init__(self, typename, value):
        self.value = value
        self.typename = typename.replace('com::sun::star::', '')

    def to_string(self):
        uno = make_uno_type(self.value)
        if uno:
            return "%s %s" % (self.typename, uno.tag)
            # return "%s %s" % (self.typename, uno.typename)
        else:
            return "invalid %s" % self.typename

class CppuThreadpoolThreadPoolPrinter(object):
    '''Prints cppu_threadpool::ThreadPool objects (a hack to avoid infinite recursion through sal.RtlReferencePrinter when printing an rtl::Reference<cppu_threadpool::ThreadPool> whose std::list<cppu_threadpool::WaitingThread*> m_lstThreads member, via rtl::Reference<cppu_threadpool::ORequestThread> thread member, via rtl::Reference<cppu_threadpool::ThreadPool> m_aThreadPool member, has a circular reference back)'''

    def __init__(self, typename, value):
        self.typename = typename
        self.value = value

    def to_string(self):
        return '%s@%s' % (self.typename, self.value.address)

printer = None

def build_pretty_printers():
    global printer

    printer = printing.Printer("libreoffice/cppu")

    # basic UNO stuff
    printer.add('_uno_Any', UnoAnyPrinter)
    printer.add('com::sun::star::uno::Any', UnoAnyPrinter)
    printer.add('com::sun::star::uno::Reference', UnoReferencePrinter)
    printer.add('com::sun::star::uno::Sequence', UnoSequencePrinter)
    printer.add('com::sun::star::uno::Type', UnoTypePrinter)
    printer.add('cppu_threadpool::ThreadPool', CppuThreadpoolThreadPoolPrinter)

def register_pretty_printers(obj):
    printing.register_pretty_printer(printer, obj)

build_pretty_printers()

# vim:set shiftwidth=4 softtabstop=4 expandtab:
