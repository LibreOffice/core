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
# check_xml.pl - check ui,xml,xcs,xcu files size, NULL character
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
    return if ( $file !~ /.+\.(ui|xcu|xml|xcs)/ ); #check ui, xml and xcu files only
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
