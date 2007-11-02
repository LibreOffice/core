PRJ=..$/..$/..

PRJNAME=setup_native
TARGET=stclient_wrapper
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE
TARGETTYPE=GUI
USE_DEFFILE=TRUE
#DYNAMIC_CRT:=
NO_DEFAULT_STL=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Resources ----------------------------------------------------

RCFILES=resource.rc

# --- Files --------------------------------------------------------

OBJFILES=\
    $(OBJ)$/stwrapper.obj

APP1OBJS=$(OBJ)$/stwrapper.obj

APP1NOSAL=TRUE
                        
APP1TARGET=$(TARGET)	

APP1NOSVRES=$(RES)$/$(TARGET).res

UWINAPILIB:=

STDLIB1=\
    $(GDI32LIB)\
    $(COMCTL32LIB)\
    $(COMDLG32LIB)\
    $(ADVAPI32LIB)\
    $(SHELL32LIB)

DLLPRE =

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

$(OBJ)$/stwrapper.obj: $(INCCOM)$/_version.h

#$(RCFILES) : resource.rc
