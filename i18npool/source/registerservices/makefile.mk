#*************************************************************************
#*
#*  $RCSfile: makefile.mk,v $
#*
#*	makefile
#*
#*  Erstellung          MT 25.6.98
#*
#*  Letzte Aenderung    $Author: bustamam $ $Date: 2001-08-30 23:33:17 $
#*
#*  $Revision: 1.1 $
#*
#*  $Source: /zpool/svn/migration/cvs_rep_09_09_08/code/i18npool/source/registerservices/makefile.mk,v $
#*
#*  Copyright (c) 1998, Star Division Corp.
#*
#*************************************************************************

PRJ=..$/..

PRJNAME=i18npool
TARGET=registerservices

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	svpre.mk
.INCLUDE :	settings.mk
.INCLUDE :	sv.mk

# --- Files --------------------------------------------------------

SLOFILES=   \
            $(SLO)$/registerservices.obj

# Make symbol renaming match library name for Mac OS X
.IF "$(OS)"=="MACOSX"
SYMBOLPREFIX=i18n$(UPD)$(DLLPOSTFIX)
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
