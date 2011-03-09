#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
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

CDEFS += -DMOZILLA_INTERNAL_API

.IF "$(GUI)"=="UNX"
  .IF "$(COMNAME)"=="sunpro5"
    CFLAGS += -features=tmplife
    #This flag is needed to build mozilla 1.7 code
  .ENDIF		# "$(COMNAME)"=="sunpro5"
.ENDIF

MOZINC = . \
        -I.. \
        -I$(MOZ_INC) \
        -I$(MOZ_INC)$/nspr \
        -I$(MOZ_INC)$/xpcom \
        -I$(MOZ_INC)$/string \
        -I$(MOZ_INC)$/rdf \
        -I$(MOZ_INC)$/msgbase \
        -I$(MOZ_INC)$/addrbook \
        -I$(MOZ_INC)$/mork \
        -I$(MOZ_INC)$/locale \
        -I$(MOZ_INC)$/pref \
        -I$(MOZ_INC)$/mime \
        -I$(MOZ_INC)$/chrome \
        -I$(MOZ_INC)$/necko \
        -I$(MOZ_INC)$/intl \
        -I$(MOZ_INC)$/profile \
        -I$(MOZ_INC)$/embed_base \
        -I$(MOZ_INC)$/mozldap \
        -I$(MOZ_INC)$/uconv \
        -I$(MOZ_INC)$/xpcom_obsolete \
        -I$(MOZ_INC)$/content
        
.IF "$(GUI)" == "WNT"
  CDEFS +=    \
        -DMOZILLA_CLIENT \
        -DXP_PC \
        -DXP_WIN \
        -DXP_WIN32 \
        -DOJI \
        -DWIN32 \
        -D_WINDOWS \
        -DMOZ_XUL \
        -DMOZ_REFLOW_PERF \
        -DMOZ_REFLOW_PERF_DSP
  .IF "$(COM)" == "GCC"
    INCPOST += $(MOZINC)
    CFLAGSCXX += \
                -fno-rtti \
                -Wall \
                -Wconversion \
                -Wpointer-arith \
                -Wcast-align \
                -Woverloaded-virtual \
                -Wsynth \
                -Wno-long-long
  .ELSE # .IF "$(COM)" == "GCC"
    INCPRE += $(MOZINC)
    .IF "$(DBG_LEVEL)" != "0"
      .IF "$(CCNUMVER)" >= "001399999999"
        CDEFS  +=   -D_STL_NOFORCE_MANIFEST
      .ENDIF # .IF "$(CCNUMVER)" >= "001399999999"
    .ENDIF # .IF "$(DBG_LEVEL)" != "0"
  .ENDIF # .IF "$(COM)" == "GCC"
.ENDIF # .IF "$(GUI)" == "WNT"

.IF "$(GUI)" == "UNX"
  INCPOST += $(MOZINC)
  CDEFS+=   -DMOZILLA_CLIENT \
            -DXP_UNIX
  .IF "$(OS)" == "LINUX"
    CFLAGS  += -fPIC -g
    CDEFS   += -DOJI
    CFLAGSCXX += \
            -fno-rtti \
            -Wconversion \
            -Wpointer-arith \
            -Wcast-align \
            -Woverloaded-virtual \
            -Wsynth \
            -Wno-long-long \
            -pthread
    CDEFS     += -DTRACING
  .ELIF "$(OS)" == "NETBSD" || "$(OS)" == "MACOSX"
    CFLAGS +=   -fPIC
    CFLAGSCXX += \
            -fno-rtti \
            -Wconversion \
            -Wpointer-arith \
            -Wcast-align \
            -Woverloaded-virtual \
            -Wsynth \
            -Wno-long-long \
            -Wno-deprecated
    CDEFS += -DTRACING
  .ENDIF # "$(OS)" == "NETBSD" || "$(OS)" == "MACOSX"

  SHL1TARGET_NAME=$(TARGET)$(MOZAB_MAJOR)

.ENDIF # "$(GUI)" == "UNX"

