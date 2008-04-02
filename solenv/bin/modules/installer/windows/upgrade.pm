#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: upgrade.pm,v $
#
#   $Revision: 1.13 $
#
#   last change: $Author: kz $ $Date: 2008-04-02 12:21:33 $
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

package installer::windows::upgrade;

use installer::files;
use installer::globals;
use installer::windows::idtglobal;

####################################################################################
# Creating the file Upgrade.idt dynamically
# Content:
# UpgradeCode VersionMin VersionMax Language Attributes Remove ActionProperty
####################################################################################

sub create_upgrade_table
{
    my ($basedir, $allvariableshashref) = @_;

    my @upgradetable = ();

    # fix for problematic OOo 1.9 versions
    my $include_ooo_fix = 0;
    my $ooomaxnew = "";
    if (($installer::globals::product =~ /OpenOffice/i ) && ( ! ( $installer::globals::product =~ /SDK/i )))
    {
        $include_ooo_fix = 1;
        $ooomaxnew = "34.0.0";
    }

    installer::windows::idtglobal::write_idt_header(\@upgradetable, "upgrade");

    my $newline = $installer::globals::upgradecode . "\t" . "\t" . $installer::globals::msiproductversion . "\t" . "\t" . "1" . "\t" . "\t" . "OLDPRODUCTS" . "\n";
    push(@upgradetable, $newline);

    if (( ! $installer::globals::patch ) && ( ! $installer::globals::languagepack ))
    {
        # preventing downgrading
        $newline = $installer::globals::upgradecode . "\t" . $installer::globals::msiproductversion . "\t" . $ooomaxnew . "\t" . "\t" . "2" . "\t" . "\t" . "NEWPRODUCTS" . "\n";
        push(@upgradetable, $newline);

        $newline = $installer::globals::upgradecode . "\t" . $installer::globals::msiproductversion . "\t" . $ooomaxnew . "\t" . "\t" . "258" . "\t" . "\t" . "SAMEPRODUCTS" . "\n";
        push(@upgradetable, $newline);

        if ( $include_ooo_fix )
        {
            $newline = $installer::globals::upgradecode . "\t" . "35.0.0" . "\t" . "36.0.0" . "\t" . "\t" . "1" . "\t" . "\t" . "OLDPRODUCTS2" . "\n";
            push(@upgradetable, $newline);
        }

        if ( $allvariableshashref->{'PATCHUPGRADECODE'} )
        {
            $newline = $allvariableshashref->{'PATCHUPGRADECODE'} . "\t" . "\t" . $installer::globals::msiproductversion . "\t" . "\t" . "1" . "\t" . "\t" . "OLDPRODUCTSPATCH" . "\n";
            push(@upgradetable, $newline);

            $newline = $allvariableshashref->{'PATCHUPGRADECODE'} . "\t" . $installer::globals::msiproductversion . "\t" . "\t" . "\t" . "2" . "\t" . "\t" . "NEWPRODUCTSPATCH" . "\n";
            push(@upgradetable, $newline);

            $newline = $allvariableshashref->{'PATCHUPGRADECODE'} . "\t" . $installer::globals::msiproductversion . "\t" . "\t" . "\t" . "258" . "\t" . "\t" . "SAMEPRODUCTSPATCH" . "\n";
            push(@upgradetable, $newline);
        }

        # also searching for the beta

        if ( $allvariableshashref->{'BETAUPGRADECODE'} )
        {
            $newline = $allvariableshashref->{'BETAUPGRADECODE'} . "\t" . "1.0" . "\t" . "\t" . "\t" . "1" . "\t" . "\t" . "BETAPRODUCTS" . "\n";
            push(@upgradetable, $newline);
        }

        # also searching for the stub

        if ( $allvariableshashref->{'STUBUPGRADECODE'} )
        {
            $newline = $allvariableshashref->{'STUBUPGRADECODE'} . "\t" . "1.0" . "\t" . "\t" . "\t" . "1" . "\t" . "\t" . "STUBPRODUCTS" . "\n";
            push(@upgradetable, $newline);
        }
    }

    # No upgrade for Beta versions!

    if (( $allvariableshashref->{'PRODUCTEXTENSION'} eq "Beta" ) && ( ! $installer::globals::patch ) && ( ! $installer::globals::languagepack ))
    {
        @upgradetable = ();
        installer::windows::idtglobal::write_idt_header(\@upgradetable, "upgrade");
        my $infoline = "Beta product -> empty Upgrade table\n";
        push(@installer::globals::logfileinfo, $infoline);
    }

    # Saving the file

    my $upgradetablename = $basedir . $installer::globals::separator . "Upgrade.idt";
    installer::files::save_file($upgradetablename ,\@upgradetable);
    my $infoline = "Created idt file: $upgradetablename\n";
    push(@installer::globals::logfileinfo, $infoline);
}

1;
