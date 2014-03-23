#**************************************************************
#
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#
#**************************************************************

package installer::patch::InstallationSet;

use installer::patch::Tools;
use installer::patch::Version;
use installer::logger;

use strict;

# Call Get7Zip() to get access to the filename of the 7z executable.
my $SevenZip = undef;


=head1 NAME

    package installer::patch::InstallationSet  -  Functions for handling installation sets

=head1 DESCRIPTION

    This package contains functions for unpacking the .exe files that
    are created by the NSIS installer creator and the .cab files in
    the installation sets.

=cut




=head2 Detect7ZipOnWindows ()

    7Zip seems to be the only program able to unpack an NSIS installer.
    Search for it.

=cut

sub Detect7ZipOnWindows ()
{
    # Use 'reg query' to read registry entry from Windows registry.
    my $registry_key = "HKEY_CURRENT_USER\\\\Software\\\\7-Zip";
    my $registry_value_name = "Path";
    my $command = sprintf("reg query %s /v %s", $registry_key, $registry_value_name);
    my $response = qx($command);

    # Process the response.
    my $path_to_7zip = undef;
    if ($response =~ /\s+REG_SZ\s+([^\r\n]*)/m)
    {
        $path_to_7zip = $1;
    }

    # If that failed, then make an educated guess.
    if ( ! defined $path_to_7zip)
    {
        $path_to_7zip = "c:\\Program Files\\7-Zip\\";
    }

    # Check if the executable exists and is, well, executable.
    return undef unless -d $path_to_7zip;
    my $fullname = File::Spec->catfile($path_to_7zip, "7z.exe");
    return undef unless -f $fullname;
    return undef unless -x $fullname;

    return $fullname;
}




sub Get7Zip ()
{
    if ( ! defined $SevenZip)
    {
        if ($ENV{'OS'} eq "WNT")
        {
            $SevenZip = Detect7ZipOnWindows();
        }
        if ( ! defined $SevenZip)
        {
            # Use an empty string to avoid repeated (and failing) detections of a missing 7z.
            $SevenZip = "";
        }
    }

    return $SevenZip eq "" ? undef : $SevenZip;
}




sub UnpackExe ($$)
{
    my ($filename, $destination_path) = @_;

    $installer::logger::Info->printf("unpacking installation set to '%s'\n", $destination_path);

    # Unpack to a temporary path and change its name to the destination path
    # only when the unpacking has completed successfully.
    File::Path::make_path($destination_path);

    my $windows_filename = installer::patch::Tools::ToEscapedWindowsPath($filename);
    my $windows_destination_path = installer::patch::Tools::ToEscapedWindowsPath($destination_path);
    my $command = join(" ",
        "\"".Get7Zip()."\"",
        "x",
        "-y",
        "-o".$windows_destination_path,
        $windows_filename);
    my $result = qx($command);
    if ( ! $result)
    {
        installer::exiter::exit_program(
            "ERROR: can not unpack downloadable installation set: ".$!,
            "installer::patch::InstallationSet::UnpackExe");
    }

    # Check the existence of the .cab files.
    my $cab_filename = File::Spec->catfile($destination_path, "openoffice1.cab");
    if ( ! -f $cab_filename)
    {
        installer::logger::PrintError("cab file '%s' was not extracted from installation set\n", $cab_filename);
        return 0;
    }
    return 1;
}




=head2 UnpackCab($cab_filename, $destination_path)

    Unpacking the cabinet file inside an .exe installation set is a
    three step process because there is no directory information stored
    inside the cab file.  This has to be taken from the 'File' and
    'Directory' tables in the .msi file.

    1. Setup the directory structure of all files in the cab from the 'File' and 'Directory' tables in the msi.

    2. Unpack the cab file.

    3. Move the files to their destination directories.

=cut
sub UnpackCab ($$$)
{
    my ($cab_filename, $msi, $destination_path) = @_;

    # Step 1
    # Extract the directory structure from the 'File' and 'Directory' tables in the given msi.
    $installer::logger::Info->printf("setting up directory tree\n");
    my $file_table = $msi->GetTable("File");
    my $file_map = $msi->GetFileMap();

    # Step 2
    # Unpack the .cab file to a temporary path.
    my $temporary_destination_path = $destination_path . ".tmp";
    if ( -d $temporary_destination_path)
    {
        # Temporary directory already exists => cab file has already been unpacked (flat), nothing to do.
        printf("%s exists\n", $temporary_destination_path);
        $installer::logger::Info->printf("cab file has already been unpacked to flat structure\n");
    }
    else
    {
        UnpackCabFlat($cab_filename, $temporary_destination_path, $file_table);
    }

    # Step 3
    # Move the files to their destinations.
    File::Path::make_path($destination_path);
    $installer::logger::Info->printf("moving files to their directories\n");
    my $directory_map = $msi->GetDirectoryMap();
    my $office_menu_folder_name = $directory_map->{'INSTALLLOCATION'}->{'target_long_name'};
    my $count = 0;
    foreach my $file_row (@{$file_table->GetAllRows()})
    {
        my $unique_name = $file_row->GetValue('File');
        my $file_item = $file_map->{$unique_name};
        my $directory_item = $file_item->{'directory'};
        my $long_file_name = $file_item->{'long_name'};
        my $full_name = $directory_item->{'full_source_long_name'};
        # Strip away the leading OfficeMenuFolder part.
        $full_name =~ s/^$office_menu_folder_name\///;
        my $flat_filename = File::Spec->catfile($temporary_destination_path, $unique_name);
        my $dir_path = File::Spec->catfile($destination_path, $full_name);
        my $dir_filename = File::Spec->catfile($dir_path, $long_file_name);

        if ( ! -d $dir_path)
        {
            File::Path::make_path($dir_path);
        }

        $installer::logger::Lang->printf("moving %s to %s\n", $flat_filename, $dir_filename);
        File::Copy::move($flat_filename, $dir_filename)
            || die("can not move file ".$flat_filename.":".$!);

        ++$count;
    }

    # Cleanup.  Remove the temporary directory.  It should be empty by now.
    rmdir($temporary_destination_path);
}




=head2 UnpackCabFlat ($cab_filename, $destination_path, $file_table)

    Unpack the flat file structure of the $cab_filename to $destination_path.

    In order to detect and handle an incomplete (arborted) previous
    extraction, the cab file is unpacked to a temprorary directory
    that after successful extraction is renamed to $destination_path.

=cut
sub UnpackCabFlat ($$$)
{
    my ($cab_filename, $destination_path, $file_table) = @_;

    # Unpack the .cab file to a temporary path (note that
    # $destination_path may alreay bee a temporary path). Using a
    # second one prevents the lengthy flat unpacking to be repeated
    # when another step fails.

    $installer::logger::Info->printf("unpacking cab file\n");
    File::Path::make_path($destination_path);
    my $windows_cab_filename = installer::patch::Tools::ToEscapedWindowsPath($cab_filename);
    my $windows_destination_path = installer::patch::Tools::ToEscapedWindowsPath($destination_path);
    my $command = join(" ",
        "\"".Get7Zip()."\"",
        "x", "-o".$windows_destination_path,
        $windows_cab_filename,
        "-y");
    open my $cmd, $command."|";
    my $extraction_count = 0;
    my $file_count = $file_table->GetRowCount();
    while (<$cmd>)
    {
        my $message = $_;
        chomp($message);
        ++$extraction_count;
        printf("%4d/%4d  %3.2f%%   \r",
            $extraction_count,
            $file_count,
            $extraction_count*100/$file_count);
    }
    close $cmd;
}




=head GetUnpackedExePath ($version, $is_current_version, $language, $package_format, $product)

    Convenience function that returns where a downloadable installation set is extracted to.

=cut
sub GetUnpackedExePath ($$$$$)
{
    my ($version, $is_current_version, $language, $package_format, $product) = @_;

    my $path = GetUnpackedPath($version, $is_current_version, $language, $package_format, $product);
    return File::Spec->catfile($path, "unpacked");
}




=head GetUnpackedCabPath ($version, $is_current_version, $language, $package_format, $product)

    Convenience function that returns where a cab file is extracted
    (with injected directory structure from the msi file) to.

=cut
sub GetUnpackedCabPath ($$$$$)
{
    my ($version, $is_current_version, $language, $package_format, $product) = @_;

    my $path = GetUnpackedPath($version, $is_current_version, $language, $package_format, $product);
    return File::Spec->catfile($path, "unpacked");
}




=head2 GetUnpackedPath($version, $is_current_version, $language, $package_format, $product)

    Internal function for creating paths to where archives are unpacked.

=cut
sub GetUnpackedPath ($$$$$)
{
    my ($version, $is_current_version, $language, $package_format, $product) = @_;

    return File::Spec->catfile(
        $ENV{'SRC_ROOT'},
        "instsetoo_native",
        $ENV{'INPATH'},
        $product,
        $package_format,
        installer::patch::Version::ArrayToDirectoryName(
            installer::patch::Version::StringToNumberArray($version)),
        installer::languages::get_normalized_language($language));
}




sub GetMsiFilename ($$)
{
    my ($path, $version) = @_;

    my $no_dot_version = installer::patch::Version::ArrayToNoDotName(
        installer::patch::Version::StringToNumberArray(
            $version));
    return File::Spec->catfile(
        $path,
        "openoffice" . $no_dot_version . ".msi");
}




sub GetCabFilename ($$)
{
    my ($path, $version) = @_;

    return File::Spec->catfile(
        $path,
        "openoffice1.cab");
}




=head2 Download($language, $release_data, $filename)

    Download an installation set to $filename.  The URL for the
    download is taken from $release_data, a snippet from the
    instsetoo_native/data/releases.xml file.

=cut
sub Download ($$$)
{
    my ($language, $release_data, $filename) = @_;

    my $url = $release_data->{'URL'};
    $release_data->{'URL'} =~ /^(.*)\/([^\/]+)$/;
    my ($location, $basename) = ($1,$2);

    $installer::logger::Info->printf("downloading %s\n", $basename);
    $installer::logger::Info->printf("    from '%s'\n", $location);
    my $filesize = $release_data->{'file-size'};
    if (defined $filesize)
    {
        $installer::logger::Info->printf("    expected size is %d\n", $filesize);
    }
    else
    {
        $installer::logger::Info->printf("    file size is not yet known\n");
    }
    my $temporary_filename = $filename . ".part";
    my $resume_size = 0;

    # Prepare checksum.
    my $checksum = undef;
    my $checksum_type = $release_data->{'checksum-type'};
    my $checksum_value = $release_data->{'checksum-value'};
    my $digest = undef;
    if ( ! defined $checksum_value)
    {
        # No checksum available.  Skip test.
    }
    elsif ($checksum_type eq "sha256")
    {
        $digest = Digest->new("SHA-256");
    }
    elsif ($checksum_type eq "md5")
    {
        $digest = Digest->new("md5");
    }
    else
    {
        installer::logger::PrintError(
            "checksum type %s is not supported.  Supported checksum types are: sha256,md5\n",
            $checksum_type);
        return 0;
    }

    # Download the extension.
    open my $out, ">$temporary_filename";
    binmode($out);

    my $mode = $|;
    my $handle = select STDOUT;
    $| = 1;
    select $handle;

    my $agent = LWP::UserAgent->new();
    $agent->timeout(120);
    $agent->show_progress(0);
    my $last_was_redirect = 0;
    my $bytes_read = 0;
    $agent->add_handler('response_redirect'
        => sub{
            $last_was_redirect = 1;
            return;
        });
    $agent->add_handler('response_data'
        => sub{
            if ($last_was_redirect)
            {
                $last_was_redirect = 0;
                # Throw away the data we got so far.
                $digest->reset() if defined $digest;
                close $out;
                open $out, ">$temporary_filename";
                binmode($out);
            }
            my($response,$agent,$h,$data)=@_;
            print $out $data;
            $digest->add($data) if defined $digest;
            $bytes_read += length($data);
            if (defined $filesize)
            {
                printf("read %*d / %d  %d%%  \r",
                    length($filesize),
                    $bytes_read,
                    $filesize,
                    $bytes_read*100/$filesize);
            }
            else
            {
                printf("read %6.2f MB\r", $bytes_read/(1024.0*1024.0));
            }
            });
    my $response;
    if ($resume_size > 0)
    {
        $response = $agent->get($url, 'Range' => "bytes=$resume_size-");
    }
    else
    {
        $response = $agent->get($url);
    }
    close $out;

    $handle = select STDOUT;
    $| = $mode;
    select $handle;

    $installer::logger::Info->print("                                        \r");

    if ($response->is_success())
    {
        if ( ! defined $digest
            || $digest->hexdigest() eq $checksum_value)
        {
            $installer::logger::Info->print("download was successfull\n");
            if ( ! rename($temporary_filename, $filename))
            {
                installer::logger::PrintError("can not rename '%s' to '%s'\n", $temporary_filename, $filename);
                return 0;
            }
            else
            {
                return 1;
            }
        }
        else
        {
            installer::logger::PrintError("%s checksum is wrong\n", $checksum_type);
            return 0;
        }
    }
    else
    {
        installer::logger::PrintError("there was a download error\n");
        return 0;
    }
}




=head2 ProvideDownloadSet ($version, $language, $package_format)

    Download an installation set when it is not yet present to
    $ENV{'TARFILE_LOCATION'}.  Verify the downloaded file with the
    checksum that is extracted from the
    instsetoo_native/data/releases.xml file.

=cut
sub ProvideDownloadSet ($$$)
{
    my ($version, $language, $package_format) = @_;

    my $release_item = installer::patch::ReleasesList::Instance()->{$version}->{$package_format}->{$language};
    return undef unless defined $release_item;

    # Get basename of installation set from URL.
    $release_item->{'URL'} =~ /^(.*)\/([^\/]+)$/;
    my ($location, $basename) = ($1,$2);

    # Is the installation set already present in ext_sources/ ?
    my $need_download = 0;
    my $ext_sources_filename = File::Spec->catfile(
        $ENV{'TARFILE_LOCATION'},
        $basename);
    if ( ! -f $ext_sources_filename)
    {
        $installer::logger::Info->printf("download set is not in ext_sources/ (%s)\n", $ext_sources_filename);
        $need_download = 1;
    }
    else
    {
        $installer::logger::Info->printf("download set exists at '%s'\n", $ext_sources_filename);
        if (defined $release_item->{'checksum-value'}
            && $release_item->{'checksum-type'} eq 'sha256')
        {
            $installer::logger::Info->printf("checking SHA256 checksum\n");
            my $digest = Digest->new("SHA-256");
            open my $in, "<", $ext_sources_filename;
            $digest->addfile($in);
            close $in;
            if ($digest->hexdigest() ne $release_item->{'checksum-value'})
            {
                $installer::logger::Info->printf("    mismatch\n", $ext_sources_filename);
                $need_download = 1;
            }
            else
            {
                $installer::logger::Info->printf("    match\n");
            }
        }
    }

    if ($need_download)
    {
        if ( ! installer::patch::InstallationSet::Download(
            $language,
            $release_item,
            $ext_sources_filename))
        {
            return 0;
        }
        if ( ! -f $ext_sources_filename)
        {
            $installer::logger::Info->printf("download set could not be downloaded\n");
            return 0;
        }
    }

    return $ext_sources_filename;
}




sub ProvideUnpackedExe ($$$$$)
{
    my ($version, $is_current_version, $language, $package_format, $product_name) = @_;

    # Check if the exe has already been unpacked.
    my $unpacked_exe_path = installer::patch::InstallationSet::GetUnpackedExePath(
        $version,
        $is_current_version,
        $language,
        $package_format,
        $product_name);
    my $unpacked_exe_flag_filename = File::Spec->catfile($unpacked_exe_path, "__exe_is_unpacked");
    my $exe_is_unpacked = -f $unpacked_exe_flag_filename;

    if ($exe_is_unpacked)
    {
        # Yes, exe has already been unpacked.  There is nothing more to do.
        $installer::logger::Info->printf("downloadable installation set has already been unpacked to\n");
        $installer::logger::Info->printf("    %s\n", $unpacked_exe_path);
        return 1;
    }
    elsif ($is_current_version)
    {
        # For the current version the exe is created from the unpacked
        # content and both are expected to be already present.

        # In order to have the .cab and its unpacked content in one
        # directory and don't interfere with the creation of regular
        # installation sets, we copy the unpacked .exe into a separate
        # directory.

        my $original_path = File::Spec->catfile(
            $ENV{'SRC_ROOT'},
            "instsetoo_native",
            $ENV{'INPATH'},
            $product_name,
            $package_format,
            "install",
            $language);
        $installer::logger::Info->printf("creating a copy\n");
        $installer::logger::Info->printf("    of %s\n", $original_path);
        $installer::logger::Info->printf("    at %s\n", $unpacked_exe_path);
        File::Path::make_path($unpacked_exe_path) unless -d $unpacked_exe_path;
    my ($file_count,$directory_count) = CopyRecursive($original_path, $unpacked_exe_path);
    return 0 if ( ! defined $file_count);
        $installer::logger::Info->printf("    copied %d files in %d directories\n",
        $file_count,
        $directory_count);

        installer::patch::Tools::touch($unpacked_exe_flag_filename);

        return 1;
    }
    else
    {
        # No, we have to unpack the exe.

        # Provide the exe.
        my $filename = installer::patch::InstallationSet::ProvideDownloadSet(
            $version,
            $language,
            $package_format);

        # Unpack it.
        if (defined $filename)
        {
            if (installer::patch::InstallationSet::UnpackExe($filename, $unpacked_exe_path))
            {
                $installer::logger::Info->printf("downloadable installation set has been unpacked to\n");
                $installer::logger::Info->printf("    %s\n", $unpacked_exe_path);

                installer::patch::Tools::touch($unpacked_exe_flag_filename);

                return 1;
            }
        }
        else
        {
            installer::logger::PrintError("could not provide .exe installation set at '%s'\n", $filename);
        }
    }

    return 0;
}




sub CopyRecursive ($$)
{
    my ($source_path, $destination_path) = @_;

    return (undef,undef) unless -d $source_path;

    my @todo = ([$source_path, $destination_path]);
    my $file_count = 0;
    my $directory_count = 0;
    while (scalar @todo > 0)
    {
    my ($source,$destination) = @{shift @todo};

    next if ! -d $source;
    File::Path::make_path($destination);
    ++$directory_count;

    # Read list of files in the current source directory.
    opendir( my $dir, $source);
    my @files = readdir $dir;
    closedir $dir;

    # Copy all files and push all directories to @todo.
    foreach my $file (@files)
    {
        next if $file =~ /^\.+$/;

        my $source_file = File::Spec->catfile($source, $file);
        my $destination_file = File::Spec->catfile($destination, $file);
        if ( -f $source_file)
        {
        File::Copy::copy($source_file, $destination_file);
        ++$file_count;
        }
        elsif ( -d $source_file)
        {
        push @todo, [$source_file, $destination_file];
        }
    }
    }

    return ($file_count, $directory_count);
}




sub CheckLocalCopy ($$$$)
{
    my ($version, $language, $package_format, $product_name) = @_;

    # Compare creation times of the original .msi and its copy.

    my $original_path = File::Spec->catfile(
        $ENV{'SRC_ROOT'},
        "instsetoo_native",
        $ENV{'INPATH'},
        $product_name,
        $package_format,
        "install",
        $language);

    my $copy_path = installer::patch::InstallationSet::GetUnpackedExePath(
        $version,
        1,
        $language,
        $package_format,
        $product_name);

    my $msi_basename = "openoffice"
        . installer::patch::Version::ArrayToNoDotName(
            installer::patch::Version::StringToNumberArray($version))
        . ".msi";

    my $original_msi_filename = File::Spec->catfile($original_path, $msi_basename);
    my $copied_msi_filename = File::Spec->catfile($copy_path, $msi_basename);

    my @original_msi_stats = stat($original_msi_filename);
    my @copied_msi_stats = stat($copied_msi_filename);
    my $original_msi_mtime = $original_msi_stats[9];
    my $copied_msi_mtime = $copied_msi_stats[9];

    if (defined $original_msi_mtime
        && defined $copied_msi_mtime
        && $original_msi_mtime > $copied_msi_mtime)
    {
        # The installation set is newer than its copy.
        # Remove the copy.
        $installer::logger::Info->printf(
            "removing copy of installation set (version %s) because it is out of date\n",
            $version);
        File::Path::remove_tree($copy_path);
    }
}




=head2 ProvideUnpackedCab

    1a. Make sure that a downloadable installation set is present.
    1b. or that a freshly built installation set (packed and unpacked is present)
    2. Unpack the downloadable installation set
    3. Unpack the .cab file.

    The 'Provide' in the function name means that any step that has
    already been made is not made again.

=cut
sub ProvideUnpackedCab ($$$$$)
{
    my ($version, $is_current_version, $language, $package_format, $product_name) = @_;

    if ($is_current_version)
    {
        # For creating patches we maintain a copy of the unpacked .exe.  Make sure that that is updated when
        # a new installation set has been built.
        CheckLocalCopy($version, $language, $package_format, $product_name);
    }

    # Check if the cab file has already been unpacked.
    my $unpacked_cab_path = installer::patch::InstallationSet::GetUnpackedCabPath(
        $version,
        $is_current_version,
        $language,
        $package_format,
        $product_name);
    my $unpacked_cab_flag_filename = File::Spec->catfile($unpacked_cab_path, "__cab_is_unpacked");
    my $cab_is_unpacked = -f $unpacked_cab_flag_filename;

    if ($cab_is_unpacked)
    {
        # Yes. Cab was already unpacked. There is nothing more to do.
        $installer::logger::Info->printf("cab has already been unpacked to\n");
        $installer::logger::Info->printf("    %s\n", $unpacked_cab_path);

        return 1;
    }
    else
    {
        # Make sure that the exe is unpacked and the cab file exists.
        ProvideUnpackedExe($version, $is_current_version, $language, $package_format, $product_name);

        # Unpack the cab file.
        my $unpacked_exe_path = installer::patch::InstallationSet::GetUnpackedExePath(
                $version,
                $is_current_version,
                $language,
                $package_format,
                $product_name);
        my $msi = new installer::patch::Msi(
                installer::patch::InstallationSet::GetMsiFilename($unpacked_exe_path, $version),
                $version,
                $is_current_version,
                $language,
                $product_name);

        my $cab_filename = installer::patch::InstallationSet::GetCabFilename(
            $unpacked_exe_path,
            $version);
        if ( ! -f $cab_filename)
        {
             # Cab file does not exist.
            installer::logger::PrintError(
                "could not find .cab file at '%s'.  Extraction of .exe seems to have failed.\n",
                $cab_filename);
            return 0;
        }

        if (installer::patch::InstallationSet::UnpackCab(
            $cab_filename,
            $msi,
            $unpacked_cab_path))
        {
            $installer::logger::Info->printf("unpacked cab file '%s'\n", $cab_filename);
            $installer::logger::Info->printf("    to '%s'\n", $unpacked_cab_path);

            installer::patch::Tools::touch($unpacked_cab_flag_filename);

            return 1;
        }
        else
        {
            return 0;
        }
    }
}
1;
