#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#
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
