#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: unxmap-to-macosx-explist.awk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: rt $ $Date: 2007-01-31 08:42:05 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
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
