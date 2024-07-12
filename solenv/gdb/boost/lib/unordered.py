# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
# Helper classes for working with Boost.Unordered.
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
import six

class Unordered(object):
    '''Common representation of Boost.Unordered types'''

    def __init__(self, value, extractor):
        self.value = value
        self.extractor = extractor
        self.node_type = self._node_type()

    def __len__(self):
        table = self.value['table_']
        if table['buckets_']:
            return int(table['size_'])
        else:
            return 0

    def __iter__(self):
        table = self.value['table_']
        buckets = table['buckets_']
        if buckets:
            first = table['cached_begin_bucket_']
            last = buckets + table['bucket_count_']
        else:
            first = last = None
        return self._iterator(first, last, self.node_type, self.extractor)

    def empty(self):
        return not self.value['table_']['buckets_']

    def _node_type(self):
        hash_table = self.value['table_'].type.fields()[0]
        assert hash_table.is_base_class
        hash_buckets = hash_table.type.fields()[0]
        assert hash_buckets.is_base_class
        node_type = gdb.lookup_type("%s::node" % hash_buckets.type)
        assert node_type is not None
        return node_type

    class _iterator(six.Iterator):
        '''Iterator for Boost.Unordered types'''

        def __init__(self, first_bucket, last_bucket, node_type, extractor):
            self.bucket = first_bucket
            self.last_bucket = last_bucket
            self.node = self.bucket
            self.node_type = node_type
            self.value_type = self._value_type()
            self.extractor = extractor

        def __iter__(self):
            return self

        def __next__(self):
            if self.node:
                self.node = self.node.dereference()['next_']

            # we finished the current bucket: go on to the next non-empty one
            if not self.node:
                while not self.node and self.bucket != self.last_bucket:
                    self.bucket += 1
                    self.node = self.bucket.dereference()['next_']

            # sorry, no node available
            if not self.node or self.node == self.bucket:
                raise StopIteration()

            mapped = self._value()
            return (self.extractor.key(mapped), self.extractor.value(mapped))

        def _value(self):
            assert self.node != self.bucket # bucket node has no value
            assert self.node is not None
            node = self.node.dereference().cast(self.node_type)
            return node['data_'].cast(self.value_type)

        def _value_type(self):
            value_base = self.node_type.fields()[1]
            assert value_base.is_base_class
            return value_base.type.template_argument(0)

class Map(Unordered):

    def __init__(self, value):
        super(Map, self).__init__(value, self._extractor())

    class _extractor(object):

        def key(self, node):
            return node['first']

        def value(self, node):
            return node['second']

class Set(Unordered):

    def __init__(self, value):
        super(Set, self).__init__(value, self._extractor())

    class _extractor(object):

        def key(self, node):
            return None

        def value(self, node):
            return node

# vim:set filetype=python shiftwidth=4 softtabstop=4 expandtab:
