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
#*  $Source: /zpool/svn/migration/cvs_rep_09_09_08/code/i18npool/source/numberformatcode/makefile.mk,v $
#*
#*  Copyright (c) 1998, Star Division Corp.
#*
#*************************************************************************

PRJ=..$/..

PRJNAME=i18npool
TARGET=numberformatcode

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	svpre.mk
.INCLUDE :	settings.mk
.INCLUDE :	sv.mk

# --- Files --------------------------------------------------------

SLOFILES=   \
            $(SLO)$/numberformatcode.obj

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk



