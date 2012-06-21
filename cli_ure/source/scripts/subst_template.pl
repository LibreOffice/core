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

use warnings;
use strict;
use diagnostics;

sub trim;
sub readRedirectionValues($);

my $usage =
   "Usage is: \n subst_template.pl configTemplate redirections policyConfig

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
#print "|$_|  |$redirectionValue{$_}|\n",  for keys %redirectionValue;


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
#CLI_TYPES_OLD_VERSION=1.1.0.0-1.1.1.0
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
