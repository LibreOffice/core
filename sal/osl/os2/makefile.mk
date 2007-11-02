#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: hr $ $Date: 2007-11-02 12:31:25 $
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

# Uncomment the following line if you want to compile with -Wall 
# it disables lots of unnecessary warnings comming from the Win32
# header files (disabling warnings via command line only works with
# MSVC >= 7.x)
#
# CFLAGS+=/wd4668 /wd4711 /wd4201 /wd4710 /wd4619 /wd4514 /wd4820

# --- Files --------------------------------------------------------

.IF "$(header)" == ""

SLOFILES=   $(SLO)$/conditn.obj  \
            $(SLO)$/diagnose.obj \
            $(SLO)$/dllentry.obj \
            $(SLO)$/semaphor.obj \
            $(SLO)$/interlck.obj \
            $(SLO)$/nlsupport.obj\
            $(SLO)$/mutex.obj    \
            $(SLO)$/module.obj   \
            $(SLO)$/process.obj  \
            $(SLO)$/time.obj     \
            $(SLO)$/signal.obj   \
            $(SLO)$/pipe.obj     \
            $(SLO)$/util.obj     \
            $(SLO)$/socket.obj   \
            $(SLO)$/thread.obj   \
            $(SLO)$/security.obj \
            $(SLO)$/profile.obj  \
            $(SLO)$/tempfile.obj  \
        $(SLO)$/printf.obj\
        $(SLO)$/process_impl.obj\
        $(SLO)$/uunxapi.obj\
        $(SLO)$/file.obj     \
        $(SLO)$/file_url.obj \
        $(SLO)$/file_error_transl.obj \
        $(SLO)$/file_path_helper.obj \
        $(SLO)$/debug.obj \
        $(SLO)$/except.obj \
        $(SLO)$/salinit.obj

OBJFILES=   $(OBJ)$/conditn.obj  \
            $(OBJ)$/diagnose.obj \
            $(OBJ)$/libutil.obj  \
            $(OBJ)$/semaphor.obj \
            $(OBJ)$/interlck.obj \
            $(OBJ)$/nlsupport.obj\
            $(OBJ)$/mutex.obj    \
            $(OBJ)$/module.obj   \
            $(OBJ)$/process.obj  \
            $(OBJ)$/time.obj     \
            $(OBJ)$/signal.obj   \
            $(OBJ)$/pipe.obj     \
            $(OBJ)$/util.obj     \
            $(OBJ)$/socket.obj   \
            $(OBJ)$/thread.obj   \
            $(OBJ)$/security.obj \
            $(OBJ)$/profile.obj  \
            $(OBJ)$/tempfile.obj  \
        $(OBJ)$/process_impl.obj\
        $(OBJ)$/uunxapi.obj\
        $(OBJ)$/file.obj     \
        $(OBJ)$/file_url.obj \
        $(OBJ)$/file_error_transl.obj \
        $(OBJ)$/file_path_helper.obj \
        $(OBJ)$/debug.obj \
        $(OBJ)$/except.obj \
        $(SLO)$/salinit.obj

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk



