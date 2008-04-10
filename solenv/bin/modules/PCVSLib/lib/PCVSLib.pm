#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: PCVSLib.pm,v $
#
# $Revision: 1.3 $
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

package PCVSLib;

use 5.006;
use strict;
use warnings;

require Exporter;

our @ISA = qw(Exporter);

use PCVSLib::Client;
use PCVSLib::Command;
use PCVSLib::Connection;
use PCVSLib::Credentials;
use PCVSLib::Directory;
use PCVSLib::Entry;
use PCVSLib::Event;
use PCVSLib::EventHandler;
use PCVSLib::Request;
use PCVSLib::Response;

# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.

# This allows declaration   use PCVSLib ':all';
# If you do not need this, moving things directly into @EXPORT or @EXPORT_OK
# will save memory.
our %EXPORT_TAGS = ( 'all' => [ qw(

) ] );

our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );

our @EXPORT = qw(

);

our $VERSION = '0.02';


# Preloaded methods go here.

1;
__END__
# Below is stub documentation for your module. You'd better edit it!

=head1 NAME

PCVSLib - pure perl implementation of the CVS client protocol

=head1 SYNOPSIS

  use PCVSLib;

=head1 DESCRIPTION

PCVSLib is a pure perl implementation of the CVS client protocol.
It currently supports only a subset of of the specified requests
and responses, just enough to support the OpenOffice.org CWS tools.
If it ever is complete and stable enough for general use, it might
be worthwhile to submit it to CPAN.

=head2 EXPORT

None by default.



=head1 SEE ALSO

The cvsclient protocol documentation which comes with the CVS
source distribution

=head1 AUTHOR

Jens-Heiner Rechtien E<lt>hr@openoffice.orgE<gt>

=head1 COPYRIGHT AND LICENSE

GNU Lesser General Public License version 3
Copyright (C) 2008 Sun Microsystems, Inc.

=cut
# vim: set ts=4 shiftwidth=4 expandtab syntax=perl:
