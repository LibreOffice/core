#*************************************************************************
#
#   $RCSfile: file.pm,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: kz $ $Date: 2004-06-11 18:19:17 $
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

package installer::windows::file;

use installer::existence;
use installer::exiter;
use installer::files;
use installer::globals;
use installer::pathanalyzer;
use installer::windows::idtglobal;
use installer::windows::language;

###############################################
# Generating the component name from a file
###############################################

sub get_file_component_name
{
    my ($fileref) = @_;

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
    $destination =~ s/\\/\_/g;
    $destination =~ s/\//\_/g;
    $destination =~ s/\_\s*$//g;    # removing ending underline

    my $componentname = $fileref->{'gid'} . "__" . $destination;

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

    $componentname =~ s/gid_file_/g_f_/g;
    $componentname =~ s/_extra_/_e_/g;
    $componentname =~ s/_config_/_c_/g;
    $componentname =~ s/_org_openoffice_/_o_o_/g;
    $componentname =~ s/_program_/_p_/g;
    $componentname =~ s/_typedetection_/_td_/g;
    $componentname =~ s/_linguistic_/_l_/g;
    $componentname =~ s/_module_/_m_/g;
    $componentname =~ s/_optional_/_opt_/g;
    $componentname =~ s/_packages/_pack/g;
    $componentname =~ s/_menubar/_mb/g;
    $componentname =~ s/_common_/_cm_/g;
    $componentname =~ s/_export_/_exp_/g;
    $componentname =~ s/_table_/_tb_/g;
    $componentname =~ s/_sofficecfg_/_sc_/g;
    $componentname =~ s/_startmodulecommands_/_smc_/g;
    $componentname =~ s/_drawimpresscommands_/_dic_/g;
    $componentname =~ s/_basiccommands_/_bac_/g;
    $componentname =~ s/_basicidecommands_/_baic_/g;
    $componentname =~ s/_genericcommands_/_genc_/g;
    $componentname =~ s/_bibliographycommands_/_bibc_/g;

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
    my ($fileref, $filetablehashref, $filesref) = @_;

    # This new filename has to be saved into $fileref, because this is needed to find the source.
    # The filename sbasic.idx/OFFSETS is changed to OFFSETS, but OFFSETS is not unique.
    # In this procedure names like OFFSETS5 are produced. And exactly this string has to be added to
    # the array of all files.

    my ($onefile, $uniquename);
    my $uniquefilename = "";
    my $alreadyexists = 0;
    my $counter = 0;

    if ( $fileref->{'Name'} ) { $uniquefilename = $fileref->{'Name'}; }

    installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$uniquefilename); # making /registry/schema/org/openoffice/VCL.xcs to VCL.xcs

    $uniquefilename =~ s/\-/\_/g;       # no "-" allowed
    $uniquefilename =~ s/\@/\_/g;       # no "@" allowed
    $uniquefilename =~ s/\$/\_/g;       # no "$" allowed
    $uniquefilename =~ s/^\s*\./\_/g;       # no "." at the beginning allowed allowed
    $uniquefilename =~ s/^\s*\d/\_d/g;      # no number at the beginning allowed allowed (even file "0.gif", replacing to "_d.gif")
    $uniquefilename =~ s/org_openoffice_/ooo_/g;    # shorten the unique file name

    for ( my $i = 0; $i <= $#{$filesref}; $i++ )
    {
        $onefile = ${$filesref}[$i];
        $uniquename = "";

        if ( $onefile->{'uniquename'} ) { $uniquename = $onefile->{'uniquename'}; }

        if (lc($uniquename) eq lc($uniquefilename)) # case insensitve comparison !
        {
            $alreadyexists = 1;
            last;
        }
    }

    if ($alreadyexists)
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

            $alreadyexists = 0;

            for ( my $i = 0; $i <= $#{$filesref}; $i++ )
            {
                $onefile = ${$filesref}[$i];

                $uniquename = "";

                if ( $onefile->{'uniquename'} ) { $uniquename = $onefile->{'uniquename'}; }

                if (lc($uniquename) eq lc($uniquefilename)) # case insensitive comparison !
                {
                    $alreadyexists = 1;
                    last;
                }
            }
        }
        until (!($alreadyexists))
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
    my ($fileref, $shortnamesref) = @_;

    my $returnstring = "";

    my $filename = $fileref->{'Name'};

    installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$filename);   # making /registry/schema/org/openoffice/VCL.xcs to VCL.xcs

    my $shortstring = installer::windows::idtglobal::make_eight_three_conform($filename, "file", $shortnamesref);

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
        # $filesize = (stat($file))[7] || installer::exiter::exit_program("ERROR: $file: stat error in function get_filesize", "get_filesize");
        $filesize = (stat($file))[7];   # file size can be "0"
    }
    else
    {
        $filesize = -1;
    }

    return $filesize;
}


#############################################
# Returning the sequence for a file
#############################################

sub get_sequence_for_file
{
    my ($number) = @_;

    my $sequence = $number + 1;

    # Idea: Each component is packed into a cab file.
    # This requires that all files in one cab file have sequences directly follwing each other,
    # for instance from 1456 to 1466. Then in the media table the LastSequence for this cab file
    # is 1466.
    # Because all files belonging to one component are directly behind each other in the file
    # collector, it is possible to use simply an increasing number as sequence value.
    # If files belonging to one component are not directly behind each other in the files collector
    # this mechanism will no longer work.

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

    if (!($language eq ""))
    {
        $language = installer::windows::language::get_windows_language($language);
    }

    return $language;
}

############################################
# Creating the file File.idt dynamically
############################################

sub create_files_table
{
    my ($filesref, $allfilecomponentsref, $basedir) = @_;

    # Structure of the files table:
    # File Component_ FileName FileSize Version Language Attributes Sequence
    # In this function, all components are created.

    my $infoline;

    my @filetable = ();

    my %file = ();

    # The filenames must be collected because of uniqueness
    # 01-44-~1.DAT, 01-44-~2.DAT, ...
    my @shortnames = ();

    installer::windows::idtglobal::write_idt_header(\@filetable, "file");

    for ( my $i = 0; $i <= $#{$filesref}; $i++ )
    {
        my $onefile = ${$filesref}[$i];

        $file{'File'} = generate_unique_filename_for_filetable($onefile, \%filetable, $filesref);

        $onefile->{'uniquename'} = $file{'File'};

        $file{'Component_'} = get_file_component_name($onefile);

        $onefile->{'componentname'} = $file{'Component_'};

        # Collecting all components
        if (!(installer::existence::exists_in_array($file{'Component_'}, $allfilecomponentsref))) { push(@{$allfilecomponentsref}, $file{'Component_'}); }

        $file{'FileName'} = generate_filename_for_filetable($onefile, \@shortnames);

        $file{'FileSize'} = get_filesize($onefile);

        $file{'Version'} = "";

        $file{'Language'} = get_language_for_file($onefile);

        $file{'Attributes'} = "16384";  # Sourcefile is packed
        # $file{'Attributes'} = "8192";     # Sourcefile is unpacked

        $file{'Sequence'} = get_sequence_for_file($i);

        $onefile->{'sequencenumber'} = $file{'Sequence'};

        my $oneline = $file{'File'} . "\t" . $file{'Component_'} . "\t" . $file{'FileName'} . "\t"
                . $file{'FileSize'} . "\t" . $file{'Version'} . "\t" . $file{'Language'} . "\t"
                . $file{'Attributes'} . "\t" .$file{'Sequence'} . "\n";

        push(@filetable, $oneline);
    }

    my $filetablename = $basedir . $installer::globals::separator . "File.idt";
    installer::files::save_file($filetablename ,\@filetable);
    $infoline = "\nCreated idt file: $filetablename\n";
    push(@installer::globals::logfileinfo, $infoline);

}

1;