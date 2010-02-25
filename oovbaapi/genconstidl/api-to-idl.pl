:
    eval 'exec perl -S $0 ${1+"$@"}'
        if 0;
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
