#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: tg_res.mk,v $
#
# $Revision: 1.12 $
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


.IF "$(RCTARGET)"!=""
$(RCTARGET): $(RCFILES) 		\
             $(RCDEPN)
    @echo "Making:   " $(@:f)
.IF "$(CCNUMVER)" > "001300000000"
.IF "$(MFC_INCLUDE)"!=""
    $(COMMAND_ECHO)$(RC) $(INCLUDE) -I$(SOLARRESDIR) $(foreach,i,$(ATL_INCLUDE) -I$(i)) -I$(MFC_INCLUDE) $(RCLANGFLAGS_$(@:d:d:b)) $(RCFLAGS)
.ELSE
    $(COMMAND_ECHO)$(RC) $(INCLUDE) -I$(SOLARRESDIR) $(foreach,i,$(ATL_INCLUDE) -I$(i)) $(RCLANGFLAGS_$(@:d:d:b)) $(RCFLAGS)
.ENDIF
.ELSE
    $(COMMAND_ECHO)$(RC) $(INCLUDE) -I$(SOLARRESDIR) $(RCLANGFLAGS_$(@:d:d:b)) $(RCFLAGS)
.ENDIF
.ENDIF

