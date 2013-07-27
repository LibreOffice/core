#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



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
        -D_X86_ \
        -D_WINDOWS \
        -DMOZ_XUL \
        -DMOZ_REFLOW_PERF \
        -DMOZ_REFLOW_PERF_DSP
  .IF "$(COM)" == "GCC"
    INCPOST += $(MOZINC)
    CFLAGSCXX += \
                -fno-rtti -DBOOST_NO_TYPEID \
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
            -fno-rtti -DBOOST_NO_TYPEID \
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
            -fno-rtti -DBOOST_NO_TYPEID \
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

