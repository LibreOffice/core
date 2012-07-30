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
BEGIN {
    CCversion = 0
    compiler_matched = 0
}
# Sun c++ compiler
/Sun WorkShop/ || /Forte Developer/ || /Sun/{
    compiler_matched = 1
    # version number right after "C++" 
    x = match( $0, /C\+\+ .*/ )
    btwn = substr( $0, RSTART, RLENGTH)
    # extract version, whitespaces get striped later
    x = match( btwn, / .*\..*[ $\t]/)
    CCversion = substr( btwn, RSTART, RLENGTH)
}
# Microsoft c++ compiler
/Microsoft/ && /..\...\...../ {
    compiler_matched = 1
    # match on the format of the ms versions ( dd.dd.dddd )
    x = match( $0, /..\...\...../ )
    CCversion = substr( $0, RSTART, RLENGTH)
}
# Java
/java version/ || /openjdk version/ {
    compiler_matched = 1
    # match on the format of the java versions ( d[d].d[d].d[d] )
    x = match( $0, /[0-9]*\.[0-9]*\.[0-9]*/ )
    CCversion = substr( $0, RSTART, RLENGTH)
}
/^[0-9]*[.][0-9]*\r*$/ {
    if ( compiler_matched == 0 ) {
# need to blow to x.xx.xx for comparing
    	CCversion = $0 ".0"
    }
}
/^[0-9]*[.][0-9]*[.][0-9]*\r*$/ {
    if ( compiler_matched == 0 ) {
        CCversion = $0
    }
}
/^[0-9]*[.][0-9]*[.][0-9]*-[0-9a-z]*$/ {
    if ( compiler_matched == 0 ) {
        CCversion = substr($0, 0, index($0, "-") - 1)
    }
}
# NDK r8b has "4.6.x-google"
/^[0-9]*[.][0-9]*[.][a-z]*-[0-9a-z]*$/ {
    if ( compiler_matched == 0 ) {
	# Include the second period in the match so that
	# we will get a micro version of zero
	x = match(  $0, /^[0-9]*[.][0-9]*[.]/ )
        CCversion = substr($0, RSTART, RLENGTH)
    }
}
END {
    if ( num == "true" ) {
        tokencount = split (CCversion,vertoken,".")
        for ( i = 1 ; i <= tokencount ; i++ ) {
            printf ("%04d",vertoken[i] )
        }
    } else
        print CCversion
}
