#*************************************************************************
#
#   $RCSfile: macosxotoolhelper.pm,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: vg $ $Date: 2008-03-18 12:57:55 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2008 by Sun Microsystems, Inc.
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

package macosxotoolhelper;
require Exporter;
our @ISA = Exporter;
our @EXPORT = otoolD;

sub otoolD($) {
    my ($file) = @_;
    my $call = "otool -D $file";
    open(IN, "-|", $call) or die "cannot $call";
    my $line = <IN>;
    $line =~ /^$file:\n$/ or
        die "unexpected otool -D output (\"$line\", expecting \"$file:\")";
    $line = <IN> or die "unexpected otool -D output";
    <IN> == undef or die "unexpected otool -D output";
    close(IN);
    return $line;
}
