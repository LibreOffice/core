:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
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

my $filename;
my $srs;
my $prjname;
my $defs;
my $solarincludes;
my $verbose = 0;

my $debug = 0;
my $filebase;
my $workfile;
my $shell_workfile;
my @cleanuplist = ();

# variables to setup the compiler line
my $appext;
my $compiler;
my $outbin_flag;
my $outobj_flag;
my $objext;
my $preprocess_flag; # preprocess to stdout

my $no_hid_files;

sub cleandie
{
    my $errstring = shift @_;
    my $erroreval = $@;

    print STDERR "$errstring\n";
    if ( not $debug ) {
        foreach my $i (@cleanuplist) {
            if ( -f "$workfile$i" ) {
                unlink "$workfile$i" or print STDERR "ERROR - couldn't remove $workfile$i\n";
            }
        }
    }
    die "$erroreval\n";
}

sub setcompiler
{
    my $whichcom = $ENV{COM};
    my $extra_cflags = $ENV{EXTRA_CFLAGS};
    $extra_cflags = "" if (!$extra_cflags);
    if ( "$whichcom" eq "GCC" ) {
        $appext = ""; # windows for now
        $compiler = "gcc -x c $extra_cflags";
        $outbin_flag = "-o ";
        $outobj_flag = "";
        $objext = ".o";
        $preprocess_flag = "-E"; # preprocess to stdout
    } elsif ( "$whichcom" eq "MSC" ) {
        $appext = ".exe"; # windows for now
        $compiler = "cl -nologo";
        $outbin_flag = "-Fe";
        $outobj_flag = "-Fo";
        $objext = ".obj";
        $preprocess_flag = "-EP"; # preprocess to stdout
        $solarincludes =~ s/\/stl/\/xstlx/g;
        $defs =~ s/\/stl/\/xstlx/g;
    } elsif ( "$whichcom" eq "C52" ) {
        $appext = ""; # windows for now
        $compiler = "cc";
        $outbin_flag = "-o ";
        $outobj_flag = "";
        $objext = ".o";
        $preprocess_flag = "-E"; # preprocess to stdout

        # hack for SO cc wrapper
        $ENV{wrapper_override_cc_wrapper} = "TRUE";
        $solarincludes =~ s/stl/xstlx/g;
        $defs =~ s/\/stl/\/xstlx/g;
    } else {
        print STDERR "----------------------------------------------------------------------\n";
        print STDERR "OOops... looks like your compiler isn't known to \n$0\n";
        print STDERR "please edit the \"setcompiler\" section of this script to make it work.\n";
        print STDERR "----------------------------------------------------------------------\n";
        die "ERROR - compiler (or \$COM settings) unknown!\n";
    }
}

#---------------------------------------------------
$filename = undef;
$srs = undef;
$prjname = undef;

my @expectedArgs = ( \$filename, \$srs, \$prjname );
my $expectedArgsIndex = 0;
while ( ( $#ARGV >= 0 ) && ( $expectedArgsIndex < 3 ) )
{
    $_ = shift @ARGV;
    if ( /^-verbose$/ )
    {
        $verbose = 1;
        next;
    }
    ${$expectedArgs[ $expectedArgsIndex ]} = $_;
    ++$expectedArgsIndex;
}

$defs = join " ",@ARGV if ($#ARGV);

if ( !defined $prjname ) { die "ERROR - check usage\n"; }

if ( $ENV{NO_HID_FILES} ) {
    $no_hid_files = $ENV{"NO_HID_FILES"};
}
$solarincludes = $ENV{SOLARINCLUDES};
if (defined $ENV{TMPDIR}) {
    $tmpdir = $ENV{TMPDIR};
} elsif (defined $ENV{TMP}) {
    $tmpdir = $ENV{TMP};
} else {
    die "ERROR - \"TMPDIR\" & \"TMP\" environment variables not set\n";
};
die "ERROR - \"$tmpdir\" doesn't exist\n" if ( ! -d $tmpdir );

setcompiler();

# convert windows only?
$srs =~ s/\\/\//g;
$filename =~ s/\\/\//g;

$filebase = $filename;
$filebase =~ s/.*[\\\/]//;
$filebase =~ s/\..*?$//;
# now stript it to something that doesn't togger vista execution prevention :(
$flbs = $filebase;
$flbs =~ s/[aeiou]//g;
# call srand ony once per script!
srand();
$workfile = "$tmpdir/${flbs}_".$$.rand();

# now get $workfile ready for shell usage...
$shell_workfile = $workfile;

print "workfile: $workfile\n" if $verbose;

#remove old objects which remained in place by a former bug
unlink "$workfile.obj";

# can't do this for modules with mixed case!

if ( -f "$workfile.hid" )
{
    unlink "$workfile.hid" or die "ERRROR - cannot remove $workfile.hid\n";
}

# hack to quit for files which cannot be handled
if ( defined $ENV{"NO_HID_FILES"} ) {
    foreach $fname ( split / /, $ENV{"NO_HID_FILES"} )
    {
        if ( $fname eq $filename )
        {
            print "No hid generation for $filename due to NO_HID_FILES\n";
            print "Touching $srs/$filebase.hid anyways\n";
            open TOUCH, ">$srs/$filebase.hid" or die "ERRROR - cannot open $srs/$filebase.hid for writing\n";
            close TOUCH;
            exit 0;
        }
    }
}

my $verboseSwitch = $verbose ? "-verbose" : "";
print         "$ENV{SOLARBINDIR}/hidc $verboseSwitch $filename ${shell_workfile}.c1 $prjname\n" if $verbose;
$ret = system "$ENV{SOLARBINDIR}/hidc $verboseSwitch $filename ${shell_workfile}.c1 $prjname";
if ( $ret ) {
    push @cleanuplist, ".c1";
    cleandie("ERROR - calling \"hidc\" failed");
}
push @cleanuplist, ".c1";

print         "$compiler $defs $solarincludes $preprocess_flag ${shell_workfile}.c1 > ${shell_workfile}.c2\n" if $verbose;
$ret = system "$compiler $defs $solarincludes $preprocess_flag ${shell_workfile}.c1 > ${shell_workfile}.c2";
if ( $ret ) {
    push @cleanuplist, ".c2";
    cleandie("ERROR - calling compiler for preprocessing failed");
}
push @cleanuplist, ".c2";

if (!open C_PROG, ">$workfile.c") {
    push @cleanuplist, ".c";
    cleandie("ERROR - open $workfile.c\n for writing failed");
}
push @cleanuplist, ".c";
print C_PROG "#include <stdio.h>\n";
print C_PROG "#include <wctype.h>\n";

if ( !open PRE, "<$workfile.c2" ) {
    cleandie("ERROR - open $workfile.c2\n for reading failed");
}

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

#cl %SOLARINCLUDES% %_srs%\%_workfile%.c /Fe%_srs%\%_workfile%$appext
my $outobj_param = "";
if ( $outobj_flag ne "" )
{
    $outobj_param = "$outobj_flag${shell_workfile}$objext";
}
print         "$compiler $defs $solarincludes ${shell_workfile}.c $outobj_param $outbin_flag${shell_workfile}$appext \n" if $verbose;
$ret = system "$compiler $defs $solarincludes ${shell_workfile}.c $outobj_param $outbin_flag${shell_workfile}$appext";
if ( $ret ) {
    push @cleanuplist, "$appext";
    cleandie("ERROR - compiling $workfile.c failed");
}
push @cleanuplist, "$objext";
push @cleanuplist, "$appext";

#awk -f %ENV_TOOLS%\hidcode.awk < %srs%\%workfile%.c3 > %srs%\%workfile%.hid
if ( !open C3,"$workfile$appext|" ) {
    cleandie("ERROR - executing $workfile$appext failed");
}
if ( !open HID,">$srs/$filebase.hid.$ENV{INPATH}" ) {
    cleandie("ERROR - open $srs/$filebase.hid.$ENV{INPATH} for writing failed");
}

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

close C3;
close HID;

rename("$srs/$filebase.hid.$ENV{INPATH}", "$srs/$filebase.hid") or cleandie("ERROR - couldn't rename tmp file to final for $filebase");

if ( not $debug ) {
    foreach my $i (@cleanuplist) {
        sleep 1;
        if ( -f "$workfile$i" ) {
            unlink "$workfile$i"  or cleandie("");
        }
    }
}
