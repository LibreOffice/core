#!/usr/bin/perl
#*************************************************************************
#*
#*  $RCSfile: filter.pl,v $
#*
#*  $Revision: 1.1 $
#*
#*  last change: $Author: mh $ $Date: 2002-11-18 15:53:53 $
#*
#*  The Contents of this file are made available subject to the terms of
#*  either of the following licenses
#*
#*         - GNU Lesser General Public License Version 2.1
#*         - Sun Industry Standards Source License Version 1.1
#*
#*  Sun Microsystems Inc., October, 2002
#*
#*  GNU Lesser General Public License Version 2.1
#*  =============================================
#*  Copyright 2002 by Sun Microsystems, Inc.
#*  901 San Antonio Road, Palo Alto, CA 94303, USA
#*
#*  This library is free software; you can redistribute it and/or
#*  modify it under the terms of the GNU Lesser General Public
#*  License version 2.1, as published by the Free Software Foundation.
#*
#*  This library is distributed in the hope that it will be useful,
#*  but WITHOUT ANY WARRANTY; without even the implied warranty of
#*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#*  Lesser General Public License for more details.
#*
#*  You should have received a copy of the GNU Lesser General Public
#*  License along with this library; if not, write to the Free Software
#*  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#*  MA  02111-1307  USA
#*
#*
#*  Sun Industry Standards Source License Version 1.1
#*  =================================================
#*  The contents of this file are subject to the Sun Industry Standards
#*  Source License Version 1.1 (the "License"); You may not use this file
#*  except in compliance with the License. You may obtain a copy of the
#*  License at http://www.openoffice.org/license.html.
#*
#*  Software provided under this License is provided on an "AS IS" basis,
#*  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
#*  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#*  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#*  See the License for the specific provisions governing your rights and
#*  obligations concerning the Software.
#*
#*  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#*
#*  Copyright: 2002 by Sun Microsystems, Inc.
#*
#*  All Rights Reserved.
#*
#*  Contributor(s): _______________________________________
#*
#*
#************************************************************************/

$debug = "";
$ctrue = 1;
$cfalse = 0;
$cskip = $ctrue;
$cread = $cfalse;

# keine umlaute
# Berechnet den Ausdruck der durch den 1. Parameter Uebergeben wird.
# Falls symbole nicht ersetzt werden koennen findet behandlung als Makro statt
# Bricht im Fehlerfall das Programm ab
sub calc_value {
  $_ = $_[0];
  s/(\||\+|\-|\*|\/|\(|\)|\<=|\>=|\!=|\>{1,2}|\<{1,2}|\={1,2})/ $1 /g;          # Blanks um Operatoren einfuegen
  s/^ *(.*) *$/$1/;                             # blanks vorn und hinten weg
  s/\( *[a-zA-Z]* *\)//;            # Casts auf Typen raus
  @ops = split;
  print join(",",@ops),"\n" if $debug;
  @neuop = "";
  foreach $op (@ops) {
    if ($op !~ /^(\||\+|\-|\*|\/|\(|\)|\>{1,2}|\<{1,2}|\<=|\>=|\!=|\={1,2}|\d+|0[xX][0-9a-fA-F]+)$/) {
      print "ersetze $op durch ",$symbol {$op},"\n" if $debug;
                                # Wenn kein Operand oder Zahl oder klammer
      $op = $symbol {$op};      # zugriff auf assotiatives Array mit {}
    }                           # ersetzen des Symbols durch seinen Wert
    if ( defined ($op) ) {
      push(@neuop,$op);
    } else {                    # Symbol nicht gefunden -> Als Text uebernehmen
      print "aborting calc_value : \"$_\"\n" if $debug;
      return $_;
    }
  }
  print @neuop,"\n" if $debug;
  $_ = join(" ",@neuop);
  print "$_\n" if $debug;
  eval "\$value = $_";              # Eigentliche Berechnung des Wertes
  if ($@) { die "$@ ($_)"; }        # Fehlermeldung in $@ auswerten (Abbruch)
  $value;
}

# liest einen Block
# Parameter:   FileHandle
#              Flag $skip ob Block .bersprungen werden soll oder Interpretiert
#              Flag $BlindSkip ob if oder else -Zweig (Dann untersuchen weiterer if-then-else)
#              Liste der Regular expressions die das Blockende darstellen.
#   bei #ifdef block waere das dann  ("^#else\$","^#endif\$")

sub read_block {

  local($file,$skip,$BlindSkip,@patterns) = @_;
  print "reading block '$file' $skip $BlindSkip @patterns ",scalar(@_),"\n" if $debug;
  while (<$file>) {
    chop;
    s/\s*$//;             # trailing whitespaces entfernen
    s/\(USHORT\)//;             # (USHORT) entfernen
    print "Input : \"$_\"\n" if $debug;
    s/\/\/.*//;         # Zeilenkommentar entfernen
#    s/\/\*.*?\*\///g;   # Kommentare inerhalb einer Zeile entfernen
#   erst ab perl 5
    s/\s+/ /g;          # Allen whitespace in einen blank aendern
    s/ *$//;            # Whitespace am Ende entfernen

# Testen ob letzte Zeile erreicht wurde
# Liefert auch im falle der ganzen Datei, also ohne RegExp das gewuenschte
# Ergebnis.

    foreach $patt (@patterns) {     # Fuer jede der Expressions testen
      print "Testing against $patt\n" if $debug;
      print "Pattern fits: $patt\n" if $debug && /$patt/;
      return $& if /$patt/;         # Ruecksprung wenn die Zeile auf das Muster passt.
    }

    if ( $skip eq $cread || !$BlindSkip ) {

      if ( /\/\*/ ) {             # kommentarbeginn entdecken
        $_ = $`;                  # Zeile auf restzeile vor kommantar setzen
        $comment = $ctrue;
        print "Kommantar erkannt\n" if $debug;
      }
      @line = split(/ /,$_,3);   # Zeile Parsen und in @line speichern

      $_ = $line[0];             # Default patternspace setzen
      if (/^#/) {                # Zeile faengt mit '#' an -> Praeprozessorbefehl
        print "Bearbeite Zeile: @line\n" if $debug;
        SELECT: {                # SELECT ist hier frei gewaehlt

          if (/#if/) {    # Alle Ifbefehle (#if,#ifdef und #ifndef)
            if ($skip) {
              print "skipping nested if\n" if $debug;
              $ende = &read_block ($file,$cskip,$cfalse,'^#else$','^#endif$');
              print "skipping nested else\n" if $debug && $ende eq "#else";
              &read_block ($file,$cskip,$cfalse,'^#endif$') if $ende eq "#else";
              last SELECT;
            }
            if (/#if$/) {
              $isif = &calc_value (join(" ",@line[1,2]));
            } else {
              $isif = defined ($symbol {$line[1]});
              if (/#ifndef/) { $isif = ! $isif; }
            }
            if ( $isif ) {
              print "if\n" if $debug;
              $ende = &read_block ($file,$cread,$cfalse,'^#else$','^#endif$');
              print "skipping else\n" if $debug && $ende eq "#else";
              &read_block ($file,$cskip,$cfalse,'^#endif$') if $ende eq "#else";
            } else {
              print "skipping if\n" if $debug;
              $ende = &read_block ($file,$cskip,$cfalse,'^#else$','^#endif$');
              print "else\n" if $debug && $ende eq "#else";
              &read_block ($file,$cread,$cfalse,'^#endif$') if $ende eq "#else";
            }
            print "endif\n" if $debug;
            last SELECT;
          }

          last SELECT if $skip eq $cskip;    #Der Rest ist egal (Skip von if und else)

          if (/#define/) {
#            print join(",",%symbol),"\n";
            if (defined ($symbol {$line[1]}) ) {
              print "Symbol schon definiert: $line[1]\n";
            }
            else {
              if (defined ($line[2])) {
                $symbol {$line[1]} = &calc_value ($line[2]);
              } else {
                $symbol {$line[1]} = "";
              }
              print "setze $line[1] = ",$symbol {$line[1]},"\n" if $debug;
              if ( $line[1] =~ /^$namefilter/ )
              {
                $mykey = $line[1];
                $mykey =~ s/^$namefilter//;
                $count += 1;
                print OUT "$mykey    ",$symbol {$line[1]},"\n";
                print OUT2 "\t{ \"$mykey\", ",$symbol {$line[1]}," },\n";
              }
            }
            last SELECT;
          }


          if (/#include/) {
             $_ = $line[1];
             s/<(.*)>/$1/;       # < .. > entfernen
             s/"(.*)"/$1/;       # " .. " entfernene
             &read_file ($_,$file);
#             print "Include abgeklemmt!!!\n" if $debug;
             last SELECT;
          }


          if (/#pragma/) {
             print "Pragma Ignoriert!!!\n" if $debug;
             last SELECT;
          }


          if (/#error/)  { die "Error: @line[1,2]\n";}

          # Ansonsten Fehlermeldung
          print "unbekannter Praeprozessorbefehl : \"$_\"\n";
        }
      }

      if ($comment) {
        &read_block ($file,$cskip,$ctrue,'\*\/');
        print "After comment $skip\n" if $debug;
        $comment = $cfalse;
#        redo main;        # Schleifenrumpf mit Restzeile neu Durchlaufen
#        kann hier entfallen, da nur Praeprozessorbefehle vorhanden.
      }
    }
  }
  print "Leaving read_block at the end\n" if $debug;
}

sub convert_path {

  $_ = $_[0];
  $GUI = $ENV {"GUI"};
  $use_shell = $ENV {"USE_SHELL"};
  if ( $GUI eq "WNT" )
  {
    if ( defined( $use_shell ) && "$use_shell" eq "4nt" )
    {
      s/\//\\/g;
    }
  }
  $_;
}

# Einlesen einer Datei.
# der erste Parameter ist der Dateiname
# Globale Variable %symbols enthaelt die Symboltabelle
sub read_file {

  local ($filename,$file) = @_;
  $comment = $cfalse;   # Am Dateianfang immer ausserhalb eines Kommentares
  $file++;                           # String increment
  local $TempFileName = &convert_path ($basename."/".$filename);
  print "reading file $TempFileName as $file\n" if $debug;
  if ( ! open($file, $TempFileName ))
  {
    print "Warning: Could not open file $TempFileName. ";
    $TempFileName = &convert_path ($basename."/../".$filename);
    print "Trying $TempFileName\n";
    open($file, $TempFileName ) || print "Warning: Could not open file $TempFileName. Ignoring.\n" || return; #( die $basename."/../$filename $!" ) ;    # Oeffnen der Datei zum lesen
  }

  &read_block($file,$cread,$cfalse);         # Daten lesen
  close($file);
  print "done reading $filename\n" if $debug;
}

# Hauptprogramm beginnt hier


print &convert_path ("//\n\n\n");


%symbol = "";                     # Assotiatives Array initialisieren

$basename = ".";
$basename = $ARGV[0] if defined($ARGV[0]);

$filename = "app.hrc";
$filename = $ARGV[1] if defined($ARGV[1]);


$filebase = $filename;
$filebase =~ s/\.[^.]+$//;           # Schneidet den suffix ab
$filebase = $ARGV[2] if defined($ARGV[2]);


$namefilter = $ARGV[3] if defined($ARGV[3]);

print "Generating $filebase:\n";

$count = 0;

open(OUT,">$filebase");
open(OUT2,">$filebase.hxx");
print OUT2 "\{\n";

&read_file ($filename,"f00");

print OUT2 "\t{ \"\" ,0 }\n\};\n";

close(OUT);
close(OUT2);

if ( $count == 0 )
{
  die "Error: No Entries Found generating \"$filebase.hxx\". Testtool will not work!"
}
