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

package installer::followme;

use File::Spec;
use installer::exiter;
use installer::files;
use installer::globals;
use installer::logger;
use installer::systemactions;

####################################################
# Creating a file, that contains all information
# to create a follow up process.
####################################################

sub save_followme_info
{
    my ($finalinstalldir, $includepatharrayref, $allvariableshashref, $downloadname, $languagestringref, $languagesarrayref, $current_install_number, $loggingdir, $installlogdir) = @_;

    my $downloadinfofilename = $installer::globals::logfilename;
    $downloadinfofilename =~ s/log_/follow_me_/;

    # Creating directory
    my $downloadinfodir = installer::systemactions::create_directory_next_to_directory($finalinstalldir, "follow_me");

    my @filecontent = ();

    push(@filecontent, "finalinstalldir: $finalinstalldir\n");
    push(@filecontent, "downloadname: $downloadname\n");
    push(@filecontent, "currentinstallnumber: $current_install_number\n");
    push(@filecontent, "loggingdir: $loggingdir\n");
    push(@filecontent, "installlogdir: $installlogdir\n");
    push(@filecontent, "languagestring: $$languagestringref\n");
    foreach my $element ( @{$languagesarrayref} ) { push(@filecontent, "languagesarray: $element\n"); }
    foreach my $path ( @{$includepatharrayref} ) { push(@filecontent, "includepatharray: $path"); }
    foreach my $key ( sort keys %{$allvariableshashref} ) { push(@filecontent, "allvariableshash: $key : $allvariableshashref->{$key}\n"); }
    push(@filecontent, "globals:updatepack: $installer::globals::updatepack\n");
    push(@filecontent, "globals:added_english: $installer::globals::added_english\n");
    push(@filecontent, "globals:iswindowsbuild: $installer::globals::iswindowsbuild\n");
    push(@filecontent, "globals:issolarisbuild: $installer::globals::issolarisbuild\n");
    push(@filecontent, "globals:issolarispkgbuild: $installer::globals::issolarispkgbuild\n");
    push(@filecontent, "globals:issolarissparcbuild: $installer::globals::issolarissparcbuild\n");
    push(@filecontent, "globals:issolarisx86build: $installer::globals::issolarisx86build\n");
    push(@filecontent, "globals:isfreebsdpkgbuild: $installer::globals::isfreebsdpkgbuild\n");
    push(@filecontent, "globals:islinuxbuild: $installer::globals::islinuxbuild\n");
    push(@filecontent, "globals:isrpmbuild: $installer::globals::isrpmbuild\n");
    push(@filecontent, "globals:islinuxintelrpmbuild: $installer::globals::islinuxintelrpmbuild\n");
    push(@filecontent, "globals:islinuxppcrpmbuild: $installer::globals::islinuxppcrpmbuild\n");
    push(@filecontent, "globals:islinuxx86_64rpmbuild: $installer::globals::islinuxx86_64rpmbuild\n");
    push(@filecontent, "globals:isdebbuild: $installer::globals::isdebbuild\n");
    push(@filecontent, "globals:islinuxinteldebbuild: $installer::globals::islinuxinteldebbuild\n");
    push(@filecontent, "globals:islinuxppcdebbuild: $installer::globals::islinuxppcdebbuild\n");
    push(@filecontent, "globals:islinuxx86_64debbuild: $installer::globals::islinuxx86_64debbuild\n");
    push(@filecontent, "globals:issolaris: $installer::globals::issolaris\n");
    push(@filecontent, "globals:islinux: $installer::globals::islinux\n");
    push(@filecontent, "globals:unpackpath: $installer::globals::unpackpath\n");
    push(@filecontent, "globals:idttemplatepath: $installer::globals::idttemplatepath\n");
    push(@filecontent, "globals:idtlanguagepath: $installer::globals::idtlanguagepath\n");
    push(@filecontent, "globals:logfilename: $installer::globals::logfilename\n");
    push(@filecontent, "globals:product: $installer::globals::product\n");
    push(@filecontent, "globals:patch: $installer::globals::patch\n");
    push(@filecontent, "globals:languagepack: $installer::globals::languagepack\n");
    push(@filecontent, "globals:helppack: $installer::globals::helppack\n");
    push(@filecontent, "globals:installertypedir: $installer::globals::installertypedir\n");
    push(@filecontent, "globals:max_lang_length: $installer::globals::max_lang_length\n");
    push(@filecontent, "globals:compiler: $installer::globals::compiler\n");
    push(@filecontent, "globals:product: $installer::globals::product\n");
    push(@filecontent, "globals:minor: $installer::globals::minor\n");
    push(@filecontent, "globals:lastminor: $installer::globals::lastminor\n");

    # Saving file
    installer::files::save_file($downloadinfodir . $installer::globals::separator . $downloadinfofilename, \@filecontent);
    installer::logger::print_message( "... creating \"follow me\" info file $downloadinfofilename.\n" );
}

1;
