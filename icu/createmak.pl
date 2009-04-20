:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: checksize.pl,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: rt $ $Date: 2006/04/21 10:48:45 $
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
#
use XML::Parser;
# ------------------------------------------------------------------------
# Other global stuff
$is_debug=0;
my $path = $ENV{'INPATH'} . "/";
my $quot = '"';
my %template_hash=();
my %vcproj_hash=();
# ------------------------------------------------------------------------
# Global stuff for parsing the *.vcproj files (XML::Parser)
#
my $ConfigurationGlobal = 1; # Release = 1, Debug=0
my $Configuration = undef;
my %configelements = ();
my %files = ();
my %files2copy = ();
my %files_special = ();
my $Release = 1;
     # Release = 1, Debug = 0, undef = 2
my $file_tmp = "";      # temporary storage for file name
my $CustomSection = 0;
# ------------------------------------------------------------------------
# ------------------------------------------------------------------------
# e.g. %object_hash with Key = blabla.cpp
# contains a Hash:
# Header: "..\\common\\unicode\\utypes.h ..\\common\\unicode\\uset.h"
# CFlags: Common (set in templates file) or special sequence
# CDefs:  Common (set in templates file) or special sequence
# ------------------------------------------------------------------------
my $configfile = shift;
my $sourcePath = shift;
if ( !$configfile ) {
    $configfile = "createmak.cfg";
}
if ( !$sourcePath ) {
    $inpath = $ENV{"INPATH"};
    $sourcePath = $inpath . "\\misc\\build\\icu\\source";
}
$dir = "";
$header = "";
$sep = "\\\\";

%project_by_id =();
%project_by_name = ();
%project_dependencies = ();
my @builddeps = prepare_allinone_all_mak(\%project_by_id,\%project_by_name,\%project_dependencies,$sourcePath);

fillTemplateHash($configfile);

create_allinone_all_mak(\@builddeps,\%project_by_id,$sourcePath);
my @dirs = ();
foreach $projectname(keys %project_by_name)
{
    my $dir = $project_by_name{$projectname}[1];
    $dir =~ /\.\.\\(.+)\\(.+)\.vcproj/;
    my $dir1 = $1;
    my $dir2 = $2;
    if ( $dir1 !~ /$dir2$/ ) {
        $dir1 .= "\.$dir2";
    }
    print "$dir1 - $dir2\n" if ($is_debug);
    push @dirs, $dir1;
}

# set nonpro switch (linking against debug runtime if nonpro=1)
my $nonpro = ($ENV{"PROEXT"} ne ".pro");
print "Non Product Build" if ($nonpro);

foreach $dir(@dirs)
{
    next if ($dir eq "data.makedata"); # very special don't overwrite this file
    # ------------------------------------------------------------------------
    # Reset global stuff for parsing the *.vcproj files (XML::Parser)
    #
    $Configuration = $ConfigurationGlobal; # Release = 1, Debug=0
    %configelements = ();
    %files = ();
       # contains all relevant *.c,*.cpp,*.h,*.rc files
    %files2copy = ();
    %files_special = ();
    $Release = 2;
     # Release = 1, Debug = 0, undef = 2
    $file_tmp = "";      # temporary storage for file name
    $CustomSection = 0;
    # ------------------------------------------------------------------------

    my $extra_mak = "";
    ($dir, $extra_mak) = handle_extra_mak_files($dir); # handle e.g. tools/genrb.derb entires

    my @mak=(); # Array for make file *.mak
    my %deps=();
    my %object_hash=();
    my %collected_header=();
    my %collect_header_target_hash=();
    my %collect_object_target_hash=();
    my $vcproj_file = "";
    my $resource_file = "";

    # $dir : common,i18n,...
    chomp $dir;
    next if ( $dir eq "" );
    my $fullpath = $sourcePath . "\\" . $dir;
    if ( $extra_mak eq "" ) {
        if ($dir =~ /(.+)+\\(.+)$/)
        {
            $vcproj_file = $fullpath ."\\$2.vcproj";
        } else
        {
            $vcproj_file = $fullpath ."\\$dir.vcproj";
        }
    } else
    {
        $vcproj_file = $fullpath . "\\" . $extra_mak . ".vcproj";
    }


    # Parse the *.vcproj file
    my $parser = new XML::Parser(ErrorContext => 2);
    $parser->setHandlers(Start => \&start_handler,
                 Char  => \&char_handler);
    $parser->parsefile($vcproj_file);
    if ( $file_tmp ) {
        # save a file which hasn't been saved yet
        $files{ $file_tmp } = 1;        # save it now
        $file_tmp = "";                 # has been saved now reset it
    }

    # is there a resource file?
    foreach $i (keys %files)
    {
        if ($i =~ /\.rc/)
        {
            $resource_file = $i;
        }
    }
    # Fill hash %deps for dependencies for all files in directory ($testdir)
    # %files contains all relevant files from *.vcproj

    getAllFilesDeps($fullpath,\%deps,\%files);
    my $includedir = $configelements{"Release"}{"OutputDirectory"};     # e.g. OutputDirectory = ..\..\lib
    $includedir =~ s/lib/include/;
    foreach $file( sort keys %deps)
    {
        $file =~ /(.+)\.(.+)/;
        my $name = $1;
        my $ext  = $2;
        next if (!defined $name);
        $object = "\$(INTDIR)\\" . "$name.obj";
        $collect_object_target_hash{$object}=1;

        createMakDepSection($dir,$name,$ext,$deps{$file},\@mak,\%files_special);
    }
    my %all_target_hash=();
    foreach $header(sort keys %files2copy)
    {
        my $dir;
        my $file;
        #$pathdepth = "..\\..";
        $file = $header;
        $header =~ s/^\.\\//;
        $inputpath = $file;
        $target = $includedir . "\\" . $header;
        $target =~ /.+\\(.+)\\.+$/;
        $targetpath = $configelements{"Release"}{"CommandLine"};
        chomp $targetpath;
        # set %all_target_hash and @mak
        createCopySection($file,$inputpath,$target,$targetpath,\@mak,\%all_target_hash);
        $collect_header_target_hash{$target}=1;
    }
    my $test = $configelements{"Release"}{"OutputFile"};
    $all_target_hash{$test}=1;

    # set name of the *.mak file
    my $mak_file="";
    if ( $extra_mak eq "" ) {
        $mak_file = $vcproj_file;
        $mak_file =~ s/vcproj/mak/;
    } else
    {
        # extra_mak eg. derb, stringperf
        $mak_file = $fullpath . "\\$extra_mak" . "\.mak";
    }

    # generate the content of the *.mak file
    # in @mak array
    print "extra_mak=$extra_mak\n" if ($is_debug);
    print "mak_file=$mak_file\n" if ($is_debug);
    open(MAKFILE, ">$mak_file") || die "Can't open $mak_file\n";
    print_all_target($fullpath, \%all_target_hash);

    # $extra_mak handles further *.mak files in a directory
    print_flags($dir,$extra_mak,'CFlags',$nonpro);    # depends on directory
    print_simple_flag("Rules");
    print_simple_flag("Link");
    print_common_linkflags();
#    print_flags($fullpath,$extra_mak,'LinkFlags'); # depends on directory
#    print_lib32_objs($fullpath,$extra_mak,\%collect_object_target_hash,$resource_file);
    print_flags($dir,$extra_mak,'LinkFlags'); # depends on directory
    print_lib32_objs($dir,$extra_mak,\%collect_object_target_hash,$resource_file);

    # write @mak array into the *.mak file
    foreach $line(@mak)
    {
        print MAKFILE $line;
    }
    $|=1;
    print "."; # user entertainment
    $|=0;
}
print "\ndone\n";
exit;

############################################################################
sub getKey      #01.04.2008 09:46
############################################################################
 {
    my $line = shift;
    $line =~ /\[(.+)\]/;
    return $1;
}   ##getKey

############################################################################
sub fillTemplateHash        #01.04.2008 10:48
############################################################################
 {
    my $file = shift;
    open (TEMPLATE, "< $file") || die "Can't open template file $file\n";
    my $key = "";
    while ( $line=<TEMPLATE> ) {
        if ( $line =~ /\[.+\]/ ) {
            print $line if ($is_debug);
            if ( $key ne "" ) {
                $template_hash{$key}=[@cmdlines];
                @cmdlines="";
                $key="";
            }
            $key = getKey( $line );
        } else
        {
            push @cmdlines, $line;
        }
    } # while
}   ##fillTemplateHash

############################################################################
sub createCopySection       #01.04.2008 11:37
############################################################################
 {
    my $header     = shift;
    my $inputpath  = shift;
    my $target     = shift;
    my $targetpath = shift;
    my $ref_make_file = shift; # Array written later to make file *.mak
    my $ref_all_target_hash = shift;  # reference to fill all_target_hash;
    if ( $target !~ /\\include/ && $target !~ /\\bin/) {
        return; # $target contains nonsense
    }
    if ( !$targetpath ) {
        # $targetpath is empty! (Due to missing entry in *.vcproj file)
        # Generate $targetpath here from $target
        my $t = $target;
        $t =~ /(.+)\\(.+)$/;
        $targetpath = "copy \"\$(InputPath)\" " . $1;
        chomp $targetpath;
    }
    $targetpath =~ s/\r$//; # remove x0A from EOL if the is one
    my @template = @{$template_hash{"Copy"}};
    my $line = "";
    foreach $line(@template)
    {
        $line =~ s/<HEADER>/$header/g;
        $line =~ s/<INPUTPATH>/$inputpath/g;
        $line =~ s/<TARGET>/$target/g;
        $line =~ s/<TARGETPATH>/$targetpath/;
        push @{$ref_make_file}, $line;           # from template
        $$ref_all_target_hash{$target} = 1;       # "\"$target\" ";
     # save for target ALL:
    }
}   ##createCopySection

############################################################################
sub createMakDepSection     #01.04.2008 13:36
############################################################################
 {
    # creates the dependency section in the make file
    my $dir         = shift;
    my $source      = shift;
    my $extension   = shift;
    my $ref_header_list = shift;
    my $ref_make_file = shift; # Array written later to make file *.mak
    my $ref_special_file = shift; # hash for special files (compiler flags, include paths)
    my $header_list = "";
    my $special_flag = 0;

    return if ( !defined $source );
    foreach $header(@{$ref_header_list})
    {
        if ( ($header =~ /^\.\.\\.+/) && (-e $header )) {
            $header_list = $header_list . " " . $header; # this header is located in an other directory
        } else
        {
            $header_list = $header_list . " .\\" . $header;
        }
    }

    #special handling
    # compile this file with other compiler switches
    my $file = $source . "\." . $extension;
    $dir =~ s/\\/_/;
    my @template = @{$template_hash{"CFlags_$dir"}};
    if ( defined $$ref_special_file{"AdditionalIncludeDirectories"}{$file} ) {
        $special_flag = 1;
        my $includepath = $$ref_special_file{"AdditionalIncludeDirectories"}{$file};
        $includepath =~ s/\;/\/I /g;                   # subst ; with /I for multiple paths
        $line = "CPP_SWITCH_INCLUDE=/I  $includepath\n";
        push @{$ref_make_file}, $line;
        foreach $line(@template)
        {
            if ( $line =~ /CPP_PROJ/)
            {
                $line =~ s/CPP_PROJ=/CPPX_PROJ=/;
                $line =~ s/\$\(CDEFS\)/\$\(CDEFS\) \$\(CPP_SWITCH_INCLUDE\)/; # include $(CPP_SWITCH_INCLUDE)
                push @{$ref_make_file}, $line;
            }
        }
    }
    if ( $$ref_special_file{"DisableLanguageExtensions"}{$file} )
    {
        # FALSE = /Ze
        $special_flag = 1;
        foreach $line(@template)
        {
            if ( $line =~ /CPP_PROJ/)
            {
                $line =~ s/CPP_PROJ=/CPPX_PROJ=/;
                $line =~ s/-Za/-Ze/;
                if ( $nonpro )
                {
                    # if non product link against debug libraries
                    $line =~ s/-MD/-MDd/;
                    $line =~ s/-MT/-MTd/;
                }
                push @{$ref_make_file}, $line;
            }
        }
    }

    @template = @{$template_hash{"Deps"}};
    my $line = "";
    foreach $line(@template)
    {
        $line =~ s/<SOURCEFILE>/$source/g;
        $line =~ s/<EXT>/$extension/g;
        $line =~ s/<HEADER_LIST>/$header_list/g;
        push @{$ref_make_file}, $line;
    }

    if ( $special_flag )
    {
        pop @{$ref_make_file}; # remove empty line
        push @{$ref_make_file},"\t\$(CPP) @<<\n";
        push @{$ref_make_file},"\t\$(CPPX_PROJ) \$(SOURCE)\n";
        push @{$ref_make_file},"<<\n\n";
        $special_flag = 0;
    }

}   ##createMakDepSection


############################################################################
sub getFilenameFromPath     #10.04.2008 13:03
############################################################################
{
    my $path = shift;
    $path =~ /.+\\(.+)$/;
    return $1;
}   ##getFilenameFromPath

############################################################################
sub getAllFilesDeps     #08.04.2008 09:39
############################################################################
{
    my $path = shift;
    my $ref_deps = shift;
    my $ref_allfiles = shift;       # contains all relevant files from *.vcproj
    my %local_header_hash=();       # contains all local header

    my @all_files = keys %{$ref_allfiles};

    # collect lokal header names in %header_hash
    foreach $file(@all_files)
    {
        if ( $file =~ /.+\.h/ ) {
            chomp $file;
            $local_header_hash{$file} = 1;
        }
    }

    foreach $file(@all_files)
    {
        my @header_deps=();
        my $skip = 0;

        $file =~ s/^\.\\//; # remove leading .\

        # exclude sub directories and several file extensions
        # *.dep,*.h,*.in,*.mak,*.pl,*.txt,*.vcproj.*.rc and origs of patch files (*.*~) and
        # .cvsignore (files beginning with .)
        next if ( (-d "$path$sep$file") || ($file =~ /.+(_|\~|dep|h|in|mak|pl|txt|vcproj|rc)$/) || ($file =~ /^\.+./));
        parse_header_deps($path,$file,\@header_deps,\%local_header_hash);
        handle_dep_to_other_directory($path,$file,\@header_deps,$$ref_vcproj{$file}) if ($$ref_vcproj{$file});
        $$ref_deps{$file}=[@header_deps];
    }
}   ##getAllFilesDeps

 ############################################################################
 sub parse_header_deps      #14.06.2006 18:04
 ############################################################################
  {
    # get includes from c/cpp file
    # call create_deps(path,file,\$link_obj)
    #
    my $path = shift;
    my $cfile = shift;
    my $ref_header_deps = shift;
    my $ref_local_header_hash = shift;

    my $fullpath = $path . $sep . $cfile;
    my $unicodedep="";
       open(IN, "<$fullpath") || die "can't open $fullpath\n";
    while ( $line = <IN> ) {
        next if ($line !~ /^.?\#include.+/); # handle include lines only
        if ($line =~ /.+?\s(.+)/)
        {
            $header = $1;
            if ( ($header !~ /^<.+/) && ($header !~ /.+\.c.+$/) ) {
                # no <stdio> etc. header
                $header =~ s/\s+\/\*.+\*\///; # delete <blanks>/* ... */
                $header =~ s/\s+\/\/.+//;      # delete <blanks>//......
                $header =~ s/\//\\/;           # subst. / with \
                $header =~ s/^\"/\".\\/;
                $header =~ s/\"//g;
                $header =~ s/\.\\//;
                my $test = $$ref_local_header_hash{$header};
                    my $header_fullpath = $path . "\\" . $header;
                if ( $test || (($header =~ /\\/) && (-e $header_fullpath))) {
                    push @{$ref_header_deps}, $header;
                }
            }
        }
    }
 }  ##parse_header_deps

############################################################################
sub handle_dep_to_other_directory       #16.04.2008 15:11
############################################################################
{
    # there has been an additional include directoy detected
    # now try to find out which header (parsed from c/cpp-file)
    # comes from this directory by checking: does it exist there?
    my $path = shift;
    my $file = shift;
    my $ref_header_deps = shift;
    my $path_additional = shift;
    my $search_path = $path . "\\" . $path_additional;
    my $counter = 0;
    foreach $header(@{$ref_header_deps})
    {
        my $full_path = $search_path . "\\" . $header;
        if ( -e "$full_path" )
        {
            $$ref_header_deps[$counter] = $path_additional . "\\" . $header;
        }
        $counter++
    }
}   ##handle_dep_to_other_directory

############################################################################
sub print_lib32_objs        #18.04.2008 12:54
############################################################################
 {
    # generate Link section
    my $path = shift;
    my $extra_mak = shift;
    my $ref_objecthash = shift;
    my $resource_file = shift;
    # output link objects
    print MAKFILE "LINK32_OBJS= \\\n";
    # print objects
    foreach $object(sort keys %{$ref_objecthash})
    {
        print MAKFILE "\t$object \\\n";
    }

    # print *.res if *.rc exists
    if ( $resource_file ne "" ) {
        my $res_file = $resource_file;
        $res_file =~ s/\.rc/\.res/;
        $res_file =~ /(.+)\\(.+)$/;
        $res_file = $2;
        print MAKFILE "\t\$(INTDIR)\\$res_file \\\n";
    }

    # add import libs and res files
    print_flags($path,$extra_mak,"AdditionalLinkObjects");
    my $outfile = $configelements{"Release"}{"OutputFile"};

    # section for linking
    print_link_template($path,$outfile);

    # section for creating res files
    # setting for rsc, res target etc.
    print "resource_file=$resource_file\n" if ($is_debug);
    print_rsc_template($resource_file) if ($resource_file);
    print_simple_flag("Special_extra_uconv") if ($outfile =~ /uconv/);
}   ##print_lib32_objs

############################################################################
sub print_all_target        #26.06.2008 13:27
############################################################################
 {
    my $path = shift;
    my $ref_all_target_hash = shift;
    my $filename = getFilenameFromPath($path);
    my $outdir = $configelements{"Release"}{"OutputDirectory"};
    print MAKFILE "NULL=\n";
    print MAKFILE "OUTDIR=$outdir\n";
    print MAKFILE "OutDir=$outdir\n";
    print MAKFILE "INTDIR=.\\Release\n\n";
    print MAKFILE "ALL: ";
    foreach $target(sort keys %{$ref_all_target_hash})
    {
        if ( $target =~ /\.exe/ ) {
            my $out = $target;
            $out =~ s/.\\Release/\.\.\\\.\.\\\.\.\\bin/;
            $out =~ s/\$\(OutDir\)/\.\.\\\.\.\\\.\.\\bin/;
            $out =~ s/\//\\/; # convert / to \
            $target = $out;
        }
        print MAKFILE "\"$target\" ";
    }

    # Append [Target_<dir>] item e.g. ../../icuxy36.dll
    my $targetkey = "Target_" . $filename;
    my @target = ();
    if ( exists $template_hash{$targetkey}  ) {
        @target = @{$template_hash{$targetkey}};
    }
    my $additional_target="";
    foreach $additional_target(@target)
    {
        print MAKFILE $additional_target if ($additional_target ne "");
    }
    print MAKFILE "\n\n";
    print MAKFILE "\"\$(OUTDIR)\" : \n";
    print MAKFILE "\tif not exist \"\$(OUTDIR)/\$(NULL)\" mkdir \"\$(OUTDIR)\"\n\n";
    print MAKFILE "!IF \"\$(OUTDIR)\" != \"\$(INTDIR)\"\n";
    print MAKFILE "\"\$(INTDIR)\" : \n";
    print MAKFILE "\tif not exist \"\$(INTDIR)/\$(NULL)\" mkdir \"\$(INTDIR)\"\n";
    print MAKFILE "!ENDIF\n";
    print MAKFILE "\n\n";
}   ##print_all_target

############################################################################
sub print_simple_flag       #18.04.2008 13:39
############################################################################
{
    my $simple_flag = shift;
    my @template = @{$template_hash{$simple_flag}};
    foreach $line(@template)
    {
        print MAKFILE $line;
    }
}   ##print_rules

############################################################################
sub print_link_template       #18.04.2008 13:39
############################################################################
{
    my $dir = shift;
    my $outfile = shift;
    my $manifest;
    # set resource id for manifest file
    if ( $outfile =~ /\.exe/ ) {
        $manifest = 1;
    } else
    {
        $manifest = 2;
    }
    my @template = ();
    if ($dir =~ /stubdata/ ) {
        @template = @{$template_hash{"Special_stubdata"}};
    } else
    {
        @template = @{$template_hash{"LinkTemplate"}};
    }

    print MAKFILE "\n"; # insert blank line
    foreach $line(@template)
    {
        $line =~ s/<OUTFILE>/$outfile/;
        $line =~ s/<MANIFEST>/$manifest/;
        print MAKFILE $line;
    }

    # insert special stuff for
    # extras/uconv/uconv.mak
    if ( $dir =~ /uconv/ ) {
        print_flags($dir,"","Special");
    }

    # for *.exe files an additional
    # copy section is required to get
    # the stuff into the global bin directory
    my %dummy = ();
    my @mak = ();
    if( $manifest ==1 )
    {
        # source,inputpath,target,action
        my $out = $outfile;
        $out =~ s/.\\.*Release/\.\.\\\.\.\\\.\.\\bin/;
        $out =~ s/\$\(OutDir\)/\.\.\\\.\.\\\.\.\\bin/;
        $out =~ s/\//\\/;       # subst / with \
        $outfile =~ s/\//\\/;   # subst / with \
        createCopySection($outfile,$outfile,$out,"copy \"\$(InputPath)\" .\\..\\..\\..\\bin",\@mak,\%dummy);
        foreach $line(@mak)
        {
            print MAKFILE $line;
        }
    }
}   ##print_rules

############################################################################
sub print_rsc_template      #04.11.2008 14:48
############################################################################
 {
    # print resource compiler setting + resource target
    my $resourcefile = shift;
    # skip this if no res file required

    return if (!$resourcefile);
    $resfile = $resourcefile;
    #remove file extension (.res)
    $resfile =~ /(.+)\\(.+)\.(.+)/;
    $resfile = $2;

    my @template = @{$template_hash{"RSC_Template"}};
    print MAKFILE "\n"; # insert blank line
    foreach $line(@template)
    {
        $line =~ s/<FILE>/$resourcefile/;
        $line =~ s/<FILEOUT>/$resfile/;
        print MAKFILE $line;
    }
}   ##print_rsc_template

############################################################################
sub print_flags     #18.04.2008 14:19
############################################################################
{
    # CFlags, LinkFlags
    my $dir = shift;
    my $extra_mak = shift; # eg. derb.mak, stringperf.mak
    my $flag = shift;
    my $nonpro = shift;
    my @template = ();
    my $switch = "";
    $dir =~ s/\\/_/g if ($dir);            # change \ to _
    $switch = "$flag" . "_" . "$dir" if ($dir);
    handle_CFlags() if ($flag eq "CFlags");  # get and print Preprocessor defines
    $switch .= "\." . $extra_mak if ( $extra_mak ne "" ) ;
    if ( exists $template_hash{$switch} ) {
        @template = @{$template_hash{$switch}};
        foreach $line(@template)
        {
            if ( $nonpro )
            {
                # if non product link against debug libraries
                $line =~ s/-MD/-MDd/;
                $line =~ s/-MT/-MTd/;
            }
            print MAKFILE $line;
        }
    }
}   ##print_flags

############################################################################
sub handle_CFlags       #28.01.2009 11:20
############################################################################
 {

    my $ppdefs = $configelements{"Release"}{"PreprocessorDefinitions"};
    my $ppinc  = $configelements{"Release"}{"AdditionalIncludeDirectories"};
    my @template = @{$template_hash{"General_CFlags"}};
    $ppdefs =~ s/;/ -D/g; # subst ; with -D switch
    $ppdefs = "-D" . $ppdefs;
    $ppinc =~ s/(;|,)/ -I/g; # subst ; with -I switch
    $ppinc = "-I" . $ppinc;
    print "ppdefs=$ppdefs\n" if ($is_debug);
    print "ppinc =$ppinc\n" if ($is_debug);
    foreach $line(@template)
    {
        $line =~ s/<AddIncDirs>/$ppinc/;
        $line =~ s/<PreProcDefs>/$ppdefs/;
        print MAKFILE $line;
    }
}   ##handle_CFlags

############################################################################
sub print_common_linkflags      #21.11.2008 11:47
############################################################################
 {
    my @template = @{$template_hash{"CommonLinkFlags"}};
    my $outfile = $configelements{"Release"}{"OutputFile"};
    my $pdbfile = $configelements{"Release"}{"ProgramDatabaseFile"};
    $pdbfile =~ s/\//\\/;   # subst / with \
    $outfile =~ s/\//\\/;   # subst / with \
    print "PATH=$path OUTFILE=$outfile\n" if ($is_debug);
    foreach $line(@template)
    {
        $line =~ s/<OUTFILE>/$outfile/;
        $line =~ s/<PDBFILE>/$pdbfile/;
        print MAKFILE $line;
    }
}   ##print_common_linkflags

############################################################################
sub handle_extra_mak_files      #25.08.2008 14:32
############################################################################
{
    # extract extra filename for *.mak file
    # e.g input: tools\genrb.derb
    #    output: derb
    my $direntry = shift;
    my $out = "";
    my $dir = "";
    if ( $direntry =~ /(.+)\.(.+)$/ ) {
        $dir = $1;
        $out = $2;
    } else
    {
        $dir = $direntry;
    }
    return ($dir,$out);
}   ##handle_extra_mak_files

############################################################################
sub prepare_allinone_all_mak
############################################################################
{
    # Read in allinone.sln
    # Fills hashes and returns an array with build order
    # uses topographical sorting

    my $href_project_by_id = shift;
    my $href_project_by_name = shift;
    my $href_project_dependencies = shift;
    my $sourcePath = shift;

    my $allslnfile = $sourcePath . "\\allinone\\allinone.sln";
    my @projectdeps;
    my $projectname;
    my $projectpath;
    my $projectid;

    # fill hash tables
    open (SLN, "< $allslnfile") || die "Can't open $allslnfile\n";
    while ($line = <SLN>)
    {
        my @project = ();
        if ( $line =~ /^Project\(/ ) {
            @project = split( /,/, $line);
            if ( $#project ) {
                $projectname = "";
                $projectpath = "";
                $projectid = "";
                @projectdeps = ();
                my @subarray = ();
                @subarray = split( /=/, $project[0]);
                $projectname = $subarray[1];
                $projectname =~ s/\"//g;      # remove "
                $projectpath = $project[1];
                $projectid = $project[2];
                $projectid =~ s/\"//g;        # remove "
                $projectid =~ s/.+\{//g;      # remove til {
                $projectid =~ s/\}\n//g;      # remove }<CR>
                my @pnp = ($projectname,$projectpath);
                my @pip = ($projectid,$projectpath);
                $$href_project_by_id{$projectid}=[@pnp];
                $$href_project_by_name{$projectname} =[@pip];
            }
        } # $line =~ /^Project\(/
        if ( $line =~ /ProjectSection\(/ ) {
            $psect = 1;
            next;
        }
        if ( $line =~ /EndProjectSection/ ) {
            $psect = 0;
            $$href_project_dependencies{$projectid}=[@projectdeps];
            next;
        }
        if ( $psect ) {
            my @tarray = split(/=/, $line);
            $depends_on_id = $tarray[0];
            $depends_on_id =~ s/.+\{//g;
            $depends_on_id =~ s/\}.+//g;
             print "+$depends_on_id-\n" if ($is_debug);

            push @projectdeps, $depends_on_id;
        }
    }
    ########################################
    # sort here and generate build order
    ########################################
    $objektzahl=0;
    %hashindex=();

    foreach $projectid(keys %{$href_project_by_id})
    {
        if ( $$href_project_dependencies{$projectid} )
        {
            @deps = @{$$href_project_dependencies{$projectid}};
        } else
        {
            @deps = ();
        }
        for  $counter(0..$#deps)
        {
            $v = find_or_create_element($deps[$counter]);
            $n = find_or_create_element($projectid);
            push @{$nachfolgerliste[$v]},$n;
        }
    }

    for $n (0..$objektzahl-1)
    {
            $vorgaengerzahl[$n]=0;
    }
    for $v (0..$objektzahl-1)
    {
            for $n (@{$nachfolgerliste[$v]})
            {
                    ++$vorgaengerzahl[$n];
            }
    }

    @hilfsliste=();
    for $n (0..$objektzahl-1)
    {
        push(@hilfsliste,$n) if ($vorgaengerzahl[$n]==0)
    }

    $ausgabe=0;
    @builddep =();
    while (defined($v=pop(@hilfsliste)))
    {
        push @builddep, $name[$v];                           # save build order by project_id;
        ++$ausgabe;
        for $n (@{$nachfolgerliste[$v]})
        {
                --$vorgaengerzahl[$n];
                push(@hilfsliste,$n) if ($vorgaengerzahl[$n]==0);
        }
    } # while
    die "Cyclic dependencies found! Stopping now.\n" if $ausgabe<$objektzahl;
    ##############################################################
    # End of sorting stuff
    ##############################################################

    return @builddep;
    ###############################################################
    ###########################
    # sub for sorting only
    ###########################
    sub find_or_create_element
    {
        my ($str)=@_;
        my ($idx)=$hashindex{$str};
        if (!defined($idx)) {               # new element ...
                $idx=$objektzahl++;
                $hashindex{$str}=$idx;
                $name[$idx]=$str;
            @{$nachfolgerliste[$idx]}=();
        }
        return $idx;
    } # find_or_create_element
}  # prepare_allinone_all_mak

############################################################################
sub create_allinone_all_mak     #09.02.2009 09:22
############################################################################
{
    my $ref_buildorder = shift;
    my $href_project_by_id = shift;
    my $sourcePath = shift;
    my $allmakfile = $sourcePath . "\\allinone\\all.mak";
    open (ALLMAK, ">$allmakfile") || die "Can't write to $allmakfile \n";
    print ALLMAK "ALL: ";
    foreach $proj(@{$ref_buildorder})
    {
        print ALLMAK $$href_project_by_id{$proj}[0];
    }
    print ALLMAK "\n\n";

    foreach $proj( @{$ref_buildorder} )
    {
        print "$proj $$href_project_by_id{$proj}[0] $$href_project_by_id{$proj}[1]\n";
        my $prjdir = $$href_project_by_id{$proj}[1];
        $prjdir =~ /(.+)\\(.+)$/;
        $prjdir = $1;
        $prjname = $2;
        $prjdir =~ s/^.+\"//;
        $prjname =~ s/\"$//;
        $prjname =~ s/vcproj/mak/;
        $allinonehelpstring = $prjdir;
        $allinonehelpstring =~ s/^\.+\\//; # remove ..\
        my $backcount = "";
        while ($allinonehelpstring=~ /.+\\/g) # counts the occuring \
        {
            $backcount .= "..\\";
        }
        $allinonedir = $backcount . "..\\allinone";

        # write all.mak
        $$href_project_by_id{$proj}[0] =~ s/^\s+//;
        if ( $$href_project_by_id{$proj}[0] ne "makedata" )
        {
            my @template = @{$template_hash{"AllInOnePrj"}};
            foreach $line(@template)
            {
                $line =~ s/<PRJ>/$$href_project_by_id{$proj}[0]/;
                $line =~ s/<PRJDIR>/$prjdir/;
                $line =~ s/<PRJMAK>/$prjname/;
                $line =~ s/<ALLINONEDIR>/$allinonedir/;
                print ALLMAK $line;
            }
        } else
        {
            #special code snippet
            print ALLMAK "makedata : \n";
            print ALLMAK "     cd \"..\\data\"\n";
            print ALLMAK "     nmake /f makedata.mak icumake=\$(MAKEDIR)\\..\\data cfg=Release\n";
            print ALLMAK "     cd \"..\\allinone\"\n\n";
        }
    }
    close ALLMAK;
}   ##create_allinone_all_mak

############################################################################

# ------------------------------------------------------------------------
# XML parser handling
# ------------------------------------------------------------------------

############################################################################
sub start_handler
############################################################################
{
    my $p = shift;           # pointer to parser
    my $el = shift;          # element

    # Deal with attributes

    my $CompilerSection = 0;
    my $LinkerSection = 0;
    my $ConfigSection = ($el eq "Configuration");

    while (@_)
    {
 #       shift if ( $el eq "FileConfiguration" );
        my $att = shift;
        my $val = shift;
        $CustomSection = 0;
        if ($special_file && defined $att & $att ne "Name")
        {
            print "$special_file - $att - $val\n";
            my @param = ($att,$val);
            $files_special{ $special_file } = [@param]; # files with special compiler switch
            @test = @{$files_special{ $special_file }};
            print "test=@test\n";
            $special_file="";
        }
        if ( $ConfigSection && $att eq "Name" && $val eq "Release|Win32" ) {
            $Release = 1;
            $config = "Release";                                                             # Release
        }
        if ( $ConfigSection && $att eq "Name" && $val eq "Debug|Win32" ) {
            $Release = 0;                                                             # Debug
            $config = "Debug";
        }
        if ( $att eq "Name" && $val eq "VCCLCompilerTool" ) {
            $CompilerSection = 1;
        }
        if ( $att eq "Name" && $val eq "VCLinkerTool" ) {
            $LinkerSection = 1;
        }
        if ( $att eq "Name" && $val eq "VCCustomBuildTool" ) {
            $CustomSection = 1;
        }

        # For Configuration Infos like compiler defines etc.
        if ( $att eq "PreprocessorDefinitions" && $CompilerSection ) {
           $configelements{$config}{$att} = $val;
        }
        if ( $att eq "AdditionalIncludeDirectories" && $CompilerSection ) {
           #$configelements{$config}{$att} = $val;
            if ( ($file_tmp ne "") && ($val ne "") ) {
                $files{ $file_tmp } = 1;        # save it now
                $file_tmp =~ s/^\.\\//;         # remove leading .\
                $files_special{"AdditionalIncludeDirectories"}{$file_tmp} = $val;
                print "Include $val: $file_tmp\n" if ($is_debug);
                $file_tmp = "";                 # has been saved now reset it
            } else
            {
               $configelements{$config}{$att} = $val;
            }
        }
        if ( ($att eq "DisableLanguageExtensions") && $CompilerSection ) {
           #$configelements{$config}{$att} = $val;
            if ( ($file_tmp ne "") && ($val ne "")) {
                $files{ $file_tmp } = 1;        # save it now
                $file_tmp =~ s/^\.\\//;         # remove leading .\
                $files_special{"DisableLanguageExtensions"}{$file_tmp} = $val;
                print "-Ze: $file_tmp\n" if ($is_debug);
                $file_tmp = "";                 # has been saved now reset it
            }
        }
        if ( $att eq "OutputDirectory" ) {
           $configelements{$config}{$att} = $val;
        }
        if ( $att eq "OutputFile" && $LinkerSection ) {
           $configelements{$config}{$att} = $val;
        }
        if ( $att eq "ProgramDatabaseFile" ) {
           $configelements{$config}{$att} = $val;
        }
        if ( $att eq "ImportLibrary" && $LinkerSection ) {
           $configelements{$config}{$att} = $val;
        }
        if ($att eq "CommandLine") {
           $configelements{$config}{$att} = $val;
        }
        if (($att eq "PreprocessorDefinitions") && $ConfigSection) {
           $configelements{$config}{$att} = $val;
        }

        # Is the file in the step before a header
        # which has to be copied into the global
        # include path?
        if ( $file_tmp ne "" )
        {
            $config = "Release";
            if ( ($att eq "CommandLine") && ($el eq "Tool") )
            {
                if ( $file_tmp =~ /.+\.h$/ ) {
                    $files2copy{ $file_tmp } = $val; # unicode + layout header to copy
                    $file_tmp = "";                 # has been saved now reset it
                }
            }
        } # if $file_tmp

        # For files
        if ( $att eq "RelativePath" ) {
            if ( $file_tmp ) {
                # no special file (include dir / compiler switch)
                $files{ $file_tmp } = 1;        # save it now
                $file_tmp = "";                 # has been saved now reset it
            }
            # store temporary the file name
            $file_tmp = $val if ($val !~ /\.mk$/);  # no *.mk files
        }
    } # while @_
}  # End start_handler

############################################################################
sub char_handler
############################################################################
{
}  # End char_handler
