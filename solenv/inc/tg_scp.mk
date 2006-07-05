#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: tg_scp.mk,v $
#
#   $Revision: 1.14 $
#
#   last change: $Author: kz $ $Date: 2006-07-05 21:59:04 $
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

#######################################################
# Anweisungen fuer das Linken
# unroll begin

.IF "$(SCP$(TNR)TARGETN)"!=""

# try to get missing parfiles
$(PAR)$/%.par : $(SOLARPARDIR)$/%.par
    @+-$(MKDIRHIER) $(@:d:d) >& $(NULLDEV)
    +$(COPY) $< $@

LOCALSCP$(TNR)FILES=$(foreach,i,$(SCP$(TNR)FILES) $(foreach,j,$(SCP$(TNR)LINK_PRODUCT_TYPE) $(PAR)$/$j$/$i ))

$(SCP$(TNR)TARGETN): $(LOCALSCP$(TNR)FILES)
    @echo ------------------------------
    @echo Making: $@
    @+-$(MKDIRHIER) $(BIN)$/$(SCP$(TNR)LINK_PRODUCT_TYPE) >& $(NULLDEV)
    +$(SCPLINK) $(SCPLINKFLAGS) @@$(mktmp $(foreach,i,$(SCP$(TNR)FILES) $(subst,$(@:d:d:d), $(@:d:d))$/$(i:+","))) -o $@
.ENDIF

# Anweisungen fuer das Linken
# unroll end
#######################################################

