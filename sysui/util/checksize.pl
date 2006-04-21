:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: checksize.pl,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: rt $ $Date: 2006-04-21 10:48:45 $
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
#

my
$is_debug=0;
my $err = 0;
my $path = "../" . $ENV{'INPATH'} . "/";

#Path of the directory from which the recursion starts (must have ending '/').
print "Checking:$path\n";
# Initiate the recursion
&RecurseDirs($path);
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
        die "Error: Can't read $path\n";
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
    # don't check dpc,flag,rpmflag, [obj for UNX] files, or etc subdirectory
    return if ( ($file =~ /.+\.(dpc|\w*?flag)/) || ($file =~ /.+\.obj/ && $ENV{GUI} eq 'UNX') || ($path =~ /.+etc/) );
    if ( -z "$path$file" ) {
        print "Error: $path$file 0 Bytes!\n";
        $err++;
    }
 }
