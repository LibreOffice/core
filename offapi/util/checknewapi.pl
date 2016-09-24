#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#
#
# checknewapi - a perl script to check for new API's
# using two outputs from regview and dump the interscetion
# of new types
#

if($#ARGV != 3)
{
    die "usage: checknewapi <new_type_library> <reference_type_library> <buildinfodescr> <fullpath_regview>\n";
}

-e "$ARGV[0]" || die "ERROR: type library \"$ARGV[0]\" does not exist\n";
-e "$ARGV[1]" || die "ERROR: reference type library \"$ARGV[1]\" does not exist\n";
-e "$ARGV[3]" || die "ERROR: invalid path to the regview tool \"$ARGV[3]\", please specify the full qualified path\n";

# debug flag
$DEBUG = 0;

$main::buildinfo = "$ARGV[2]";
$main::regview = "$ARGV[3]";
%{$main::reftypes} = ();
%{$main::currenttypes} = ();
%{$main::removedtypes} = ();

open ( FILEIN, "$main::regview \"$ARGV[0]\" |" ) || die "could not use content of current typelibrary \"$ARGV[0]\", regview doesn't work\n";

if ($DEBUG == 1)
{
    open( CURRENT, ">current_types.txt" ) || die "\nERROR: could not open current_types.txt for writing";
}

$first = 1;
$linebefore = "";
$published = 0;
$typeclass = "";
while (<FILEIN>)
{
    if ($first == 0)
    {
        if ( $linebefore =~ m#type class: published (.+)# )
        {
            $published = 1;
            $typeclass = $1;
        } elsif ( $linebefore =~ m#type class: (.+)# )
        {
            $published = 0;
            $typeclass = $1;
        } else
        {
            $published = 0;
            $typeclass = "";
        }
    } else
    {
        $first = 0;
    }

    if ( (!$typeclass eq "") && ($_ =~ m# *type name: \"([^\[.]+)\"#) )
    {
        if ($DEBUG == 1)
        {
            print CURRENT "$1\n";
        }
        if ( ! exists $main::currenttypes->{$1} )
        {
            $main::currenttypes->{$1} = { PUBLISHED => $published,
                                          TYPECLASS => $typeclass,
                                          COUNT => 1 };
#           print "### $main::currenttypes->{$1}->{PUBLISHED} $main::currenttypes->{$1}->{TYPECLASS} $main::currenttypes->{$1}->{COUNT}\n";
        }
    }
    $linebefore = $_;
}
close( FILEIN );
close( CURRENT );

open ( FILEIN, "$main::regview \"$ARGV[1]\" |" ) || die "could not use content of reference type library \"$ARGV[1]\", regview doesn't work\n";

if ($DEBUG == 1)
{
    open( REFERENCE, ">reference_types.txt" ) || die "\nERROR: could not open reference_types.txt for writing";
}

# reset variables
$first = 1;
$linebefore = "";
$published = 0;
$typeclass = "";
while (<FILEIN>)
{
    if ($first == 0)
    {
        if ( $linebefore =~ m#type class: published (.+)# )
        {
            $published = 1;
            $typeclass = $1;
        } elsif ( $linebefore =~ m#type class: (.+)# )
        {
            $published = 0;
            $typeclass = $1;
        } else
        {
            $published = 0;
            $typeclass = "";
        }
    } else
    {
        $first = 0;
    }

    if ( (!$typeclass eq "") && ($_ =~ m# *type name: \"([^\[.]+)\"#) )
    {
        if ($DEBUG == 1)
        {
            print REFERENCE "$1\n";
        }
        if ( ! exists $main::reftypes->{$1} )
        {
            $main::reftypes->{$1}++;

            if ( exists $main::currenttypes->{$1} )
            {
                $main::currenttypes->{$1}->{COUNT}++;
#               print "###### $main::currenttypes->{$1}->{PUBLISHED} $main::currenttypes->{$1}->{TYPECLASS} $main::currenttypes->{$1}->{COUNT}\n";
            } else
            {
                if ($published == 0)
                {
                    $main::removedtypes->{$1} = { PUBLISHED => $published,
                                                  TYPECLASS => $typeclass };
                } else
                {
                    print "ERROR: type $1 is only in reference type library, this can't be happen\n";
                }
            }
        }
    }
    $linebefore = $_;
}
close( FILEIN );
close( REFERENCE );


@typekeys = keys %{$main::currenttypes};
$allunotypes = $#typekeys+1;
$newunotypes = 0;
$newpublished = 0;
$draftscount = 0;
$draftspublished = 0;
foreach $i ( sort @typekeys )
{
    if ( $main::currenttypes->{$i}->{COUNT} == 1 &&
         !("$main::currenttypes->{$i}->{TYPECLASS}" eq "module"))
    {
        $newunotypes++;
        my $t = $i;
        $t =~ s#/#\.#go;
        if ($main::currenttypes->{$i}->{PUBLISHED} == 1)
        {
            print "published ";
            $newpublished++;
        }
        if ( $t =~ m#drafts\.com.+#)
        {
            $draftscount++;
            if ($main::currenttypes->{$i}->{PUBLISHED} == 1)
            {
                $draftspublished++;
            }
        }
        print "$main::currenttypes->{$i}->{TYPECLASS} = $t\n";
    }
}

# count removed not yet published types
$removednotpublished = 0;

@removedtypekeys = keys %{$main::removedtypes};
foreach $i ( sort @removedtypekeys )
{
    $removednotpublished++;
    my $t = $i;
    $t =~ s#/#\.#go;
    print "removed not yet published $main::currenttypes->{$i}->{TYPECLASS} = $t\n";
}

print "\n=======================================================\n\n";
print "Summary [last check for $main::buildinfo]:\n\n";
print "Number of UNO types = $allunotypes\n";
print "Number of new UNO types = $newunotypes\n";
print "New and published types = $newpublished\n";
print "New and draft types = $draftscount\n";
print "New, draft and published = $draftspublished\n";
print "Removed and not published = $removednotpublished\n";

exit 0;
