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

package installer::patch::FileOperations;

use File::Basename;
use File::Copy;
use IO::Compress::Bzip2;
use IO::Uncompress::Bunzip2;

my $CompressionMethod = "bzip2";


=head1 NAME

    package installer::patch::FileOperations - Class for collecting, checking and executing file operations.

=cut


sub new ($)
{
    my ($class) = (@_);

    my $self = {
        'operations' => []
    };
    bless($self, $class);

    return $self;
}




sub AddCopyOperation ($$$)
{
    my ($self, $source_name, $target_name) = @_;

    push
        @{$self->{'operations'}},
        [
            'copy',
            $source_name,
            $target_name
        ];
}




sub AddMakeDirectoryOperation ($$)
{
    my ($self, $path) = @_;

    push
        @{$self->{'operations'}},
        [
            'mkdir',
            $path
        ];
}




sub AddCompressOperation ($$)
{
    my ($self, $filename) = @_;

    push
        @{$self->{'operations'}},
        [
            'compress',
            $filename
        ];
}




sub AddUncompressOperation ($$$)
{
    my ($self, $source_name, $target_name) = @_;

    push
        @{$self->{'operations'}},
        [
            'uncompress',
            $source_name,
            $target_name
        ];
}




sub Check ($)
{
    my ($self) = @_;

    # Keep track of which directories or files would be created to check if
    # operations that depend on these files will succeed.
    my %files = ();
    my %directories = ();

    my @error_messages = ();
    foreach my $operation (@{$self->{'operations'}})
    {
        my $command = $operation->[0];

        if ($command eq "copy")
        {
            my ($source_name, $destination_name) = ($operation->[1], $operation->[2]);
            if ( ! -f $source_name)
            {
                push @error_messages, sprintf("%s is not a regular file and can not be copied", $source_name);
            }
            my $destination_path = dirname($destination_name);
            if ( ! -d $destination_path && ! defined $directories{$destination_path})
            {
                push @error_messages, sprintf("destination path %s does not exist", $destination_path);
            }
            if ( -f $destination_name)
            {
                # The destination file already exists. We have to overwrite it.
                if ( ! -w $destination_name)
                {
                    push @error_messges, sprintf("destination file %s exists but can not be overwritten", $destination_name);
                }
            }
            $files{$destination_name} = 1;
        }
        elsif ($command eq "mkdir")
        {
            my $path = $operation->[1];
            if ( -d $path)
            {
                # Directory already exists.  That is OK, the mkdir command will be silently ignored.
            }
            else
            {
                $directories{$path} = 1;
            }
        }
        elsif ($command eq "compress")
        {
            my $filename = $operation->[1];
            if ( ! -f $filename && ! defined $files{$filename})
            {
                # File does not exist and will not be created by an earlier operation.
                push @error_messages, sprintf("file %s does not exist and can not be compressed", $filename);
            }
        }
        elsif ($command eq "uncompress")
        {
            my ($source_filename, $destination_filename) = ($operation->[1], $operation->[2]);
            if ($CompressionMethod eq "bzip2")
            {
                $source_filename .= ".bz2";
            }
            if ( ! -f $source_filename && ! defined $files{$source_filename})
            {
                # File does not exist and will not be created by an earlier operation.
                push @error_messages, sprintf("file %s does not exist and can not be decompressed", $source_filename);
            }
            if ( -f $destination_filename && ! -w $destination_filename)
            {
                # Destination file aleady exists but can not be replaced.
                push @error_messages, sprintf("compress destination file %s exists but can not be replaced", $destination_filename);
            }
        }
        else
        {
            push @error_messages, sprintf("unknown operation %s", $command);
        }
    }

    return @error_messages;
}




sub CheckAndExecute ($)
{
    my ($self) = @_;

    my @error_messages = $self->Check();
    if (scalar @error_messages > 0)
    {
        $installer::logger::Lang->printf("can not execute all operations:\n");
        for my $message (@error_messages)
        {
            $installer::logger::Lang->printf("ERROR: %s\n", $message);
        }
        return 0;
    }
    else
    {
        return $self->Execute();
    }
}




sub Execute ($)
{
    my ($self) = @_;

    foreach my $operation (@{$self->{'operations'}})
    {
        my $command = $operation->[0];

        if ($command eq "copy")
        {
            my ($source_name, $destination_name) = ($operation->[1], $operation->[2]);
            $installer::logger::Lang->printf("copy from %s\n    to %s\n", $source_name, $destination_name);
            if ( ! $DryRun)
            {
                my $result = copy($source_name, $destination_name);
                if ( ! $result)
                {
                    $installer::logger::Lang->printf("ERROR: copying from %s to %s failed",
                        $source_name, $destination_name);
                }
            }
        }
        elsif ($command eq "mkdir")
        {
            my $path = $operation->[1];
            if ( -d $path)
            {
                # Path exists already. Do nothing.
            }
            else
            {
                $installer::logger::Lang->printf("creating directory %s\n", $path);
                if ( ! $DryRun)
                {
                    if (File::Path::make_path($path, {'mode' => 0775}) == 0)
                    {
                        $installer::logger::Lang->printf("could not create directory %s\n", $path);
                    }
                }
            }
        }
        elsif ($command eq "compress")
        {
            my $filename = $operation->[1];
            $installer::logger::Lang->printf("compressing %s\n", $filename);
            if ( ! $DryRun)
            {
                my $result = 0;
                if ($CompressionMethod eq "bzip2")
                {
                    $result = IO::Compress::Bzip2::bzip2($filename => $filename.".bz2");
                }
                if ($result == 0)
                {
                    $installer::logger::Lang->printf("ERROR: could not compress %s\n", $filename);
                }
                else
                {
                    unlink($filename);
                }
            }
        }
        elsif ($command eq "uncompress")
        {
            my ($source_name, $destination_name) = ($operation->[1], $operation->[2]);
            if ($CompressionMethod eq "bzip2")
            {
                $source_name .= ".bz2";
            }
            $installer::logger::Lang->printf("uncompressing %s to %s\n", $source_name, $destination_name);

            my $destination_base_name = basename($destination_name);

            if ( ! $DryRun)
            {
                my $result = 0;
                if ($CompressionMethod eq "bzip2")
                {
                    $result = IO::Uncompress::Bunzip2::bunzip2($source_name => $destination_name);
                }
                if ($result == 0)
                {
                    $installer::logger::Lang->printf("ERROR: failed to extract content of '%s' from '%s'\n",
                        $destination_name, $source_name);
                    return 0;
                }
            }
        }

        else
        {
            die "unknown operation $command\n";
        }
    }

    return 1;
}



sub GetOperationCount ($)
{
    my ($self) = @_;
    return scalar @{$self->{'operations'}};
}


1;
