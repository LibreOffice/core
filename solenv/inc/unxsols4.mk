#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: unxsols4.mk,v $
#
#   $Revision: 1.25 $
#
#   last change: $Author: kz $ $Date: 2006-07-05 22:05:50 $
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


ASM=/usr/ccs/bin/as
# needs -D__sparcv8plus because it's not defined by the assembler with -xarch=v8plus
AFLAGS=-P -xarch=v8plus -D__sparcv8plus


CDEFS+=-D_PTHREADS -DSYSV -DSUN -DSUN4 -D_REENTRANT -D_POSIX_PTHREAD_SEMANTICS -DSTLPORT_VERSION=400

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

CFLAGSOBJGUIST=-KPIC
CFLAGSOBJCUIST=-KPIC
CFLAGSOBJGUIMT=-KPIC -mt
CFLAGSOBJCUIMT=-KPIC -mt
CFLAGSSLOGUIMT=-KPIC -mt
CFLAGSSLOCUIMT=-KPIC -mt
CFLAGSPROF=-xpg
CFLAGSDEBUG=-g
CFLAGSDBGUTIL=
#  -xarch=v8plus            restrict target to v8plus (UltraSparc)
#  -xO3                     optimization level 3
#  -xspace                  don't do optimizations which do increase binary size
#  -xprefetch=yes           do prefetching (helps on UltraSparc III)
CFLAGSOPT=-xarch=v8plus -xO3 -xspace -xprefetch=yes
CFLAGSNOOPT=
CFLAGSOUTOBJ=-o

# Warnings switched off for CXX:
# - doubunder: we have many identifiers containing double underscores, some of
#   them in the stable UDK API we cannot change
# - inllargeuse: "function is too large and will not be expanded inline" is
#   merely a hint
# - notemsource: "could not find source for function" appears to be spurious
# - reftotemp: warns about calling non-const functions on temporary objects,
#   something legally done by boost::scoped_array<T>::reset, for example
#   (this_type(p).swap(*this))
CFLAGSWARNCC=
CFLAGSWARNCXX=+w2 -erroff=doubunder,inllargeuse,notemsource,reftotemp
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
    binfilter \
    canvas \
    chart2 \
    cppcanvas \
    desktop \
    devtools \
    dxcanvas \
    extensions \
    filter \
    finalize \
    glcanvas \
    instset_native \
    instsetoo_native \
    lingu \
    postprocess \
    r_tools \
    sc \
    sch \
    sd \
    slideshow \
    smoketest_native \
    smoketestoo_native \
    starmath \
    svx \
    sw \
    top \
    writerperfect \
    xmlsecurity

STDOBJVCL=$(L)$/salmain.o

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
LD_OPTIONS+=-L$(SYSBASE)$/usr$/lib
CDEFS+=-DSYSBASE="$(SYSBASE)"
CFLAGSCC+=$(C_RESTRICTIONFLAGS)
.EXPORT : LD_OPTIONS
.ENDIF          # "$(SYSBASE)"!=""

# -z combreloc combines multiple relocation sections. Reduces overhead on startup
# -norunpath prevents the compiler from recording his own libs in the runpath
LINKFLAGSRUNPATH*=-R\''$$ORIGIN'\'
LINKFLAGS=-w -mt -z combreloc -PIC -temp=/tmp $(LINKFLAGSRUNPATH) -norunpath -library=no%Cstd
LINKCFLAGS=-w -mt -z combreloc $(LINKFLAGSRUNPATH) -norunpath

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
STDLIBGUIST=$(DYNAMIC) -lm
STDLIBCUIST=$(DYNAMIC) -lm
STDLIBGUIMT=$(DYNAMIC) -lpthread -lm
STDLIBCUIMT=$(DYNAMIC) -lpthread -lm
STDSHLGUIST=$(DYNAMIC) CPPRUNTIME -lm -lc
STDSHLCUIST=$(DYNAMIC) CPPRUNTIME -lm -lc
STDSHLGUIMT=$(DYNAMIC) -lpthread CPPRUNTIME -lm -lc
STDSHLCUIMT=$(DYNAMIC) -lpthread CPPRUNTIME -lm -lc

STDLIBGUIST+=-lX11
STDLIBGUIMT+=-lX11
STDSHLGUIST+=-lX11
STDSHLGUIMT+=-lX11

# @@@ interposer needed for -Bdirect @@@
# LIBSALCPPRT*=-z allextract -lsalcpprt -z defaultextract
LIBSALCPPRT=

.IF "$(USE_STLP_DEBUG)" != ""
LIBSTLPORT=$(DYNAMIC) -lstlport_sunpro_debug
LIBSTLPORTST=$(STATIC) -lstlport_sunpro_debug $(DYNAMIC)
.ELSE
LIBSTLPORT=$(DYNAMIC) -lstlport_sunpro
LIBSTLPORTST=$(STATIC) -lstlport_sunpro $(DYNAMIC)
.ENDIF # "$(USE_STLP_DEBUG)" != ""


LIBMGR=CC
LIBFLAGS=-xar -o

IMPLIB=
IMPLIBFLAGS=

MAPSYM=
MAPSYMFLAGS=

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
