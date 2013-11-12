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


my $Unpacker = "/c/Program\\ Files/7-Zip/7z.exe";

=head1 NAME

    package installer::patch::InstallationSet  -  Functions for handling installation sets

=head1 DESCRIPTION

    This package contains functions for unpacking the .exe files that
    are created by the NSIS installer creator and the .cab files in
    the installation sets.

=cut

sub UnpackExe ($$)
{
    my ($filename, $destination_path) = @_;

    $installer::logger::Info->printf("unpacking installation set to '%s'\n", $destination_path);

    # Unpack to a temporary path and change its name to the destination path
    # only when the unpacking has completed successfully.
    my $temporary_destination_path = $destination_path . ".tmp";
    File::Path::make_path($temporary_destination_path);

    my $windows_filename = installer::patch::Tools::CygpathToWindows($filename);
    my $windows_destination_path = installer::patch::Tools::CygpathToWindows($temporary_destination_path);
    my $command = join(" ",
        $Unpacker,
        "x", "-o".$windows_destination_path,
        $windows_filename);
    my $result = qx($command);

    # Check the existence of the .cab files.
    my $cab_filename = File::Spec->catfile($temporary_destination_path, "openoffice1.cab");
    if ( ! -f $cab_filename)
    {
        installer::logger::PrintError("cab file '%s' was not extracted from installation set\n", $cab_filename);
        return 0;
    }
    if (rename($temporary_destination_path, $destination_path) == 0)
    {
        installer::logger::PrintError("can not rename temporary extraction directory\n");
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
    my $file_to_directory_map = $msi->GetFileToDirectoryMap();

    # Step 2
    # Unpack the .cab file to a temporary path.
    my $temporary_destination_path = $destination_path . ".tmp";
    if ( -d $temporary_destination_path)
    {
        # Temporary directory already exists => cab file has already been unpacked (flat), nothing to do.
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
    my $count = 0;
    foreach my $file_row (@{$file_table->GetAllRows()})
    {
        my $unique_name = $file_row->GetValue('File');
        my $directory_full_names = $file_to_directory_map->{$unique_name};
        my ($source_full_name, $target_full_name) = @$directory_full_names;

        my $flat_filename = File::Spec->catfile($temporary_destination_path, $unique_name);
        my $dir_path = File::Spec->catfile($destination_path, $source_full_name);
        my $dir_filename = File::Spec->catfile($dir_path, $unique_name);

        printf("%d: making path %s and copying %s to %s\n",
            $count,
            $dir_path,
            $unique_name,
            $dir_filename);
        File::Path::make_path($dir_path);
        File::Copy::move($flat_filename, $dir_filename);

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
    my $temporary_destination_path = $destination_path . ".tmp";
    File::Path::make_path($temporary_destination_path);
    my $windows_cab_filename = installer::patch::Tools::CygpathToWindows($cab_filename);
    my $windows_destination_path = installer::patch::Tools::CygpathToWindows($temporary_destination_path);
    my $command = join(" ",
        $Unpacker,
        "x", "-o".$windows_destination_path,
        $windows_cab_filename,
        "-y");
    printf("running command '%s'\n", $command);
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
    printf("extraction done                               \n");

    rename($temporary_destination_path, $destination_path)
        || installer::logger::PrintError(
            "can not rename the temporary directory '%s' to '%s'\n",
            $temporary_destination_path,
            $destination_path);
}




=head GetUnpackedMsiPath ($version, $language, $package_format, $product)

    Convenience function that returns where a downloadable installation set is extracted to.

=cut
sub GetUnpackedMsiPath ($$$$)
{
    my ($version, $language, $package_format, $product) = @_;

    return File::Spec->catfile(
        GetUnpackedPath($version, $language, $package_format, $product),
        "unpacked_msi");
}




=head GetUnpackedCabPath ($version, $language, $package_format, $product)

    Convenience function that returns where a cab file is extracted
    (with injected directory structure from the msi file) to.

=cut
sub GetUnpackedCabPath ($$$$)
{
    my ($version, $language, $package_format, $product) = @_;

    return File::Spec->catfile(
        GetUnpackedPath($version, $language, $package_format, $product),
        "unpacked_cab");
}




=head2 GetUnpackedPath($version, $language, $package_format, $product)

    Internal function for creating paths to where archives are unpacked.

=cut
sub GetUnpackedPath ($$$$)
{
    my ($version, $language, $package_format, $product) = @_;

    return File::Spec->catfile(
        $ENV{'SRC_ROOT'},
        "instsetoo_native",
        $ENV{'INPATH'},
        $product,
        $package_format,
        installer::patch::Version::ArrayToDirectoryName(installer::patch::Version::StringToNumberArray($version)),
        $language);
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
    $installer::logger::Info->printf("    expected size is %d\n", $filesize);
    my $temporary_filename = $filename . ".part";
    my $resume_size = 0;
    if ( -f $temporary_filename)
    {
        $resume_size = -s $temporary_filename;
        $installer::logger::Info->printf(" trying to resume at %d/%d bytes\n", $resume_size, $filesize);
    }

    # Prepare checksum.
    my $checksum = undef;
    my $checksum_type = $release_data->{'checksum-type'};
    my $checksum_value = $release_data->{'checksum-value'};
    my $digest = undef;
    if ($checksum_type eq "sha256")
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
    open my $out, ">>$temporary_filename";
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
                $digest->reset();
                close $out;
                open $out, ">$temporary_filename";
                binmode($out);
            }
            my($response,$agent,$h,$data)=@_;
            print $out $data;
            $digest->add($data);
            $bytes_read += length($data);
            printf("read %*d / %d  %d%%  \r",
                length($filesize),
                $bytes_read,
                $filesize,
                $bytes_read*100/$filesize);
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
        if ($digest->hexdigest() eq $checksum_value)
        {
            $installer::logger::Info->PrintInfo("download was successfull\n");
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
        if ($release_item->{'checksum-type'} eq 'sha256')
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

1;
