: # -*- perl -*-
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;

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
#
# mkdir - a perl script to substitute mkdir -p
# accepts "/", ":", and "\" as delimiters of subdirectories
# options -p (for compatibility)
#         -mode mode
#

use Cwd;

$currdir = cwd;

$MODE = 00777 ;

while ( $#ARGV >= 0 ) {
    if ( $ARGV[0] eq "-mode" ) {
        $MODE = oct $ARGV[1] ;
        shift @ARGV ;
        shift @ARGV ;
        }
    elsif ( $ARGV[0] eq "-p" ) {
        shift @ARGV ;
        # -p does not do anything, it's supported just for compatibility
        }
    else {

        $ARGV[0] =~ s?\\|:?/?g ;
        @SUBDIRS = split "/", $ARGV[0] ;

        # absolute path UNIX
        if ( $SUBDIRS[0] eq "" ) {
            chdir '/' ;
            shift @SUBDIRS ;
        }
        # absolute path WINDOWS
        if ( $#SUBDIRS > 1 ) {
            if ( $SUBDIRS[1] eq "" ) {
                if ( $SUBDIRS[0] =~ /\w/ ) {
                    chdir "$SUBDIRS[0]:\\" ;
                    shift @SUBDIRS ;
                    shift @SUBDIRS ;
                } ;
            } ;
        }

        while (@SUBDIRS) {
            if ( -e $SUBDIRS[0] ) {
                if ( ! -d $SUBDIRS[0] ) {
                    die "file exists\n"
                }
            }
            else {
                mkdir $SUBDIRS[0], $MODE or die "Can't create directory $SUBDIRS[0]"
            }
            chdir $SUBDIRS[0] or die "Can't cd to $SUBDIRS[0]" ;
            shift @SUBDIRS ;
        } ;

        shift @ARGV ;
    } ;
    chdir $currdir;
}
