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

use warnings;
use strict;
use diagnostics;

sub trim;
sub readIncVersions($);
sub processLine($$);
sub checkName($);
sub incrementNewVersion($);
sub incrementOldVersion($);
sub incrementPolicyVersion($);
my $usage =
"The tool increments the minor version of assemblies and the major version of ".
"the respective policy files. This is only done if new uno types have been added since".
"the last product upate. This information is obtained from the file which is passed as ".
"argument changedTypes. The names in the version file must have a particular form. ".
"They must end on one of folling terms: NEW_VERSION, OLD_VERSION, POLICY_VERSION\n".
"If no new published types habe been added then no output, argument  newVersions, is written".
"Usage is: \n increment_version.pl oldVersions  incVersions newVersions changedTypes\n\n".
"oldVersion: Contains name value pairs, which are used for forming the config files of ".
"the policy assemblies, for building the assemblies. \n\n".
"incVersions: File containing the names of which the versions are to be incremented. ".
"Every line may only contain one name. The names must exactly match those from the ".
"oldVersion file.\n\n".
"newVersions: Contains all entries from oldVersions, but the values of the names,".
"which occur in selection, have been incremented.\n\n".
"changedTypes: File that contains the information if new published types have been added ".
"since the last product update.\n\n" ;

my $sNameForm =
"The names must end on one of these names: NEW_VERSION, OLD_VERSION, POLICY_VERSION\n".
"For example, valid names are: \n".
"CLI_URETYPES_NEW_VERSION\nCLI_URETYPES_OLD_VERSION\nCLI_URETYPES_POLICY_VERSION\n";

if (scalar @ARGV < 4) {
   print $usage;
   exit -1;
}

-e "$ARGV[0]" or die "Error: wrong arguments. \n".$usage;
-e "$ARGV[1]" or die "Error: wrong arguments. \n".$usage;
-e "$ARGV[3]" or die "Error: wrong arguments. \n".$usage;

#check if new types have been added since last release.
#If not, then there is nothing to be done.
#read in oldVersions line by line and apply the increment operation
open TYPES, "$ARGV[3]" or die "Cannot open to $ARGV[3] $!";

my $newTypes;

#We look for the line that contains the number of new types
while(<TYPES>)
{
    if (/New and published types/i)
    {
    $_ =~ /=\s*(\d+)/;
    if ( ! defined $1)
    {
        print "\n###$ARGV[3] contains an invalid entry for 'New and published types'.  \n\n";
        exit -1;
    }
    $newTypes = $1;
    }
}

#Check if changeTypes contained the line we are looking for
if (! defined $newTypes)
{
    print "\n###$ARGV[3] does not contain entry about the new types ".
    "or we are looking for the wrong string! \n\n";
    exit -1;
}

if ( $newTypes == 0)
{
    print "\nNo new UNO types since las product update.\n";
    exit 0;
}
else
{
    print "\nNew UNO types were addes since last release. The version will be increased.\n\n";
}

#read in incVersions in a list
my @incVersions = readIncVersions($ARGV[1]);
#print "@incVersions";

#read in oldVersions line by line and apply the increment operation
open OLDVERSION, "$ARGV[0]" or die "Cannot open to $ARGV[0] $!";

#open file we want to write to
open NEWVERSION, "> $ARGV[2]" or die "Cannot write to $ARGV[2] $!";

print NEWVERSION processLine($_, @incVersions) while(<OLDVERSION>);

close NEWVERSION;
close OLDVERSION;

exit 0;

sub processLine($$)
{
    my $line = $_[0];
    #skip empty lines
    my $trimmed;
    return $line if (length($trimmed = trim($line)) == 0);
    #Skip comment symbol: #
    return $line if ($trimmed =~ /^#/);

    #Get the left part of '='
    my $i = index($line, "=");
    if( $i == -1)
    {
    print "Error: No '=' found in line:,: \n $line \n";
    exit -1;
    }
    my $name = substr($line, 0, $i);
    $name = trim($name);
    #We do not check the names here because the file can contain
    #other names, e.g. CLI_URETYPES_POLICY_ASSEMBLY
    if (length($name) == 0) {
    print "Wrong line in $ARGV[0]\n", $sNameForm;
    exit -1;
    }
    my $value = substr($line, $i + 1);
    $value = trim($value);

    #Check if the entry shall be incremented, this information is in the second
    #argument
    my $found;
    for(@incVersions) {
    if ($_ eq $name) {
        $found = 1;
        last;
    }
    }
    if ( ! defined($found)) {
     return $line;
    }

    #Check if the name represents a version we need to change
    if ($name =~ /NEW_VERSION$/)
    {
    $value = incrementNewVersion($value);
    }
    elsif ($name =~ /OLD_VERSION$/)
    {
    $value = incrementOldVersion($value);
    }
    elsif ($name =~ /POLICY_VERSION$/)
    {
    $value = incrementPolicyVersion($value);
    }
    else
    {
    #other name which we ignore
    return $line;
    }
    return "${name}=${value}\n";
}

#The value of a new version has the form x.x.x.x
#We increment the third position from the left.
#Te argument must already be trimmed.
sub incrementNewVersion($)
{
   my @parts = split /\./,$_[0];
   if (scalar @parts != 4)
   {
       print "Error, no valid version given in $ARGV[0]\n. A 'new version' has four parts.";
       exit -1;
   }
   $parts[2]++;
   #build the version string and return
   return "$parts[0].$parts[1].$parts[2].$parts[3]";
}

#The value of a new version has the form x.x.x.x-x.x.x.x
#We increment the third position of the second part.
#Te argument must already be trimmed.
sub incrementOldVersion($)
{
    my @parts = split /[\.-]/,$_[0];
    if (scalar @parts != 8)
    {
    print "Error, no valid version given in $ARGV[0]\n. A 'old version' has the form
x.x.x.x-x.x.x.x\n.";
    exit -1;
    }
    $parts[6]++;
    return "$parts[0].$parts[1].$parts[2].$parts[3]-$parts[4].$parts[5].$parts[6].$parts[7]";
    return $_[0];
}

sub incrementPolicyVersion($)
{
   my @parts = split /\./,$_[0];
   if (scalar @parts != 4)
   {
       print "Error, no valid version given in $ARGV[0]\n. A 'policy version' has four parts.";
       exit -1;
   }
   $parts[0]++;
   #build the version string and return
   return "$parts[0].$parts[1].$parts[2].$parts[3]";
}


sub readIncVersions($)
{
    open INC, $_[0] or die "Could not open $_[0] $!";
    my $arg = $_[0];
    my @names;

    while(<INC>)
    {
    chomp;
    #Skip empty lines
    my $line;
    if (length($line = trim($_)) == 0) {
        next;
    }
    #Skip comment symbol: #
    if ($line =~ /^#/) {
        next;
    }
    if (!checkName($line)) {
        print "Wrong entry in file $_[0]\n", $sNameForm;
        exit -1;
    }
    push @names, $line;
    }
    print "No entries found in $arg\n" if(scalar @names == 0);
    return @names;
}

#The argument must already be trimmed
#returns 1 if ok
sub checkName($)
{
    my $name = $_[0];
    if ( $name !~/NEW_VERSION$|OLD_VERSION$|POLICY_VERSION$/) {
    return 0;
    }
    return 1;
}

sub trim($)
{
    my $string = shift;
    $string =~ s/^\s+//;
    $string =~ s/\s+$//;
    return $string;
}
