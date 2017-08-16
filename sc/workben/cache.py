# -*- Mode: python; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import sys
import struct

column_block_type = {
        0 : "empty block",
        1 : "numeric block",
        2 : "string block"
        }


def parse_block(data, index):
    start_row = struct.unpack('Q', data[index:index+8])[0]
    index += 8
    data_size = struct.unpack('Q', data[index:index+8])[0]
    index += 8
    block_type = struct.unpack('B', data[index:index+1])[0]
    index += 1
    vals = {}
    if block_type == 1:
        # numeric block
        for i in range(data_size):
            vals[start_row + i] = struct.unpack('d', data[index:index+8])[0]
            index += 8
    elif block_type == 2:
        # string block
        for i in range(data_size):
            str_length = struct.unpack('i', data[index:index+4])[0]
            index += 4
            vals[start_row + i] = data[index:index+str_length].decode("utf-8")
            index += str_length
    elif block_type == 3:
        # formula block
        read_rows = 0
        while read_rows < data_size:
            formula_group_size = struct.unpack('Q', data[index:index+8])[0]
            index += 8
            str_length = struct.unpack('i', data[index:index+4])[0]
            index += 4
            vals[start_row + read_rows] = (data[index:index+str_length].decode("utf-8"), "formula group length %i"% formula_group_size)
            read_rows += formula_group_size
            index += str_length

    return index, data_size, vals


def parse_column(data, index):
    column_index = struct.unpack('Q', data[index:index+8])[0]
    index += 8
    column_entries = struct.unpack('Q', data[index:index+8])[0]
    index += 8
    imported_columns = 0
    column_values = {}
    while imported_columns < column_entries:
        index, block_size, vals = parse_block(data, index)
        imported_columns += block_size
        column_values.update(vals)
    return index, column_values

def parse_columns(data, index):
    column_count = struct.unpack('Q', data[index:index+8])[0]
    index += 8
    columns = {}
    for i in range(column_count):
        index, column_values = parse_column(data, index)
        columns[i] = column_values

    return columns

def main():
    filename = sys.argv[1]
    with open(filename, "rb") as f:
        content = f.read()
        index = 0
        columns = parse_columns(content, index)
        print(columns)

if __name__ == "__main__":
    main()

# vim:set shiftwidth=4 softtabstop=4 expandtab: */
