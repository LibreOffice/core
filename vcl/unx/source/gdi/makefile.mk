#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.23 $
#
#   last change: $Author: vg $ $Date: 2008-02-12 14:07:02 $
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

PRJ=..$/..$/..

PRJNAME=vcl
TARGET=salgdi

.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile2.pmk

# --- Files --------------------------------------------------------

.IF "$(GUIBASE)"!="unx"

dummy:
    @echo "Nothing to build for GUIBASE $(GUIBASE)"

.ELSE		# "$(GUIBASE)"!="unx"

SLOFILES=	\
        $(SLO)$/salgdi2.obj		\
        $(SLO)$/salbmp.obj		\
        $(SLO)$/salgdi.obj		\
        $(SLO)$/salvd.obj		\
        $(SLO)$/salogl.obj		\
        $(SLO)$/dtint.obj		\
        $(SLO)$/salcvt.obj		\
        $(SLO)$/xfont.obj		\
        $(SLO)$/xlfd_attr.obj		\
        $(SLO)$/xlfd_extd.obj		\
        $(SLO)$/xlfd_smpl.obj		\
        $(SLO)$/salgdi3.obj		\
        $(SLO)$/gcach_xpeer.obj		\
        $(SLO)$/xrender_peer.obj	\
        $(SLO)$/pspgraphics.obj
        
EXCEPTIONSFILES=\
        $(SLO)$/xlfd_extd.obj	\
        $(SLO)$/salbmp.obj		\
        $(SLO)$/salgdi3.obj		\
        $(SLO)$/salcvt.obj


.IF "$(OS)"=="MACOSX"
SLOFILES += $(SLO)$/macosxint.obj
MACOSXRC = $(MISC)$/macosxrc.txt
.ENDIF # "$(OS)"=="MACOSX"

.IF "$(USE_XPRINT)" == "TRUE"
CFLAGS+=-D_USE_PRINT_EXTENSION_=1
SLOFILES+=$(SLO)$/xprintext.obj
.ELSE
SLOFILES+=$(SLO)$/salprnpsp.obj
EXCEPTIONSFILES+=$(SLO)$/salprnpsp.obj
.ENDIF

.IF "$(OS)"=="SOLARIS"
SLOFILES+=$(SLO)$/cdeint.obj
ENVCFLAGS+=-DUSE_CDE
.ENDIF

.IF "$(XRENDER_LINK)" == "YES"
CFLAGS+=-DXRENDER_LINK
.ENDIF

.ENDIF	# "$(GUIBASE)"!="unx"

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

.INCLUDE :  $(PRJ)$/util$/target.pmk

ALLTAR : $(MACOSXRC)

XSALSETLIBNAME=$(DLLPRE)spa$(UPD)$(DLLPOSTFIX)$(DLLPOST)

$(INCCOM)$/rtsname.hxx:
    rm -f $(INCCOM)$/rtsname.hxx ; \
    echo "#define _XSALSET_LIBNAME "\"$(XSALSETLIBNAME)\" > $(INCCOM)$/rtsname.hxx

$(SLO)$/salpimpl.obj : $(INCCOM)$/rtsname.hxx
$(SLO)$/salprnpsp.obj : $(INCCOM)$/rtsname.hxx

$(MISC)$/macosxrc.txt : $$(@:f)
    $(COPY) $< $@
