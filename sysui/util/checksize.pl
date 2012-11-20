:
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
    # don't check dpc,flag,rpmflag,sdf [obj for UNX] files, or etc subdirectory
    return if ( ($file =~ /.+\.(dpc|\w*?flag)/) || ($file =~ /.+\.obj/ && $ENV{GUI} eq 'UNX') || ($path =~ /.+etc/) || ($path =~ /.+logs/) || ($file =~ /.+sdf/) );
    if ( -z "$path$file" ) {
        print "Error: $path$file 0 Bytes!\n";
        $err++;
    }
 }
