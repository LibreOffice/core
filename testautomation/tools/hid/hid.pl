#**************************************************************************
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
#******************************************************************
#*
#* Owner : thorsten.bosbach@oracle.com
#*
#* short description : Generate a cleaned hid.lst
#*
#******************************************************************
#
# syntax : hid.pl Input Output ConstantEntries
# usually: hid.pl hid.lst hid.txt const.txt
#

for (@ARGV){print $_."\n";}
open (HID,"<".@ARGV[0]) || die "Can't find old HID-file (first argument)";
@ary=( <HID> );
close HID;
open (HID,">".@ARGV[1]) || die "Can't find new HID-file (second argument)";

@longnum = @longname = ();

for (@ary) {
   s/MN_VIEW 21//g;                       # remove slots that are wrong
   s/MN_INSERT 24//g;
   s/MN_SUB_TOOLBAR 92//g;
   s/SID_OBJECT_MIRROR 27085//g;
   s/UID_SQLERROR_BUTTONMORE 38844//g;
   s/MN_EXTRA 22//g;                     # -------------------------------------------
   s/RID_UNDO_DELETE_WARNING 20558//g;
   s/.* 01010101010//g;
   s/ +/ /g;                              # remove double blanks
   @x = split(/\s+/) ;                    # seperate Longnames and HIDs
#   @x[0]=~ tr/a-z/A-Z/;
#   @x[1]=~ tr/a-z/A-Z/;
   $longname[++$#longname] = @x[0];
   $longnum[++$#longnum]   = @x[1];
   $_=@x[0]." ".@x[1]."\n";
}

@ary = @ary[ sort{                        # sort
                @longnum[$a] <=> @longnum[$b] ||
                @longname[$a] cmp @longname[$b]
                }0..$#ary
           ];

# @ary = grep( !/^ *$/, @ary);

#remove double entries

$n="";
for (@ary) {
    if ($n eq $_   || $_>0  ){
       $_="";
     }
     else{
        $n=$_;
     }
 }

@ary = grep( !/^ *$/, @ary);

# to insert the constant entries  at the beginning, read it and write it out
open (CON,"<".@ARGV[2]) || die "Can't find constant entries-file: const.txt (third argument)";
@const=( <CON> );
close CON;
print HID @const;

print HID @ary;
close HID;
