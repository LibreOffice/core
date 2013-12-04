#**************************************************************
#
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#
#**************************************************************



package installer::windows::file;

use Digest::MD5;
use installer::existence;
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
use installer::patch::InstallationSet;
use installer::patch::FileSequenceList;
use File::Basename;
use File::Spec;
use strict;

##########################################################################
# Assigning one cabinet file to each file. This is requrired,
# if cabinet files shall be equivalent to packages.
##########################################################################

sub assign_cab_to_files
{
    my ( $filesref ) = @_;

    my $infoline = "";

    foreach my $file (@$filesref)
    {
        if ( ! exists($file->{'modules'}) )
        {
            installer::exiter::exit_program(
                sprintf("ERROR: No module assignment found for %s", $file->{'gid'}),
                "assign_cab_to_files");
        }
        my $module = $file->{'modules'};
        # If modules contains a list of modules, only taking the first one.
        if ( $module =~ /^\s*(.*?)\,/ ) { $module = $1; }

        if ( ! exists($installer::globals::allcabinetassigns{$module}) )
        {
            installer::exiter::exit_program(
                sprintf("ERROR: No cabinet file assigned to module \"%s\" %s",
                    $module,
                    $file->{'gid'}),
                "assign_cab_to_files");
        }
        $file->{'assignedcabinetfile'} = $installer::globals::allcabinetassigns{$module};

        # Counting the files in each cabinet file
        if ( ! exists($installer::globals::cabfilecounter{$file->{'assignedcabinetfile'}}) )
        {
            $installer::globals::cabfilecounter{$file->{'assignedcabinetfile'}} = 1;
        }
        else
        {
            $installer::globals::cabfilecounter{$file->{'assignedcabinetfile'}}++;
        }
    }

    # assigning startsequencenumbers for each cab file

    my %count = ();
    my $offset = 1;
    foreach my $cabfile ( sort keys %installer::globals::cabfilecounter )
    {
        my $filecount = $installer::globals::cabfilecounter{$cabfile};
        $count{$cabfile} = $filecount;
        $installer::globals::cabfilecounter{$cabfile} = $offset;
        $offset = $offset + $filecount;

        $installer::globals::lastsequence{$cabfile} = $offset - 1;
    }

    # logging the number of files in each cabinet file

    $installer::logger::Lang->print("\n");
    $installer::logger::Lang->print("Cabinet files:\n");
    foreach my $cabfile (sort keys %installer::globals::cabfilecounter)
    {
        $installer::logger::Lang->printf(
            "%-30s : %4s files, from %4d to %4d\n",
            $cabfile,
            $count{$cabfile},
            $installer::globals::cabfilecounter{$cabfile},
            $installer::globals::lastsequence{$cabfile});
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

    my $onefile = installer::existence::get_specified_file($filesref, $gid);
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

sub generate_unique_filename_for_filetable ($)
{
    my ($oldname) = @_;

    # This new filename has to be saved into $fileref, because this is needed to find the source.
    # The filename sbasic.idx/OFFSETS is changed to OFFSETS, but OFFSETS is not unique.
    # In this procedure names like OFFSETS5 are produced. And exactly this string has to be added to
    # the array of all files.

    my $uniquefilename = $oldname;
    if ( ! defined $uniquefilename || $uniquefilename eq "")
    {
        installer::logger::PrintError("file name does not exist or is empty, can not create unique name for it.");
        die;
        return;
    }

       # making /registry/schema/org/openoffice/VCL.xcs to VCL.xcs
    installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$uniquefilename);

    $uniquefilename =~ s/\-/\_/g;       # no "-" allowed
    $uniquefilename =~ s/\@/\_/g;       # no "@" allowed
    $uniquefilename =~ s/\$/\_/g;       # no "$" allowed
    $uniquefilename =~ s/^\s*\./\_/g;       # no "." at the beginning allowed allowed
    $uniquefilename =~ s/^\s*\d/\_d/g;      # no number at the beginning allowed allowed (even file "0.gif", replacing to "_d.gif")
    $uniquefilename =~ s/org_openoffice_/ooo_/g;    # shorten the unique file name

    my $lcuniquefilename = lc($uniquefilename); # only lowercase names

    my $newname = 0;

    if ( ! exists($installer::globals::alllcuniquefilenames{$lcuniquefilename}))
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

        my $counter = 0;
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

            if ( ! exists($installer::globals::alllcuniquefilenames{$lcuniquefilename}))
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

sub generate_filename_for_filetable ($$)
{
    my ($fileref, $shortnamesref) = @_;

    my $returnstring = "";

    my $filename = $fileref->{'Name'};

    # making /registry/schema/org/openoffice/VCL.xcs to VCL.xcs
    installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$filename);

    my $shortstring = installer::windows::idtglobal::make_eight_three_conform_with_hash($filename, "file", $shortnamesref);

    if ( $shortstring eq $filename )
    {
        # nothing changed
        $returnstring = $filename;
    }
    else
    {
        $returnstring = $shortstring . "\|" . $filename;
    }

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
    my ($onefile, $allvariables) = @_;

    my $fileversion = "";

    if ( $allvariables->{'USE_FILEVERSION'} )
    {
        if ( ! $allvariables->{'LIBRARYVERSION'} )
        {
            installer::exiter::exit_program("ERROR: USE_FILEVERSION is set, but not LIBRARYVERSION", "get_fileversion");
        }
        my $libraryversion = $allvariables->{'LIBRARYVERSION'};
        if ( $libraryversion =~ /^\s*(\d+)\.(\d+)\.(\d+)\s*$/ )
        {
            my $major = $1;
            my $minor = $2;
            my $micro = $3;
            my $concat = 100 * $minor + $micro;
            $libraryversion = $major . "\." . $concat;
        }
        my $vendornumber = 0;
        if ( $allvariables->{'VENDORPATCHVERSION'} )
        {
            $vendornumber = $allvariables->{'VENDORPATCHVERSION'};
        }
        $fileversion = $libraryversion . "\." . $installer::globals::buildid . "\." . $vendornumber;
        if ( $onefile->{'FileVersion'} )
        {
            # overriding FileVersion in scp
            $fileversion = $onefile->{'FileVersion'};
        }
    }

    if ( $installer::globals::prepare_winpatch )
    {
        # Windows patches do not allow this version # -> who says so?
        $fileversion = "";
    }

    return $fileversion;
}




sub retrieve_sequence_and_uniquename ($$)
{
    my ($file_list, $source_data) = @_;

    my @added_files = ();

    # Read the sequence numbers of the previous version.
    if ($installer::globals::is_release)
    {
        foreach my $file (@$file_list)
        {
            # Use the source path of the file as key to retrieve sequence number and unique name.
            # The source path is the part of the 'destination' without the first part.
            # There is a special case when 'Dir' is PREDEFINED_OSSHELLNEWDIR.
            my $source_path;
            if (defined $file->{'Dir'} && $file->{'Dir'} eq "PREDEFINED_OSSHELLNEWDIR")
            {
                $source_path = $installer::globals::templatefoldername
                    . $installer::globals::separator
                    . $file->{'Name'};
            }
            else
            {
                $source_path = $file->{'destination'};
                $source_path =~ s/^[^\/]+\///;
            }
            my ($sequence, $uniquename) = $source_data->get_sequence_and_unique_name($source_path);
            if (defined $sequence && defined $uniquename)
            {
                $file->{'sequencenumber'} = $sequence;
                $file->{'uniquename'} = $uniquename;
            }
            else
            {
                # No data found in the source release.  File has been added.
                push @added_files, $file;
            }
        }
    }

    return @added_files;
}




=head2 assign_mssing_sequence_numbers ($file_list)

    Assign sequence numbers where still missing.

    When we are preparing a patch then all files that have no sequence numbers
    at this point are new.  Otherwise no file has a sequence number yet.

=cut
sub assign_missing_sequence_numbers ($)
{
    my ($file_list) = @_;

    # First, set up a hash on the sequence numbers that are already in use.
    my %used_sequence_numbers = ();
    foreach my $file (@$file_list)
    {
        next unless defined $file->{'sequencenumber'};
        $used_sequence_numbers{$file->{'sequencenumber'}} = 1;
    }

    # Assign sequence numbers.  Try consecutive numbers, starting at 1.
    my $current_sequence_number = 1;
    foreach my $file (@$file_list)
    {
        # Skip over all files that already have sequence numbers.
        next if defined $file->{'sequencenumber'};

        # Find the next available number.
        while (defined $used_sequence_numbers{$current_sequence_number})
        {
            ++$current_sequence_number;
        }

        # Use the number and mark it as used.
        $file->{'sequencenumber'} = $current_sequence_number;
        $used_sequence_numbers{$current_sequence_number} = 1;
    }
}




sub create_items_for_missing_files ($$$)
{
    my ($missing_items, $msi, $directory_list) = @_;

    # For creation of the FeatureComponent table (in a later step) we
    # have to provide references from the file to component and
    # modules (ie features).  Note that Each file belongs to exactly
    # one component but one component can belong to multiple features.
    my $component_to_features_map = create_feature_component_map($msi);

    my @new_files = ();
    foreach my $row (@$missing_items)
    {
        $installer::logger::Info->printf("creating new file item for '%s'\n", $row->GetValue('File'));
        my $file_item = create_script_item_for_deleted_file($row, $msi, $component_to_features_map);
        push @new_files, $file_item;
    }

    return @new_files;
}




sub create_script_item_for_deleted_file ($$$)
{
    my ($file_row, $msi, $component_to_features_map) = @_;

    my $uniquename = $file_row->GetValue('File');

    my $file_map = $msi->GetFileMap();

    my $directory_item = $file_map->{$uniquename}->{'directory'};
    my $source_path = $directory_item->{'full_source_long_name'};
    my $target_path = $directory_item->{'full_target_long_name'};
    my $full_source_name = File::Spec->catfile(
        installer::patch::InstallationSet::GetUnpackedCabPath(
            $msi->{'version'},
            $msi->{'is_current_version'},
            $msi->{'language'},
            $msi->{'package_format'},
            $msi->{'product_name'}),
        $source_path,
        $uniquename);
    my ($long_name, undef) = installer::patch::Msi::SplitLongShortName($file_row->GetValue("FileName"));
    my $target_name = File::Spec->catfile($target_path, $long_name);
    if ( ! -f $full_source_name)
    {
        installer::logger::PrintError("can not find file '%s' in previous version (tried '%s')\n",
            $uniquename,
            $full_source_name);
        return undef;
    }
    my $cygwin_full_source_name = qx(cygpath -w '$full_source_name');
    my $component_name = $file_row->GetValue('Component_');
    my $module_names = join(",", @{$component_to_features_map->{$component_name}});
    my $sequence_number = $file_row->GetValue('Sequence');

    return {
        'uniquename' => $uniquename,
        'destination' => $target_name,
        'componentname' => $component_name,
        'modules' => $module_names,
        'UnixRights' => 444,
        'Name' => $long_name,
        'sourcepath' => $full_source_name,
        'cyg_sourcepath' => $cygwin_full_source_name,
        'sequencenumber' => $sequence_number
        };
}




=head2 create_feature_component_maps($msi)

    Return a hash map that maps from component names to arrays of
    feature names.  In most cases the array of features contains only
    one element.  But there can be cases where the number is greater.

=cut
sub create_feature_component_map ($)
{
    my ($msi) = @_;

    my $component_to_features_map = {};
    my $feature_component_table = $msi->GetTable("FeatureComponents");
    my $feature_column_index = $feature_component_table->GetColumnIndex("Feature_");
    my $component_column_index = $feature_component_table->GetColumnIndex("Component_");
    foreach my $row (@{$feature_component_table->GetAllRows()})
    {
        my $feature = $row->GetValue($feature_column_index);
        my $component = $row->GetValue($component_column_index);
        if ( ! defined $component_to_features_map->{$component})
        {
            $component_to_features_map->{$component} = [$feature];
        }
        else
        {
            push @{$component_to_features_map->{$component}}, $feature;
        }
    }

    return $component_to_features_map;
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
# Creating a new KeyPath for components in TemplatesFolder.
####################################################################

sub generate_registry_keypath
{
    my ($onefile) = @_;

    my $keypath = $onefile->{'Name'};
    $keypath =~ s/\.//g;
    $keypath = lc($keypath);
    $keypath = "userreg_" . $keypath;

    return $keypath;
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


sub process_language_conditions ($)
{
    my ($onefile) = @_;

    # Collecting all languages specific conditions
    if ( $onefile->{'ismultilingual'} )
    {
        if ( $onefile->{'ComponentCondition'} )
        {
            installer::exiter::exit_program(
                "ERROR: Cannot set language condition. There is already another component condition for file $onefile->{'gid'}: \"$onefile->{'ComponentCondition'}\" !", "create_files_table");
        }

        if ( $onefile->{'specificlanguage'} eq "" )
        {
            installer::exiter::exit_program(
                "ERROR: There is no specific language for file at language module: $onefile->{'gid'} !", "create_files_table");
        }
        my $locallanguage = $onefile->{'specificlanguage'};
        my $property = "IS" . $onefile->{'windows_language'};
        my $value = 1;
        my $condition = $property . "=" . $value;

        $onefile->{'ComponentCondition'} = $condition;

        if ( exists($installer::globals::componentcondition{$onefile->{'componentname'}}))
        {
            if ( $installer::globals::componentcondition{$onefile->{'componentname'}} ne $condition )
            {
                installer::exiter::exit_program(
                    sprintf(
                        "ERROR: There is already another component condition for file %s: \"%s\" and \"%s\" !",
                        $onefile->{'gid'},
                        $installer::globals::componentcondition{$onefile->{'componentname'}},
                        $condition),
                    "create_files_table");
            }
        }
        else
        {
            $installer::globals::componentcondition{$onefile->{'componentname'}} = $condition;
        }

        # collecting all properties for table Property
        if ( ! exists($installer::globals::languageproperties{$property}) )
        {
            $installer::globals::languageproperties{$property} = $value;
        }
    }
}




sub has_style ($$)
{
    my ($style_list_string, $style_name) = @_;

    return 0 unless defined $style_list_string;
    return $style_list_string =~ /\b$style_name\b/ ? 1 : 0;
}




sub prepare_file_table_creation ($$$)
{
    my ($file_list, $directory_list, $allvariables) = @_;

    if ( $^O =~ /cygwin/i )
    {
        installer::worker::generate_cygwin_pathes($file_list);
    }

    # Reset the fields 'sequencenumber' and 'uniquename'. They should not yet exist but better be sure.
    foreach my $file (@$file_list)
    {
        delete $file->{'sequencenumber'};
        delete $file->{'uniquename'};
    }

    # Create FileSequenceList object for the old sequence data.
    if (defined $installer::globals::source_msi)
    {
        my $previous_sequence_data = new installer::patch::FileSequenceList();
        $previous_sequence_data->SetFromMsi($installer::globals::source_msi);
        my @added_files = retrieve_sequence_and_uniquename($file_list, $previous_sequence_data);

        # Extract just the unique names.
        my %target_unique_names = map {$_->{'uniquename'} => 1} @$file_list;
        my @removed_items = $previous_sequence_data->get_removed_files(\%target_unique_names);

        $installer::logger::Lang->printf(
            "there are %d files that have been removed from source and %d files added\n",
            scalar @removed_items,
            scalar @added_files);

        my $file_map = $installer::globals::source_msi->GetFileMap();
        my $index = 0;
        foreach my $removed_row (@removed_items)
        {
            $installer::logger::Lang->printf("    removed file %d: %s\n",
                ++$index,
                $removed_row->GetValue('File'));
            my $directory = $file_map->{$removed_row->GetValue('File')}->{'directory'};
            while (my ($key,$value) = each %$directory)
            {
                $installer::logger::Lang->printf("        %16s -> %s\n", $key, $value);
            }
        }
        $index = 0;
        foreach my $added_file (@added_files)
        {
            $installer::logger::Lang->printf("    added file %d: %s\n",
                ++$index,
                $added_file->{'uniquename'});
            installer::scriptitems::print_script_item($added_file);
        }
        my @new_files = create_items_for_missing_files(
            \@removed_items,
            $installer::globals::source_msi,
            $directory_list);
        push @$file_list, @new_files;
    }
    assign_missing_sequence_numbers($file_list);

    foreach my $file (@$file_list)
    {
        if ( ! defined $file->{'componentname'})
        {
            $file->{'componentname'} = get_file_component_name($file, $file_list);
        }
        if ( ! defined $file->{'uniquename'})
        {
            $file->{'uniquename'} = generate_unique_filename_for_filetable($file->{'Name'});
        }

        # Collecting all component conditions
        if ( $file->{'ComponentCondition'} )
        {
            if ( ! exists($installer::globals::componentcondition{$file->{'componentname'}}))
            {
                $installer::globals::componentcondition{$file->{'componentname'}}
                = $file->{'ComponentCondition'};
            }
        }
        # Collecting also all tree conditions for multilayer products
        get_tree_condition_for_component($file, $file->{'componentname'});

        # Collecting all component names, that have flag VERSION_INDEPENDENT_COMP_ID
        # This should be all components with constant API, for example URE
        if (has_style($file->{'Styles'}, "VERSION_INDEPENDENT_COMP_ID"))
        {
            $installer::globals::base_independent_components{$file->{'componentname'}} = 1;
        }

        # Special handling for files in PREDEFINED_OSSHELLNEWDIR. These components
        # need as KeyPath a RegistryItem in HKCU
        if ($file->{'needs_user_registry_key'}
            || (defined $file->{'Dir'} && $file->{'Dir'} =~ /\bPREDEFINED_OSSHELLNEWDIR\b/))
        {
            my $keypath = generate_registry_keypath($file);
            $file->{'userregkeypath'} = $keypath;
            push(@installer::globals::userregistrycollector, $file);
            $installer::globals::addeduserregitrykeys = 1;
        }

        $file->{'windows_language'} = get_language_for_file($file);

        process_language_conditions($file);
    }

    # The filenames must be collected because of uniqueness
    # 01-44-~1.DAT, 01-44-~2.DAT, ...
    my %shortnames = ();
    foreach my $file (@$file_list)
    {
        $file->{'short_name'} = generate_filename_for_filetable($file, \%shortnames);
    }
}




sub create_file_table_data ($$)
{
    my ($file_list, $allvariables) = @_;

    my @file_table_data = ();
    foreach my $file (@$file_list)
    {
        my $attributes;
        if (has_style($file->{'Styles'}, "DONT_PACK"))
        {
            # Sourcefile is unpacked (msidbFileAttributesNoncompressed).
            $attributes = "8192";
        }
        else
        {
            # Sourcefile is packed (msidbFileAttributesCompressed).
            $attributes = "16384";
        }

        my $row_data = {
            'File' => $file->{'uniquename'},
            'Component_' => $file->{'componentname'},
            'FileName' => $file->{'short_name'},
            'FileSize' => get_filesize($file),
            'Version' => get_fileversion($file, $allvariables),
            'Language' => $file->{'windows_language'},
            'Attributes' => $attributes,
            'Sequence' => $file->{'sequencenumber'}
            };
        push @file_table_data, $row_data;
    }

    return \@file_table_data;
}




sub collect_components ($)
{
    my ($file_list) = @_;

    my %components = ();
    foreach my $file (@$file_list)
    {
        $components{$file->{'componentname'}} = 1;
    }
    return keys %components;
}




=head filter_files($file_list, $allvariables)

    Filter out Java files when not building a Java product.

    Is this still triggered?

=cut
sub filter_files ($$)
{
    my ($file_list, $allvariables) = @_;

    if ($allvariables->{'JAVAPRODUCT'})
    {
        return $file_list;
    }
    else
    {
        my @filtered_files = ();
        foreach my $file (@$file_list)
        {
            if ( ! has_style($file->{'Styles'}, "JAVAFILE"))
            {
                push @filtered_files, $file;
            }
        }
        return \@filtered_files;
    }
}




# Structure of the files table:
# File Component_ FileName FileSize Version Language Attributes Sequence
sub create_file_table ($$)
{
    my ($file_table_data, $basedir) = @_;

    # Set up the 'File' table.
    my @filetable = ();
    installer::windows::idtglobal::write_idt_header(\@filetable, "file");
    my @keys = ('File', 'Component_', 'FileName', 'FileSize', 'Version', 'Language', 'Attributes', 'Sequence');
    my $index = 0;
    foreach my $row_data (@$file_table_data)
    {
        ++$index;
        my @values = map {$row_data->{$_}} @keys;
        my $line = join("\t", @values) . "\n";
        push(@filetable, $line);
    }

    my $filetablename = $basedir . $installer::globals::separator . "File.idt";
    installer::files::save_file($filetablename ,\@filetable);
    $installer::logger::Lang->print("\n");
    $installer::logger::Lang->printf("Created idt file: %s\n", $filetablename);
}




sub create_filehash_table ($$)
{
    my ($file_list, $basedir) = @_;

    my @filehashtable = ();

    if ( $installer::globals::prepare_winpatch )
    {

        installer::windows::idtglobal::write_idt_header(\@filehashtable, "filehash");

        foreach my $file (@$file_list)
        {
            my $path = $file->{'sourcepath'};
            if ($^O =~ /cygwin/i)
            {
                $path = $file->{'cyg_sourcepath'};
            }

            open(FILE, $path) or die "ERROR: Can't open $path for creating file hash";
            binmode(FILE);
            my $hashinfo = pack("l", 20);
            $hashinfo .= Digest::MD5->new->addfile(*FILE)->digest;

            my @i = unpack ('x[l]l4', $hashinfo);
            my $oneline = join("\t",
                (
                    $file->{'uniquename'},
                    "0",
                    @i
                ));
            push (@filehashtable, $oneline . "\n");
        }

        my $filehashtablename = $basedir . $installer::globals::separator . "MsiFileHash.idt";
        installer::files::save_file($filehashtablename ,\@filehashtable);
        $installer::logger::Lang->print("\n");
        $installer::logger::Lang->printf("Created idt file: %s\n", $filehashtablename);
    }
}


1;
