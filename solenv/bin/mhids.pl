:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
#*************************************************************************
#
#   $RCSfile: mhids.pl,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: rt $ $Date: 2004-12-10 17:15:27 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc..
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

$filename = shift @ARGV;
$srs = shift @ARGV;
$prjname = shift @ARGV;
$defs = join " ",@ARGV;

my $filebase;

$srs =~ /([\\\/])/;
$delim = $1;

if ( $filename =~ /.*[\\\/](.*)\..*?/ )
{
    $filebase = $1;
}
else
{
    $filename =~ /(.*)\..*?/;
    $filebase = $1;
}

#remove old objects which remained in place by a former bug
unlink "$filebase.obj";

$filebase = $srs.$delim.$filebase;

print "filebase: $filebase\n";

$prjname =~ lc $prjname;

if ( -f "$filebase.hid" )
{
    unlink "$filebase.hid";
}

# hack to quit for files which cannot be handled
foreach $fname ( split / /, $ENV{"NO_HID_FILES"} )
{
    if ( $fname eq $filename )
    {
        print "No hid generation for $filename due to NO_HID_FILES\n";
        open TOUCH, ">$filebase.hid";
        close TOUCH;
        exit 0;
    }
}

#echo "perl5 -p -e "s/=[ \t]*\".*\"/=\"\"/go; s/\".*\"[ \t]*;/\"\" ;/go ; s/(\".*)\/\/(.*\")/$1\/\\\/$2/go ;" < %filename% > %srs%\%filebase%.c0"
#call  perl5 -p -e "s/=[ \t]*\".*\"/=\"\"/go; s/\".*\"[ \t]*;/\"\" ;/go ; s/(\".*)\/\/(.*\")/$1\/\\\/$2/go ;" < %filename% > %srs%\%filebase%.c0

print  "hidc.exe $filename $filebase.c1 $prjname \n";
system "hidc.exe $filename $filebase.c1 $prjname";

print  "cl $ENV{SOLARINCLUDES} $defs /EP $filebase.c1     > $filebase.c2 \n";
system "cl $ENV{SOLARINCLUDES} $defs /EP $filebase.c1     > $filebase.c2";

open C_PROG, ">$filebase.c";
print C_PROG "#include <wctype.h>\n";

open PRE, "<$filebase.c2";
$InMain = 0;
while (<PRE>)
{
    if ( /int\s*main/ )
    {
        $InMain = 1;
    }

    if ( $InMain && !/^\s*$/ )
    {
        print C_PROG;
    }
}

close PRE;
close C_PROG;

#cl %SOLARINCLUDES% %_srs%\%_filebase%.c /Fe%_srs%\%_filebase%.exe
print         "cl $ENV{SOLARINCLUDES} $defs $filebase.c /Fo$filebase.obj /Fe$filebase.exe \n";
$ret = system "cl $ENV{SOLARINCLUDES} $defs $filebase.c /Fo$filebase.obj /Fe$filebase.exe";
if ( $ret ) { die "$@\n"; }    # die on returncode != 0


#awk -f %ENV_TOOLS%\hidcode.awk < %srs%\%filebase%.c3 > %srs%\%filebase%.hid
open C3,"$filebase.exe|";
open HID,">$filebase.hid";

while (<C3>)
{
    @fields = split /\s+/;

    if ( $fields[1] eq "HelpID" )
    {
        print HID "$fields[0] $fields[2]\n";
        next;
    }

    @arr = split /:/, $fields[0];
    if( $arr[1] =~ /^leer$|^bitmap$|^font$|^color$|^image$|^imagelist$|^date$|^brush$|^fixedtext$|^keycode$|^time$|^mapmode$/i )
    {
        #print "skipping $arr[1]\n";
        next;
    }

    if ( $fields[1] eq "Norm" )
    {
        # Felder der Zeile auf Variable verteilen
        $helpIDString = $fields[0];
        $GClass = lc($fields[2]);
        $GID = $fields[3];
        $LClass  = lc($fields[4]);
        $LID = $fields[5] || 0;

        #print $LID
        #print $LClass
        #print $GID
        #print $GClass

        $nHID=0;

        $VAL1  = 536870912; #2 hoch 29
        if   ( $GClass eq "workwindow"     ) { $nHID= $VAL1 *5; }
        elsif( $GClass eq "modelessdialog" ) { $nHID= $VAL1 *4; }
        elsif( $GClass eq "floatingwindow" ) { $nHID= $VAL1 *3; }
        elsif( $GClass eq "modaldialog"    ) { $nHID= $VAL1 *2; }
        elsif( $GClass eq "tabpage"        ) { $nHID= $VAL1 *1; }
        elsif( $GClass eq "dockingwindow"  ) { $nHID= $VAL1 *6; }
                            #Maximal bis 7 dann sind 32Bit ausgeschoepft
        else {
            $nHID=0;
            $outline = "No GClass ".$helpIDString." ".$nHID." ".$GClass;
            #print "$outline\n";
            next;
        }
        if( $LID != 0 ) {
            if   ( $LClass eq "tabcontrol"        ) { $nHID += 0;      }
            elsif( $LClass eq "radiobutton"       ) { $nHID += 2*256;  }
            elsif( $LClass eq "checkbox"          ) { $nHID += 4*256;  }
            elsif( $LClass eq "tristatebox"       ) { $nHID += 6*256;  }
            elsif( $LClass eq "edit"              ) { $nHID += 8*256;  }
            elsif( $LClass eq "multilineedit"     ) { $nHID += 10*256; }
            elsif( $LClass eq "multilistbox"      ) { $nHID += 12*256; }
            elsif( $LClass eq "listbox"           ) { $nHID += 14*256; }
            elsif( $LClass eq "combobox"          ) { $nHID += 16*256; }
            elsif( $LClass eq "pushbutton"        ) { $nHID += 18*256; }
            elsif( $LClass eq "spinfield"         ) { $nHID += 20*256; }
            elsif( $LClass eq "patternfield"      ) { $nHID += 22*256; }
            elsif( $LClass eq "numericfield"      ) { $nHID += 24*256; }
            elsif( $LClass eq "metricfield"       ) { $nHID += 26*256; }
            elsif( $LClass eq "currencyfield"     ) { $nHID += 28*256; }
            elsif( $LClass eq "datefield"         ) { $nHID += 30*256; }
            elsif( $LClass eq "timefield"         ) { $nHID += 32*256; }
            elsif( $LClass eq "imageradiobutton"  ) { $nHID += 34*256; }
            elsif( $LClass eq "numericbox"        ) { $nHID += 36*256; }
            elsif( $LClass eq "metricbox"         ) { $nHID += 38*256; }
            elsif( $LClass eq "currencybox"       ) { $nHID += 40*256; }
            elsif( $LClass eq "datebox"           ) { $nHID += 42*256; }
            elsif( $LClass eq "timebox"           ) { $nHID += 44*256; }
            elsif( $LClass eq "imagebutton"       ) { $nHID += 46*256; }
            elsif( $LClass eq "menubutton"        ) { $nHID += 48*256; }
            elsif( $LClass eq "morebutton"        ) { $nHID += 50*256; }
            else {
                $nHID=0;
                $outline = "No LClass ".$helpIDString." ".$nHID;
                #print "$outline\n";
                next;
            }

            #GID und LID auch beruecksichtigen
            $nHID += $LID;
        }
        $nHID += $GID * 16384;  #14 Bit nach links shiften

        # check here and not above to avoid warnings for restypes not generated anyways
        if( $GID == 0 || $GID >32767 ||  $LID > 511 )
        {
            #GID & LID ungueltig
            print STDERR "Invalid Global or Local ID: 0 < GID <= 32767 ; LID <= 511\n";
            print STDERR "$helpIDString GID = $GID; LID = $LID\n";
            next;
        }

        #
        # 1. Stelle selber ausgeben, falls groesser als 2^21
        # wg. problemen von awk/gawk bei printf mit %u
        #
        $x=0;
        if( $nHID >=       4000000000 ) {
            $nHID -=       4000000000;
            $x=4;
        }elsif( $nHID >= 3000000000) {
            $nHID -=       3000000000;
            $x=3;
        }elsif( $nHID >= 2000000000) {
            $nHID -=       2000000000;
            $x=2;
        }
        if( $x != 0)
            { printf HID "%s %d%u \n",$helpIDString,$x, $nHID; }
        else
            { printf HID "%s %u \n",$helpIDString, $nHID; }
    }
}


unlink "$filebase.c";
unlink "$filebase.c1";
unlink "$filebase.c2";
unlink "$filebase.obj";
unlink "$filebase.exe";
