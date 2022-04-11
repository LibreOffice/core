#!/usr/bin/env perl
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

sub loadData()
{
    open (IN, "<custom-shapes.log");

    my %sources;

    while (<IN>)
    {
        if (/==csdata== /)
        {
            if (/shape name: '/)
            {
                chop;
                s/.*shape name: '([^']+)'.*/$1/;
                $name = $_;
            }
            else
            {
                if (/==csdata== begin/)
                {
                    $inside = true;
                    @code = ();
                }
                else
                {
                    if (/==csdata== end/)
                    {
                        s/^  <\/([^>]+)>/$1/;
                        undef $inside;
                        $sources{$name} = [ @code ];
                    }
                }
            }
        }
        else
        {
            if ($inside)
            {
                push @code, $_;
            }
        }
    }

    close (IN);

    return \%sources;
}

sub generateData
{
    my $sources = shift;
    open (OUT, ">oox-drawingml-cs-presets");

    foreach $shape (sort(keys %$sources))
    {
        printf OUT "/* %s */\n", $shape;
        print OUT @{$sources->{$shape}};
    }

    close OUT;
}

generateData(loadData());

# vim:set ft=perl shiftwidth=4 softtabstop=4 expandtab: #
