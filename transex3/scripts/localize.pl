:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;

#*************************************************************************
#
#   $RCSfile: localize.pl,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: hr $ $Date: 2004-08-02 16:24:26 $
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
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

use strict;
use Getopt::Long;
use IO::Handle;
use File::Find;
use File::Temp;

# ver 1.1
#
#### module lookup
use lib ("$ENV{SOLARENV}/bin/modules", "$ENV{COMMON_ENV_TOOLS}/modules");

#### globals ####
my $sdffile = '';
my $outputfile = '';
my $mode = '';
my $bVerbose="0";
my $srcpath = '';
my $WIN;
my $languages;

         #        (                          leftpart                                                     )        (         rightpart                                   )
         #           prj      file     dummy     type      gid        lid        helpid   pform     width     lang      text     helptext  qhelptext  title    timestamp
my $sdf_regex  = "((([^\t]*)\t([^\t]*)\t([^\t]*)\t([^\t]*)\t([^\t]*)\t([^\t]*)\t([^\t])*\t([^\t]*)\t([^\t]*))\t([^\t]*)\t(([^\t]*)\t([^\t]*)\t([^\t]*)\t([^\t]*)\t)([^\t]*))";
my $file_types = "(src|hrc|xcs|xcu|lng|ulf|xrm|xml|xcd|xgf|xxl|xrb)";


#### main ####
parse_options();

if ( $^O eq 'MSWin32' ) {
   $WIN = 'TRUE';
}
 else {
   $WIN = '';
}

if   ( $mode eq "merge"    )    {   splitfile( $sdffile );      }
elsif( $mode eq "extract"  )    {   collectfiles( $outputfile );}
else                            {   usage();                    }

exit(0);

#########################################################
sub splitfile{

    my $lastFile        = '';
    my $currentFile     = '';
    my $delim;
    my $badDelim;
    my $start           = 'TRUE';
    my %index  = ();
    my %block;

    STDOUT->autoflush( 1 );

    #print STDOUT "Open File $sdffile\n";
    open MYFILE , "< $sdffile"
    or die "Can't open '$sdffile'\n";

    while( <MYFILE>){
         if( /$sdf_regex/ ){
            my $line           = defined $_ ? $_ : '';
            my $prj            = defined $3 ? $3 : '';
            my $file           = defined $4 ? $4 : '';
            my $type           = defined $6 ? $6 : '';
            my $gid            = defined $7 ? $7 : '';
            my $lid            = defined $8 ? $8 : '';
            my $lang           = defined $12 ? $12 : '';
            my $text           = defined $14 ? $14 : '';
            my $plattform      = defined $10 ? $10 : '';

            chomp( $line );
            $currentFile  = $srcpath . '\\' . $prj . '\\' . $file;
            if ( $WIN ) { $currentFile  =~ s/\//\\/g; }
            else        { $currentFile  =~ s/\\/\//g; }

            if( $start ){
                $start='';
                $lastFile = $currentFile;
            }
            if( ( $lang eq "en-US" || $lang eq "de") ){}
            elsif( $currentFile eq $lastFile ){
                $block{ $prj.$lang.$gid.$lid.$file.$type } =  $line ;
            }else{
                writesdf( $lastFile , \%block );
                $lastFile = $currentFile;
                %block = ();
                if( !( $lang eq "en-US" || $lang eq "de") ){  $block{ $prj.$lang.$gid.$lid.$file.$type.$plattform } =  $line ; }
            }
        } #else { print STDOUT "splitfile REGEX kaputt\n";}

    }
    writesdf( $lastFile , \%block );
    %block = ();
    close( MYFILE );
}
#########################################################

sub writesdf{

    my $lastFile        = shift;
    my $blockhash_ref   = shift;
    my $localizeFile    = $lastFile;
    my %index=();

    if( $localizeFile =~ /\.$file_types[\s]*$/ ){
        if( $WIN ) { $localizeFile =~ s/\\[^\\]*\.$file_types[\s]*$/\\localize.sdf/; }
        else       { $localizeFile =~ s/\/[^\/]*\.$file_types[\s]*$/\/localize.sdf/; }
        }else {
            print STDERR "Strange filetype found '$localizeFile'\n";
            return;
        }
        if( $bVerbose ){ print STDOUT "$localizeFile\n"; }
        if( open DESTFILE , "< $localizeFile" ){

        #or die "Can't open/create '\$localizeFile'";

        #### Build hash
        while(<DESTFILE>){
         if( /$sdf_regex/ ){
            my $line           = defined $_ ? $_ : '';
            my $prj            = defined $3 ? $3 : '';
            my $file           = defined $4 ? $4 : '';
            my $type           = defined $6 ? $6 : '';
            my $gid            = defined $7 ? $7 : '';
            my $lid            = defined $8 ? $8 : '';
            my $lang           = defined $12 ? $12 : '';
            my $plattform      = defined $10 ? $10 : '';
                 chomp( $line );
                 $index{ $prj.$lang.$gid.$lid.$file.$type.$plattform } =  $line ;
         } #else { print STDOUT "writesdf REGEX kaputt $_\n";}

        }
        close( DESTFILE );
    }
    #### Copy new strings

    my @mykeys = keys( %{ $blockhash_ref } );
    my $isDirty = "FALSE";
    foreach my $key( @mykeys ){
        if( ! defined $index{ $key } ){
            # Add new entry
            $index{ $key }  = $blockhash_ref->{ $key} ;
            $isDirty = "TRUE";
        }elsif( $index{ $key } ne $blockhash_ref->{ $key } ){
            # Overwrite old entry
            $index{ $key } = $blockhash_ref->{ $key };
            $isDirty = "TRUE";
        }else {
        }
    }

    #### Write file

    if( !$bVerbose ){ print STDOUT "."; }
    if( $isDirty eq "TRUE" ){
        if( open DESTFILE , "+> $localizeFile" ){
            @mykeys = keys( %index );
            foreach my $key( @mykeys ){
                print DESTFILE ( $index{ $key } , "\n" );
            }
            close DESTFILE;
        }else {
            print STDERR "Can't open/create '$localizeFile'";
        }
    }
}

#########################################################
sub collectfiles{
    print STDOUT "### Localize\n";
    my @sdfparticles;
    my $localizehash_ref;
    my ( $bAll , $bUseLocalize, $langhash_ref ) = parseLanguages();
    # Enable autoflush on STDOUT
    # $| = 1;
    STDOUT->autoflush( 1 );

    ### Search sdf particles
    print STDOUT "### Searching sdf particles\n";
    find sub {
        my $file = $File::Find::name;
        if( -f && $file =~ /.*localize.sdf$/ ) {
            push   @sdfparticles , $file;
            if( $bVerbose eq "1" ) { print STDOUT "$file\n"; }
            else { print ".";  }

         }
    }, $srcpath;
    my $nFound  = $#sdfparticles +1;
    print "\n    $nFound files found !\n";

    my ( $LOCALIZEPARTICLE , $localizeSDF ) = File::Temp::tempfile();
    my ( $ALLPARTICLES_MERGED , $particleSDF_merged )     = File::Temp::tempfile();
    my ( $LOCALIZE_LOG , $my_localize_log ) = File::Temp::tempfile();

    ## Get the localize de,en-US extract
    if( $bAll || $bUseLocalize ){
        print "### Fetching source language strings\n";
        my $command = "";
        my $args    = "";
        if( $WIN eq "TRUE" ){
            if( $ENV{SOLARVER} ){
                $command = "$ENV{SOLARVERSION}\\$ENV{INPATH}\\bin.$ENV{UPDMINOR}\\localize_sl.exe";
            }else{ # Not set -> OOo environment . Very optimistic!
                $command = "guw.pl $ENV{SOLARVERSION}\\$ENV{INPATH}\\bin.$ENV{UPDMINOR}\\localize_sl.exe";
            }
        }
        else{
            $command = "$ENV{SOLARVERSION}/$ENV{INPATH}/bin.$ENV{UPDMINOR}/localize_sl";
            # Test $ENV{UPDMINOR} before concat
        }
      # -e
        if ( -x $command ){
            if( !$bVerbose  ){ $args .= " -QQ "; }
            $args .= " -e -f $localizeSDF -l ";
            my $bFlag="";
            if( $bAll ) {$args .= " en-US,de";}
            else{
              my @list;
              foreach my $isokey ( keys( %{ $langhash_ref } ) ){
                push @list , $isokey;
                if( $langhash_ref->{ $isokey } ne "" ){
                    push @list , $langhash_ref->{ $isokey };
                }
              }
              remove_duplicates( \@list );
              foreach my $isokey ( @list ){
                switch :{
                        ( $isokey=~ /^de$/i  )
                        && do{
                                if( $bFlag eq "TRUE" ){ $args .= ",de"; }
                                else  {
                                    $args .=  "de";  $bFlag = "TRUE";
                                 }
                              };
                        ( $isokey=~ /^en-US$/i  )
                        && do{
                                if( $bFlag eq "TRUE" ){ $args .= ",en-US"; }
                                else {
                                    $args .= "en-US";  $bFlag = "TRUE";
                                 }
                              };

                    } #switch
                } #foreach
              } # if
        } # if
#        if ( !$bVerbose ){
#            if ( $WIN eq "TRUE" ) { $args .= " > $my_localize_log";  }
#            else                  { $args .= " >& $my_localize_log"; }
#        }
        if ( $bVerbose ) { print STDOUT $command.$args."\n"; }

        my $rc = system( ( $command.$args ) );

        if( $rc < 0 ){    print STDERR "ERROR: localize rc = $rc\n"; exit( -1 ); }
        ( $localizehash_ref )  = read_file( $localizeSDF , $langhash_ref );

    }
    ## Get sdf particles
   open ALLPARTICLES_MERGED , "+>> $particleSDF_merged"
    or die "Can't open $particleSDF_merged";

    ## Fill fackback hash
    my( $fallbackhashhash_ref ) = fetch_fallback( \@sdfparticles , $localizeSDF , $langhash_ref );
    my %block;
    my $cur_fallback;
    if( !$bAll) {
        foreach my $cur_lang ( keys( %{ $langhash_ref } ) ){
            $cur_fallback = $langhash_ref->{ $cur_lang };
            if( $cur_fallback ne "" ){
                # Insert fallback strings
                #print STDOUT "DBG: Renaming $cur_fallback to $cur_lang in fallbackhash\n";
                rename_language(  $fallbackhashhash_ref ,  $cur_fallback , $cur_lang );
            }
            foreach my $currentfile ( @sdfparticles ){
                if ( open MYFILE , "< $currentfile" ) {
                    while(<MYFILE>){
                        if( /$sdf_regex/ ){
                            my $line           = defined $_ ? $_ : '';
                            my $prj            = defined $3 ? $3 : '';
                            my $file           = defined $4 ? $4 : '';
                            my $type           = defined $6 ? $6 : '';
                            my $gid            = defined $7 ? $7 : '';
                            my $lid            = defined $8 ? $8 : '';
                            my $lang           = defined $12 ? $12 : '';
                            my $plattform      = defined $10 ? $10 : '';

                            chomp( $line );

                            if ( $lang eq $cur_lang ){
                                # Overwrite fallback strings with collected strings
                                if( $cur_lang ne "de" || $cur_lang ne "en-US" ){
                                     $fallbackhashhash_ref->{ $cur_lang }{ $prj.$gid.$lid.$file.$type.$plattform } =  $line ;
                               }

                            }
                        }
                    }
                }
            }
            foreach my $line ( keys( %{$fallbackhashhash_ref->{ $cur_lang } } )) {
                if( $cur_lang ne "de" && $cur_lang ne "en-US" ){
                    print ALLPARTICLES_MERGED ( $fallbackhashhash_ref->{ $cur_lang }{ $line }, "\n" );
                }
             }
        }
    } else {
        foreach my $currentfile ( @sdfparticles ){
            if ( open MYFILE , "< $currentfile" ) {
                while( <MYFILE> ){
                    print ALLPARTICLES_MERGED ( $_, "\n" );  # recheck de / en-US !
                }
            }
        }
    }
    close ALLPARTICLES_MERGED;


    ## Join both
    if( $outputfile ){
        if( open DESTFILE , "+> $outputfile" ){
            open LOCALIZEPARTICLE ,  "< $localizeSDF" ;
            open ALLPARTICLES_MERGED , "< $particleSDF_merged";

            # Insert localize
            my $extract_date="";
            while ( <LOCALIZEPARTICLE> ){
                if( /$sdf_regex/ ){
                    my $leftpart       = defined $2 ? $2 : '';
                    my $lang           = defined $12 ? $12 : '';
                    my $rightpart      = defined $13 ? $13 : '';
                    my $timestamp      = defined $18 ? $18 : '';

                    if( $extract_date eq "" ) { $extract_date = $timestamp; }

                    if( $bAll ){ print DESTFILE $leftpart."\t".$lang."\t".$rightpart.$extract_date ; }
                    else {
                        foreach my $sLang ( keys( %{ $langhash_ref } ) ){
                            if( $sLang=~ /all/i )                       {  print DESTFILE $leftpart."\t".$lang."\t".$rightpart.$extract_date;  }
                            if( $sLang eq "de" && $lang eq "de" )       {  print DESTFILE $leftpart."\t".$lang."\t".$rightpart.$extract_date;  }
                            if( $sLang eq "en-US" && $lang eq "en-US" ) {  print DESTFILE $leftpart."\t".$lang."\t".$rightpart.$extract_date;  }

                         }
                    }
                }
            }

            # Insert particles
            while ( <ALLPARTICLES_MERGED> ){
                if( /$sdf_regex/ ){
                    my $leftpart       = defined $2 ? $2 : '';
                    my $prj            = defined $3 ? $3 : '';
                    my $lang           = defined $12 ? $12 : '';
                    my $rightpart      = defined $13 ? $13 : '';
                    my $timestamp      = defined $18 ? $18 : '';

                    if( $extract_date eq "" ) { $extract_date = $timestamp; }

                    if( ! ( $prj =~ /binfilter/i ) ) {
                        print DESTFILE $leftpart."\t".$lang."\t".$rightpart.$extract_date ;
                    }
                 }
            }

        }else { print STDERR "Can't open $outputfile";}
    }
    close DESTFILE;
    close LOCALIZEPARTICLE;
    close ALLPARTICLES_MERGED;
#    unlink $localizeSDF , $particleSDF_merged ,  $my_localize_log;

    sort_outfile( $outputfile );
    }
#########################################################
   sub sort_outfile{
        my $outfile = shift;
        print STDOUT "### Sorting ... $outfile\n";
        my @lines;
        my @sorted_lines;
        if ( open ( SORTEDFILE , "< $outputfile" ) ){
            while ( <SORTEDFILE> ){
                push @lines , $_;
            }
            close SORTEDFILE;

            @sorted_lines = sort {
                my $xa_left_part     = "";
                my $xa_lang          = "";
                my $xa_right_part    = "";
                my $xa_timestamp     = "";
                my $xb_left_part     = "";
                my $xb_lang          = "";
                my $xb_right_part    = "";
                my $xb_timestamp     = "";
                my $xa               = "";
                my $xb               = "";

                if( $a=~ /$sdf_regex/ ){
                    $xa_left_part       = defined $2 ? $2 : '';
                    $xa_lang           = defined $12 ? $12 : '';
                    $xa_right_part      = defined $13 ? $13 : '';
                }
                if( $b=~ /$sdf_regex/ ){
                    $xb_left_part       = defined $2 ? $2 : '';
                    $xb_lang           = defined $12 ? $12 : '';
                    $xb_right_part      = defined $13 ? $13 : '';
                }
                $xa_left_part cmp $xb_left_part ||
                $xa_lang cmp $xb_lang           ||
                $xa_right_part cmp $xb_right_part

            } @lines;
            if ( open ( SORTEDFILE , "> $outputfile" ) ){
                foreach my $newline ( @sorted_lines ) {
                    print SORTEDFILE $newline;
            }
            close SORTEDFILE;
        }
    }
}

#########################################################
sub rename_language{
    my $fallbackhashhash_ref    = shift;
    my $cur_fallback            = shift;
    my $cur_lang                = shift;
    my $line;

    foreach my $key( keys ( %{ $fallbackhashhash_ref->{ $cur_fallback } } ) ){
        $line = $fallbackhashhash_ref->{ $cur_fallback }{ $key };
        if( $line =~ /$sdf_regex/ ){
            my $leftpart       = defined $2 ? $2 : '';
            my $lang           = defined $12 ? $12 : '';
            my $rightpart      = defined $13 ? $13 : '';

            $fallbackhashhash_ref->{ $cur_lang }{ $key } = $leftpart."\t".$cur_lang."\t".$rightpart;
        }
    }
}

############################################################
sub remove_duplicates{
    my $list_ref    = shift;
    my %tmphash;
    foreach my $key ( @{ $list_ref } ){ $tmphash{ $key } = '' ; }
    @{$list_ref} = keys( %tmphash );
}

##############################################################
sub fetch_fallback{
    my $sdfparticleshash_ref   = shift;
    my $localizeSDF            = shift;
    my $langhash_ref           = shift;
    my %fallbackhashhash;
    my $cur_lang;
    my @langlist;

    foreach my $key ( keys ( %{ $langhash_ref } ) ){
        $cur_lang = $langhash_ref->{ $key };
        if ( $cur_lang ne "" ) {
            push @langlist , $cur_lang;
        }
    }
    remove_duplicates( \@langlist );
    foreach  $cur_lang ( @langlist ){
        if( $cur_lang eq "de" || $cur_lang eq "en-US" ){
            #print STDOUT "DBG: Loading $cur_lang into Fallbackhash\n";
            read_file_fallback( $localizeSDF , $cur_lang , \%fallbackhashhash );
        }
    }

    # remove de / en-US
    my @tmplist;
    foreach $cur_lang( @langlist ){
        if( $cur_lang ne "de" && $cur_lang ne "en-US" ){
            push @tmplist , $cur_lang;
        }
    }
    @langlist = @tmplist;
    if ( $#langlist +1 ){
        #print STDOUT "DBG: Loading @langlist into Fallbackhash\n";
        read_file_fallbacks( $localizeSDF , \@langlist , \%fallbackhashhash );
    }
    return (\%fallbackhashhash);
}

#########################################################
sub write_file{

    my $localizeFile = shift;
    my $index_ref    = shift;

    if( open DESTFILE , "+> $localizeFile" ){
        foreach my $key( %{ $index_ref } ){
            print DESTFILE ($index_ref->{ $key }, "\n" );
        }
        close DESTFILE;
    }else {
      print STDERR "Can't open/create '$localizeFile'";
    }
}

#########################################################
sub read_file{

    my $sdffile         = shift;
    my $langhash_ref    = shift;
    my %block           = ();

    open MYFILE , "< $sdffile"
        or die "Can't open '$sdffile'\n";
        while( <MYFILE>){
          if( /$sdf_regex/ ){
            my $line           = defined $_ ? $_ : '';
            my $prj            = defined $3 ? $3 : '';
            my $file           = defined $4 ? $4 : '';
            my $type           = defined $6 ? $6 : '';
            my $gid            = defined $7 ? $7 : '';
            my $lid            = defined $8 ? $8 : '';
            my $plattform      = defined $10 ? $10 : '';
            my $lang           = defined $12 ? $12 : '';

            foreach my $isolang ( keys ( %{ $langhash_ref } ) ){
                if( $isolang=~ /$lang/i || $isolang=~ /all/i ) { $block{$prj.$gid.$lid.$file.$type.$plattform } =  $line ; }
            }
        }
    }
    return (\%block);
}

#########################################################
sub read_file_fallbacks{

    my $sdffile                 = shift;
    my $isolanglist_ref         = shift;
    my $fallbackhashhash_ref    = shift;
    my $block_ref;
    # read fallback for all files
    open MYFILE , "< $sdffile"
        or die "Can't open '$sdffile'\n";

    while( <MYFILE>){
          if( /$sdf_regex/ ){
            my $line           = defined $_ ? $_ : '';
            my $prj            = defined $3 ? $3 : '';
            my $file           = defined $4 ? $4 : '';
            my $type           = defined $6 ? $6 : '';
            my $gid            = defined $7 ? $7 : '';
            my $lid            = defined $8 ? $8 : '';
            my $lang           = defined $12 ? $12 : '';
            my $plattform      = defined $10 ? $10 : '';

            chomp( $line );
            foreach my $isolang ( @{$isolanglist_ref}  ){
                if( $isolang=~ /$lang/i ) { $fallbackhashhash_ref->{ $isolang }{ $prj.$gid.$lid.$file.$type.$plattform } =  $line ;
                    # aaaaa
                    #print STDOUT "$line\n";
                }
            }
        }
    }
}

#########################################################
sub read_file_fallback{

    my $sdffile                 = shift;
    my $isolang                 = shift;
    my $fallbackhashhash_ref    = shift;
    my $block_ref;
    # read fallback for single file
    open MYFILE , "< $sdffile"
        or die "Can't open '$sdffile'\n";

    while( <MYFILE>){
          if( /$sdf_regex/ ){
            my $line           = defined $_ ? $_ : '';
            my $prj            = defined $3 ? $3 : '';
            my $file           = defined $4 ? $4 : '';
            my $type           = defined $6 ? $6 : '';
            my $gid            = defined $7 ? $7 : '';
            my $lid            = defined $8 ? $8 : '';
            my $helpid         = defined $9 ? $9 : '';
            my $lang           = defined $12 ? $12 : '';
            my $plattform      = defined $10 ? $10 : '';

            chomp( $line );
            if( $isolang=~ /$lang/i ) { $fallbackhashhash_ref->{ $isolang }{ $prj.$gid.$lid.$file.$type.$plattform } =  $line ;
                    # aaaaaa
                    #print STDOUT "{ $isolang }{ $prj.$gid.$lid.$file.$type.$plattform->$plattform<- }='$line'\n";
            }
        }
    }
}

#########################################################
sub parseLanguages{

    my $bAll;
    my $bUseLocalize;
    my %langhash;
    my $iso="";
    my $fallback="";

    #### -l all
    if(   $languages=~ /all/ ){
        $bAll = "TRUE";
    }
    ### -l fr=de,de
    elsif( $languages=~ /.*,.*/ ){
        my @tmpstr =  split "," , $languages;
        for my $lang ( @tmpstr ){
            if( $lang=~ /([a-zA-Z]{2,3}(-[a-zA-Z\-]*)*)(=([a-zA-Z]{2,3}(-[a-zA-Z\-]*)*))?/ ){
                $iso        = $1;
                $fallback   = $4;

                if( ( $iso && $iso=~ /(de|en-US)/i )  || ( $fallback && $fallback=~ /(de|en-US)/i ) ) {
                    $bUseLocalize = "TRUE";
                }
             if( $fallback ) { $langhash{ $iso } = $fallback;   }
             else            { $langhash{ $iso } = "";          }
            }
        }
    }
    ### -l de
    else{
        if( $languages=~ /([a-zA-Z]{2,3}(-[a-zA-Z\-]*)*)(=([a-zA-Z]{2,3}(-[a-zA-Z\-]*)*))?/ ){
            $iso        = $1;
            $fallback   = $4;

            if( ( $iso && $iso=~ /(de|en-US)/i )  || ( $fallback && $fallback=~ /(de|en-US)/i ) ) {
                $bUseLocalize = "TRUE";
            }
             if( $fallback ) { $langhash{ $iso } = $fallback;   }
             else            { $langhash{ $iso } = "";          }
        }
    }
    return ( $bAll ,  $bUseLocalize , \%langhash );
}

#########################################################
sub parse_options{

    my $help;
    my $merge;
    my $extract;
    my $success = GetOptions('f=s' => \$sdffile , 'l=s' => \$languages , 's=s' => \$srcpath ,  'h' => \$help , 'v' => \$bVerbose , 'm' => \$merge , 'e' => \$extract );
    $outputfile = $sdffile;
    if( !$srcpath ){
        $srcpath = "$ENV{SRC_ROOT}";
        if( !$srcpath ){
            print STDERR "No path to the source root found!\n\n";
            usage();
            exit(1);
        }
    }
    if( $help || !$success || $#ARGV > 1 || ( !$sdffile ) ){
        usage();
        exit(1);
    }
    if( $merge && $sdffile && ! ( -r $sdffile)){
        print STDERR "Can't open file '$sdffile'\n";
        exit(1);
    }
    if( !( $languages=~ /[a-zA-Z]{2,3}(-[a-zA-Z\-]*)*(=[a-zA-Z]{2,3}(-[a-zA-Z\-]*)*)?(,[a-zA-Z]{2,3}(-[a-zA-Z\-]*)*(=[a-zA-Z]{2,3}(-[a-zA-Z\-]*)*)?)*/ ) ){
        print STDERR "Please check the -l iso code\n";
        exit(1);
    }
    if( ( !$merge && !$extract ) || ( $merge && $extract ) ){ usage();exit( -1 );}
    if( $extract ){ $mode = "extract"; }
    else          { $mode = "merge";   }
}

#########################################################
sub usage{

    print STDERR "Usage: localize.pl\n";
    print STDERR "Split or collect SDF files\n";
    print STDERR "           merge: -m -f <sdffile>    -l l1[=f1][,l2[=f2]][...] [ -s <sourceroot> ]\n";
    print STDERR "         extract: -e -f <outputfile> -l <lang> [ -s <sourceroot> ]\n";
    print STDERR "Options:\n";
    print STDERR "    -h              help\n";
    print STDERR "    -m              Merge mode\n";
    print STDERR "    -e              Extract mode\n";
    print STDERR "    -f <sdffile>    To split a big SDF file into particles\n";
    print STDERR "       <outputfile> To collect and join all particles to one big file\n";
    print STDERR "    -s <sourceroot> Path to the modules, if no \$SRC_ROOT is set\n";
    print STDERR "    -l ( all | <isocode> | <isocode>=fallback ) comma seperated languages\n";
    print STDERR "    -v              Verbose\n";
}

#            my $line           = defined $_ ? $_ : '';
#            my $leftpart       = defined $2 ? $2 : '';
#            my $prj            = defined $3 ? $3 : '';
#            my $file           = defined $4 ? $4 : '';
#            my $dummy          = defined $5 ? $5 : '';
#            my $type           = defined $6 ? $6 : '';
#            my $gid            = defined $7 ? $7 : '';
#            my $lid            = defined $8 ? $8 : '';
#            my $helpid         = defined $9 ? $9 : '';
#            my $plattform      = defined $10 ? $10 : '';
#            my $width          = defined $11 ? $11 : '';
#            my $lang           = defined $12 ? $12 : '';
#            my $rightpart      = defined $13 ? $13 : '';
#            my $text           = defined $14 ? $14 : '';
#            my $helptext       = defined $15 ? $15 : '';
#            my $quickhelptext  = defined $16 ? $16 : '';
#            my $title          = defined $17 ? $17 : '';
#            my $timestamp      = defined $18 ? $18 : '';

