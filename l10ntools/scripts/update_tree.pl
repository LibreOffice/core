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

use Cwd 'abs_path';
use File::Find;
use File::Copy qw/cp mv/;
use File::Basename;

# update the tree files in <platform>/misc/*

$| = 1;

my $prj = $ENV{PWD};

my $inpath = $ENV{WORKDIR};
terminate() if ( ! defined $inpath );

my $destpath = $inpath;
my $with_lang = $ARGV[1];
my $xmllint = $ENV{XMLLINT};

$tree_target_prefix = $ARGV[4];

# Always use / directory separators
$prj =~ s/\\/\//g if defined($prj);
$inpath =~ s/\\/\//g;
$destpath =~ s/\\/\//g;

if ( ! defined $prj ) {
# do someting that works for manual call
    ($scriptname = `pwd`) =~ s/\n/\/$0/;
    ($tree_src = $scriptname) =~ s/\/update_tree.pl/\/..\/source\/auxiliary/;
    ($tree_dest = $scriptname) =~ s/\/update_tree.pl/\/..\/$destpath\/misc/;
    ($source_dir = $scriptname) =~ s/\/update_tree.pl/\/..\/source/;
    ($source_dir_xhp = $scriptname) =~ s/\/update_tree.pl/\/..\/source/;

    if ( defined $ENV{TRYSDF} || defined $ENV{LOCALIZESDF} )
    {
        if( defined $ENV{LOCALIZATION_FOUND} && $ENV{LOCALIZATION_FOUND} eq "YES" )
        {
            $source_dir = $ENV{TRYSDF};
        }
        elsif( defined $ENV{LOCALIZESDF} && $ENV{LOCALIZESDF} ne "" )
        {
            $source_dir = $ENV{LOCALIZESDF};
        }
        $source_dir =~ s/\/auxiliary\/localize.sdf$// ;
    }
    $treestrings = "$ARGV[0]";
        $treestrings=~ s/\/*.tree//g;
} else {
    $source_dir_xhp = "$prj/source/presenter/help";
    $tree_src = "$ARGV[0]";
        $tree_src =~ s/\/help.tree//g;
    $tree_dest = "$ARGV[3]";
        $tree_dest =~ s/\/help.tree//g;
    $source_dir = "$ARGV[2]";
        $source_dir =~ s/\/localize.sdf//g;
    $treestrings = "$ARGV[0]";
        $treestrings=~ s/\/help.tree/\/tree_strings.xhp/g;

    if( defined $ENV{LOCALIZATION_FOUND} && $ENV{LOCALIZATION_FOUND} eq "YES" )
    {
        $source_dir = $ENV{TRYSDF};
    }
    elsif ( defined $ENV{LOCALIZESDF} && $ENV{LOCALIZESDF} ne "" )
    {
        $source_dir = $ENV{LOCALIZESDF};
    }
    $source_dir =~ s/\/auxiliary\/localize.sdf$// ;
}

# Get the English tree files as master
#-------------------------------
# Update English from xhp
#-------------------------------
&do_english;
#-------------------------------
# Update localizations from sdf
#-------------------------------

if( defined $with_lang && $with_lang ne "" )
{
    @langs = split /\s+/, $with_lang;
    &read_loc;
    for $l(@langs)
    {
        #if ($l ne "en-US") {
            &do_lang($l);
        #}
    }
}
else
{
    print "\nNo WITH_LANG set, skipping l10n\n";
}
#-------------------------------
#

####################
# SUBS
####################
sub terminate {
    $err = shift;
    print "$err\n\n";
    $msg = <<"MSG";

update_tree.pl
   all languages in WITH_LANG are processed. WITH_LANG=ALL is
   not supported in manual calls.

   Updates the *.tree files.
   At first, the English file is updated based on the English
   help topic titles as read from the help files. Then, the
   localized tree files are written based on the English tree
   file and the localized help topic titles.

   Requires a valid LibreOffice build environment.
MSG
   print "$msg\n";
   exit( -1 );
   # die "$msg\n";
}

#---------------------------------------------------

sub do_english {
    undef %helpsection; undef %node;
    &readtreestrings;
    &gettreefiles;
}

#---------------------------------------------------
sub do_lang {
    $lng = shift;
    print "Processing $lng\n";
    &processtreefiles($lng);
}

#---------------------------------------------------
sub readtreestrings {
    if (open TREE, $treestrings) {
         print "Processing readtreestrings\n";
        while (<TREE>) {
            chomp;
            s/<\/*help:productname>//gis;
            if (/help_section/) {
                s/^\s*<.*help_section//;
                s/<\/.*$//;
                ($id = $_) =~ s/^.*id=&quot;(\d+)&quot;.*$/$1/;
                ($title = $_) =~ s/^.*title=&quot;(.*)&quot;.*$/$1/;
                $helpsection{$id} = $title;
            }

            if (/node id=/) {
                s/^\s*<.*node //;
                s/<\/.*$//;
                ($id = $_) =~ s/^.*id=&quot;(\d+)&quot;.*$/$1/;
                ($title = $_) =~ s/^.*title=&quot;(.*)&quot;.*$/$1/;
                $node{$id} = $title;
            }
        }
        close TREE;
    } else {
        &terminate("Error opening $treestrings");
    }
}

#------------------------------------
sub gettreefiles {
    # Read the tree files from the directory
    # this list is also used for all foreign languages
    if (opendir ENUS, "$tree_src") {
        @treeviews = grep /\.tree/, readdir ENUS;
        closedir ENUS;
    } else {
        &terminate("Cannot open directory $tree_src");
    }
}

#------------------------------------
sub processtreefiles {
    $lng = shift;
    use File::Temp qw/ tempfile /;
    use File::Spec;

    for $tv(@treeviews) {
        @lines = &readtv("$tree_src/$tv");
        for $l(@lines) {
            if ($l =~ /topic/) {
                ($id = $l) =~ s/^.*id="([^"]*)".*$/$1/gis;
                ($module = $id) =~ s/^([^\/]*).*$/$1/;
                $id =~ s/^.*?\///;
                $file = "$source_dir_xhp/$lng/$id";

                if ($lng eq 'en-US') { # english comes from the file
                    $temp = $l;
                        $temp =~ s/^.*<topic[^>]+id=".*"[^>]*>([^<]*)<\/topic>.*$/$1/gis;
                        $temp =~ s/&apos;/\'/gis; $temp=~ s/&amp;/+/gis;
                        $temp =~ s/&quot;/\'/gis; $temp =~ s/&/+/gis;

                        $id =~s/^.*\/(.*.xhp)/$1/;
                        $l = "<topic id=\"$module/$tree_target_prefix/$id\">$temp</topic>\n";
                } else { # localized comes from the localize sdf
                    if (defined($loc_title{$lng}->{$id})) {
                        $xhpname = $id;
                        $xhpname =~s/^.*\/(.*.xhp)/$1/;
                        $l = "<topic id=\"$module/$tree_target_prefix/$xhpname\">$loc_title{$lng}->{$id}</topic>\n";
                    } else {
                    }
                }
            }

            if ($l =~/<node/) {
                ($id = $l) =~ s/^.*id="(\d+)".*$/$1/gis;
                if ($lng eq 'en-US') {
                    if (defined($node{$id})) {
                        $l =~ s/title="(.*)"/title="$node{$id}"/;
                    } else {
                        $l =~ s/title="(.*)"/title="NOTFOUND:$id"/;
                    }
                } else {
                    if (defined($node{$lng}->{$id})) {
                        $l =~ s/title="(.*)"/title="$node{$lng}->{$id}"/;
                    }
                }
            }

            if ($l =~/<help_section/) {
                ($id = $l) =~ s/^.*id="(\d+)".*$/$1/gis;
                if ($lng eq 'en-US') {
                    if (defined($helpsection{$id})) {
                        $l =~ s/title="(.*)"/title="$helpsection{$id}"/;
                    } else {
                        $l =~ s/title="(.*)"/title="NOTFOUND:$id"/;
                    }
                } else {
                    if (defined($helpsection{$lng}->{$id})) {
                        $l =~ s/title="(.*)"/title="$helpsection{$lng}->{$id}"/;
                    }
                }
            }
        }


        my $treeoutdir = "$tree_dest";
        my $tmpname_template=$tv."_XXXXX";
        my ( $treetmpfilehandle, $treetmpfile ) = tempfile($tmpname_template , DIR => File::Spec->tmpdir() );
        close $treetmpfilehandle ;

        if (open TV, ">$treetmpfile") {
            for $line(@lines) {
                $line =~ s/\$\[officename\]/%PRODUCTNAME/g;
                $line =~ s/\$\[officeversion\]/%PRODUCTVERSION/g;
                print TV $line;
            }
            close TV;
            chmod 0664, $treetmpfile or &terminate("Cannot change rights on $treetmpfile");
            if( $^O eq 'MSWin32' )
            {
                $tree_dest =~ s/\//\\/g ;
                unlink "$tree_dest\\$tv" ;
                mv $treetmpfile , "$tree_dest\\$tv" or &terminate("Cannot mv $treetmpfile to $tree_dest\\$tv" );
            }
            else
            {
                unlink "$tree_dest/$tv" ;
                my $ret=mv $treetmpfile , "$tree_dest/$tv" or &terminate("Cannot write to $tree_dest/$tv - Error $!");
                my $ret=mv "$tree_dest/$tv" , "$tree_dest/$tv" or &terminate("Cannot write to $tree_dest/$tv - Error $!");
                #xmllint is crashing on windows, fixme
                if( $^O ne 'cygwin' )
                {
                    system("$xmllint --noout --noent $tree_dest/$tv") == 0 or &terminate("$tree_dest/$tv is illformed xml ($xmllint on $^O)" );
                }
            }
      } else {
            &terminate("Cannot write to $tvout");
        }
    }
}

#------------------------------------
sub readtv {
    my $f = shift;
    if (open TV, $f) {
        $/ = "\n";
        my @l = <TV>;
        close TV;
        return @l;
    } else {
        &terminate("Error opening $f");
    }
}

#-------------------------------------
# read entries from localize.sdf files
#-------------------------------------
sub read_loc {
    $/ = "\n";
    my $path = "$source_dir";
    @files = `find $source_dir -name localize.sdf`;
    for my $fname (@files) {
        $FS = '\t';
        open(LOCALIZE_SDF, $fname) || die 'Cannot open "localize.sdf".'."$fname";
        while (<LOCALIZE_SDF>) {
            my $sdf_line = $_;
        my ($Fld1,$file,$Fld3,$Fld4,$id,$Fld6,$Fld7,$Fld8,$Fld9,$lang,$text) = split($FS, $sdf_line , 12);
            next if ( $Fld1 =~ /^#/);
        if ($id eq 'tit') {
                #strip filename
                $file =~ s/.*text\\/text\\/g;
                #convert \ to / in filename
                $file =~ s/\\/\//g;
                $file =~ s/^.*\/(.*)\/(.*)$/$1\/$2/;
                #fpe: i46823 - need to encode &s, added encoding
                $text =~ s/&(?!amp;)/&amp;/g;
                #help xml tags are not allowed in .tree files
                $text =~ s/\\<.*?\\>//g;
                # add entry to the hash

                $loc_title{$lang}->{$file} = $text;
            }
            if ($file =~ /tree_strings.xhp/) {
                #strip filename
                $file =~ s/.*text/text/g;
                #convert \ to / in filename
                $file =~ s/\\/\//g;
                if ($text =~ /^<help_section/) {
                    #example: <help_section application="scalc" id="08" title="表計算ドキュメント">
                    my ($fld1,$app,$fld3,$id,$fld5,$sec_title) = split('"', $text, 7);
                    #fpe: i46823 - need to encode &s, added encoding
                    if( defined $sec_title )
                    {
                        $sec_title =~ s/&(?!amp;)/&amp;/g;
                        #help xml tags are not allowed in .tree files
                        $sec_title =~ s/\\<.*?\\>//g;
                        #unquot \<item ... /\>
                        terminate( "\n\nERROR: Bad string in file '$fname' will cause invalid xml tree file \n---\n'$sdf_line'\n---\nPlease remove or replace < = '&lt;' and  > = '&gt;' within the title attribute '$sec_title'\n") , if( $sec_title =~ /[\<\>]/ );
                        $helpsection{$lang}->{$id} = $sec_title;
                    }
                } elsif ($text =~/<node id=/) {
                    # example: <node id="0205" title="Tabelas em documentos de texto">
                    # BEWARE: title may contain escaped '"' so only match " not preceded by \
                    # using a zero‐width negative look‐behind assertion.
                    my ($fld1,$id,$fld3,$node_title,$Fld5) = split(/(?<!\\)"/, $text, 5);
                    #fpe: i46823 - need to encode &s, added encoding
                    if( defined $node_title )
                    {
                        $node_title =~ s/&(?!amp;)/&amp;/g;
                        #help xml tags are not allowed in .tree files
                        $node_title =~ s/\\<.*?\\>//g;
                           terminate( "\n\nERROR: Bad string in '$fname' will cause invalid xml tree file \n---\n'$sdf_line'\n---\nPlease remove or replace < = '&lt;' and  > = '&gt;' within the title attribute '$node_title'\n") , if( $node_title =~ /[\<\>]/ );
                    }
                    $node{$lang}->{$id} = $node_title;
                }
            }
        }
        close LOCALIZE_SDF;
    }
    # statistics
    $total_elements=0;
    foreach $lang (keys %loc_title) {
        $no_elements = scalar(keys(%{$loc_title{$lang}}));
        push(@langstat, "$lang:\t ".$no_elements." matches\n");
        $total_elements += $no_elements;
    }
}
