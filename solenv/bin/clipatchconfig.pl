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

sub trim;
sub readRedirectionValues($);

my $usage =
   "Usage is: \n clipatchconfig.pl configTemplate redirections policyConfig

   configTemplate: The config file which is used for the policy assembly. It
   contains place holders for the binding redirection.

   redirections: file containing the values for oldVersion and newVersion tags
   which are used in the BindingRedirect element of the config files.

   policyConfig: Name of the file in which we want to write the config file.
";


if (scalar @ARGV < 3) {
   print $usage;
   exit -1;
}


my %redirectionValue = readRedirectionValues($ARGV[1]);


#Read config file in which we will replace the versions
$/ = undef;
open TEMPLATE, $ARGV[0] or die $!;
my $templ = <TEMPLATE>;

#Open the config file we are goint to write to
open CONFIG, "> $ARGV[2]" or die "Cannot write to $ARGV[2] $!";

#No substitute the place holders for oldVersion and new Version in the config template with
#the values obtained from the redirections file
for (keys %redirectionValue) {
    $templ=~ s/\b$_\b/$redirectionValue{$_}/;
}
#Write the config file
print CONFIG $templ;

#Reads the key value pairs from the files, which name must be passed in
#the parameter. The file contains lines of the form name=value, for example
#CLI_URETYPES_OLD_VERSION=1.1.0.0-1.1.1.0
sub readRedirectionValues($)
{
    #Read in the values for the version redirection
    open REDIR, $_[0] or die $!;

    my %redirectionValues;

    while (<REDIR>)
    {
        chomp;
    my $trimmed;
        #Skip empty lines
        if (length($trimmed = trim($_)) == 0) {
            next;
        }

    #Skip comment symbol: #
    if ($trimmed =~ /^#/) {
        next;
    }

        my @lineParts = split /=/,$_;

        #Check if we have valid name value pairs.
        if (scalar @lineParts != 2) {
            print "Error: Values in $ARGV[1] are not correct (Entries must have the form name=value). Invalid line: \n$_\n";
            exit -1;
        }

        #Trim the strings and check if they still contain characters
        my $name = trim($lineParts[0]);
        my $value = trim($lineParts[1]);
        if (length($name) == 0 || length($value) == 0) {
            print "Error: Values in $ARGV[1] are not correct. Invalid line: \n$_\n";
            exit -1;
        }

        #Check if we have duplicate key names
        for (keys %redirectionValues) {
            if ( $name eq $_) {
                print "Error: Values in $ARGV[1] are not correct. The name $_ is not unique.\n";
                exit -1;
            }
        }

        $redirectionValues{$name} = $value;
    }
    return %redirectionValues;
}

sub trim($)
{
    my $string = shift;
    $string =~ s/^\s+//;
    $string =~ s/\s+$//;
    return $string;
}