#*************************************************************************
#*
#*  $RCSfile: makefile.mk,v $
#*
#*	makefile
#*
#*  Erstellung          MT 25.6.98
#*
#*  Letzte Aenderung    $Author: bustamam $ $Date: 2001-08-30 23:33:16 $
#*
#*  $Revision: 1.1 $
#*
#*  $Source: /zpool/svn/migration/cvs_rep_09_09_08/code/i18npool/source/localedata/makefile.mk,v $
#*
#*  Copyright (c) 1998, Star Division Corp.
#*
#*************************************************************************

PRJ=..$/..


PRJNAME=i18npool
TARGET=localedata

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk


# --- Files --------------------------------------------------------

SLOFILES=   \
            $(SLO)$/localedata.obj \
            $(SLO)$/indexentrysupplier.obj


# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

