:
    eval 'exec perl -S $0 ${1+"$@"}'
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




my ( $srcfile, $destfile ) = @ARGV;
my ( @srclines, @destlines );
my $dest_existing = 0;
@destlines = ();

usage() if ( ! defined $srcfile || ! defined $destfile);

open(SRCFILE, "$srcfile") or die "ERROR: Can't open $srcfile\n";
@srclines = <SRCFILE>;
close SRCFILE;

if ( -f $destfile ) {
    open(DESTFILE, "$destfile") or die "ERROR: Can't open $destfile\n";
    @destlines = <DESTFILE>;
    close DESTFILE;
    $dest_existing = 1;
}

if ( ! check_if_lists_equal(\@srclines,  \@destlines) ) {
    print STDERR "Updating \"$destfile\".\n";
    unlink "$destfile" or die "ERROR: Can't remove old $destfile\n" if ( $dest_existing );
    rename "$srcfile", "$destfile" or die "ERROR: Can't rename $srcfile to $destfile\n";
} else {
    print STDERR "\"$destfile\" unchanged.\n";
}

sub check_if_lists_equal
{
    my  ( $srclist_ref, $destlist_ref ) = @_;
    my @srclist  = @{ $srclist_ref };
    my @destlist  = @{ $destlist_ref };
    return  0 if ( $#srclist != $#destlist );
    for ( my $i = 0; $i  < $#srclist; $i++ ) {
        return 0 if  ( $srclist[$i] ne $destlist[$i]);
    }
    return  1;
}

sub usage
{
    print STDERR "Usage: diffmv sourcefile destfile\n";
    print STDERR "Do move diffing file only\n";
    exit 1;
}

