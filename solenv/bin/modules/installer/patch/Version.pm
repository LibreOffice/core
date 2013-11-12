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

package installer::patch::Version;


=head1 NAME

    package installer::patch::Version - Functions for handling version numbers.

=cut



# We handle version numbers that consist of three parts: major, minor and micro version number.
my $VersionPartCount = 3;



=head StringToNumberArray($version_string)

    Convert a version string (where the individual parts are separated by '.') into an array of three numbers.
    Missing numbers are filled with 0.

    Returns an array with three elements (major, minor, micro).
=cut
sub StringToNumberArray ($)
{
    my ($version_string) = @_;

    my @version_parts = split(/\./, $version_string);
    while (scalar @version_parts < $VersionPartCount)
    {
        push @version_parts, "0";
    }
    return @version_parts;
}




=head ArrayToDirectoryName (@)

    Return a directory name (without any path) for the given array of version numbers.

=cut
sub ArrayToDirectoryName (@)
{
    return "v-".join("-", @_);
}





1;
