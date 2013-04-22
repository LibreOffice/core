#!/usr/bin/env python
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2011 Marco Cecchetti <mrcekets@gmail.com>
#
# All Rights Reserved.
#
# For minor contributions see the git repository.

import os, sys

MAX_LINES = 200
VARIABLE_NAME = 'aSVGScript'

def get_var_decl(n):
    return 'static const char %s%d[] =' % ( VARIABLE_NAME, n )

script_name = os.path.basename( sys.argv[0] )
infile_name = sys.argv[1]
outfile_name = sys.argv[2]


# collect input JavaScript file lines
if( not os.path.isfile( infile_name ) ):
    print ( '%s: error: file "%s" not found' % ( script_name, infile_name ) )
    sys.exit( -1 )

infile = open( infile_name, 'r' )
in_lines = [line.rstrip() for line in infile.readlines()]
infile.close()


valid_lines=[]
is_multiline_comment = False
lineNumber = 0
emptyLineCount = 0
for line in in_lines:
    lineNumber += 1
    index = line.find('"')
    if( index != -1 ):
        print ( '%s: warning: processed file contains \'"\' at %d:%d' % ( script_name, lineNumber, index ) )

    sline = line.strip()

    # strip comment lines except multilines comments that begins with one '/' and exactly 5 '*'
    if( is_multiline_comment and sline.endswith( '*/' ) ):
        is_multiline_comment = False
        continue

    if( is_multiline_comment ):
        continue

    if( sline.startswith( '//' ) ):
        continue

    if( sline.startswith( '/*' ) and sline.endswith( '*/' ) ):
        continue

    if( ( sline.startswith( '/*' ) and not sline.startswith( '/*****' ) )
         or sline.startswith( '/******' ) ):
        is_multiline_comment = True
        continue

    # disable any debug printer
    dline = line.replace( 'NAVDBG.on', 'NAVDBG.off' )
    dline = dline.replace( 'ANIMDBG.on', 'ANIMDBG.off' )
    dline = dline.replace( 'DebugPrinter.on', 'DebugPrinter.off' )

    escaped_line = '%s' % dline
    escaped_line = escaped_line.rstrip().lstrip()

    # no more than 2 consecutive empty lines
    if( escaped_line == '' ):
        emptyLineCount += 1
    else:
        emptyLineCount = 0

    if( emptyLineCount > 2 ):
        continue

    # append to some escape sequence another '\'
    escaped_line = escaped_line.replace( '\\', '\\\\' )
    escaped_line = escaped_line.replace( '\n', '\\n')
    escaped_line = escaped_line.replace( '\t', '\\t' )

    valid_lines.append( escaped_line )


# compute the number of needed fragments that is of C constant strings
total_valid_lines = len (valid_lines) + 2
total_fragments = total_valid_lines / MAX_LINES
if( ( total_valid_lines % MAX_LINES ) != 0 ):
    total_fragments += 1



out_lines = []
out_lines.append( '' )
out_lines.append( '#define N_SVGSCRIPT_FRAGMENTS %d' % total_fragments )
out_lines.append( '' )
out_lines.append( get_var_decl( 0 ) )
out_lines.append( '"<![CDATA[\\n\\' )
i = 2
fragment = 0
for line in valid_lines:
    out_lines.append( line + '\\n\\' )
    if( i == MAX_LINES ):
        i = 0
        fragment += 1
        out_lines.append( '";' )
        out_lines.append( '' )
        out_lines.append( get_var_decl( fragment ) )
        out_lines.append( '"\\' )
    i += 1

out_lines.append( ']]>";' )
out_lines.append( '' )


outfile = open( outfile_name, 'w' )
if( not os.path.isfile( outfile_name ) ):
    print ( '%s: error: I cannot create file "%s"' % ( script_name, outfile_name ) )
    sys.exit( -1 )


# C++ header
header_info = '/* !! This file is auto-generated, do not edit !! */'

outfile.write( header_info +'\n\n' )

for line in out_lines:
    outfile.write( line + '\n' )

outfile.close()
