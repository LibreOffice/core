#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

package installer::windows::file;

use Digest::MD5;
use installer::exiter;
use installer::files;
use installer::globals;
use installer::logger;
use installer::pathanalyzer;
use installer::worker;
use installer::windows::font;
use installer::windows::idtglobal;
use installer::windows::msiglobal;
use installer::windows::language;
use installer::windows::component;

##########################################################################
# Assigning one cabinet file to each file. This is requrired,
# if cabinet files shall be equivalent to packages.
##########################################################################

sub assign_cab_to_files
{
    my ( $filesref ) = @_;

    my $infoline = "";

    for ( my $i = 0; $i <= $#{$filesref}; $i++ )
    {
        if ( ! exists(${$filesref}[$i]->{'modules'}) ) { installer::exiter::exit_program("ERROR: No module assignment found for ${$filesref}[$i]->{'gid'} !", "assign_cab_to_files"); }
        my $module = ${$filesref}[$i]->{'modules'};
        # If modules contains a list of modules, only taking the first one.
        if ( $module =~ /^\s*(.*?)\,/ ) { $module = $1; }

        if ( ! exists($installer::globals::allcabinetassigns{$module}) ) { installer::exiter::exit_program("ERROR: No cabinet file assigned to module \"$module\" (${$filesref}[$i]->{'gid'}) !", "assign_cab_to_files"); }
        ${$filesref}[$i]->{'assignedcabinetfile'} = $installer::globals::allcabinetassigns{$module};

        # Counting the files in each cabinet file
        if ( ! exists($installer::globals::cabfilecounter{${$filesref}[$i]->{'assignedcabinetfile'}}) )
        {
            $installer::globals::cabfilecounter{${$filesref}[$i]->{'assignedcabinetfile'}} = 1;
        }
        else
        {
            $installer::globals::cabfilecounter{${$filesref}[$i]->{'assignedcabinetfile'}}++;
        }
    }

    # logging the number of files in each cabinet file

    $infoline = "\nCabinet file content:\n";
    push(@installer::globals::logfileinfo, $infoline);
    my $cabfile;
    foreach $cabfile ( sort keys %installer::globals::cabfilecounter )
    {
        $infoline = "$cabfile : $installer::globals::cabfilecounter{$cabfile} files\n";
        push(@installer::globals::logfileinfo, $infoline);
    }

    # assigning startsequencenumbers for each cab file

    my $offset = 1;
    foreach $cabfile ( sort keys %installer::globals::cabfilecounter )
    {
        my $filecount = $installer::globals::cabfilecounter{$cabfile};
        $installer::globals::cabfilecounter{$cabfile} = $offset;
        $offset = $offset + $filecount;

        $installer::globals::lastsequence{$cabfile} = $offset - 1;
    }

    # logging the start sequence numbers

    $infoline = "\nCabinet file start sequences:\n";
    push(@installer::globals::logfileinfo, $infoline);
    foreach $cabfile ( sort keys %installer::globals::cabfilecounter )
    {
        $infoline = "$cabfile : $installer::globals::cabfilecounter{$cabfile}\n";
        push(@installer::globals::logfileinfo, $infoline);
    }

    # logging the last sequence numbers

    $infoline = "\nCabinet file last sequences:\n";
    push(@installer::globals::logfileinfo, $infoline);
    foreach $cabfile ( sort keys %installer::globals::lastsequence )
    {
        $infoline = "$cabfile : $installer::globals::lastsequence{$cabfile}\n";
        push(@installer::globals::logfileinfo, $infoline);
    }
}

##########################################################################
# Assigning sequencenumbers to files. This is requrired,
# if cabinet files shall be equivalent to packages.
##########################################################################

sub assign_sequencenumbers_to_files
{
    my ( $filesref ) = @_;

    my %directaccess = ();
    my %allassigns = ();

    for ( my $i = 0; $i <= $#{$filesref}; $i++ )
    {
        my $onefile = ${$filesref}[$i];

        # Keeping order in cabinet files
        # -> collecting all files in one cabinet file
        # -> sorting files and assigning numbers

        # Saving counter $i for direct access into files array
        # "destination" of the file is a unique identifier ('Name' is not unique!)
        if ( exists($directaccess{$onefile->{'destination'}}) ) { installer::exiter::exit_program("ERROR: 'destination' at file not unique: $onefile->{'destination'}", "assign_sequencenumbers_to_files"); }
        $directaccess{$onefile->{'destination'}} = $i;

        my $cabfilename = $onefile->{'assignedcabinetfile'};
        # collecting files in cabinet files
        if ( ! exists($allassigns{$cabfilename}) )
        {
            my %onecabfile = ();
            $onecabfile{$onefile->{'destination'}} = 1;
            $allassigns{$cabfilename} = \%onecabfile;
        }
        else
        {
            $allassigns{$cabfilename}->{$onefile->{'destination'}} = 1;
        }
    }

    # Sorting each hash and assigning numbers
    # The destination of the file determines the sort order, not the filename!
    my $cabfile;
    foreach $cabfile ( sort keys %allassigns )
    {
        my $counter = $installer::globals::cabfilecounter{$cabfile};
        my $dest;
        foreach $dest ( sort keys %{$allassigns{$cabfile}} ) # <- sorting the destination!
        {
            my $directaccessnumber = $directaccess{$dest};
            ${$filesref}[$directaccessnumber]->{'assignedsequencenumber'} = $counter;
            $counter++;
        }
    }
}

#########################################################
# Create a shorter version of a long component name,
# because maximum length in msi database is 72.
# Attention: In multi msi installation sets, the short
# names have to be unique over all packages, because
# this string is used to create the globally unique id
# -> no resetting of
# %installer::globals::allshortcomponents
# after a package was created.
# Using no counter because of reproducibility.
#########################################################

sub generate_new_short_componentname
{
    my ($componentname) = @_;

    my $startversion = substr($componentname, 0, 60); # taking only the first 60 characters
    my $subid = installer::windows::msiglobal::calculate_id($componentname, 9); # taking only the first 9 digits
    my $shortcomponentname = $startversion . "_" . $subid;

    if ( exists($installer::globals::allshortcomponents{$shortcomponentname}) ) { installer::exiter::exit_program("Failed to create unique component name: \"$shortcomponentname\"", "generate_new_short_componentname"); }

    $installer::globals::allshortcomponents{$shortcomponentname} = 1;

    return $shortcomponentname;
}

###############################################
# Generating the component name from a file
###############################################

sub get_file_component_name
{
    my ($fileref, $filesref) = @_;

    my $componentname = "";

    # Special handling for files with ASSIGNCOMPOMENT

    my $styles = "";
    if ( $fileref->{'Styles'} ) { $styles = $fileref->{'Styles'}; }
    if ( $styles =~ /\bASSIGNCOMPOMENT\b/ )
    {
        $componentname = get_component_from_assigned_file($fileref->{'AssignComponent'}, $filesref);
    }
    else
    {
        # In this function exists the rule to create components from files
        # Rule:
        # Two files get the same componentid, if:
        # both have the same destination directory.
        # both have the same "gid" -> both were packed in the same zip file
        # All other files are included into different components!

        # my $componentname = $fileref->{'gid'} . "_" . $fileref->{'Dir'};

        # $fileref->{'Dir'} is not sufficient! All files in a zip file have the same $fileref->{'Dir'},
        # but can be in different subdirectories.
        # Solution: destination=share\Scripts\beanshell\Capitalise\capitalise.bsh
        # in which the filename (capitalise.bsh) has to be removed and all backslashes (slashes) are
        # converted into underline.

        my $destination = $fileref->{'destination'};
        installer::pathanalyzer::get_path_from_fullqualifiedname(\$destination);
        $destination =~ s/\s//g;
        $destination =~ s/\\/\_/g;
        $destination =~ s/\//\_/g;
        $destination =~ s/\_\s*$//g;    # removing ending underline

        $componentname = $fileref->{'gid'} . "__" . $destination;

        # Files with different languages, need to be packed into different components.
        # Then the installation of the language specific component is determined by a language condition.

        if ( $fileref->{'ismultilingual'} )
        {
            my $officelanguage = $fileref->{'specificlanguage'};
            $componentname = $componentname . "_" . $officelanguage;
        }

        $componentname = lc($componentname);    # componentnames always lowercase

        $componentname =~ s/\-/\_/g;            # converting "-" to "_"
        $componentname =~ s/\./\_/g;            # converting "-" to "_"

        # Attention: Maximum length for the componentname is 72
        # %installer::globals::allcomponents_in_this_database : resetted for each database
        # %installer::globals::allcomponents : not resetted for each database
        # Component strings must be unique for the complete product, because they are used for
        # the creation of the globally unique identifier.

        my $fullname = $componentname;  # This can be longer than 72

        if (( exists($installer::globals::allcomponents{$fullname}) ) && ( ! exists($installer::globals::allcomponents_in_this_database{$fullname}) ))
        {
            # This is not allowed: One component cannot be installed with different packages.
            installer::exiter::exit_program("ERROR: Component \"$fullname\" is already included into another package. This is not allowed.", "get_file_component_name");
        }

        if ( exists($installer::globals::allcomponents{$fullname}) )
        {
            $componentname = $installer::globals::allcomponents{$fullname};
        }
        else
        {
            if ( length($componentname) > 70 )
            {
                $componentname = generate_new_short_componentname($componentname); # This has to be unique for the complete product, not only one package
            }

            $installer::globals::allcomponents{$fullname} = $componentname;
            $installer::globals::allcomponents_in_this_database{$fullname} = 1;
        }

        # $componentname =~ s/gid_file_/g_f_/g;
        # $componentname =~ s/_extra_/_e_/g;
        # $componentname =~ s/_config_/_c_/g;
        # $componentname =~ s/_org_openoffice_/_o_o_/g;
        # $componentname =~ s/_program_/_p_/g;
        # $componentname =~ s/_typedetection_/_td_/g;
        # $componentname =~ s/_linguistic_/_l_/g;
        # $componentname =~ s/_module_/_m_/g;
        # $componentname =~ s/_optional_/_opt_/g;
        # $componentname =~ s/_packages/_pack/g;
        # $componentname =~ s/_menubar/_mb/g;
        # $componentname =~ s/_common_/_cm_/g;
        # $componentname =~ s/_export_/_exp_/g;
        # $componentname =~ s/_table_/_tb_/g;
        # $componentname =~ s/_sofficecfg_/_sc_/g;
        # $componentname =~ s/_soffice_cfg_/_sc_/g;
        # $componentname =~ s/_startmodulecommands_/_smc_/g;
        # $componentname =~ s/_drawimpresscommands_/_dic_/g;
        # $componentname =~ s/_basiccommands_/_bac_/g;
        # $componentname =~ s/_basicidecommands_/_baic_/g;
        # $componentname =~ s/_genericcommands_/_genc_/g;
        # $componentname =~ s/_bibliographycommands_/_bibc_/g;
        # $componentname =~ s/_gentiumbookbasicbolditalic_/_gbbbi_/g;
        # $componentname =~ s/_share_/_s_/g;
        # $componentname =~ s/_extension_/_ext_/g;
        # $componentname =~ s/_extensions_/_exs_/g;
        # $componentname =~ s/_modules_/_ms_/g;
        # $componentname =~ s/_uiconfig_zip_/_ucz_/g;
        # $componentname =~ s/_productivity_/_pr_/g;
        # $componentname =~ s/_wizard_/_wz_/g;
        # $componentname =~ s/_import_/_im_/g;
        # $componentname =~ s/_javascript_/_js_/g;
        # $componentname =~ s/_template_/_tpl_/g;
        # $componentname =~ s/_tplwizletter_/_twl_/g;
        # $componentname =~ s/_beanshell_/_bs_/g;
        # $componentname =~ s/_presentation_/_bs_/g;
        # $componentname =~ s/_columns_/_cls_/g;
        # $componentname =~ s/_python_/_py_/g;

        # $componentname =~ s/_tools/_ts/g;
        # $componentname =~ s/_transitions/_trs/g;
        # $componentname =~ s/_scriptbinding/_scrb/g;
        # $componentname =~ s/_spreadsheet/_ssh/g;
        # $componentname =~ s/_publisher/_pub/g;
        # $componentname =~ s/_presenter/_pre/g;
        # $componentname =~ s/_registry/_reg/g;

        # $componentname =~ s/screen/sc/g;
        # $componentname =~ s/wordml/wm/g;
        # $componentname =~ s/openoffice/oo/g;
    }

    return $componentname;
}

####################################################################
# Returning the component name for a defined file gid.
# This is necessary for files with flag ASSIGNCOMPOMENT
####################################################################

sub get_component_from_assigned_file
{
    my ($gid, $filesref) = @_;

    my ($onefile) = grep {$_->{gid} eq $gid} @{$filesref};
    if (! defined $onefile) {
        installer::exiter::exit_program("ERROR: Could not find file $gid in list of files!", "get_component_from_assigned_file");
    }

    my $componentname = "";
    if ( $onefile->{'componentname'} ) { $componentname = $onefile->{'componentname'}; }
    else { installer::exiter::exit_program("ERROR: No component defined for file: $gid", "get_component_from_assigned_file"); }

    return $componentname;
}

####################################################################
# Generating the special filename for the database file File.idt
# Sample: CONTEXTS, CONTEXTS1
# This name has to be unique.
# In most cases this is simply the filename.
####################################################################

sub generate_unique_filename_for_filetable
{
    my ($fileref, $component, $uniquefilenamehashref) = @_;

    # This new filename has to be saved into $fileref, because this is needed to find the source.
    # The filename sbasic.idx/OFFSETS is changed to OFFSETS, but OFFSETS is not unique.
    # In this procedure names like OFFSETS5 are produced. And exactly this string has to be added to
    # the array of all files.

    my $uniquefilename = "";
    my $counter = 0;

    if ( $fileref->{'Name'} ) { $uniquefilename = $fileref->{'Name'}; }

    installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$uniquefilename); # making /registry/schema/org/openoffice/VCL.xcs to VCL.xcs

    # Reading unique filename with help of "Component_" in File table from old database
    if (( $installer::globals::updatedatabase ) && ( exists($uniquefilenamehashref->{"$component/$uniquefilename"}) ))
    {
        $uniquefilename = $uniquefilenamehashref->{"$component/$uniquefilename"};  # syntax of $value: ($uniquename;$shortname)
        if ( $uniquefilename =~ /^\s*(.*?)\;\s*(.*?)\s*$/ ) { $uniquefilename = $1; }
         $lcuniquefilename = lc($uniquefilename);
        $installer::globals::alluniquefilenames{$uniquefilename} = 1;
        $installer::globals::alllcuniquefilenames{$lcuniquefilename} = 1;
        return $uniquefilename;
    }
    elsif (( $installer::globals::prepare_winpatch ) && ( exists($installer::globals::savedmapping{"$component/$uniquefilename"}) ))
    {
        # If we have a FTK mapping for this component/file, use it.
        $installer::globals::savedmapping{"$component/$uniquefilename"} =~ m/^(.*);/;
        $uniquefilename = $1;
         $lcuniquefilename = lc($uniquefilename);
        $installer::globals::alluniquefilenames{$uniquefilename} = 1;
        $installer::globals::alllcuniquefilenames{$lcuniquefilename} = 1;
        return $uniquefilename;
    }

    $uniquefilename =~ s/\-/\_/g;       # no "-" allowed
    $uniquefilename =~ s/\@/\_/g;       # no "@" allowed
    $uniquefilename =~ s/\$/\_/g;       # no "$" allowed
    $uniquefilename =~ s/^\s*\./\_/g;       # no "." at the beginning allowed allowed
    $uniquefilename =~ s/^\s*\d/\_d/g;      # no number at the beginning allowed allowed (even file "0.gif", replacing to "_d.gif")
    $uniquefilename =~ s/org_openoffice_/ooo_/g;    # shorten the unique file name

    my $lcuniquefilename = lc($uniquefilename); # only lowercase names

    my $newname = 0;

    if ( ! exists($installer::globals::alllcuniquefilenames{$lcuniquefilename}) &&
         ! exists($installer::globals::savedrevmapping{$lcuniquefilename}) )
    {
        $installer::globals::alluniquefilenames{$uniquefilename} = 1;
        $installer::globals::alllcuniquefilenames{$lcuniquefilename} = 1;
        $newname = 1;
    }

    if ( ! $newname )
    {
        # adding a number until the name is really unique: OFFSETS, OFFSETS1, OFFSETS2, ...
        # But attention: Making "abc.xcu" to "abc1.xcu"

        my $uniquefilenamebase = $uniquefilename;

        do
        {
            $counter++;

            if ( $uniquefilenamebase =~ /\./ )
            {
                $uniquefilename = $uniquefilenamebase;
                $uniquefilename =~ s/\./$counter\./;
            }
            else
            {
                $uniquefilename = $uniquefilenamebase . $counter;
            }

            $newname = 0;
            $lcuniquefilename = lc($uniquefilename);    # only lowercase names

            if ( ! exists($installer::globals::alllcuniquefilenames{$lcuniquefilename}) &&
                 ! exists($installer::globals::savedrevmapping{$lcuniquefilename}) )
            {
                $installer::globals::alluniquefilenames{$uniquefilename} = 1;
                $installer::globals::alllcuniquefilenames{$lcuniquefilename} = 1;
                $newname = 1;
            }
        }
        until ( $newname )
    }

    return $uniquefilename;
}

####################################################################
# Generating the special file column for the database file File.idt
# Sample: NAMETR~1.TAB|.nametranslation.table
# The first part has to be 8.3 conform.
####################################################################

sub generate_filename_for_filetable
{
    my ($fileref, $shortnamesref, $uniquefilenamehashref) = @_;

    my $returnstring = "";

    my $filename = $fileref->{'Name'};

    installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$filename);   # making /registry/schema/org/openoffice/VCL.xcs to VCL.xcs

    my $shortstring;

    # Reading short string with help of "FileName" in File table from old database
    if (( $installer::globals::updatedatabase ) && ( exists($uniquefilenamehashref->{"$fileref->{'componentname'}/$filename"}) ))
    {
        my $value = $uniquefilenamehashref->{"$fileref->{'componentname'}/$filename"};  # syntax of $value: ($uniquename;$shortname)
        if ( $value =~ /^\s*(.*?)\;\s*(.*?)\s*$/ ) { $shortstring = $2; } # already collected in function "collect_shortnames_from_old_database"
        else { $shortstring = $filename; }
    }
    elsif (( $installer::globals::prepare_winpatch ) && ( exists($installer::globals::savedmapping{"$fileref->{'componentname'}/$filename"}) ))
    {
        $installer::globals::savedmapping{"$fileref->{'componentname'}/$filename"} =~ m/.*;(.*)/;
        if ($1 ne '')
        {
            $shortstring = $1;
        }
        else
        {
            $shortstring = installer::windows::idtglobal::make_eight_three_conform_with_hash($filename, "file", $shortnamesref);
        }
    }
    else
    {
        $shortstring = installer::windows::idtglobal::make_eight_three_conform_with_hash($filename, "file", $shortnamesref);
    }

    if ( $shortstring eq $filename ) { $returnstring = $filename; } # nothing changed
    else {$returnstring = $shortstring . "\|" . $filename; }

    return $returnstring;
}

#########################################
# Returning the filesize of a file
#########################################

sub get_filesize
{
    my ($fileref) = @_;

    my $file = $fileref->{'sourcepath'};

    my $filesize;

    if ( -f $file ) # test of existence. For instance services.rdb does not always exist
    {
        $filesize = ( -s $file );   # file size can be "0"
    }
    else
    {
        $filesize = -1;
    }

    return $filesize;
}

#############################################
# Returning the file version, if required
# Sample: "8.0.1.8976";
#############################################

sub get_fileversion
{
    my ($onefile, $allvariables, $styles) = @_;

    my $fileversion = "";

    if ( $onefile->{'Name'} =~ /\.bin$|\.com$|\.dll$|\.exe$|\.pyd$/ )
    {
        open (EXE, "<$onefile->{'sourcepath'}");
        binmode EXE;
        {local $/ = undef; $exedata = <EXE>;}
        close EXE;

        my $binaryfileversion = "(V\x00S\x00_\x00V\x00E\x00R\x00S\x00I\x00O\x00N\x00_\x00I\x00N\x00F\x00O\x00\x00\x00\x00\x00\xbd\x04\xef\xfe\x00\x00\x01\x00)(........)";

        if ($exedata =~ /$binaryfileversion/ms)
        {
            my ($header, $subversion, $version, $vervariant, $microversion) = ($1,unpack( "vvvv", $2));
            $fileversion = $version . "." . $subversion . "." . $microversion . "." . $vervariant;
        }
    }

    return $fileversion;
}

#############################################
# Returning the sequence for a file
#############################################

sub get_sequence_for_file
{
    my ($number, $onefile, $fileentry, $allupdatesequenceshashref, $allupdatecomponentshashref, $allupdatefileorderhashref, $allfilecomponents) = @_;

    my $sequence = "";
    my $infoline = "";
    my $pffcomponentname = $onefile->{'componentname'} . "_pff";

    if ( $installer::globals::updatedatabase )
    {
        if (( exists($allupdatesequenceshashref->{$onefile->{'uniquename'}}) ) &&
            (( $onefile->{'componentname'} eq $allupdatecomponentshashref->{$onefile->{'uniquename'}} ) ||
             ( $pffcomponentname eq $allupdatecomponentshashref->{$onefile->{'uniquename'}} )))
        {
            # The second condition is necessary to find shifted files, that have same "uniquename", but are now
            # located in another directory. This can be seen at the component name.
            $sequence = $allupdatesequenceshashref->{$onefile->{'uniquename'}};
            $onefile->{'assignedsequencenumber'} = $sequence;
            # Collecting all used sequences, to guarantee, that no number is unused
            $installer::globals::allusedupdatesequences{$sequence} = 1;
            # Special help for files, that already have a "pff" component name (for example after ServicePack 1)
            if ( $pffcomponentname eq $allupdatecomponentshashref->{$onefile->{'uniquename'}} )
            {
                $infoline = "Warning: Special handling for component \"$pffcomponentname\". This file was added after the final, but before this ServicePack.\n";
                push(@installer::globals::logfileinfo, $infoline);
                $onefile->{'componentname'} = $pffcomponentname; # pff for "post final file"
                $fileentry->{'Component_'} = $onefile->{'componentname'};
                if ( ! exists($allfilecomponents->{$fileentry->{'Component_'}}) ) { $allfilecomponents->{$fileentry->{'Component_'}} = 1; }
            }
        }
        else
        {
            $installer::globals::updatesequencecounter++;
            $sequence = $installer::globals::updatesequencecounter;
            $onefile->{'assignedsequencenumber'} = $sequence;
            # $onefile->{'assignedcabinetfile'} = $installer::globals::pffcabfilename; # assigning to cabinet file for "post final files"
            # Collecting all new files
            $installer::globals::newupdatefiles{$sequence} = $onefile;
            # Saving in sequence hash
            $allupdatefileorderhashref->{$sequence} = $onefile->{'uniquename'};

            # If the new file is part of an existing component, this must be changed now. All files
            # of one component have to be included in one cabinet file. But because the order must
            # not change, all new files have to be added to new components.
            # $onefile->{'componentname'} = $file{'Component_'};

            $onefile->{'componentname'} = $onefile->{'componentname'} . "_pff"; # pff for "post final file"
            $fileentry->{'Component_'} = $onefile->{'componentname'};
            if ( ! exists($allfilecomponents->{$fileentry->{'Component_'}}) ) { $allfilecomponents->{$fileentry->{'Component_'}} = 1; }
            $onefile->{'PostFinalFile'} = 1;
            # $installer::globals::pfffileexists = 1;
            # The sequence for this file has changed. It has to be inserted at the end of the files collector.
            $installer::globals::insert_file_at_end = 1;
            $installer::globals::newfilescollector{$sequence} = $onefile; # Adding new files to the end of the filescollector
            $installer::globals::newfilesexist = 1;
        }
    }
    else
    {
        $sequence = $number;
        # my $sequence = $number + 1;

        # Idea: Each component is packed into a cab file.
        # This requires that all files in one cab file have sequences directly follwing each other,
        # for instance from 1456 to 1466. Then in the media table the LastSequence for this cab file
        # is 1466.
        # Because all files belonging to one component are directly behind each other in the file
        # collector, it is possible to use simply an increasing number as sequence value.
        # If files belonging to one component are not directly behind each other in the files collector
        # this mechanism will no longer work.
    }

    return $sequence;
}

#############################################
# Returning the Windows language of a file
#############################################

sub get_language_for_file
{
    my ($fileref) = @_;

    my $language = "";

    if ( $fileref->{'specificlanguage'} ) { $language = $fileref->{'specificlanguage'}; }

    if ( $language eq "" )
    {
        $language = 0;  # language independent
        # If this is not a font, the return value should be "0" (Check ICE 60)
        my $styles = "";
        if ( $fileref->{'Styles'} ) { $styles = $fileref->{'Styles'}; }
        if ( $styles =~ /\bFONT\b/ ) { $language = ""; }
    }
    else
    {
        $language = installer::windows::language::get_windows_language($language);
    }

    return $language;
}

####################################################################
# Check, if in an update process files are missing. No removal
# of files allowed for Windows Patch creation.
# Also logging all new files, that have to be included in extra
# components and cab files.
####################################################################

sub check_file_sequences
{
    my ($allupdatefileorderhashref, $allupdatecomponentorderhashref) = @_;

    # All used sequences stored in %installer::globals::allusedupdatesequences
    # Maximum sequence number of old database stored in $installer::globals::updatelastsequence
    # All new files stored in %installer::globals::newupdatefiles

    my $infoline = "";

    my @missing_sequences = ();
    my @really_missing_sequences = ();

    for ( my $i = 1; $i <= $installer::globals::updatelastsequence; $i++ )
    {
        if ( ! exists($installer::globals::allusedupdatesequences{$i}) ) { push(@missing_sequences, $i); }
    }

    if ( $#missing_sequences > -1 )
    {
        # Missing sequences can also be caused by files included in merge modules. This files are added later into the file table.
        # Therefore now it is time to check the content of the merge modules.

        for ( my $j = 0; $j <= $#missing_sequences; $j++ )
        {
            my $filename = $allupdatefileorderhashref->{$missing_sequences[$j]};

            # Is this a file from a merge module? Then this is no error.
            if ( ! exists($installer::globals::mergemodulefiles{$filename}) )
            {
                push(@really_missing_sequences, $missing_sequences[$j]);
            }
        }
    }

    if ( $#really_missing_sequences > -1 )
    {
        my $errorstring = "";
        for ( my $j = 0; $j <= $#really_missing_sequences; $j++ )
        {
            my $filename = $allupdatefileorderhashref->{$really_missing_sequences[$j]};
            my $comp = $allupdatecomponentorderhashref->{$really_missing_sequences[$j]};
            $errorstring = "$errorstring$filename (Sequence: $really_missing_sequences[$j], Component: \"$comp\")\n";
        }

        $infoline = "ERROR: Files are removed compared with update database.\nThe following files are missing:\n$errorstring";
        push(@installer::globals::logfileinfo, $infoline);
        installer::exiter::exit_program($infoline, "check_file_sequences");
    }

    # Searching for new files

    my $counter = 0;

    foreach my $key ( keys %installer::globals::newupdatefiles )
    {
        my $onefile = $installer::globals::newupdatefiles{$key};
        $counter++;
        if ( $counter == 1 )
        {
            $infoline = "\nNew files compared to the update database:\n";
            push(@installer::globals::logfileinfo, $infoline);
        }

        $infoline = "$onefile->{'Name'} ($onefile->{'gid'}) Sequence: $onefile->{'assignedsequencenumber'}\n";
        push(@installer::globals::logfileinfo, $infoline);
    }

    if ( $counter == 0 )
    {
        $infoline = "Info: No new file compared with update database!\n";
        push(@installer::globals::logfileinfo, $infoline);
    }

}

###################################################################
# Collecting further conditions for the component table.
# This is used by multilayer products, to enable installation
# of separate layers.
###################################################################

sub get_tree_condition_for_component
{
    my ($onefile, $componentname) = @_;

    if ( $onefile->{'destination'} )
    {
        my $dest = $onefile->{'destination'};

        # Comparing the destination path with
        # $installer::globals::hostnametreestyles{$hostname} = $treestyle;
        # (-> hostname is the key, the style the value!)

        foreach my $hostname ( keys %installer::globals::hostnametreestyles )
        {
            if (( $dest eq $hostname ) || ( $dest =~ /^\s*\Q$hostname\E\\/ ))
            {
                # the value is the style
                my $style = $installer::globals::hostnametreestyles{$hostname};
                # the condition is saved in %installer::globals::treestyles
                my $condition = $installer::globals::treestyles{$style};
                # Saving condition to be added in table Property
                $installer::globals::usedtreeconditions{$condition} = 1;
                $condition = $condition . "=1";
                # saving this condition
                $installer::globals::treeconditions{$componentname} = $condition;

                # saving also at the file, for usage in fileinfo
                $onefile->{'layer'} = $installer::globals::treelayername{$style};
            }
        }
    }
}

############################################
# Collecting all short names, that are
# already used by the old database
############################################

sub collect_shortnames_from_old_database
{
    my ($uniquefilenamehashref, $shortnameshashref) = @_;

    foreach my $key ( keys %{$uniquefilenamehashref} )
    {
        my $value = $uniquefilenamehashref->{$key};  # syntax of $value: ($uniquename;$shortname)

        if ( $value =~ /^\s*(.*?)\;\s*(.*?)\s*$/ )
        {
            my $shortstring = $2;
            $shortnameshashref->{$shortstring} = 1; # adding the shortname to the array of all shortnames
        }
    }
}

############################################
# Creating the file File.idt dynamically
############################################

sub create_files_table
{
    my ($filesref, $dirref, $allfilecomponentsref, $basedir, $allvariables, $uniquefilenamehashref, $allupdatesequenceshashref, $allupdatecomponentshashref, $allupdatefileorderhashref) = @_;

    installer::logger::include_timestamp_into_logfile("Performance Info: File Table start");

    # Structure of the files table:
    # File Component_ FileName FileSize Version Language Attributes Sequence
    # In this function, all components are created.
    #
    # $allfilecomponentsref is empty at the beginning

    my $infoline;

    my @allfiles = ();
    my @filetable = ();
    my @filehashtable = ();
    my %allfilecomponents = ();
    my $counter = 0;

    if ( $^O =~ /cygwin/i ) { installer::worker::generate_cygwin_paths($filesref); }

    # The filenames must be collected because of uniqueness
    # 01-44-~1.DAT, 01-44-~2.DAT, ...
    my %shortnames = ();

    if ( $installer::globals::updatedatabase ) { collect_shortnames_from_old_database($uniquefilenamehashref, \%shortnames); }

    installer::windows::idtglobal::write_idt_header(\@filetable, "file");
    installer::windows::idtglobal::write_idt_header(\@filehashtable, "filehash");
    installer::windows::idtglobal::write_idt_header(\@installer::globals::removefiletable, "removefile");

    for ( my $i = 0; $i <= $#{$filesref}; $i++ )
    {
        my %file = ();

        my $onefile = ${$filesref}[$i];

        my $styles = "";
        if ( $onefile->{'Styles'} ) { $styles = $onefile->{'Styles'}; }

        $file{'Component_'} = get_file_component_name($onefile, $filesref);
        $file{'File'} = generate_unique_filename_for_filetable($onefile, $file{'Component_'}, $uniquefilenamehashref);

        $onefile->{'uniquename'} = $file{'File'};
        $onefile->{'componentname'} = $file{'Component_'};

        # Collecting all components

        if ( ! exists($allfilecomponents{$file{'Component_'}}) ) { $allfilecomponents{$file{'Component_'}} = 1; }

        $file{'FileName'} = generate_filename_for_filetable($onefile, \%shortnames, $uniquefilenamehashref);

        $file{'FileSize'} = get_filesize($onefile);

        $file{'Version'} = get_fileversion($onefile, $allvariables, $styles);

        $file{'Language'} = get_language_for_file($onefile);

        if ( $styles =~ /\bDONT_PACK\b/ ) { $file{'Attributes'} = "8192"; }
        else { $file{'Attributes'} = "16384"; }

        # $file{'Attributes'} = "16384";    # Sourcefile is packed
        # $file{'Attributes'} = "8192";     # Sourcefile is unpacked

        $installer::globals::insert_file_at_end = 0;
        $counter++;
        $file{'Sequence'} = get_sequence_for_file($counter, $onefile, \%file, $allupdatesequenceshashref, $allupdatecomponentshashref, $allupdatefileorderhashref, \%allfilecomponents);

        $onefile->{'sequencenumber'} = $file{'Sequence'};

        my $oneline = $file{'File'} . "\t" . $file{'Component_'} . "\t" . $file{'FileName'} . "\t"
                . $file{'FileSize'} . "\t" . $file{'Version'} . "\t" . $file{'Language'} . "\t"
                . $file{'Attributes'} . "\t" . $file{'Sequence'} . "\n";

        push(@filetable, $oneline);

        if ( $file{'File'} =~ /\.py$/ )
        {
            my %removefile = ();

            $removefile{'FileKey'} = "remove_" . $file{'File'} . "c";
            $removefile{'Component_'} = $file{'Component_'};
            $removefile{'FileName'} = $file{'FileName'};
            $removefile{'FileName'} =~ s/\.py$/.pyc/;
            $removefile{'FileName'} =~ s/\.PY\|/.PYC|/;
            $removefile{'DirProperty'} = installer::windows::component::get_file_component_directory($file{'Component_'}, $filesref, $dirref);
            $removefile{'InstallMode'} = 2; # msiInstallStateAbsent
            $oneline = $removefile{'FileKey'} . "\t" . $removefile{'Component_'} . "\t" . $removefile{'FileName'} . "\t"
                        . $removefile{'DirProperty'} . "\t" . $removefile{'InstallMode'} . "\n";

            push(@installer::globals::removefiletable, $oneline);
        }

        if ( ! $installer::globals::insert_file_at_end ) { push(@allfiles, $onefile); }

        # Collecting all component conditions
        if ( $onefile->{'ComponentCondition'} )
        {
            if ( ! exists($installer::globals::componentcondition{$file{'Component_'}}))
            {
                $installer::globals::componentcondition{$file{'Component_'}} = $onefile->{'ComponentCondition'};
            }
        }

        # Collecting also all tree conditions for multilayer products
        get_tree_condition_for_component($onefile, $file{'Component_'});

        # Collecting all component names, that have flag VERSION_INDEPENDENT_COMP_ID
        # This should be all components with constant API, for example URE
        if ( $styles =~ /\bVERSION_INDEPENDENT_COMP_ID\b/ )
        {
            $installer::globals::base_independent_components{$onefile->{'componentname'}} = 1;
        }

        unless ( $file{'Version'} )
        {
            my $path = $onefile->{'sourcepath'};
            if ( $^O =~ /cygwin/i ) { $path = $onefile->{'cyg_sourcepath'}; }

            open(FILE, $path) or die "ERROR: Can't open $path for creating file hash";
            binmode(FILE);
            my $hashinfo = pack("l", 20);
            $hashinfo .= Digest::MD5->new->addfile(*FILE)->digest;

            my @i = unpack ('x[l]l4', $hashinfo);
            $oneline = $file{'File'} . "\t" .
                "0" . "\t" .
                $i[0] . "\t" .
                $i[1] . "\t" .
                $i[2] . "\t" .
                $i[3] . "\n";
            push (@filehashtable, $oneline);
        }

        # Saving the sequence number in a hash with uniquefilename as key.
        # This is used for better performance in "save_packorder"
        $installer::globals::uniquefilenamesequence{$onefile->{'uniquename'}} = $onefile->{'sequencenumber'};

        my $destdir = "";
        if ( $onefile->{'Dir'} ) { $destdir = $onefile->{'Dir'}; }
    }

    # putting content from %allfilecomponents to $allfilecomponentsref for later usage
    foreach $localkey (keys %allfilecomponents ) { push( @{$allfilecomponentsref}, $localkey); }

    my $filetablename = $basedir . $installer::globals::separator . "File.idt";
    installer::files::save_file($filetablename ,\@filetable);
    $infoline = "\nCreated idt file: $filetablename\n";
    push(@installer::globals::logfileinfo, $infoline);

    installer::logger::include_timestamp_into_logfile("Performance Info: File Table end");

    my $filehashtablename = $basedir . $installer::globals::separator . "MsiFileHash.idt";
    installer::files::save_file($filehashtablename ,\@filehashtable);
    $infoline = "\nCreated idt file: $filehashtablename\n";
    push(@installer::globals::logfileinfo, $infoline);

    # Now the new files can be added to the files collector (only in update packaging processes)
    if ( $installer::globals::newfilesexist )
    {
        foreach my $seq (sort keys %installer::globals::newfilescollector) { push(@allfiles, $installer::globals::newfilescollector{$seq}) }
    }

    return \@allfiles;
}

1;
