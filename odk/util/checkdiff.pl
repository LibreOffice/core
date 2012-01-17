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



#
# checkit - a perl script to check the output of 'dircmp'
#

$return = 0;
$possible_error = 0;
$possible_error_descript = "";
while( <STDIN> )
{
    if( /^diff/ )
    {
        $possible_error = 1;
        $possible_error_descript = $_;
    }
    elsif( /^Binary/ )
    {
        print STDERR "ERROR : $_";
        $return++;
    }
    elsif( /^[0-9]/ && $possible_error == 1 )
    {
        print STDERR "ERROR : diff ".$possible_error_descript;
        $return++;
        $possible_error = 0;
    }
    else
    {
        $possible_error = 0;
    }
}
if( $return != 0 )
{
    unlink "$ARGV[0]";
}
exit $return;
