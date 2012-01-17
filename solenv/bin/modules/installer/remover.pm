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



package installer::remover;

use installer::globals;

############################################
# Remover
############################################

sub remove_leading_and_ending_whitespaces
{
    my ( $stringref ) = @_;

    $$stringref =~ s/^\s*//;
    $$stringref =~ s/\s*$//;
}

sub remove_leading_and_ending_quotationmarks
{
    my ( $stringref ) = @_;

    $$stringref =~ s/^\s*\"//;
    $$stringref =~ s/\"\s*$//;
}

sub remove_leading_and_ending_slashes
{
    my ( $stringref ) = @_;

    $$stringref =~ s/^\s*\///g;
    $$stringref =~ s/\/\s*$//g;
}

sub remove_ending_slashes
{
    my ( $stringref ) = @_;

    $$stringref =~ s/\/\s*$//g;
}

sub remove_leading_and_ending_pathseparators
{
    my ( $stringref ) = @_;

    $$stringref =~ s/^\s*\Q$installer::globals::separator\E//;
    $$stringref =~ s/\Q$installer::globals::separator\E\s*$//;
}

sub remove_ending_pathseparator
{
    my ( $stringref ) = @_;

    $$stringref =~ s/\Q$installer::globals::separator\E\s*$//;
}

1;
