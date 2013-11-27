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

package installer::patch::Tools;

=head1 NAME

    package installer::patch::Tools - Collection of functions that don't fit anywhere else

=cut




=head2 CygpathToWindows ($path)

    Convert the given path with the 'cygpath' command into Windows format.  Quote backslashes.

=cut
sub CygpathToWindows($)
{
    my ($path) = @_;
    my $windows_path = qx(cygpath -w "$path");
    $windows_path =~ s/(^\s+|\s+$)//g;
    $windows_path =~ s/\\/\\\\/g;
    return $windows_path;
}

1;
