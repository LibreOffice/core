:
    eval 'exec perl -S $0 ${1+"$@"}'
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

sub usage() {
    print "Usage: api-to-idl.pl source.api destination_path\n";
    print;
    print "This tool converts oovbaapi *.api files into *.idl's.\n";
    exit 1;
}

my $src = shift;
my $dest = shift;

if ( !defined( $src ) || !defined( $dest ) || $src eq "-h" || $src eq "--help" ) {
    usage();
}

# Parsing functions
my $state = "";
my $source = "";
my $name = "";
my $value = "";

my %result;

# Process element start event
sub start_element($) {
    my ($el) = @_;

    @element_attr = split( /\s+/, $el );
    my $element = $element_attr[0];

    if ( $element eq "element" ) {
        if ( $element_attr[1] =~ /type="?([^"]*)"?/ && $1 eq "constant" ) {
            $state = "constant";
            $source = "";
            $name = "";
            $value = "";
        }
    }
    elsif ( $state eq "constant" && $element eq "source" ) {
        $state = "source";
        if ( $element_attr[1] =~ /id="?([^"]*)"?/ ) {
            chomp( $source = $1 );
        }
    }
    elsif ( $state eq "source" && $element eq "name" ) {
        $state = "name";
    }
    elsif ( $state eq "source" && $element eq "value" ) {
        $state = "value";
    }
}

# Process element end event
sub end_element($) {
    my ($element) = @_;

    if ( $state eq "name" && $element eq "name" ) {
        $state = "source";
    }
    elsif ( $state eq "value" && $element eq "value" ) {
        $state = "source";
    }
    elsif ( $state ne "" && $element eq "element" ) {
        $state = "";

        my @destination = split( /\./, $source );
        my $module = shift( @destination );
        my $type = shift( @destination );

        $module =~ tr/[A-Z]/[a-z]/;

        $result{$module} = {} unless exists $result{$module};
        $result{$module}{$type} = [] unless exists $result{$module}{$type};

        push( @{$result{$module}{$type}},
              { "name" => $name, "value" => $value } );
    }
}

# Process characters
sub characters($) {
    my ($data) = @_;

    if ( $state eq "name" ) {
        chomp( $name = $data );
    }
    elsif ( $state eq "value" ) {
        chomp( $value = $data );
    }
}

# Create idls from the parsed data
sub generate_idls($) {
    my ($path) = @_;

    foreach $module ( keys %result ) {
        foreach $type ( keys %{$result{$module}} ) {
            my $fname = $path . "/" . $type . ".idl";
            open( IDL, ">$fname" ) || die "Cannot write $fname.";

            if( $module eq "vba" ) {
        print IDL "module ooo { module $module {\n";
        }
        else {
                print IDL "module ooo { module vba { module $module {\n";
            }

            print IDL "    constants $type {\n";
            foreach $constant ( @{$result{$module}{$type}} ) {
                print IDL "        const long $constant->{'name'} = $constant->{'value'};\n";
            }
            if( $module eq "vba" ) {
        print IDL "    };\n}; };\n";
        }
        else {
                print IDL "    };\n}; }; };\n";
            }

            close( IDL );
        }
    }
}

# Parse the input
open( IN, "<$src" ) || die "Cannot open $src.";

my $in_comment = 0;
my $line = "";
while (<IN>) {
    # ignore comments
    s/<!--[^>]*-->//g;
    if ( /<!--/ ) {
        $in_comment = 1;
        s/<!--.*//;
    }
    elsif ( /-->/ && $in_comment ) {
        $in_comment = 0;
        s/.*-->//;
    }
    elsif ( $in_comment ) {
        next;
    }
    # ignore empty lines
    chomp;
    s/^\s*//;
    s/\s*$//;
    next if ( $_ eq "" );

    # take care of lines where element continues
    if ( $line ne "" ) {
    $line .= " " . $_;
    }
    else {
    $line = $_;
    }
    next if ( !/>$/ );

    # the actual parsing
    my @starts = split( /</, $line );
    $line = "";
    foreach $start ( @starts ) {
        next if ( $start eq "" );

        @ends = split( />/, $start );
        my $element = $ends[0];
        my $data = $ends[1];

        # start or end element
        if ( $element =~ /^\/(.*)/ ) {
            end_element( $1 );
        }
        else {
            start_element( $element );
        }

        # the data
        characters( $data );
    }
}
close( IN );

# Generate the output
generate_idls($dest);
