#*******************************************************************
#*
#*    $Workfile:   scpre.mk  $
#*
#*    Beschreibung      Ausnahmen fuer die StarCalc
#*
#*    Ersterstellung    TH 28.03.94
#*    Letzte Aenderung  $Author: hr $ $Date: 2000-09-20 14:43:17 $
#*    $Revision: 1.1.1.1 $
#*
#*    $Logfile:   T:/solar/inc/scpre.mkv  $
#*
#*    Copyright (c) 1990-1994, STAR DIVISION
#*
#*******************************************************************


#======================================================================
# standardmaessig mit Optimierung, muss explizit mit nopt=t ausgeschaltet
# werden, wenn nicht gewuenscht
#----------------------------------------------------------------------

#.IF defined(nopt) || defined(NOPT)
#nopt=true
#NOPT=TRUE
#!undef optimize
#!undef OPTIMIZE
#.ELSE
#.IF !(defined(debug) || defined(DEBUG))
#optimize=true
#OPTIMIZE=TRUE
#.ENDIF
#.ENDIF

# falls dependencies auf s:\solver\... erstellt werden sollen
# MKDEPENDSOLVER=YES


