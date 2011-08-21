:
eval 'exec perl -wS $0 ${1+"$@"}'
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
# Description: Wrapper script to change '/' to '\' in command-line
# arguments.

#---------------------------------------------------------------------------
# external modules
use Text::ParseWords;

# global vars
@params = ();

#---------------------------------------------------------------------------
# procedures


#----------------------------------------------------------
# Function name: WinFormat
# Description:   Format variables to Windows Format.
# Arguments:     1. Variable (string) with one token
# Return value:  Reformatted String
#----------------------------------------------------------
sub WinFormat {
    my $variable = shift @_;

    $variable =~ s!(.)/!$1\\!g; # Replace all but the leading slashes with backslashes

    if ( defined $debug ) {
        print(STDERR "WinFormat:\nresult:$variable\n");
    }

    return $variable;
}

#----------------------------------------------------------
# Function name: replace_cyg
# Description:   Process all arguments and change them to Windows Format.
# Arguments:     Reference to array with arguments
# Return value:  -
#----------------------------------------------------------
sub replace_cyg {
    my $args = shift;
    my( @cmd_file, @cmd_temp );
    my $atchars;
    foreach my $para ( @$args ) {
        if ( $para =~ "^@" ) {
            # it's a command file
            if ( defined $debug ) {
                print(STDERR "----------------------------\n");
            }
            ;
            # Workaround, iz28717, keep number of @'s.
            $para =~ s/(^\@+)//;
            $atchars = $1;
            $filename = $para;
            if ( defined $debug ) {
                print(STDERR "filename = $filename \n");
            }
            ;
            # open this command file for reading
            open(CMD, "$filename");
            while ( <CMD> ) {
                # Remove DOS lineendings. Bug in Cygwin / Perl?
                $_ =~ s/\r//g;
                # Remove lineendings and trailing spaces. ( Needed by &parse_line )
                $_ =~ s/\n$//g;
                $_ =~ s/\s+$//g;
                # Fill all tokens into array
                @cmd_temp = &parse_line('\s+', 1, $_ );
                if ( $#cmd_temp > -1 ) {
                    push( @cmd_file, @cmd_temp);
                }
            }
            close(CMD);
            # reformat all tokens
            replace_cyg(\@cmd_file);
            if ( defined $debug ) {
                print(STDERR "Tokens processed:\n");
            }
            ;
            foreach $i (@cmd_file) {
                if ( defined $debug ) {
                    print(STDERR "!".$i."!\n");
                }
                ;
            }
            # open this filename for writing (truncate) Textmode?
            open(CMD, '>', $filename);
            # write all tokens back into this file
            print(CMD join(' ', @cmd_file));
            close(CMD);
            # convert '@filename' to dos style
            $para = WinFormat( $para );
            if ( defined $debug ) {
                print(STDERR "----------------------------\n");
            }
            ;
            if ( (defined $debug_light) or (defined $debug) ) {
                print(STDERR "\nParameter in File:".join(' ', @cmd_file).":\n");
            }
            $para = $atchars.$para;
        } else {
            # it's just a parameter
            if ( defined $debug ) {
                print(STDERR "\nParameter:---${para}---\n");
            }
            ;
            # If $tmp1 is empty then $para is a parameter.
            my $is_no_para = 1;
            # remove .exe and convert to lower case
            $shortcommand = lc $command ;
            $shortcommand =~ s/\.exe$//;
            $shortcommand =~ /([^\/]+$)/;
            $shortcommand = $1;
            if ( $is_no_para ) {
                $para = WinFormat($para);
            }
            if ( defined $debug ) {
                print(STDERR "Converted line:${para}:\n" );
            }
        }                       # else
    }                           # foreach loop
}


#---------------------------------------------------------------------------
# main
@params = @ARGV;

$command = shift(@params);

while ( $command =~ /^-/ )
{
    if ( $command eq "-dbg" ) {
        $debug="true";
    }
    elsif ( $command eq "-ldbg" ) {
        $debug_light="true";
    }

    $command = shift(@params);
}

if ( (defined $debug_light) or (defined $debug) ) { print( STDERR "Command: $command\n" ); }

replace_cyg(\@params);
if ( (defined $debug_light) or (defined $debug) ) { print(STDERR "\n---------------------\nExecute: $command @params\n----------------\n");};
exec( "$command", @params) or die( "\nError: slfl.pl: executing $command failed!\n" );

