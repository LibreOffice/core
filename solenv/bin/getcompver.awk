#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************
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
/^[0-9]*[.][0-9]*\x0d*$/ {
    if ( compiler_matched == 0 ) {
# need to blow to x.xx.xx for comparing
    	CCversion = $0 ".0"
    }
}
/^[0-9]*[.][0-9]*[.][0-9]*\x0d*$/ {
    if ( compiler_matched == 0 ) {
        CCversion = $0
    }
}
/^[0-9]*[.][0-9]*[.][0-9]*-[0-9a-z]*$/ {
    if ( compiler_matched == 0 ) {
        CCversion = substr($0, 0, index($0, "-") - 1)
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
