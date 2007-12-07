:
    eval 'exec perl -S $0 ${1+"$@"}'
        if 0;
#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: api-to-idl.pl,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: vg $ $Date: 2007-12-07 11:17:00 $
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

            print IDL "module org { module openoffice { module $module {\n";
            print IDL "    constants $type {\n";
            foreach $constant ( @{$result{$module}{$type}} ) {
                print IDL "        const long $constant->{'name'} = $constant->{'value'};\n";
            }
            print IDL "    };\n}; }; };\n";

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
