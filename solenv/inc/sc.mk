#*******************************************************************
#*
#*	  $Workfile:   sc.mk  $
#*
#*	  Beschreibung		Ausnahmen fuer die StarView-Projekte
#*
#*	  Ersterstellung	TH 28.03.94
#*	  Letzte Aenderung	$Author: hjs $ $Date: 2001-02-15 13:47:17 $
#*	  $Revision: 1.2 $
#*
#*	  $Logfile:   pvcs.data1:solar:inc:sc.mkv  $
#*
#*	  Copyright (c) 1990-1994, STAR DIVISION
#*
#*******************************************************************

.INCLUDE :  sv.mk

# MakeDepn nicht das segdefs.hxx und die PCHs auswerten lassen
# PCHs werden nur in den ..._1st Verzeichnissen aufgenommen
EXCDEPN=segdefs.hxx
EXCDEPNPCH=core_pch.hxx;filt_pch.hxx;ui_pch.hxx
# done by protecting define now
.IF "$(GUI)" != "MAC"
#MKDEPFLAGS+=-E:$(EXCDEPN);$(EXCDEPNPCH)
.ELSE
#MKDEPFLAGS+=-E:"$(EXCDEPN);$(EXCDEPNPCH)"
.ENDIF

.IF "$(GUI)" == "WIN"
LINKFLAGSAPPGUI=/A:128
LIBFLAGS=$(LIBFLAGS) /PAGE:128
.ENDIF


.IF "$(GUI)$(COM)" == "WINMSC"
.IF "$(optimize)" != ""
CFLAGS=$(CFLAGS) -OV9
.ENDIF

# kein -GEfs wegen dann fehlender inlinings
CFLAGSOBJGUIST=-Alfd -GA -GEd

# Wer hatte dieses denn aus welchem Grunde eingebaut ????
#CFLAGSOBJGUIST=-Alfd -GA
# !ENDIF
.ENDIF
