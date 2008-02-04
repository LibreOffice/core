#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: tg_res.mk,v $
#
#   $Revision: 1.11 $
#
#   last change: $Author: ihi $ $Date: 2008-02-04 12:55:45 $
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


.IF "$(RCTARGET)"!=""
$(RCTARGET): $(RCFILES) 		\
             $(RCDEPN)
    @echo ------------------------------
    @echo Making: $@
.IF "$(CCNUMVER)" > "001300000000"
.IF "$(MFC_INCLUDE)"!=""
    $(RC) $(INCLUDE) -I$(SOLARRESDIR) $(foreach,i,$(ATL_INCLUDE) -I$(i)) -I$(MFC_INCLUDE) $(RCLANGFLAGS_$(@:d:d:b)) $(RCFLAGS)
.ELSE
    $(RC) $(INCLUDE) -I$(SOLARRESDIR) $(foreach,i,$(ATL_INCLUDE) -I$(i)) $(RCLANGFLAGS_$(@:d:d:b)) $(RCFLAGS)
.ENDIF
.ELSE
    $(RC) $(INCLUDE) -I$(SOLARRESDIR) $(RCLANGFLAGS_$(@:d:d:b)) $(RCFLAGS)
.ENDIF
.ENDIF

