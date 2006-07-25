#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: rt $ $Date: 2006-07-25 07:55:26 $
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

PRJ = ..
PRJNAME = cli_ure

# for dummy
TARGET = cliureversion.mk

.INCLUDE : settings.mk
.INCLUDE : $(PRJ)$/util$/makefile.pmk

.INCLUDE : $(PRJ)$/util$/target.pmk
.INCLUDE : target.mk


.IF "$(USE_SHELL)"!="4nt"
ECHOQUOTE='
.ELSE
ECHOQUOTE=
.ENDIF

.IF "$(BUILD_FOR_CLI)" != ""

ALLTAR : \
    $(BIN)$/cliureversion.mk 

#	INCVERSION


$(BIN)$/cliureversion.mk: version.txt 
    $(GNUCOPY) $< $@

#disabled because of #67482
#Create the config file that is used with the policy assembly 
#only if new published types have been added, the cliureversion.mk is written.
#INCVERSION .PHONY: version.txt incversions.txt  
#	+$(PERL) $(PRJ)$/source$/scripts$/increment_version.pl \
#	$< $(BIN)$/cliureversion.mk $(SOLARBINDIR)$/unotype_statistics.txt

.END


