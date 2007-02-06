#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: tg_yxx.mk,v $
#
#   $Revision: 1.14 $
#
#   last change: $Author: vg $ $Date: 2007-02-06 14:01:53 $
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


$(MISC)$/%.cxx : %.y
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX" || "$(USE_SHELL)" != "4nt"
    tr -d "\015" < $< > $(MISC)$/stripped_$<
.ELSE
    cat $< > $(MISC)$/stripped_$<
.ENDIF
    $(BISON) $(YACCFLAGS) -o $(YACCTARGET) $(MISC)$/stripped_$<
# removing -f switch - avoid getting nothing when copying a file to itself
    @@-$(COPY:s/-f//) $@.h $(INCCOM)$/$(@:b).hxx
    @@-$(COPY:s/-f//) $(@:d)$/$(@:b).hxx $(INCCOM)$/$(@:b).hxx
# fail on not existing .hxx
    $(TYPE) $(INCCOM)$/$(@:b).hxx >& $(NULLDEV)

$(INCCOM)$/yy%.cxx : %.y
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="UNX" || "$(USE_SHELL)" != "4nt"
    tr -d "\015" < $< > $(MISC)$/stripped_$<
.ELSE
    cat $< > $(MISC)$/stripped_$<
.ENDIF
    $(BISON) $(YACCFLAGS) -o $(YACCTARGET) $(MISC)$/stripped_$<
# removing -f switch - avoid getting nothing when copying a file to itself
    @@-$(COPY:s/-f//) $@.h $(INCCOM)$/$(@:b).hxx
    @@-$(COPY:s/-f//) $(@:d)$/$(@:b).hxx $(INCCOM)$/$(@:b).hxx
# fail on not existing .hxx
    $(TYPE) $(INCCOM)$/$(@:b).hxx >& $(NULLDEV)

