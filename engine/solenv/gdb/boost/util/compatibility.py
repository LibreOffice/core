# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
# Compatibility with older versions of GDB.
#
# Copyright (C) 2012 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#
# This file is part of boost-gdb-printers.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.


import gdb

use_gdb_printing = True
try:
    import gdb.printing
except ImportError:
    use_gdb_printing = False

use_lazy_string = hasattr(gdb.Value, 'lazy_string')

# vim:set filetype=python shiftwidth=4 softtabstop=4 expandtab:
