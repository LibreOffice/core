#*************************************************************************
#
#   $RCSfile: createfolder.pm,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: kz $ $Date: 2004-06-11 18:18:51 $
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

package installer::windows::createfolder;

use installer::existence;
use installer::files;
use installer::globals;
use installer::windows::idtglobal;

##############################################################
# Returning directory for createfolder table.
##############################################################

sub get_createfolder_directory
{
    my ($onedir) = @_;

    my $uniquename = $onedir->{'uniquename'};

    return $uniquename;
}

##############################################################
# Returning component for createfolder table.
##############################################################

sub get_createfolder_component
{
    my ($onedir, $filesref) = @_;

    # Directories do not belong to a module.
    # Therefore they can only belong to the root module and
    # will be added to a component at the root module.
    # All directories will be added to the component
    # containing the file "gid_File_Lib_Vcl"

    my $vclgid = "gid_File_Lib_Vcl";

    if ($installer::globals::product =~ /ada/i ) { $vclgid = "gid_File_Adabas"; }

    my $onefile = installer::existence::get_specified_file($filesref, $vclgid);

    return $onefile->{'componentname'};
}

####################################################################################
# Creating the file CreateFo.idt dynamically for creation of empty directories
# Content:
# Directory_ Component_
####################################################################################

sub create_createfolder_table
{
    my ($dirref, $filesref, $basedir) = @_;

    my @createfoldertable = ();

    my $infoline;

    installer::windows::idtglobal::write_idt_header(\@createfoldertable, "createfolder");

    for ( my $i = 0; $i <= $#{$dirref}; $i++ )
    {
        my $onedir = ${$dirref}[$i];

        my $styles = "";

        if ( $onedir->{'Styles'} ) { $styles = $onedir->{'Styles'}; }

        if ( $styles =~ /\bCREATE\b/ )
        {
            my %directory = ();

            $directory{'Directory_'} = get_createfolder_directory($onedir);
            $directory{'Component_'} = get_createfolder_component($onedir, $filesref);

            my $oneline = $directory{'Directory_'} . "\t" . $directory{'Component_'} . "\n";

            push(@createfoldertable, $oneline);
        }
    }

    # Saving the file

    my $createfoldertablename = $basedir . $installer::globals::separator . "CreateFo.idt";
    installer::files::save_file($createfoldertablename ,\@createfoldertable);
    $infoline = "Created idt file: $createfoldertablename\n";
    push(@installer::globals::logfileinfo, $infoline);

}

1;