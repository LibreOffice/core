#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

# Generate an exported symbols list out of a map file (as use on Linux/Solaris) in order to
# build shared libraries on Mac OS X
#
# The below code fails may fail with 'perverted' mapfiles (using a strange line layout etc.)

# Skip 'SECTION_NAME {' lines
/^[\t ]*.*[\t ]*\{/ { next }

# Skip 'global:' or 'local:' lines
/global:/ || /local:/ { next }

# Skip '*;' lines
/^[\t ]*\*;[\t ]*/ { next }

# Skip section end '}?;' lines
/^[\t ]*\}[\t ]*.*[;]*/ { next }

# Skip comment or empty lines
/^[\t ]*#.*/ || /^[\t ]*$/ || /^$/ { next }

# Echo all lines containing symbol names and prefix them with '_'
# because symbols on Mac OS X start always with '__'
{
    # There may appear multiple symbols in one line
    # e.g. "sym1; sym2; # and finally a comment"
    # take this into account
    for (i = 1; i <= NF ; i++) {
	if ($i !~ /^[\t ]*#.*/) { # as long as the current field doesn't start with '#'
	    gsub(/[\t ;]/, "", $i) # Remove leading spaces and trailing ';'
	    printf("_%s\n",$i)
	}
	else { # ignore everything after a '#' (comment) sign
	    break
	}
    }
}
