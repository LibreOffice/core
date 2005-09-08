:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: checkxml.pl,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 16:31:30 $
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
#
#
# check_xml.pl - check xml,xcs,xcu files size, NULL character
#

my
$is_debug=0;
my $err = 0;
my $path = $ENV{'SOLARVERSION'} . '/' . $ENV{'INPATH'} . '/xml' . "$ENV{'UPDMINOREXT'}/";
my $pck_path = $ENV{'SOLARVERSION'} . '/' . $ENV{'INPATH'} . '/pck' . "$ENV{'UPDMINOREXT'}/";
my $unzipexe="unzip";

#Path of the directory from which the recursion starts (must have ending '/').
print "Checking:$path\n";
# Initiate the recursion
&RecurseDirs($path);
$err += &check_registry_zips($pck_path);
if ($err > 0)
{
    print "Error: $err damaged files encountered\n";
    exit(1); # stop dmake
} else
{
    print "ok.\n";
}
exit;

#### SUBROUTINES SECTION ####

# Function that recurses through the directory tree calling FileFunction on all files
sub RecurseDirs {
    my ($path) = @_;
    my $file;    #Variable for a file

    opendir (DIRECTORY, $path) or
        die "Can't read $path\n";
    my @all_files = grep (!/^\.\.?$/, readdir (DIRECTORY)); #Read all the files except for '.' and '..'
    closedir (DIRECTORY);

    foreach $file (@all_files) {
        if (-d "$path$file/") {
            &RecurseDirs("$path$file/");
        } else {
            &check($path, $file);
        }
    }
}

############################################################################
sub check       #04.02.2005 13:40
############################################################################
 {
    my $path = shift;
    my $file = shift;
    print "$path$file\n" if ((-e "$path$file") && $is_debug);
    return if ( $file !~ /.+\.(xcu|xml|xcs)/ ); #check xml and xcu files only
    if ( -z "$path$file" ) {
        print "Error: $path$file 0 Bytes!\n";
        $err++;
    } else
    {
        open( FH, "<$path$file" );
        while ( $line = <FH> ) {
        #print $line;
            if ( $line =~ /\000+/ ) {
                print "Error: NULL characters detected in $path$file\n";
                $err++;
            }
        }
        close(FH);
    }
 }

 ############################################################################
 sub check_registry_zips        #20.04.2005 18:47
 ############################################################################
  {
    my $path = shift;
    my $error = 0;
    my $commandargs;
    opendir (DIRECTORY, $path) or
        die "Can't read $path\n";
    my @all_files = grep (!/^\.\.?$/, readdir (DIRECTORY)); #Read all the files except for '.' and '..'
    closedir (DIRECTORY);
    foreach $file (@all_files) {
        if ( $file =~ /registry_.+\.zip$/ ) {
            $commandargs="$path$file";
            # Cygwin's perl needs escaped \ in system() and open( COMMAND ... )
            if ( "$^O" eq "cygwin" ) { $commandargs =~ s/\\/\\\\/g; }
            print "file=$commandargs\n" if ($is_debug);
            open(UNZIP,"$unzipexe -l $commandargs |");
            my $ferror = 0;
            while ( $line = <UNZIP> ) {
                #print $line;
                my @param = split(" ",$line);
                if ( $param[0] =~ /\d+/ ) {
                    if ( $param[0] == 0 && $param[3] =~ /.+\.xcu$/)
                    {
                        $error++; $ferror=1;
                    }
                }
            }
            if ( $ferror ) {
                print "Error: $commandargs contains files with 0 byte size\n";
            }
            close(UNZIP);
        }
    }

    ($error);
 }  ##check_registry_zips
