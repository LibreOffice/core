#*************************************************************************
#
#   $RCSfile: MAKEFILE.MK,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: obr $ $Date: 2000-10-23 09:53:37 $
#
#   Copyright according the GNU Public License.
#
#*************************************************************************

PRJ=..$/..

PRJNAME=sal
TARGET=cpposl
USE_LDUMP2=TRUE

PROJECTPCH4DLL=TRUE
PROJECTPCH=cont_pch
PROJECTPCHSOURCE=cont_pch

MULTITHREAD_OBJ=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

# --- Files --------------------------------------------------------

.IF "$(header)" == ""

#CFILES=     conditn.c       \
#            diagnose.c      \
#            dllentry.c      \
#            semaphor.c      \
#            socket.c        \
#            interlck.c      \
#            mutex.c         \
#            thread.c        \
#            module.c        \
#            process.c       \
#            security.c      \
#            profile.c       \
#            time.c          \
#            file.c          \
#            signal.c        \
#            pipe.c          \
#            daemon.c		\
#            daemregw.c		\
#            channel.c		\
#            util.c		\
#			ports.c	\
#			dir.c	\
#			dirw9x.c

SLOFILES=   $(SLO)$/conditn.obj  \
            $(SLO)$/diagnose.obj \
            $(SLO)$/dllentry.obj \
            $(SLO)$/semaphor.obj \
            $(SLO)$/socket.obj   \
            $(SLO)$/interlck.obj \
            $(SLO)$/mutex.obj    \
            $(SLO)$/thread.obj   \
            $(SLO)$/module.obj   \
            $(SLO)$/process.obj  \
            $(SLO)$/security.obj \
            $(SLO)$/profile.obj  \
            $(SLO)$/time.obj     \
            $(SLO)$/file.obj     \
            $(SLO)$/signal.obj   \
            $(SLO)$/pipe.obj     \
            $(SLO)$/daemon.obj	 \
            $(SLO)$/daemregw.obj \
            $(SLO)$/channel.obj \
            $(SLO)$/util.obj \
            $(SLO)$/dir.obj \
            $(SLO)$/dirw9x.obj


OBJFILES=   $(OBJ)$/conditn.obj  \
            $(OBJ)$/diagnose.obj \
            $(OBJ)$/libutil.obj  \
            $(OBJ)$/semaphor.obj \
            $(OBJ)$/socket.obj   \
            $(OBJ)$/interlck.obj \
            $(OBJ)$/mutex.obj    \
            $(OBJ)$/thread.obj   \
            $(OBJ)$/module.obj   \
            $(OBJ)$/process.obj  \
            $(OBJ)$/security.obj \
            $(OBJ)$/profile.obj  \
            $(OBJ)$/time.obj     \
            $(OBJ)$/file.obj     \
            $(OBJ)$/signal.obj   \
            $(OBJ)$/pipe.obj     \
            $(OBJ)$/daemon.obj	 \
            $(OBJ)$/daemregw.obj \
            $(OBJ)$/channel.obj \
            $(OBJ)$/util.obj \
            $(OBJ)$/dir.obj \
            $(OBJ)$/dirw9x.obj

DEPOBJFILES = $(OBJ)$/sdaemon.obj

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk



