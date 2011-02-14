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

use strict;
use Getopt::Long;
use IO::Handle;
use File::Find;
use File::Temp;
use File::Path;
use File::Copy;
use File::Glob qw(:glob csh_glob);
use Cwd;

my $CVS_BINARY = "/usr/bin/cvs";
# ver 1.1
#
#### module lookup
#use lib ("$ENV{SOLARENV}/bin/modules", "$ENV{COMMON_ENV_TOOLS}/modules");

#### module lookup
# OOo conform
my @lib_dirs;
BEGIN {
    if ( !defined($ENV{SOLARENV}) ) {
        die "No environment found (environment variable SOLARENV is undefined)";
    }
    push(@lib_dirs, "$ENV{SOLARENV}/bin/modules");
    push(@lib_dirs, "$ENV{COMMON_ENV_TOOLS}/modules") if defined($ENV{COMMON_ENV_TOOLS});
}
use lib (@lib_dirs);

#### globals ####
my $sdffile                 = '';
my $no_sort                 = '';
my $create_dirs             = '';
my $multi_localize_files    = '';
my $module_to_merge         = '';
my $sort_sdf_before         = '';
my $outputfile              = '';
my $no_gsicheck             = '';
my $mode                    = '';
my $bVerbose                = "0";
my $srcpath                 = '';
my $languages;
#my %sl_modules;     # Contains all modules where en-US and de is source language
my $use_default_date = '0';
my $force_ooo_module = '0';
my %is_ooo_module;
my %is_so_module;

         #         (                           leftpart                                                     )            (           rightpart                    )
         #            prj      file      dummy     type       gid       lid      helpid    pform     width      lang       text    helptext  qhelptext   title    timestamp
my $sdf_regex  = "((([^\t]*)\t([^\t]*)\t([^\t]*)\t([^\t]*)\t([^\t]*)\t([^\t]*)\t([^\t]*)\t([^\t]*)\t([^\t]*))\t([^\t]*)\t(([^\t]*)\t([^\t]*)\t([^\t]*)\t([^\t]*)\t)([^\t]*))";
my $file_types = "(src|hrc|xcs|xcu|lng|ulf|xrm|xhp|xcd|xgf|xxl|xrb)";
# Always use this date to prevent cvs conflicts
my $default_date = "2002-02-02 02:02:02";
my @sdfparticles;

#### main ####
parse_options();

my $binpath = '';
if( defined $ENV{UPDMINOREXT} )
{
    $binpath = $ENV{SOLARVER}."/".$ENV{INPATH}."/bin".$ENV{UPDMINOREXT}."/" ;
}
else
{
    $binpath = $ENV{SOLARVER}."/".$ENV{INPATH}."/bin/" ;
}

#%sl_modules = fetch_sourcelanguage_dirlist();


if   ( $mode eq "merge"    )    {
    if ( ! $no_gsicheck ){
        merge_gsicheck();
    }
    splitfile( $sdffile );
    if ( ! $no_gsicheck ){
        unlink $sdffile;             # remove temp file!
    }
}
elsif( $mode eq "extract"  )    {
    collectfiles( $outputfile );
}
else                            {
    usage();
}

exit(0);

#########################################################
sub splitfile{

    my $lastFile        = '';
    my $currentFile     = '';
    my $cur_sdffile     = '';
    my $last_sdffile    = '';
    my $delim;
    my $badDelim;
    my $start           = 'TRUE';
    my %index  = ();
    my %block;

    STDOUT->autoflush( 1 );

    #print STDOUT "Open File $sdffile\n";
    open MYFILE , "< $sdffile"
    or die "Can't open '$sdffile'\n";

#    my %lang_hash;
    my %string_hash_ooo;
    my %string_hash_so;
    my %so_modules;
    $so_modules{ "extras_full" } = "TRUE";

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
            my $helpid         = defined $9 ? $9 : '';
            next if( $prj eq "binfilter" );     # Don't merge strings into binfilter module
            chomp( $line );

            if( $force_ooo_module )
            {
                $string_hash_ooo { $lang }{ "$prj\t$file\t$type\t$gid\t$lid\t$helpid\t$plattform\t$lang" } = $line;
            }
            else
            {
                $string_hash_so{ $lang }{ "$prj\t$file\t$type\t$gid\t$lid\t$helpid\t$plattform\t$lang" } = $line;
            }
        }
    }
    close( MYFILE );

    if( !defined $ENV{SOURCE_ROOT_DIR} ){
        print "Error, no SOURCE_ROOT_DIR in env found.\n";
        exit( -1 );
    }
    my $src_root = $ENV{SOURCE_ROOT_DIR};
    my $ooo_src_root = $src_root."/l10n/l10n";
    my $so_l10n_path  = $src_root."/sun/l10n_so/source";
    my $ooo_l10n_path = $ooo_src_root."/l10n/source";

    #print "$so_l10n_path\n";
    #print "$ooo_l10n_path\n";

    if( $force_ooo_module )
    {
        write_sdf( \%string_hash_ooo , $ooo_l10n_path );
    }
    else
    {
        write_sdf( \%string_hash_so , $so_l10n_path );
    }
}

sub write_sdf
{
    my $string_hash         = shift;
    my $l10n_file           = shift;

    foreach my $lang( keys( %{ $string_hash } ) )
    {
        my @sdf_file;
        next , if( $lang eq "en-US" );

        mkdir $l10n_file."/$lang";
        # mkdir!!!!
        my $current_l10n_file = $l10n_file."/$lang/localize.sdf";
        print "Writing '$current_l10n_file'\n";
        if( open DESTFILE , "< $current_l10n_file" ){

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
                    my $helpid         = defined $9 ? $9 : '';

                    chomp( $line );
                    if ( defined $string_hash->{ $lang }{ "$prj\t$file\t$type\t$gid\t$lid\t$helpid\t$plattform\t$lang" } )
                    {
                        # Changed String!
                        push @sdf_file , $string_hash->{ $lang }{ "$prj\t$file\t$type\t$gid\t$lid\t$helpid\t$plattform\t$lang" } ;
                        $string_hash->{ $lang }{ "$prj\t$file\t$type\t$gid\t$lid\t$helpid\t$plattform\t$lang" } = undef;
                    }
                    else
                    {
                        # No new string
                        push @sdf_file , $line;
                    }
                }
            }
        }
        close( DESTFILE );
        #Now just append the enw strings
        #FIXME!!! Implement insertion in the correct order
        foreach my $key ( keys ( %{ $string_hash->{ $lang } } ) )
        {
            push @sdf_file , $string_hash->{ $lang }{ $key } , if ( defined $string_hash->{ $lang }{ $key } );
            #print "WARNING: Not defined = ".$string_hash->{ $lang }{ $key }."\n", if( ! defined  $string_hash->{ $lang }{ $key } );
        }

        # Write the new file
        my ( $TMPFILE , $tmpfile ) = File::Temp::tempfile();
        if( open DESTFILE , "+> $tmpfile " ){
            print DESTFILE get_license_header();
            foreach my $string( @sdf_file ){
                print DESTFILE "$string\n";
            }
            close ( DESTFILE );
            if( move( $current_l10n_file , $current_l10n_file.".backup" ) ){
                if( copy( $tmpfile , $current_l10n_file ) ){
                    unlink $l10n_file.".backup";
                 } else { print STDERR "Can't open/create '$l10n_file', original file is renamed to $l10n_file.backup\n"; }
            } else { print STDERR "Can't open/create '$l10n_file'\n"; }
         }else{
            print STDERR "WARNING: Can't open/create '$l10n_file'\n";
         }
         unlink $tmpfile;
     }
}

#########################################################

sub get_license_header{
    return
"#\n".
"#    ####    ###     #   #   ###   #####    #####  ####   #####  #####  \n".
"#    #   #  #   #    ##  #  #   #    #      #      #   #    #      #    \n".
"#    #   #  #   #    # # #  #   #    #      ###    #   #    #      #    \n".
"#    #   #  #   #    #  ##  #   #    #      #      #   #    #      #    \n".
"#    ####    ###     #   #   ###     #      #####  ####   #####    #    \n".
"#\n".
"#    DO NOT EDIT! This file will be overwritten by localisation process\n".
"#\n".
"#*************************************************************************\n".
"#\n".
"# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.\n".
"# \n".
"# Copyright 2000, 2010 Oracle and/or its affiliates.\n".
"#\n".
"# OpenOffice.org - a multi-platform office productivity suite\n".
"#\n".
"# This file is part of OpenOffice.org.\n".
"#\n".
"# OpenOffice.org is free software: you can redistribute it and/or modify\n".
"# it under the terms of the GNU Lesser General Public License version 3\n".
"# only, as published by the Free Software Foundation.\n".
"#\n".
"# OpenOffice.org is distributed in the hope that it will be useful,\n".
"# but WITHOUT ANY WARRANTY; without even the implied warranty of\n".
"# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n".
"# GNU Lesser General Public License version 3 for more details\n".
"# (a copy is included in the LICENSE file that accompanied this code).\n".
"#\n".
"# You should have received a copy of the GNU Lesser General Public License\n".
"# version 3 along with OpenOffice.org.  If not, see\n".
"# <http://www.openoffice.org/license.html>\n".
"# for a copy of the LGPLv3 License.\n".
"#\n".
"#*************************************************************************\n";
}
######## Check input sdf file and use only the correct part
sub merge_gsicheck{
    my $command = '';
    my ( $TMPHANDLE , $tmpfile ) = File::Temp::tempfile();
    close ( $TMPHANDLE );

    $command = "$ENV{WRAPCMD} " if( $ENV{WRAPCMD} );
    $command .= "$ENV{SOLARVER}/$ENV{INPATH}/bin/gsicheck";

    my $errfile = $sdffile.".err";
    $command .= " -k -c -wcf $tmpfile -wef $errfile -l \"\" $sdffile";
    #my $rc = system( $command );
    my $output = `$command`;
    my $rc = $? << 8;
    if ( $output ne "" ){
        print STDOUT "### gsicheck ###\n";
        print STDOUT "### The file $errfile have been written containing the errors in your sdf file. Those lines will not be merged: ###\n\n";
        print STDOUT "$output\n";
        print STDOUT "################\n";

    }else{
        # Remove the 0 Byte file
        unlink $errfile;
    }
    $sdffile = $tmpfile;
}
#########################################################
# find search function
sub wanted
{
    my $file = $File::Find::name;
    if( -f $file && $file =~ /.*localize.sdf$/ && !( $file =~ /.*\.svn.*/ ) ) {
        push   @sdfparticles , $file;
        if( $bVerbose eq "1" ) { print STDOUT "$file\n"; }
        else { print ".";  }
    }
}

sub add_paths
{
    my $langhash_ref            = shift;
    my $root_dir = $ENV{ SRC_ROOT };
    my $ooo_l10n_dir = "$root_dir/l10n/source";
    my $so_l10n_dir  = "$root_dir/l10n_so/source";

    if( -e $ooo_l10n_dir )
    {
        foreach my $lang ( keys( %{ $langhash_ref } ) )
        {
            my $loc_file = "$ooo_l10n_dir/$lang/localize.sdf";
            if( -e $loc_file )
            {
                push @sdfparticles , "$ooo_l10n_dir/$lang/localize.sdf";
            }
            else { print "WARNING: $loc_file not found ....\n"; }
        }
    }
    else { die "ERROR: Can not find directory $ooo_l10n_dir!!!" }
    if( -e $so_l10n_dir )
    {
        foreach my $lang ( keys( %{ $langhash_ref } ) )
        {
            my $loc_file = "$so_l10n_dir/$lang/localize.sdf";
            if( -e $loc_file )
            {
                push @sdfparticles , "$ooo_l10n_dir/$lang/localize.sdf";
            }
            else { #print "WARNING: $loc_file not found ....\n";
            }
        }

    }
}
sub collectfiles{
    print STDOUT "### Localize\n";
    my $localizehash_ref;
    my ( $bAll , $bUseLocalize, $langhash_ref , $bHasSourceLanguage , $bFakeEnglish ) = parseLanguages();

    # Enable autoflush on STDOUT
    # $| = 1;
    STDOUT->autoflush( 1 );

    my $working_path = getcwd();
    chdir $ENV{SOURCE_ROOT_DIR}, if defined $ENV{SOURCE_ROOT_DIR};
    add_paths( $langhash_ref );

    my ( $LOCALIZEPARTICLE , $localizeSDF ) = File::Temp::tempfile();
    close( $LOCALIZEPARTICLE );

    my ( $ALLPARTICLES_MERGED , $particleSDF_merged )     = File::Temp::tempfile();
    close( $ALLPARTICLES_MERGED );
    my ( $LOCALIZE_LOG , $my_localize_log ) = File::Temp::tempfile();
    close( $LOCALIZE_LOG );

    ## Get the localize en-US extract
    if( $bAll || $bUseLocalize ){
        print "### Fetching source language strings\n";
        my $command = "";
        my $args    = "";

        if( $ENV{WRAPCMD} ){
            $command = $ENV{WRAPCMD}.$binpath."localize_sl";
        }else{
            $command = $binpath."localize_sl";
        }
        print $command;
        # -e
        # if ( -x $command ){
        if( $command ){
            if( !$bVerbose  ){ $args .= " "; }
            $args .= " -e -f $localizeSDF -l ";
            my $bFlag="";
            if( $bAll ) {$args .= " en-US";}
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
        if ( $bVerbose ) { print STDOUT $command.$args."\n"; }

        my $rc = system( $command.$args );

        if( $rc < 0 ){    print STDERR "ERROR: localize rc = $rc\n"; exit( -1 ); }
        ( $localizehash_ref )  = read_file( $localizeSDF , $langhash_ref );

    }
    ## Get sdf particles
#*****************
    open ALLPARTICLES_MERGED , "+>> $particleSDF_merged"
    or die "Can't open $particleSDF_merged";

    ## Fill fackback hash
    my( $fallbackhashhash_ref ) = fetch_fallback( \@sdfparticles , $localizeSDF ,  $langhash_ref );
    my %block;
    my $cur_fallback;
    if( !$bAll) {
        foreach my $cur_lang ( keys( %{ $langhash_ref } ) ){
            #print STDOUT "DBG: G1 cur_lang=$cur_lang\n";
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
                            my $helpid         = defined $9 ? $9 : '';

                            chomp( $line );

                            if ( $lang eq $cur_lang ){
                                # Overwrite fallback strings with collected strings
                                #if( ( !has_two_sourcelanguages( $cur_lang) && $cur_lang eq "de" ) || $cur_lang ne "en-US" ){
                                     $fallbackhashhash_ref->{ $cur_lang }{ $prj.$gid.$lid.$file.$type.$plattform.$helpid } =  $line ;
                                     #}

                            }
                        }
                    }
                }else { print STDERR "WARNING: Can't open file $currentfile"; }
            }

            foreach my $line ( keys( %{$fallbackhashhash_ref->{ $cur_lang } } )) {
                if( #$cur_lang ne "de" &&
                    $cur_lang ne "en-US" ){
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
            else { print STDERR "WARNING: Can't open file $currentfile"; }
        }
    }
    close ALLPARTICLES_MERGED;

    # Hash of array
    my %output;
    my @order;

    ## Join both
    if( $outputfile ){
        if( open DESTFILE , "+> $outputfile" ){
            if( !open LOCALIZEPARTICLE ,  "< $localizeSDF" ) { print STDERR "ERROR: Can't open file $localizeSDF\n"; }
            if( !open ALLPARTICLES_MERGED , "< $particleSDF_merged" ) { print STDERR "ERROR: Can't open file $particleSDF_merged\n"; }

            # Insert localize
            my $extract_date="";
            while ( <LOCALIZEPARTICLE> ){
                if( /$sdf_regex/ ){
                    my $leftpart       = defined $2 ? $2 : '';
                    my $lang           = defined $12 ? $12 : '';
                    my $rightpart      = defined $13 ? $13 : '';
                    my $timestamp      = defined $18 ? $18 : '';

                    my $prj            = defined $3 ? $3 : '';
                    my $file           = defined $4 ? $4 : '';
                    my $type           = defined $6 ? $6 : '';
                    my $gid            = defined $7 ? $7 : '';
                    my $lid            = defined $8 ? $8 : '';
                    #my $lang           = defined $12 ? $12 : '';
                    my $plattform      = defined $10 ? $10 : '';
                    my $helpid         = defined $9 ? $9 : '';


                    if( $use_default_date )
                    {
                        $extract_date = "$default_date\n" ;
                    }
                    elsif( $extract_date eq "" ) {
                        $extract_date = $timestamp ;
                        $extract_date =~ tr/\r\n//d;
                        $extract_date .= "\n";
                    }

                    if( $bAll ){ print DESTFILE $leftpart."\t".$lang."\t".$rightpart.$extract_date ; }
                    else {
                        foreach my $sLang ( keys( %{ $langhash_ref } ) ){
                            if( $sLang=~ /all/i )                       {
                                push @{ $output{ $prj.$gid.$lid.$file.$type.$plattform.$helpid } } ,  $leftpart."\t".$lang."\t".$rightpart.$extract_date ;
                                #print DESTFILE $leftpart."\t".$lang."\t".$rightpart.$extract_date;
                            }
                            #if( $sLang eq "de" && $lang eq "de" )       {
                            #    push @{ $output{ $prj.$gid.$lid.$file.$type.$plattform.$helpid } } ,  $leftpart."\t".$lang."\t".$rightpart.$extract_date ;
                                #print DESTFILE $leftpart."\t".$lang."\t".$rightpart.$extract_date;
                                #}
                            if( $sLang eq "en-US" && $lang eq "en-US" ) {
                                push @order , $prj.$gid.$lid.$file.$type.$plattform.$helpid;
                                if( !$bFakeEnglish ){ push @{ $output{ $prj.$gid.$lid.$file.$type.$plattform.$helpid } } ,  $leftpart."\t".$lang."\t".$rightpart.$extract_date ; }
                                #print DESTFILE $leftpart."\t".$lang."\t".$rightpart.$extract_date;
                            }

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

                    #my $prj            = defined $3 ? $3 : '';
                    my $file           = defined $4 ? $4 : '';
                    my $type           = defined $6 ? $6 : '';
                    my $gid            = defined $7 ? $7 : '';
                    my $lid            = defined $8 ? $8 : '';
                    #my $lang           = defined $12 ? $12 : '';
                    my $plattform      = defined $10 ? $10 : '';
                    my $helpid         = defined $9 ? $9 : '';


                    if( $use_default_date )
                    {
                        $extract_date = "$default_date\n" ;
                    }
                    elsif( $extract_date eq "" )
                    {
                        $extract_date = $timestamp;
                    }

                    if( ! ( $prj =~ /binfilter/i ) ) {
                        push @{ $output{ $prj.$gid.$lid.$file.$type.$plattform.$helpid } } , $leftpart."\t".$lang."\t".$rightpart.$extract_date ;
                        #print DESTFILE $leftpart."\t".$lang."\t".$rightpart.$extract_date ;
                    }
                 }
            }

            # Write!
            foreach my $curkey ( @order ){
                foreach my $curlist ( $output{ $curkey } ){
                    foreach my $line ( @{$curlist} ){
                        print DESTFILE $line;
                    }
                }
            }

        }else { print STDERR "Can't open $outputfile";}
    }
    close DESTFILE;
    close LOCALIZEPARTICLE;
    close ALLPARTICLES_MERGED;
    chdir $working_path;

    #print STDOUT "DBG: \$localizeSDF $localizeSDF \$particleSDF_merged $particleSDF_merged\n";
    unlink $localizeSDF , $particleSDF_merged ,  $my_localize_log;

    #sort_outfile( $outputfile );
    #remove_obsolete( $outputfile ) , if $bHasSourceLanguage ne "";
    }

#########################################################
sub remove_obsolete{
    my $outfile = shift;
    my @lines;
    my $enusleftpart;
    my @good_lines;

    print STDOUT "### Removing obsolete strings\n";

    # Kick out all strings without en-US reference
    if ( open ( SORTEDFILE , "< $outfile" ) ){
        while( <SORTEDFILE> ){
            if( /$sdf_regex/ ){
                my $line           = defined $_ ? $_ : '';
                my $language       = defined $12 ? $12 : '';
                my $prj            = defined $3 ? $3 : '';
                my $file           = defined $4 ? $4 : '';
                my $type           = defined $6 ? $6 : '';
                my $gid            = defined $7 ? $7 : '';
                my $lid            = defined $8 ? $8 : '';
                my $plattform      = defined $10 ? $10 : '';
                my $helpid         = defined $9 ? $9 : '';

                my $leftpart = $prj.$gid.$lid.$file.$type.$plattform.$helpid;

                if( $language eq "en-US" ){                 # source string found, 1. entry
                    $enusleftpart = $leftpart;
                    push @good_lines , $line;
                }else{
                    if( !defined $enusleftpart or !defined $leftpart ){
                        print STDERR "BADLINE: $line\n";
                        print STDERR "\$enusleftpart = $enusleftpart\n";
                        print STDERR "\$leftpart = $leftpart\n";
                    }
                    if( $enusleftpart eq $leftpart ){   # matching language
                        push @good_lines , $line;
                    }
                    #else{
                    #    print STDERR "OUT:  \$enusleftpart=$enusleftpart \$leftpart=$leftpart \$line=$line\n";
                    #}
                }
            }
        }
        close SORTEDFILE;
    } else { print STDERR "ERROR: Can't open file $outfile\n";}

    # Write file
    if ( open ( SORTEDFILE , "> $outfile" ) ){
        foreach my $newline ( @good_lines ) {
            print SORTEDFILE $newline;
        }
        close SORTEDFILE;
    } else { print STDERR "ERROR: Can't open file $outfile\n";}

}
#########################################################
sub sort_outfile{
        my $outfile = shift;
        print STDOUT "### Sorting ... $outfile ...";
        my @lines;
        my @sorted_lines;


        #if ( open ( SORTEDFILE , "< $outputfile" ) ){
        if ( open ( SORTEDFILE , "< $outfile" ) ){
            my $line;
            while ( <SORTEDFILE> ){
                $line = $_;
                if( $line =~ /^[^\#]/ ){
                    push @lines , $line;
                }
            }
            close SORTEDFILE;
            @sorted_lines = sort {
                my $xa_lang          = "";
                my $xa_left_part    = "";
                my $xa_right_part    = "";
                my $xa_timestamp     = "";
                my $xb_lang          = "";
                my $xb_left_part    = "";
                my $xb_right_part    = "";
                my $xb_timestamp     = "";
                my $xa               = "";
                my $xb               = "";
                my @alist;
                my @blist;

                if( $a=~ /$sdf_regex/ ){
                    $xa_left_part       = defined $2 ? $2 : '';
                    $xa_lang           = defined $12 ? $12 : '';
                    $xa_right_part     = defined $13 ? $13 : '';
                    $xa_left_part = remove_last_column( $xa_left_part );

                }
                if( $b=~ /$sdf_regex/ ){
                    $xb_left_part       = defined $2 ? $2 : '';
                    $xb_lang           = defined $12 ? $12 : '';
                    $xb_right_part     = defined $13 ? $13 : '';
                    $xb_left_part = remove_last_column( $xb_left_part );


                }
                if( (  $xa_left_part cmp $xb_left_part ) == 0 ){         # Left part equal
                     if( ( $xa_lang cmp $xb_lang ) == 0 ){               # Lang equal
                         return ( $xa_right_part cmp $xb_right_part );   # Right part compare
                    }
                    elsif( $xa_lang eq "en-US" ) { return -1; }        # en-US wins
                    elsif( $xb_lang eq "en-US" ) { return 1;  }        # en-US wins
                    else { return $xa_lang cmp $xb_lang; }             # lang compare
                }
                else {
                    return $xa_left_part cmp $xb_left_part;        # Left part compare
                }
            } @lines;

            if ( open ( SORTEDFILE , "> $outfile" ) ){
                print SORTEDFILE get_license_header();
                foreach my $newline ( @sorted_lines ) {
                    print SORTEDFILE $newline;
                    #print STDOUT $newline;
                }
            }
            close SORTEDFILE;
        } else { print STDERR "WARNING: Can't open file $outfile\n";}
    print "done\n";

}
#########################################################
sub remove_last_column{
    my $string                  = shift;
    my @alist = split ( "\t" , $string );
    pop @alist;
    return join( "\t" , @alist );
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
    my $sdfparticleslist_ref   = shift;
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
        if( $cur_lang eq "en-US" ){
            read_fallbacks_from_source( $localizeSDF , $cur_lang , \%fallbackhashhash );
        }
    }

    # remove de / en-US
    my @tmplist;
    foreach $cur_lang( @langlist ){
        if(  $cur_lang ne "en-US" ){
           push @tmplist , $cur_lang;

        }
    }
    @langlist = @tmplist;
    if ( $#langlist +1 ){
        read_fallbacks_from_particles( $sdfparticleslist_ref , \@langlist , \%fallbackhashhash );

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
            my $helpid         = defined $9 ? $9 : '';

            foreach my $isolang ( keys ( %{ $langhash_ref } ) ){
                if( $isolang=~ /$lang/i || $isolang=~ /all/i ) { $block{$prj.$gid.$lid.$file.$type.$plattform.$helpid } =  $line ; }
            }
        }
    }
    return (\%block);
}

#########################################################
sub read_fallbacks_from_particles{

    my $sdfparticleslist_ref    = shift;
    my $isolanglist_ref         = shift;
    my $fallbackhashhash_ref    = shift;
    my $block_ref;
    foreach my $currentfile ( @{ $sdfparticleslist_ref } ){
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
                    my $helpid         = defined $9 ? $9 : '';

                    chomp( $line );

                    foreach my $isolang ( @{$isolanglist_ref}  ){
                        if( $isolang=~ /$lang/i ) {
                            $fallbackhashhash_ref->{ $isolang }{ $prj.$gid.$lid.$file.$type.$plattform.$helpid } =  $line ;
                        }
                    }
                }
            }
       }else { print STDERR "WARNING: Can't open file $currentfile"; }
    }
}

#########################################################
sub read_fallbacks_from_source{

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
            if( $isolang=~ /$lang/i ) { $fallbackhashhash_ref->{ $isolang }{ $prj.$gid.$lid.$file.$type.$plattform.$helpid } =  $line ;
            }
        }
    }
}

#########################################################
sub parseLanguages{

    my $bAll;
    my $bUseLocalize;
    my $bHasSourceLanguage="";
    my $bFakeEnglish="";
    my %langhash;
    my $iso="";
    my $fallback="";

    #### -l all
    if(   $languages=~ /all/ ){
        $bAll = "TRUE";
        $bHasSourceLanguage = "TRUE";
    }
    ### -l fr=de,de
    elsif( $languages=~ /.*,.*/ ){
        my @tmpstr =  split "," , $languages;
        for my $lang ( @tmpstr ){
            if( $lang=~ /([a-zA-Z]{2,3}(-[a-zA-Z\-]*)*)(=([a-zA-Z]{2,3}(-[a-zA-Z\-]*)*))?/ ){
                $iso        = $1;
                $fallback   = $4;

                if( ( $iso && $iso=~ /(en-US)/i )  || ( $fallback && $fallback=~ /(en-US)/i ) ) {
                    $bUseLocalize = "TRUE";
                }
                if( ( $iso && $iso=~ /(en-US)/i ) ) {
                    $bHasSourceLanguage = "TRUE";
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

            if( ( $iso && $iso=~ /(en-US)/i )  || ( $fallback && $fallback=~ /(en-US)/i ) ) {
                $bUseLocalize = "TRUE";

            }
            if( ( $iso && $iso=~ /(en-US)/i )  ) {
                $bHasSourceLanguage = "TRUE";
            }

             if( $fallback ) { $langhash{ $iso } = $fallback;   }
             else            { $langhash{ $iso } = "";          }
        }
    }
    # HACK en-US always needed!
    if( !$bHasSourceLanguage ){
        #$bHasSourceLanguage = "TRUE";
        $bUseLocalize = "TRUE";
        $bFakeEnglish = "TRUE";
        $langhash{ "en-US" } = "";
    }
    return ( $bAll ,  $bUseLocalize , \%langhash , $bHasSourceLanguage, $bFakeEnglish);
}

#########################################################
sub parse_options{

    my $help;
    my $merge;
    my $extract;
    my $success = GetOptions('f=s' => \$sdffile , 'l=s' => \$languages , 's=s' => \$srcpath ,  'h' => \$help , 'v' => \$bVerbose ,
                             'm' => \$merge , 'e' => \$extract , 'x' => \$no_sort , 'd' => \$use_default_date , 'c' => \$create_dirs ,
                             'n' => \$no_gsicheck , 'o' => \$force_ooo_module );
    $outputfile = $sdffile;

    #print STDOUT "DBG: lang = $languages\n";
    if( !$srcpath ){
        $srcpath = "$ENV{SRC_ROOT}";
        if( !$srcpath ){
            print STDERR "No path to the source root found!\n\n";
            usage();
            exit(1);
        }
    }
    if( $help ){
        usage();
        exit(0);
    }
    if( !$success || $#ARGV > 1 || ( !$sdffile ) ){
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
#my $multi_localize_files    = ''; h
#my $module_to_merge         = ''; i
#my $sort_sdf_before         = ''; g

#########################################################
sub usage{

    print STDERR "Usage: localize.pl\n";
    print STDERR "Split or collect SDF files\n";
    print STDERR "           merge: -m -f <sdffile>    -l l1[=f1][,l2[=f2]][...] [ -s <sourceroot> ] [ -c ]\n";
    print STDERR "         extract: -e -f <outputfile> -l <lang> [ -s <sourceroot> ] [-d]\n";
    print STDERR "Options:\n";
    print STDERR "    -h              help\n";
    print STDERR "    -m              Merge mode\n";
    print STDERR "    -e              Extract mode\n";
    print STDERR "    -f <sdffile>    To split a big SDF file into particles\n";
    print STDERR "       <outputfile> To collect and join all particles to one big file\n";
    print STDERR "    -s <sourceroot> Path to the modules, if no \$SRC_ROOT is set\n";
    print STDERR "    -l ( all | <isocode> | <isocode>=fallback ) comma seperated languages\n";
    print STDERR "    -d              Use default date in extracted sdf file\n";
    print STDERR "    -c              Create needed directories\n";
    print STDERR "    -g              Sort sdf file before mergeing\n";
    print STDERR "    -h              File with localize.sdf's\n!";
    print STDERR "    -n              No gsicheck\n";
    print STDERR "    -i              Module to merge\n";
    print STDERR "    -o              force using ooo localization from the l10n module instead of l10n_so; \n";
    print STDERR "                    useful if the type can't be detected by the .svn tags; \n";
    print STDERR "    -v              Verbose\n";
    print STDERR "\nExample:\n";
    print STDERR "\nlocalize -e -l en-US,pt-BR=en-US -f my.sdf\n( Extract en-US and pt-BR with en-US fallback )\n";
    print STDERR "\nlocalize -m -l cs -f my.sdf\n( Merge cs translation into the sourcecode ) \n";
}

