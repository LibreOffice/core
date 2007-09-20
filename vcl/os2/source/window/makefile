#*************************************************************************
#*
#*	  $Workfile:   makefile  $
#*
#*	  Ersterstellung	TH 01.04.97
#*	  Letzte Aenderung	$Author: vg $ $Date: 2007-09-20 16:23:20 $
#*	  $Revision: 1.3 $
#*
#*	  $Logfile:   T:/vcl/os2/source/window/makefile.__v  $
#*
#*	  Copyright (c) 1990 - 1998, STAR DIVISION
#*
#*************************************************************************

PRJ=..\..\..

PRJNAME=SV
TARGET=salwin

# --- Settings -----------------------------------------------------

!INCLUDE <svpre.mak>
!INCLUDE <settings.mak>
!INCLUDE <sv.mak>

!IF "$(COM)"=="ICC"
ENVCFLAGS=$(ENVCFLAGS) -D_STD_NO_NAMESPACE -D_VOS_NO_NAMESPACE -D_UNO_NO_NAMESPACE
!ENDIF

# --- Files --------------------------------------------------------

CXXFILES=	salframe.cxx			\
			salobj.cxx

SLOFILES=	$(SLO)\salframe.obj 	\
			$(SLO)\salobj.obj

# --- Targets ------------------------------------------------------

!INCLUDE <target.mak>
