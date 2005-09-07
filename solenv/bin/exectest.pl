#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: exectest.pl,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 22:08:24 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************

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
