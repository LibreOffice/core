#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.12 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 19:08:48 $
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
# Quick and dirty way to run all tests in sub-directories; probably only works
# on Unix.
# Can be removed once tests are included in regular builds.

all .PHONY:
    cd com/sun/star/comp/bridgefactory && dmake
    cd com/sun/star/comp/connections && dmake
    cd com/sun/star/lib/uno/bridges/java_remote && dmake
    cd com/sun/star/lib/uno/environments/java && dmake
    cd com/sun/star/lib/uno/environments/remote && dmake
    cd com/sun/star/lib/uno/protocols/urp && dmake
    cd com/sun/star/lib/util && dmake
    cd com/sun/star/uno && dmake
