:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
#*************************************************************************
#
#   $RCSfile: checkxml.pl,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: obo $ $Date: 2005-02-09 08:59:14 $
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
#
#
# check_xml.pl - check xml,xcs,xcu files size, NULL character
#

my
$is_debug=0;
my $err = 0;
my $path = $ENV{'SOLARVERSION'} . '/' . $ENV{'INPATH'} . '/xml/';
#Path of the directory from which the recursion starts (must have ending '/').
print "Checking:$path\n";

# Initiate the recursion
&RecurseDirs($path);
if ($err > 0)
{
    print "$err damaged files encountered\n";
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
