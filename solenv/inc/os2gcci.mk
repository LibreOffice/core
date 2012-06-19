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



# --- OS2-Environment ----------------------------------------------

#change output dir name using the object binary format.
#.IF "$(aout)"==""
#OUTFMT=omf
#.ELSE
#OUTFMT=aout
#.ENDIF
#OUTPATH+=OUTFMT
#INPATH+=OUTFMT
#GVERDIR+=OUTFMT

# mk file for os2gcci
ASM=as
AFLAGS=

# until we get a solution to libc ticket#251, use dmik's -q option.
EMXOMFOPT = -q
.EXPORT : EMXOMFOPT

#//YD SOLAR_JAVA=TRUE
JAVAFLAGSDEBUG=-g

# filter for supressing verbose messages from linker
#not needed at the moment
#LINKOUTPUT_FILTER=" |& $(SOLARENV)/bin/msg_filter"

.IF "$(ttt)"!=""
CDEFS+=-E
.ENDIF

# _PTHREADS is needed for the stl
CDEFS+=-DX86 -D_REENTRANT -DNEW_SOLAR -D_USE_NAMESPACE=1 -DSTLPORT_VERSION=400

# this is a platform with JAVA support
.IF "$(SOLAR_JAVA)"!=""
JAVADEF=-DSOLAR_JAVA
.IF "$(debug)"==""
JAVA_RUNTIME=javai.lib
.ELSE
JAVA_RUNTIME=javai_g.lib
.ENDIF
.ENDIF

# architecture dependent flags for the C and C++ compiler that can be changed by
# exporting the variable ARCH_FLAGS="..." in the shell, which is used to start build
ARCH_FLAGS*=

# name of C++ Compiler
CXX*=g++
# name of C Compiler
CC*=gcc
# flags for C and C++ Compiler
CFLAGS+=-c
# flags for the C++ Compiler
CFLAGSCC= $(ARCH_FLAGS)

.IF "$(aout)"==""
CFLAGS+=-Zomf
.ENDIF

# Flags for enabling exception handling
CFLAGSEXCEPTIONS=-fexceptions
# Flags for disabling exception handling
CFLAGS_NO_EXCEPTIONS=-fno-exceptions

CFLAGSCXX= -frtti $(ARCH_FLAGS)
CFLAGSCXX+= -Wno-ctor-dtor-privacy  -fmessage-length=0
PICSWITCH:=

# Compiler flags for compiling static object in single threaded environment with graphical user interface
CFLAGSOBJGUIST=-Zcrtdll
# Compiler flags for compiling static object in single threaded environment with character user interface
CFLAGSOBJCUIST=-Zcrtdll
# Compiler flags for compiling static object in multi threaded environment with graphical user interface
CFLAGSOBJGUIMT=-Zcrtdll
# Compiler flags for compiling static object in multi threaded environment with character user interface
CFLAGSOBJCUIMT=-Zcrtdll
# Compiler flags for compiling shared object in multi threaded environment with graphical user interface
CFLAGSSLOGUIMT=$(PICSWITCH) -Zdll -Zcrtdll
# Compiler flags for compiling shared object in multi threaded environment with character user interface
CFLAGSSLOCUIMT=$(PICSWITCH) -Zdll -Zcrtdll
# Compiler flags for profiling
CFLAGSPROF=
# Compiler flags for debugging
CFLAGSDEBUG=-g
CFLAGSDBGUTIL=
# Compiler flags for enabling optimazations
CFLAGSOPT=-s -O1 -march=pentium -mtune=pentium4
# Compiler flags for disabling optimazations
CFLAGSNOOPT=-O0
# Compiler flags for discibing the output path
CFLAGSOUTOBJ=-o

# Warnings switched off for CXX:
# - doubunder: we have many identifiers containing double underscores, some of
#   them in the stable UDK API we cannot change
# - inllargeuse: "function is too large and will not be expanded inline" is
#   merely a hint
# - inllargeint: "function is too large to generate inline, consider writing
#   it yourself" is merely a hint
# - notemsource: "could not find source for function" appears to be spurious
# - reftotemp: warns about calling non-const functions on temporary objects,
#   something legally done by boost::scoped_array<T>::reset, for example
#   (this_type(p).swap(*this))
# - truncwarn: "conversion of 64 bit type value to smaller type causes
#   truncation" at least with CC 5.8 is reported only at the end of a
#   compilation unit that uses std::hash_map<sal_Int64, sal_Int64> (see
#   sfx2/source/toolbox/imgmgr.cxx:1.27) and thus unfortunately needs to be
#   disabled globally
CFLAGSWARNCC=
CFLAGSWARNCXX=+w2 -erroff=doubunder,inllargeuse,inllargeint,notemsource,reftotemp,truncwarn
CFLAGSWARNCXX=
CFLAGSWALLCC=$(CFLAGSWARNCC)
CFLAGSWALLCXX=$(CFLAGSWARNCXX)
CFLAGSWERRCC=-errwarn=%all
CFLAGSWERRCXX=-xwe

# Once all modules on this platform compile without warnings, set
# COMPILER_WARN_ERRORS=TRUE here instead of setting MODULES_WITH_WARNINGS (see
# settings.mk):
MODULES_WITH_WARNINGS := \
    b_server \
    basctl \
    basebmp \
    canvas \
    chart2 \
    cppcanvas \
    desktop \
    devtools \
    dxcanvas \
    extensions \
    filter \
    glcanvas \
    lingu \
    r_tools \
    sc \
    sd \
    slideshow \
    starmath \
    svx \
    sw \
    writerperfect \
    xmlsecurity


# switches for dynamic and static linking
STATIC		= 
DYNAMIC		= 

# this is needed to allow stl headers to include os2.h instead of svpm.h
# in earlier stages of project build. Once svpm.h is in place, we can
# compile without this flag.
.IF "$(STL_OS2_BUILDING)" != ""
CFLAGS+=-DSTL_OS2_BUILDING
CFLAGSCXX+=-DSTL_OS2_BUILDING
.ENDIF

CFLAGS+=$(LIBXML_CFLAGS) -I$(JAVA_HOME)/include -I$(JAVA_HOME)/include/os2
CFLAGSCXX+=$(LIBXML_CFLAGS) -I$(JAVA_HOME)/include -I$(JAVA_HOME)/include/os2

# name of linker
LINK*=gcc
# default linker flags
LINKFLAGS= -Zbin-files -Zmap 
#27/01/06 bin-files confuses transex/rsc work, removed.
LINKFLAGS= -Zno-fork -Zhigh-mem -Zmap
.IF "$(OS2_ARGS_WILD)" != ""
LINKFLAGS+=-Zargs-wild -Zargs-resp
.ENDIF

.IF "$(aout)"==""
#LINKFLAGS+= -Zlinker /NOI -Zlinker /PACKD:65536 -Zlinker /EXEPACK:2
#LINKFLAGS+= -Zlinker /NOExtdictionary
LINKFLAGS+= -Zomf
LINKFLAGS+= -Zlinker "DISABLE 1121"
.ENDIF


# linker flags for linking applications
.IF "$(aout)"==""
#LINKFLAGSAPPGUI= -Zlinker /PM:PM -Zstack 0x2000
#LINKFLAGSAPPCUI= -Zlinker /PM:VIO
.ELSE
LINKFLAGSAPPGUI=
LINKFLAGSAPPCUI=
.ENDIF

# linker flags for linking shared libraries
LINKFLAGSSHLGUI= -Zdll #-Zsym
LINKFLAGSSHLCUI= -Zdll #-Zsym

.IF "$(aout)"==""
LINKFLAGSTACK=-Zlinker /STACK:
LINKFLAGSPROF=
#LINKFLAGSDEBUG=-Zlinker /DEBUG
LINKFLAGSDEBUG=-g
LINKFLAGSOPT=-g
.ELSE
LINKFLAGSTACK=-Zstack
LINKFLAGSPROF=
LINKFLAGSDEBUG=-g
LINKFLAGSOPT=-s
.ENDIF

.IF "$(NO_BSYMBOLIC)"==""
.IF "$(PRJNAME)" != "envtest"
LINKFLAGSSHLGUI+=
LINKFLAGSSHLCUI+=
.ENDIF
.ENDIF				# "$(NO_BSYMBOLIC)"==""

LINKVERSIONMAPFLAG=-Wl,--version-script

SONAME_SWITCH=-Wl,-h

# Sequence of libs does matter !

#STDLIBCPP=stdc++ supc++ gcc_eh
#STDLIBCPP=stdc++ gcc432
STDLIBCPP=-lstdc++ -lgcc_so_d

# default objectfilenames to link
STDOBJVCL=$(L)/salmain.o
STDOBJGUI=
STDSLOGUI=
STDOBJCUI=
STDSLOCUI=

# libraries for linking applications
STDLIBCUIST=$(STDLIBCPP)
STDLIBGUIMT=$(STDLIBCPP)
STDLIBCUIMT=$(STDLIBCPP)
STDLIBGUIST=$(STDLIBCPP)
# libraries for linking shared libraries
STDSHLGUIMT=$(STDLIBCPP)
STDSHLCUIMT=$(STDLIBCPP)

LIBSTLPORT=stlp45.lib $(DYNAMIC) 
LIBSTLPORTST=stlp45.lib $(STATIC) $(DYNAMIC)


# name of library manager
LIBMGR=ar
LIBFLAGS=-crv

.IF "$(aout)"==""
LIBMGR=emxomfar -p2048
.ENDIF

# tool for generating import libraries
IMPLIB=emximp
IMPLIBFLAGS=-o

MAPSYM=mapsym
MAPSYMFLAGS=

RC=wrc -q -bt=os2 -i$(PATH_EMXPGCC)\include
RCFLAGS=-r $(RCFILES) -fo=$@
RCLINK=wrc -bt=os2
RCLINKFLAGS=
RCSETVERSION=

# platform specific identifier for shared libs
DLLPOSTFIX=
DLLPRE=lib
DLLPOST=.dll

