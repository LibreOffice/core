:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: mhids.pl,v $
#
#   $Revision: 1.10 $
#
#   last change: $Author: kz $ $Date: 2008-03-05 16:25:00 $
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

my $filename;
my $srs;
my $prjname;
my $defs;
my $solarincludes;

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
            unlink "$workfile$i" if -f "$workfile$i" or print STDERR "ERROR - couldn't remove $workfile$i\n";
        }
    }
    die "$erroreval\n";
}

sub setcompiler
{
    my $whichcom = $ENV{COM};
    if ( "$whichcom" eq "GCC" ) {
        $appext = ""; # windows for now
        $compiler = "gcc -x c";
        $outbin_flag = "-o ";
        $outobj_flag = "";
        $objext = ".o";
        $preprocess_flag = "-E"; # preprocess to stdout
    } elsif ( "$whichcom" eq "MSC" ) {
        $appext = ".exe"; # windows for now
        $compiler = "cl";
        $outbin_flag = "-Fe";
        $outobj_flag = "-Fo";
        $objext = ".obj";
        $preprocess_flag = "-EP"; # preprocess to stdout
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
$filename = shift @ARGV;
$srs = shift @ARGV;
$prjname = shift @ARGV;
$defs = join " ",@ARGV if ($#ARGV);

if ( !defined $prjname ) { die "ERROR - check usage\n"; }

if ( $ENV{NO_HID_FILES} ) {
    $no_hid_files = $ENV{"NO_HID_FILES"};
}
$solarincludes = $ENV{SOLARINCLUDES};
$tmpdir = $ENV{TMP};
die "ERROR - \"TMP\" environment variable not set\n" if ( !defined $tmpdir );
die "ERROR - \"$tmpdir\" doesn't exist\n" if ( ! -d $tmpdir );

setcompiler();

# convert windows only?
$srs =~ s/\\/\//g;
$filename =~ s/\\/\//g;

$filebase = $filename;
$filebase =~ s/.*[\\\/]//;
$filebase =~ s/\..*?$//;
$workfile = "$tmpdir/${filebase}_".$$;

# now get $workfile ready for shell usage...
$shell_workfile = $workfile;
$shell_workfile =~ s/\//\\/g if ( "$ENV{USE_SHELL}" eq "4nt" );
if (( "$ENV{USE_SHELL}" eq "4nt" ) && ( "$^O" eq "cygwin" )) {
    $shell_workfile =~ s/\//\\\\/;
}

print "workfile: $workfile\n";

#remove old objects which remained in place by a former bug
unlink "$workfile.obj";

# can't do this for modules with mixed case!
#$prjname =~ lc $prjname;

if ( -f "$workfile.hid" )
{
    unlink "$workfile.hid" or die "ERRROR - cannot remove $workfile.hid\n";;
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

#echo "perl5 -p -e "s/=[ \t]*\".*\"/=\"\"/go; s/\".*\"[ \t]*;/\"\" ;/go ; s/(\".*)\/\/(.*\")/$1\/\\\/$2/go ;" < %filename% > %srs%\%workfile%.c0"
#call  perl5 -p -e "s/=[ \t]*\".*\"/=\"\"/go; s/\".*\"[ \t]*;/\"\" ;/go ; s/(\".*)\/\/(.*\")/$1\/\\\/$2/go ;" < %filename% > %srs%\%workfile%.c0

print  "hidc $filename ${shell_workfile}.c1 $prjname \n";
$ret = system "hidc $filename ${shell_workfile}.c1 $prjname";
if ( $ret ) {
    push @cleanuplist, ".c1";
    cleandie("ERROR - calling \"hidc\" failed");
}
push @cleanuplist, ".c1";

print         "$compiler $solarincludes $defs $preprocess_flag ${shell_workfile}.c1 > ${shell_workfile}.c2\n";
$ret = system "$compiler $solarincludes $defs $preprocess_flag ${shell_workfile}.c1 > ${shell_workfile}.c2";
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
print         "$compiler $solarincludes $defs ${shell_workfile}.c $outobj_param $outbin_flag${shell_workfile}$appext \n";
$ret = system "$compiler $solarincludes $defs ${shell_workfile}.c $outobj_param $outbin_flag${shell_workfile}$appext";
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

close C3;
close HID;

rename("$srs/$filebase.hid.$ENV{INPATH}", "$srs/$filebase.hid") or cleandie("ERROR - couldn't rename tmp file to final for $filebase");

if ( not $debug ) {
    foreach my $i (@cleanuplist) {
        if ( -f "$workfile$i" ) {
            unlink "$workfile$i"  or cleandie("");
        }
    }
}
