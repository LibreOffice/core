:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
#*************************************************************************
#
#   $RCSfile: guw.pl,v $
#
#   $Revision: 1.16 $
#
#   last change: $Author: rt $ $Date: 2004-07-13 16:38:50 $
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
#   Contributor(s): Volker Quetschke <quetschke@scytek.de>
#
#
#
#*************************************************************************
# Description: ??

#---------------------------------------------------------------------------
# external modules
use Text::ParseWords;

# global vars
@params = ();

# set debug mode here:
#$debug="true";
#$debug_light="true";

#---------------------------------------------------------------------------
# Define known parameter exceptions
%knownpara = ( 'echo', [ '/TEST', 'QQQ', 'CCC' ],
               'cl', [ '-clr:' ],
               'csc', [ '-target:', '-out:', '-reference:' ],
               'lib', [ 'OUT:', 'EXTRACT:','out:', 'def:', 'machine:' ],
               'link', [ 'BASE:', 'DEBUG', 'DLL', 'LIBPATH', 'MACHINE:',
                         'MAP', 'NODEFAULTLIB', 'OPT', 'PDB', 'RELEASE',
                         'SUBSYSTEM', 'STACK', 'out:', 'map:', 'ENTRY:',
                         'implib:', 'def' ],
               'regcomp', [ '-env:', 'vnd.sun.star.expand:' , 'vnd.openoffice.pymodule' ],
               'regmerge', [ '/UCR' ] );

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
  my( $d1, $d1_prefix, $d2 );

  $variable =~ s/(\$\w+)/$1/eeg ; # expand the variables
  $variable =~ s/(\$\w+)/$1/eeg ; # expand the variables twice!
  $variable =~ s/:/;/g;
  $variable =~ s/([;]|\A)(\w);/$1$2:/g; # get back the drives

  # Search for posix path ;entry; and replace with cygpath -w entry, accept quotes.
  # iz28717 Accept ',' as path seperator.
  while ( $variable =~ /(?:[;,]|\A)[\'\"]?((?:\/[\w\.\- ~]+)+)[\'\"]?(?:[;,]|\Z)/ ) { # Normal paths
    if ( defined $debug ) { print(STDERR "WinFormat:\nnormal path:\n$variable\n");};
    $d1 = $1 ;
    chomp( $d2 = qx{cygpath -w "$d1"} ) ;
    $variable =~ s/$d1/$d2/ ;
  }

  # Include paths or parameters with filenames
  if ( $variable =~ /\A(-\w)[\'\"]?((?:\/[\w\.\- ~]+)+\/?)[\'\"]?\Z/ ) {
      # This regex evaluates -X<path>, sometimes with quotes or "/" at the end
      # option -> $1, filename without quotes -> $2
      if ( defined $debug ) { print(STDERR "WinFormat:\ninclude (-X<path>) path:\n$variable\n"); }
      $d1_prefix = $1;
      $d1 = $2;
  } elsif ( $variable =~ /\A(-?\w[\w\.]*=)[\'\"]?((?:\/[\w\.\- ~]+)+\/?)[\'\"]?\Z/ ) {
      # This regex evaluates [-]X<something>=<path>, sometimes with quotes or "/" at the end
      # option -> $1, filename without quotes -> $2
      if ( defined $debug ) { print(STDERR "WinFormat:\ninclude ([-]<something>=<path>) path:\n$variable\n"); }
      $d1_prefix = $1;
      $d1 = $2;
  } else {
      $d1 = "";
  }
  if ( $d1 ne "" ) {
    # Some programs (e.g. rsc have problems with filenames with spaces), use short dos paths
    if ( $d1 =~ / / ) {
        chomp( $d1 = qx{cygpath -d "$d1"} );
    } else {
        chomp( $d1 = qx{cygpath -w "$d1"} );
    }
    # "cygpath -d" returns "" if the file doesn't exist.
    if ($d1 eq "") {
      $d1 = ".";
      print(STDERR "Error: guw.pl: Option:$variable:\nhas a problem! Probably nonexistent filename with space.\n");
    }
    $variable = $d1_prefix.$d1;
  }

  # Sanity check for -X<path>
  if ( $variable =~ /-\w[\'\"]?(?:(?:\/[\w\.\- ~]+)+)/ ) {
      print(STDERR "Error: guw.pl: WinFormat: Not converted -X/... type switch in :$variable:.\n");
      if ( (defined $debug_light) or (defined $debug) ) { die "\nNot processed -X/...\n"; }
  }
  # Sanity check for [-]X<something>=<path> case
  if ( $variable =~ /-?\w[\w\.]*=[\'\"]?(?:\/[\w\.\- ~]+)+/ ) {
      print(STDERR "Error: guw.pl: WinFormat: Not converted [-]X<something>=<path> type switch in :$variable:.\n");
      if ( (defined $debug_light) or (defined $debug) ) { die "\nNot processed [-]X<something>=/...\n"; }
  }

  $variable =~ s/\//\\/g;       # Remaining \ come from e.g.: ../foo/baa
  $variable =~ s/^\\$/\//g; # a single "/" needs to be preserved

  if ( defined $debug ) { print(STDERR "WinFormat:\nresult:$variable\n");};
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
    foreach my $para ( @$args )
      {
        if ( $para =~ "^@" ) {
          # it's a command file
          if ( defined $debug ) { print(STDERR "----------------------------\n");};
          # Workaround, iz28717, keep number of @'s.
          $para =~ s/(^\@+)//;
          $atchars = $1;
          $filename = $para;
          if ( defined $debug ) { print(STDERR "filename = $filename \n");};
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
          if ( defined $debug ) { print(STDERR "Tokens processed:\n");};
          foreach $i (@cmd_file) {
            if ( defined $debug ) { print(STDERR "!".$i."!\n");};
          }
          # open this filename for writing (truncate) Textmode?
          open(CMD, '>', $filename);
          # write all tokens back into this file
          print(CMD join(' ', @cmd_file));
          close(CMD);
          # convert '@filename' to dos style
          $para = WinFormat( $para );
          if ( defined $debug ) { print(STDERR "----------------------------\n");};
          if ( (defined $debug_light) or (defined $debug) ) { print(STDERR "\nParameter in File:".join(' ', @cmd_file).":\n");}
          $para = $atchars.$para;
        } else {
          # it's just a parameter
          if ( defined $debug ) { print(STDERR "\nParameter:---${para}---\n");};
          # If $tmp1 is empty then $para is a parameter.
          my $is_no_para = 1;
          # remove .exe and convert to lower case
          $shortcommand = lc $command ;
          $shortcommand =~ s/\.exe$//;
          $shortcommand =~ /(\w+$)/;
          $shortcommand = $1;
          foreach $i (@{$knownpara{$shortcommand}}) {
            if( $para =~ /$i/ ) {
              $is_no_para = 0;
              if ( defined $debug ) { print(STDERR "Is parameter exception for ${shortcommand}: ${para}:\n" );};
              last;
            }
          }
          if( $is_no_para ) {
            $para = WinFormat($para);
          }
          if ( defined $debug ) { print(STDERR "Converted line:${para}:\n" );};
        } # else
      } # foreach loop
}

#----------------------------------------------------------
# Function name: replace_cyg_env
# Description:   Process selected environment variables and change
#                them to Windows Format.
# Arguments:     -
# Return value:  -
#----------------------------------------------------------
sub replace_cyg_env {
    @affected_vars = (
        'SOLAR_VERSION',
        'SOLARVERSION',
        'SOLARVER',
        'SRC_ROOT',
        'LOCALINI',
        'GLOBALINI',
        'SOLARENV',
        'STAR_INSTPATH',
        'STAR_SOLARPATH',
        'STAR_PACKMISC',
        'STAR_SOLARENVPATH',
        'STAR_INITROOT',
        'STAR_STANDLST'
    );
    foreach my $one_var ( @affected_vars )
    {
        my $this_var = $ENV{ $one_var };
        if ( defined $this_var )
        {
            if ( defined $debug ) { print(STDERR "ENV $one_var before: ".$ENV{ $one_var}."\n" );};
            $ENV{ $one_var } = WinFormat( $this_var );
            if ( defined $debug ) { print(STDERR "ENV $one_var after : ".$ENV{ $one_var}."\n" );};
        }
    }

}
#---------------------------------------------------------------------------
# main
@params = @ARGV;

$command = shift(@params);
while ( $command =~ /^-/ )
{
    if ( $command eq "-env" )
    {
        replace_cyg_env;
    }

    $command = shift(@params);
}
if ( (defined $debug_light) or (defined $debug) ) { print( STDERR "Command: $command\n" ); }

replace_cyg(\@params);
if ( (defined $debug_light) or (defined $debug) ) { print(STDERR "\n---------------------\nExecute: $command @params\n----------------\n");};
exec( "$command", @params) or die( "\nError: guw.pl: executing $command failed!\n" );

