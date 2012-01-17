:
eval 'exec perl -wS $0 ${1+"$@"}'
   if 0;
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



$ENV{'LC_MESSAGES'} = 'C';

$Dest = pop(@ARGV) || die "No destination to copy to";

$cc = $ENV{'CC'} || die "No CC environment set";

if ($Dest =~ /--help/ || @ARGV < 1) {
    print "Syntax:\n  gcc-instlib <library-in-libpath ...> <destination-dir>\n";
    exit (0);
}

%SrcAndDest = ();

foreach $File (@ARGV) {
    my $string;
    my $normalized_file = $File;
    $normalized_file =~ s/\.so\.\d+/.so/;
    open (GCCOut, "LANGUAGE=C LC_ALL=C $cc -print-file-name=$normalized_file|") || die "Failed to exec $cc -print-file-name=$normalized_file $!";
    $string=<GCCOut>;
    chomp ($string);
    $SrcAndDest{$string} = "$Dest/$File";
    close (GCCOut);
}

while (($Src, $FullDest) = each %SrcAndDest) {
    printf "copy $Src to $FullDest\n";
    system ("/bin/cp $Src $FullDest") && die "copy failed: $!";
}


foreach $File (@ARGV) {
    #https://bugzilla.redhat.com/show_bug.cgi?id=149465
    printf "unprelinking $Dest/$File\n";
    #If it's already unprelinked .i.e. no .gnu.prelink_undo section, that's fine
    #If prelink is not installed, it's massively unlikely that it's prelinked
    system ("prelink -u $Dest/$File > /dev/null 2>&1");
}

exit (0);
