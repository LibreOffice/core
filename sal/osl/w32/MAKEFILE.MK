#*************************************************************************
#
#   $RCSfile: MAKEFILE.MK,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: tra $ $Date: 2000-11-22 13:50:41 $
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

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

CFLAGS+=/DSAL_EXPORT_SYSTOOLS	

.IF "$(header)" == ""

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
            $(SLO)$/util.obj \
            $(SLO)$/dir.obj \
            $(SLO)$/dirw9x.obj \
            $(SLO)$/systoolinit.obj


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
            $(OBJ)$/util.obj \
            $(OBJ)$/dir.obj \
            $(OBJ)$/dirw9x.obj\
            $(OBJ)$/systoolinit.obj

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk



