# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import six

from libreoffice.util import printing

class O3tlWrapperPrinter(object):
    '''Prints o3tl::wrapper'''

    def __init__(self, typename, value):
        self.value = value
        self.typename = typename

    def to_string(self):
        return "%s" % (self.typename)

    def children(self):
        pimpl = self.value['m_pimpl']
        if pimpl:
            # I am collapsing the o3tl::wrapper and the inner impl_t struct here, just to
            # things easier to view, may not be ideal.
            refcount = pimpl.dereference()['m_ref_count']
            innerval = pimpl.dereference()['m_value']
            children = [ ( 'm_ref_count', refcount ), ( 'm_value', innerval ) ]
            return children.__iter__()
        else:
            return None

printer = None

def build_pretty_printers():
    global printer

    printer = printing.Printer("libreoffice/o3tl")

    printer.add('o3tl::cow_wrapper', O3tlWrapperPrinter)

def register_pretty_printers(obj):
    printing.register_pretty_printer(printer, obj)

build_pretty_printers()

# vim:set shiftwidth=4 softtabstop=4 expandtab:
