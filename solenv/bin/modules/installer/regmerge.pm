#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: regmerge.pm,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: vg $ $Date: 2008-03-18 13:00:42 $
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

package installer::regmerge;

use Cwd;
use installer::converter;
use installer::existence;
use installer::exiter;
use installer::globals;
use installer::logger;
use installer::pathanalyzer;
use installer::remover;
use installer::scriptitems;
use installer::systemactions;

################################################################
# Collecting all files with content:
# Regmergefile = "mydatabasepart.rdb";
################################################################

sub collect_all_regmergefiles
{
    my ($filesarrayref) = @_;

    my @regmergefiles = ();

    for ( my $i = 0; $i <= $#{$filesarrayref}; $i++ )
    {
        my $onefile = ${$filesarrayref}[$i];
        if ( $onefile->{'Regmergefile'} ) { push(@regmergefiles, $onefile); }
    }

    return \@regmergefiles;
}

################################################################
# Collecting all gids of the databases, that are part of
# the file definition
################################################################

sub collect_all_database_gids
{
    my ($filesarrayref) = @_;

    my @databasegids = ();

    for ( my $i = 0; $i <= $#{$filesarrayref}; $i++ )
    {
        my $onefile = ${$filesarrayref}[$i];

        if ( $onefile->{'RegistryID'} )
        {
            my $databasegid = $onefile->{'RegistryID'};
            if (! installer::existence::exists_in_array($databasegid, \@databasegids)) { push(@databasegids, $databasegid); }
        }
        else
        {
            installer::exiter::exit_program("ERROR: File defintion error. File :$onefile->{'gid'} without RegistryID!", "collect_all_database_gids");
        }
    }

    return \@databasegids;
}

################################################################
# Returning the database file from the files collector. In the
# future this file does not need to exist, but currently it
# has to exist already in the files collector.
################################################################

sub get_database_file
{
    my ($databasegid, $filesarrayref) = @_;

    my $found = 0;
    my $onefile;

    for ( my $i = 0; $i <= $#{$filesarrayref}; $i++ )
    {
        $onefile = ${$filesarrayref}[$i];
        my $gid = $onefile->{'gid'};

        if ( $databasegid eq $gid )
        {
            $found = 1;
            last;
        }
    }

    if ( ! $found ) { installer::exiter::exit_program("ERROR: Did not find StarRegistry file $databasegid!", "get_database_file"); }

    return $onefile;
}

################################################################
# The regmerge file has to be found the in include pathes
################################################################

sub get_regmerge_file
{
    my ($includepatharrayref) = @_;

    my $searchname;

    if ($installer::globals::isunix) { $searchname = "regcomplazy"; }
    else { $searchname = "regcomplazy.exe"; }

    my $regmergefileref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$searchname, $includepatharrayref, 1);
    if ( $$regmergefileref eq "" ) { installer::exiter::exit_program("ERROR: Could not find file $searchname for merging the StarRegistry!", "get_regmerge_file"); }

    return $$regmergefileref;
}

################################################################
# Collecting all files that are merged to one defined
# StarRegistry database
################################################################

sub collect_all_files_for_one_registry
{
    my ($regmergefiles, $databasegid) = @_;

    my @regmergefiles = ();

    for ( my $i = 0; $i <= $#{$regmergefiles}; $i++ )
    {
        my $onefile = ${$regmergefiles}[$i];
        if ( $onefile->{'RegistryID'} eq $databasegid ) { push(@regmergefiles, $onefile); }
    }

    return \@regmergefiles;
}

################################################################
# Collecting all particles from the regmerge files
################################################################

sub collect_all_regmerge_particles
{
    my ($databaseregisterfiles) = @_;

    my @regmergeparticles = ();

    for ( my $i = 0; $i <= $#{$databaseregisterfiles}; $i++ )
    {
        my $onefile = ${$databaseregisterfiles}[$i];
        if ( $onefile->{'Regmergefile'} ) { push(@regmergeparticles, $onefile->{'Regmergefile'}); }
        else { installer::exiter::exit_program("ERROR: Could not find entry for \"Regmergefile\" in $onefile->{'gid'}!", "collect_all_regmerge_particles"); }
    }

    return \@regmergeparticles;
}

################################################################
# Collecting all source pathes of the regmerge particles
################################################################

sub get_all_source_pathes
{
    my ($regmergeparticles, $includepatharrayref) = @_;

    my @regmergeparticles = ();

    for ( my $i = 0; $i <= $#{$regmergeparticles}; $i++ )
    {
        my $filename = ${$regmergeparticles}[$i];

        my $fileref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$filename, $includepatharrayref, 1);
        if ( $$fileref eq "" ) { installer::exiter::exit_program("ERROR: Could not find file $filename for merging the StarRegistry!", "get_all_source_pathes"); }

        push(@regmergeparticles, $$fileref);
    }

    return \@regmergeparticles;
}

################################################################
# Merging the rdb files into the StarRegistry database
################################################################

sub merge_files
{
    my ($regmergefile, $databasefile, $registerfiles, $databasedir, $allvariableshashref) = @_;

    my $databasesource = $databasefile->{'sourcepath'};
    my $databasename = $databasefile->{'Name'};
    my $databasedest = $databasedir . $installer::globals::separator . $databasename;

    installer::systemactions::copy_one_file($databasesource, $databasedest);
    $databasefile->{'sourcepath'} = $databasedest;  # new sourcepath for the StarRegistry file

    # One call for every merge particle. This is only possible, if there are only a few merge particles.

    my $prefix = $databasefile->{'NativeServicesURLPrefix'};
    # TODO: "NativeServicesURLPrefix" or "JavaServicesURLPrefix"

    my $error_occured = 0;

    for ( my $i = 0; $i <= $#{$registerfiles}; $i++ )
    {
        my $registerfile = $databasedir . $installer::globals::separator . $i . ".tmp";
        open (IN, '<', $registerfiles->[$i]) or $error_occured = 1;
        open (OUT, '>', $registerfile) or $error_occured = 1;
        while (<IN>)
        {
            s/^ComponentName=/ComponentName=$prefix/;
            print OUT $_ or $error_occured = 1;
        }
        close IN or $error_occured = 1;
        close OUT or $error_occured = 1;

        my $systemcall = $regmergefile . " -v " . $databasedest . " " . $registerfile . " 2\>\&1 |";

        my @regmergeoutput = ();

        open (REG, "$systemcall");
        while (<REG>) {push(@regmergeoutput, $_); }
        close (REG);

        my $returnvalue = $?;   # $? contains the return value of the systemcall

        my $infoline = "Systemcall: $systemcall\n";
        push( @installer::globals::logfileinfo, $infoline);

        for ( my $j = 0; $j <= $#regmergeoutput; $j++ ) { push( @installer::globals::logfileinfo, "$regmergeoutput[$j]"); }

        if ($returnvalue)
        {
            $infoline = "ERROR: $systemcall\n";
            push( @installer::globals::logfileinfo, $infoline);
            $error_occured = 1;
        }
        else
        {
            $infoline = "SUCCESS: $systemcall\n";
            push( @installer::globals::logfileinfo, $infoline);
        }
    }

    return $error_occured;
}

################################################################
# Expanding the registry database files by merging rdb files
# into this registry database files.
################################################################

sub merge_registration_files
{
    my ($filesarrayref, $includepatharrayref, $languagestringref, $allvariableshashref) = @_;

    installer::logger::include_header_into_logfile("Creating starregistry databases:");

    # Test if there is something to do. At least one file has to have the content:
    # Regmergefile = "mydatabasepart.rdb";

    my $regmergefiles = collect_all_regmergefiles($filesarrayref);

    if ( $#{$regmergefiles} > -1 )  # not empty -> at least one regmerge file
    {
        # prepare registration

        my $regmergefile = get_regmerge_file($includepatharrayref); # searching for regmerge (regcomplazy.exe)

        my $databasegids = collect_all_database_gids($regmergefiles);

        # iterating over all database gids

        my $regmergeerror = 0;

        for ( my $i = 0; $i <= $#{$databasegids}; $i++ )
        {
            $databasegid = ${$databasegids}[$i];

            # my $databasedirname = "starregistryrdb"; <- not unique!
            my $databasedirname = $databasegid . "_rdb"; # <- unique!
            my $databasedir = installer::systemactions::create_directories($databasedirname, $languagestringref);
            push(@installer::globals::removedirs, $databasedir);

            my $databasefile = get_database_file($databasegid, $filesarrayref);
            my $databaseregisterfiles = collect_all_files_for_one_registry($regmergefiles, $databasegid);

            if ( $#{$databaseregisterfiles} > -1 )  # not empty -> at least one regmerge file
            {
                my $regmergeparticles = collect_all_regmerge_particles($databaseregisterfiles);
                $regmergeparticles = get_all_source_pathes($regmergeparticles, $includepatharrayref);
                my $oneregmergeerror = merge_files($regmergefile, $databasefile, $regmergeparticles, $databasedir, $allvariableshashref);
                if ($oneregmergeerror) { $regmergeerror = 1; }
            }
        }

        if ( $regmergeerror ) { installer::exiter::exit_program("ERROR: regmerge !", "merge_registration_files"); }

    }
}

1;
