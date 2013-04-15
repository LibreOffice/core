#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

package installer::filelists;

use installer::files;
use installer::globals;
use installer::logger;
use installer::pathanalyzer;

sub resolve_filelist_flag
{
    my ($files, $outdir) = @_;
    my @newfiles = ();

    foreach my $file (@{$files})
    {
        my $is_filelist = 0;
        if ($file->{'Styles'})
        {
            if ($file->{'Styles'} =~ /\bFILELIST\b/)
            {
                $is_filelist = 1;
            }
        }

        if ($is_filelist)
        {
            my $filelist_path = $file->{'sourcepath'};
            my $filelist = read_filelist($filelist_path);
            if (@{$filelist})
            {
                my $destination = $file->{'destination'};
                installer::pathanalyzer::get_path_from_fullqualifiedname(\$destination);

                foreach my $path (@{$filelist})
                {
                    if ((index $path, $outdir) != 0)
                    {
                        installer::logger::print_error("file '$path' is not in '$outdir'");
                    }
                    if (!-f $path)
                    {
                        installer::logger::print_error("file '$path' does not exist");
                    }

                    my $subpath = substr $path, ((length $outdir) + 1); # drop separator too

                    my %newfile = ();
                    %newfile = %{$file};
                    $newfile{'Name'} = $subpath;
                    $newfile{'sourcepath'} = $path;
                    $newfile{'destination'} = $destination . $subpath;
                    $newfile{'filelistname'} = $file->{'Name'};
                    $newfile{'filelistpath'} = $file->{'sourcepath'};

                    push @newfiles, \%newfile;
                }
            }
            else
            {
                installer::logger::print_message("filelist $filelist_path is empty\n");
            }
        }
        else # not a filelist, just pass the current file over
        {
            push @newfiles, $file;
        }
    }

    return \@newfiles;
}

sub read_filelist
{
    my ($path) = @_;
    my $content = installer::files::read_file($path);
    my @filelist = ();

    foreach my $line (@{$content})
    {
        chomp $line;
        foreach my $file (split /\s+/, $line)
        {
            if ($file ne "")
            {
                push @filelist, $file;
            }
        }
    }

    return \@filelist;
}

1;

# vim: set expandtab shiftwidth=4 tabstop=4:
