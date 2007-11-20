#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.17 $
#
#   last change: $Author: ihi $ $Date: 2007-11-20 19:25:42 $
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
PRJ=..

PRJNAME=sal
TARGET=whole_sal_qa

# LLA: irrelevant
# ENABLE_EXCEPTIONS=TRUE
# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# BEGIN ------------------------------------------------------------
# END --------------------------------------------------------------


# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

.IF "$(runtests)"!=""
ALLTAR : test_all
.ENDIF

# OTHER STUFF ------------------------------------------------------
# test : test_all

# start tests with consideration of libs2test.txt with 'dmake test'
# run through all tests directories (from libs2test.txt) and try to start all tests
# use 'dmake test TESTOPT="buildall"'

# ALLTAR
test_all: 
        @echo ----------------------------------------------------------
        @echo - start sal unit tests
        @echo ----------------------------------------------------------
        $(PERL) buildall.pl $(TESTOPT)
