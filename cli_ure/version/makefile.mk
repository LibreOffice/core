#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.6 $
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

PRJ = ..
PRJNAME = cli_ure

# for dummy
TARGET = cliureversion.mk

.INCLUDE : settings.mk
.INCLUDE : $(PRJ)$/util$/makefile.pmk

.INCLUDE : $(PRJ)$/util$/target.pmk
.INCLUDE : target.mk

ALLTAR : \
    $(BIN)$/cliureversion.mk 

#	INCVERSION

#always deliver a cliureversion.mk. It is needed for the packing process even for all other
#platforms. Therefore BUILD_FOR_CLI is not used here 
$(BIN)$/cliureversion.mk: version.txt 
    $(GNUCOPY) $< $@


#disabled because of #67482
#Create the config file that is used with the policy assembly 
#only if new published types have been added, the cliureversion.mk is written.
#INCVERSION .PHONY: version.txt incversions.txt  
#	$(PERL) $(PRJ)$/source$/scripts$/increment_version.pl \
#	$< $(BIN)$/cliureversion.mk $(SOLARBINDIR)$/unotype_statistics.txt




