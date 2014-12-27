#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

package installer::filelists;

use File::stat;

use installer::files;
use installer::globals;
use installer::logger;
use installer::pathanalyzer;

sub resolve_filelist_flag
{
    my ($files, $links, $outdir) = @_;
    my @newfiles = ();

    foreach my $file (@{$files})
    {
        my $is_filelist = 0;
        my $use_internal_rights = 0;
        if ($file->{'Styles'})
        {
            if ($file->{'Styles'} =~ /\bFILELIST\b/)
            {
                $is_filelist = 1;
            }
            if ($file->{'Styles'} =~ /\bUSE_INTERNAL_RIGHTS\b/ && !$installer::globals::iswin)
            {
                $use_internal_rights = 1;
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
                    my $is_symlink = 0;

                    if ((index $path, $outdir) != 0)
                    {
                        installer::logger::print_error("file '$path' is not in '$outdir'");
                    }
                    if (-l $path)
                    {
                        $is_symlink = 1;
                    }
                    else
                    {
                        if (!-e $path)
                        {
                            installer::logger::print_error("file '$path' does not exist");
                        }
                    }

                    my $subpath = substr $path, ((length $outdir) + 1); # drop separator too

                    my %newfile = ();
                    %newfile = %{$file};
                    $newfile{'Name'} = $subpath;
                    $newfile{'sourcepath'} = $path;
                    $newfile{'destination'} = $destination . $subpath;
                    $newfile{'filelistname'} = $file->{'Name'};
                    $newfile{'filelistpath'} = $file->{'sourcepath'};

                    if ($is_symlink)
                    {
                        # FIXME: for symlinks destination is mangled later in
                        # get_Destination_Directory_For_Item_From_Directorylist
                        $newfile{'DoNotMessWithSymlinks'} = 1;
                        $newfile{'Target'} = readlink($path);
                        push ( @{$links}, \%newfile );
                    }
                    else
                    {
                        if ($use_internal_rights)
                        {
                            my $st = stat($path);
                            if ($st)
                            {
                                $newfile{'UnixRights'} = sprintf("%o", $st->mode & 0777);
                            }
                        }

                        push @newfiles, \%newfile;
                    }
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

    return (\@newfiles, $links);
}

sub read_filelist
{
    my ($path) = @_;
    my $content = installer::files::read_file($path);
    my @filelist = ();

    # split on space, but only if followed by / (don't split within a filename)
    my $splitRE = qr!\s+(?=/)!;
    # filelist on win have C:/cygwin style however - also reading dos-file under
    # cygwin retains \r\n - so chomp below still leaves \r to strip in the RE
    $splitRE    = qr!\s+(?:$|(?=[A-Z]:/))! if ($installer::globals::os eq "WNT");

    foreach my $line (@{$content})
    {
        chomp $line;
        foreach my $file (split $splitRE, $line)
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
