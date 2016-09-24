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
