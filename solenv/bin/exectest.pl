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



$#ARGV >= 1
    or die "Usage: $0 <input file>|-SUCCESS|-FAILURE <command> <arguments...>";
if ($ARGV[0] eq "-SUCCESS")
{
    $expect = "SUCCESS";
    $input = 0;
}
elsif ($ARGV[0] eq "-FAILURE")
{
    $expect = "FAILURE";
    $input = 0;
}
else
{
    open INPUT, $ARGV[0] or die "cannot open $ARGV[0]: $!";
    $input = 1;
}
shift @ARGV;
$failed = 0;
$open = 0;
while (1) {
    $eof = $input ? eof INPUT : $open;
    $in = <INPUT> if $input && !$eof;
    if (!$input || $eof
        || $in =~ /^EXPECT (SUCCESS|FAILURE|\d+)( "([^"]*)")?:\n$/)
    {
        if ($open)
        {
            close PIPE;
            if ($? % 256 == 0)
            {
                $exit = $? / 256;
                $ok = $expect eq "SUCCESS" ? $exit == 0
                    : $expect eq "FAILURE" ? $exit != 0 : $exit == $expect;
            }
            else
            {
                $exit = "signal";
                $ok = 0;
            }
            print "\"$title\", " if defined $title;
            print "expected $expect, got $exit ($?): ";
            if ($ok)
            {
                print "ok\n";
            }
            else
            {
                print "FAILED!\n";
                $failed = 1;
            }
        }
        last if $eof;
        $expect = $1 if $input;
        if (defined $3)
        {
            $title = $3;
        }
        else
        {
            undef $title;
        }
        open PIPE, "| @ARGV" or die "cannot start process: $!";
        $open = 1;
    }
    elsif ($open && $input)
    {
        print PIPE $in or die "cannot write to pipe: $!";
    }
}
exit $failed;
