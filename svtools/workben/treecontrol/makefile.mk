#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 17:04:45 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************

PRJ=..$/..

PRJNAME=svtools
TARGET=treetest
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

APP1NOSAL=	TRUE
APP1TARGET= treetest
APP1OBJS=	$(OBJ)$/treetest.obj
APP1STDLIBS=$(SOTLIB)			\
            $(COMPHELPERLIB)	\
            $(CPPULIB)			\
            $(CPPUHELPERLIB)	\
            $(SALLIB)

#			$(SVTOOLLIB)		\

APP2DEF=	$(MISC)$/treetest.def

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk


# ------------------------------------------------------------------
# Windows
# ------------------------------------------------------------------

.IF "$(GUI)" == "WIN"

$(MISC)$/treetest.def: makefile.mk
    echo  NAME			treetest							>$@
    echo  DESCRIPTION	'StarView - Testprogramm'          >>$@
    echo  EXETYPE		WINDOWS 						   >>$@
    echo  STUB			'winSTUB.EXE'                      >>$@
    echo  PROTMODE										   >>$@
    echo  CODE			PRELOAD MOVEABLE DISCARDABLE	   >>$@
    echo  DATA			PRELOAD MOVEABLE MULTIPLE		   >>$@
    echo  HEAPSIZE		8192							   >>$@
    echo  STACKSIZE 	32768							   >>$@

.ENDIF

ALLTAR : $(BIN)$/treetest.rdb 

$(BIN)$/treetest.rdb : makefile.mk $(UNOUCRRDB)
    rm -f $@
    $(GNUCOPY) $(UNOUCRRDB) $@
     +cd $(BIN) && \
         regcomp -register -r treetest.rdb \
             -c i18nsearch.uno$(DLLPOST) \
             -c i18npool.uno$(DLLPOST) \
             -c connector.uno$(DLLPOST) \
            -c remotebridge.uno$(DLLPOST) \
            -c bridgefac.uno$(DLLPOST) \
            -c uuresolver.uno$(DLLPOST) \
            -c $(DLLPRE)tk$(DLLPOSTFIX)$(DLLPOST)
