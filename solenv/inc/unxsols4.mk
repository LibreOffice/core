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


ASM=/usr/ccs/bin/as
# needs -D__sparcv8plus because it's not defined by the assembler with -xarch=v8plus
AFLAGS=-P -xarch=v8plus -D__sparcv8plus

CDEFS+=-D_PTHREADS -DSYSV -DSUN -DSUN4 -D_REENTRANT -D_POSIX_PTHREAD_SEMANTICS

SOLAR_JAVA*=TRUE
.IF "$(SOLAR_JAVA)"!=""
JAVADEF=-DSOLAR_JAVA
JAVAFLAGSDEBUG=-g
.IF "$(debug)"==""
JAVA_RUNTIME=-ljava
.ELSE
JAVA_RUNTIME=-ljava_g
.ENDIF
.ENDIF

# architecture dependent flags for the C and C++ compiler that can be changed by
# exporting the variable ARCH_FLAGS="..." in the shell, which is used to start build
ARCH_FLAGS*=

CXX*=CC
CC*=cc

CFLAGS=$(PREENVCFLAGS) -c -temp=/tmp
# CC defines __sparcv8plus with -xarch=v8plus, cc does not (sigh)
CFLAGSCC=-xCC -D__sparcv8plus $(ARCH_FLAGS)
CFLAGSCXX=-features=no%altspell -library=no%Cstd $(ARCH_FLAGS)

# flags to enable build with symbols; required for crashdump feature
CFLAGSENABLESYMBOLS=-g0 -xs # was temporarily commented out, reenabled before Beta
CFLAGSENABLESYMBOLS_CC_ONLY=-g -xs # was temporarily commented out, reenabled before Beta

CFLAGSEXCEPTIONS=
CFLAGS_NO_EXCEPTIONS=-noex

CFLAGSOBJGUIMT=-KPIC -mt
CFLAGSOBJCUIMT=-KPIC -mt
CFLAGSSLOGUIMT=-KPIC -mt
CFLAGSSLOCUIMT=-KPIC -mt
CFLAGSPROF=-xpg
CFLAGSDEBUG=-g
CFLAGSDBGUTIL=
#  -m32 -xarch=sparc        restrict target to 32 bit sparc 
#  -xO3                     optimization level 3
#  -xspace                  don't do optimizations which do increase binary size
#  -xprefetch=yes           do prefetching (helps on UltraSparc III)
CFLAGSOPT=-m32 -xarch=sparc -xO3 -xspace -xprefetch=yes
CFLAGSNOOPT=
CFLAGSOUTOBJ=-o

# Warnings switched off for CXX:
# - doubunder: we have many identifiers containing double underscores, some of
#   them in the stable UDK API we cannot change
# - identexpected: Identifier expected instead of "}"
#   if an enum ends with a comma before the '}'
#   this warning does not seem to heed #pragma disable_warn, and is not helpful
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
#   compilation unit that uses boost::unordered_map<sal_Int64, sal_Int64> (see
#   sfx2/source/toolbox/imgmgr.cxx:1.27) and thus unfortunately needs to be
#   disabled globally
# - wnoretvalue: warning about the last statement of a function not 
#   returning a value. Unfortunately triggers on perfectly acceptable
#   code, for example if the last statement in is a throw statement
# - anonnotype: Warns if a type is declared in an anonymous union. Temporary
#   disabled until issue i97325 is fixed. Note: The compiler is actually
#   right about this warning, the C++ standard is explicit about this.
CFLAGSWARNCC=
CFLAGSWARNCXX=+w2 -erroff=doubunder,identexpected,inllargeuse,inllargeint,notemsource,reftotemp,truncwarn,wnoretvalue,anonnotype
CFLAGSWALLCC=$(CFLAGSWARNCC)
CFLAGSWALLCXX=$(CFLAGSWARNCXX)
CFLAGSWERRCC=-errwarn=%all
CFLAGSWERRCXX=-xwe

# Once all modules on this platform compile without warnings, set
# COMPILER_WARN_ERRORS=TRUE here instead of setting MODULES_WITH_WARNINGS (see
# settings.mk):
MODULES_WITH_WARNINGS :=

STDOBJVCL=$(L)/salmain.o

THREADLIB=
.IF "$(PURIFY)"!=""
LINK=/usr/local/purify-4.2-solaris2/purify CC
.ELSE
LINK=$(CXX)
.ENDIF
LINKC=$(CC)

# link against set of baseline libraries
.IF "$(SYSBASE)"!=""
C_RESTRICTIONFLAGS*=-xc99=none
#LD_OPTIONS+:=-L$(SYSBASE)/usr/lib
CDEFS+=-DSYSBASE="$(SYSBASE)"
CFLAGSCC+=$(C_RESTRICTIONFLAGS)
#.EXPORT : LD_OPTIONS
.ENDIF          # "$(SYSBASE)"!=""

# -z combreloc combines multiple relocation sections. Reduces overhead on startup
# -norunpath prevents the compiler from recording his own libs in the runpath
LINKFLAGSRUNPATH_URELIB=-R\''$$ORIGIN'\'
LINKFLAGSRUNPATH_UREBIN=-R\''$$ORIGIN/../lib:$$ORIGIN'\'
    #TODO: drop $ORIGIN once no URE executable is also shipped in OOo
LINKFLAGSRUNPATH_OOO=-R\''$$ORIGIN:$$ORIGIN/../ure-link/lib'\'
LINKFLAGSRUNPATH_SDK=-R\''$$ORIGIN/../../ure-link/lib'\'
LINKFLAGSRUNPATH_BRAND=-R\''$$ORIGIN:$$ORIGIN/../basis-link/program:$$ORIGIN/../basis-link/ure-link/lib'\'
LINKFLAGSRUNPATH_OXT=
LINKFLAGSRUNPATH_BOXT=-R\''$$ORIGIN/../../../basis-link/program'\'
LINKFLAGSRUNPATH_NONE=
LINKFLAGS=-w -mt -z combreloc -PIC -temp=/tmp -norunpath -library=no%Cstd
LINKCFLAGS=-w -mt -z combreloc -norunpath

# -z text force fatal error if non PIC code is linked into shared library. Such code
#    would be expensive on startup
CHECKFORPIC =-z text
LINKFLAGSSHLGUI=$(CHECKFORPIC) -G
LINKFLAGSSHLCUI=$(CHECKFORPIC) -G

# switches for dynamic and static linking
LINKFLAGSDEFS*= -z defs
STATIC		= -Bstatic
DIRECT		= -Bdirect $(LINKFLAGSDEFS)
DYNAMIC		= -Bdynamic

LINKFLAGSAPPGUI+=$(DIRECT)
LINKFLAGSAPPCUI+=$(DIRECT)
LINKFLAGSSHLGUI+=$(DIRECT)
LINKFLAGSSHLCUI+=$(DIRECT)

LINKFLAGSTACK=
LINKFLAGSPROF=-L$(COMPATH)/WS6U1/lib/libp -xpg -z allextract
LINKFLAGSDEBUG=
LINKFLAGSOPT=
LINKVERSIONMAPFLAG=-M

# mapfile for non-executable stack
LINKFLAGSNOEXSTK*=$(LINKVERSIONMAPFLAG) $(SOLARENV)/src/solaris_noexstk.map
LINKFLAGSAPPGUI+=$(LINKFLAGSNOEXSTK)
LINKFLAGSAPPCUI+=$(LINKFLAGSNOEXSTK)

APPLINKSTATIC=$(STATIC)
APPLINKSHARED=$(DIRECT)
APP_LINKTYPE=

STDLIBCPP=-lCrun

# reihenfolge der libs NICHT egal!
STDOBJGUI=
.IF "DBG_UTIL" != ""
STDSLOGUI=#-lpthread
.ELSE
STDSLOGUI=
.ENDIF
STDOBJCUI=
STDSLOCUI=

# CPPRUNTIME - define where to place C++ runtime if required
STDLIBGUIMT=$(DYNAMIC) -lpthread -lm
STDLIBCUIMT=$(DYNAMIC) -lpthread -lm
STDSHLGUIMT=$(DYNAMIC) -lpthread CPPRUNTIME -lm -lc
STDSHLCUIMT=$(DYNAMIC) -lpthread CPPRUNTIME -lm -lc

# libdl.so - no really an GUI library but required in this context
STDLIBGUIMT+=-ldl
STDSHLGUIMT+=-ldl
X11LINK_DYNAMIC = -lXext -lX11

# @@@ interposer needed for -Bdirect @@@
# LIBSALCPPRT*=-z allextract -lsalcpprt -z defaultextract
LIBSALCPPRT=

LIBMGR=CC
LIBFLAGS=-xar -o

IMPLIB=
IMPLIBFLAGS=

MAPSYM=
MAPSYMFLAGS=
IGNORE_SYMBOLS=S-LP64

RC=irc
RCFLAGS=-fo$@ $(RCFILES)
RCLINK=
RCLINKFLAGS=
RCSETVERSION=

DLLPOSTFIX=ss

DLLPRE=lib
DLLPOST=.so

LDUMP=cppfilt /b /n /o /p

CFLAGSCXXSLO +=
CFLAGSCXXOBJ +=

LINKFLAGSAPPGUI+=
LINKFLAGSSHLGUI+=
LINKFLAGSAPPCUI+=
LINKFLAGSSHLCUI+=
