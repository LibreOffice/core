#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: font.pm,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 09:16:47 $
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

package installer::windows::font;

use installer::files;
use installer::globals;
use installer::windows::idtglobal;


#################################################################################
# Creating the file Font.idt dynamically
# Content:
# File_ FontTitle
#################################################################################

sub create_font_table
{
    my ($filesref, $basedir) = @_;

    my @fonttable = ();

    installer::windows::idtglobal::write_idt_header(\@fonttable, "font");

    for ( my $i = 0; $i <= $#{$filesref}; $i++ )
    {
        my $onefile = ${$filesref}[$i];
        my $styles = "";

        if ( $onefile->{'Styles'} ) { $styles = $onefile->{'Styles'}; }

        if ( $styles =~ /\bFONT\b/ )
        {
            my %font = ();

            $font{'File_'} = $onefile->{'uniquename'};
            # $font{'FontTitle'} = $onefile->{'FontName'};  # results in a warning during validation
            $font{'FontTitle'} = "";

            my $oneline = $font{'File_'} . "\t" . $font{'FontTitle'} . "\n";

            push(@fonttable, $oneline);
        }
    }

    # Saving the file

    my $fonttablename = $basedir . $installer::globals::separator . "Font.idt";
    installer::files::save_file($fonttablename ,\@fonttable);
    my $infoline = "Created idt file: $fonttablename\n";
    push(@installer::globals::logfileinfo, $infoline);

}

1;