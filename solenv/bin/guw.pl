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
%knownpara = ( 'echo', [ '/TEST', 'QQQ', 'CCC', 'uno:' ],
               'cl', [ '-clr:', '-Z' ],
               'csc', [ '-target:' ],
               'lib', [ 'OUT:', 'EXTRACT:','out:', 'def:', 'machine:' ],
               'link', [ 'BASE:', 'DEBUG', 'DLL', 'LIBPATH', 'MACHINE:',
                         'MAP', 'NODEFAULTLIB', 'OPT', 'PDB', 'RELEASE',
                         'SUBSYSTEM', 'STACK', 'out:', 'map:', 'ENTRY:',
                         'implib:', 'delayload:', 'def', 'COMMENT:' ],
               'regcomp', [ '-env:', 'vnd.sun.star.expand:' , 'vnd.openoffice.pymodule' ],
               'regmerge', [ '/UCR' ],
               'rc', [ '-D' ],
               'rsc', [ '-DOOO_' ] );

#---------------------------------------------------------------------------
# procedures


#----------------------------------------------------------
# Function name: myCygpath
# Description:   Transform POSIX path to DOS path
# Arguments:     1. Variable (string) with one token
#                2. optional - if set remove spaces and shorten to 8.3
#                   representation.
# Return value:  Reformatted String
#----------------------------------------------------------
sub myCygpath {
    my $posixpath = shift;
    my $shortenpath = shift || '';

    my $dospath;

    if ( $posixpath =~ / / and $shortenpath ) {
        chomp( $dospath = qx{cygpath -d "$posixpath"} );
        # "cygpath -d" returns "" if the file doesn't exist.
        if ($dospath eq "") {
            $dospath = ".";
            print(STDERR "Error: guw.pl: Path: $posixpath:\nhas a problem! Probably nonexistent filename with space.\n");
            if ( (defined $debug_light) or (defined $debug) ) {
                die "exiting ...\n";
            }
        }
    } else {
        if ( $posixpath =~ /^\// ) {
            chomp( $dospath = qx{cygpath -w "$posixpath"} );
        } else {
            $dospath = $posixpath;
            $dospath =~ s/\//\\/g;
        }
    }
    return $dospath;
}

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

  # Include paths or parameters with filenames
  if ( $variable =~ /\A(-D[\w\.]*=)[\'\"]?((?:\/?[\w\.\-\+ ~]+)+\/?)[\'\"]?\Z/ ) {
      # This regex evaluates -D<something>=<path>, sometimes with quotes or "/" at the end
      # option -> $1, filename without quotes -> $2
      if ( defined $debug ) { print(STDERR "WinFormat:\ninclude (-D<something>=<path>) path:\n$variable\n"); }
      $d1_prefix = $1;
      $d1 = $2;
      $d2 = myCygpath($2,1);
      if ( $d2 ne "" ) {
             $d2 =~ s/\\/\\\\/g ;
      }
  } elsif ( $variable =~ /\A(-?\w[\w\.]*=)[\'\"]?((?:\/?[\w\.\-\+ ~]+)+\/?)[\'\"]?\Z/ ) {
      # This regex evaluates [-]X<something>=<path>, sometimes with quotes or "/" at the end
      # option -> $1, filename without quotes -> $2
      if ( defined $debug ) { print(STDERR "WinFormat:\ninclude ([-]<something>=<path>) path:\n$variable\n"); }
      $d1_prefix = $1;
      $d1 = $2;
      $d2 = myCygpath($2,1);
  } elsif ( $variable =~ /\A(--\w[\w\.\-]*=)[\'\"]?((?:\/?[\w\.\-\+ ~]+)+\/?)[\'\"]?\Z/ ) {
      # This regex evaluates --<something>=<path>, sometimes with quotes or "/" at the end
      # option -> $1, filename without quotes -> $2
      if ( defined $debug ) { print(STDERR "WinFormat:\ninclude (--<something>=<path>) path:\n$variable\n"); }
      $d1_prefix = $1;
      $d1 = $2;
      $d2 = myCygpath($2,1);
  } elsif ( $variable =~ /\A(-\w[\w\.]*:)[\'\"]?((?:\/?[\w\.\-\+ ~]+)+\/?)[\'\"]?\Z/ ) {
      # This regex evaluates -X<something>:<path>, sometimes with quotes or "/" at the end
      # option -> $1, filename without quotes -> $2
      if ( defined $debug ) { print(STDERR "WinFormat:\nFound (-<something>:<path>):\n$variable\n"); }
      $d1_prefix = $1;
      $d1 = $2;
      $d2 = myCygpath($2,1);
  } elsif ( $variable =~ /\A(-\w+:)(.*)\Z/ ) {
      # This regex evaluates -X<something>:<NO-path>, and prevents translating of these.
      # option -> $1, rest -> $2
      if ( defined $debug ) { print(STDERR "WinFormat:\nFound (-<something>:<no-path>):\n$variable\n"); }
      $d1_prefix = $1;
      $d1 = $2;
      $d2 = myCygpath($2,1);
  } elsif ( $variable =~ /\A(\w+:)[\'\"]?\/\/\/((?:\/?[\w\.\-\+ ~]+)+\/?)[\'\"]?\Z/ ) {
      # See iz35982 for the reason for the special treatment of this switch.
      # This regex evaluates <something>:///<path>, sometimes with quotes or "/" at the end
      # option -> $1, filename without quotes -> $2
      if ( defined $debug ) { print(STDERR "WinFormat:\nFound (<something>:///<path>):\n$variable\n"); }
      $d1_prefix = $1."///";
      $d1 = $2;
      $d2 = myCygpath($2,1);
      $d2 =~ s/\\/\//g ;
  } elsif ( $variable =~ /\A(-\w)[\'\"]?((?:\/[\w\.\-\+ ~]+)+\/?)[\'\"]?\Z/ ) {
      # This regex evaluates -X<path>, sometimes with quotes or "/" at the end
      # option -> $1, filename without quotes -> $2
      if ( defined $debug ) { print(STDERR "WinFormat:\ninclude (-X<absolute path>) path:\n$variable\n"); }
      $d1_prefix = $1;
      $d1 = $2;
      $d2 = myCygpath($2,1);
  } elsif ( $variable =~ /\A(-F[ARdemopr])[\'\"]?((?:\/[\w\.\-\+ ~]+)+\/?)[\'\"]?\Z/ ) {
      # This regex evaluates -FX<path> (MSVC switches for output naming), sometimes with quotes or "/" at the end
      # option -> $1, filename without quotes -> $2
      if ( defined $debug ) { print(STDERR "WinFormat:\ncompiler naming (-FX<absolute path>) path:\n$variable\n"); }
      $d1_prefix = $1;
      $d1 = $2;
      $d2 = myCygpath($2,1);
  } else {
      $d2 = "";
  }
  if ( $d2 ne "" ) {
      # Found a parameter
      $d1 =~ s/\+/\\\+/ ;
      $d1 =~ s/\./\\\./ ;
      $variable =~ s/$d1/$d2/ ;
  } else {
    # Found no parameter, assume a path
    $variable =~ s/:/;/g;
    $variable =~ s/([;]|\A)(\w);/$1$2:/g; # get back the drives

    # Search for posix path ;entry; (The regex accepts valid paths with at least one /)
    # and replace with DOS path, accept quotes.
    # iz28717 Accept ',' as path seperator.
    while ( $variable =~ /(?:[;,]|\A)[\'\"]?([\w\.\-\+ ~]*(?:\/[\w\.\-\+ ~]+)+\/?)[\'\"]?(?:[;,]|\Z)/ ) {
        # Normal paths
        $d1 = $1;
        $d2 = myCygpath($d1);
        if ( defined $debug ) {
            print(STDERR "WinFormat:\nFull path:\n$variable\nTranslated part:$d2\n");
        }
    $d1 =~ s/\+/\\\+/ ;
        $variable =~ s/$d1/$d2/ ;
    }
  }

  # Sanity check for -X<path>
  if ( $variable =~ /-\w[\'\"]?(?:(?:\/[\w\.\-\+ ~]+)+)/ ) {
      print(STDERR "Error: guw.pl: WinFormat: Not converted -X/... type switch in :$variable:.\n");
      if ( (defined $debug_light) or (defined $debug) ) { die "\nNot processed -X/...\n"; }
  }
  # Sanity check for [-]X<something>(:|=)<path> case
  if ( $variable =~ /\A-?\w[\w\.]*[=:][\'\"]?(?:\/[\w\.\-\+ ~]+)+/ ) {
      print(STDERR "Error: guw.pl: WinFormat: Not converted [-]X<something>(=|:)/<path> type switch in :$variable:.\n");
      if ( (defined $debug_light) or (defined $debug) ) { die "\nNot processed [-]X<something>(=|:)/...\n"; }
  }

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
          $shortcommand =~ /([^\/]+$)/;
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
        'STAR_STANDLST',
        'CLASSPATH',
        'JAVA_HOME'
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

