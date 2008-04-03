#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.31 $
#
#   last change: $Author: kz $ $Date: 2008-04-03 17:07:21 $
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
TARGET=salapp
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile2.pmk

# --- Files --------------------------------------------------------

.IF "$(GUIBASE)"!="unx"

dummy:
    @echo "Nothing to build for GUIBASE $(GUIBASE)"

.ELSE		# "$(GUIBASE)"!="unx"

SLOFILES=\
            $(SLO)$/i18n_cb.obj			\
            $(SLO)$/i18n_ic.obj			\
            $(SLO)$/i18n_im.obj			\
            $(SLO)$/i18n_xkb.obj		\
            $(SLO)$/i18n_wrp.obj		\
            $(SLO)$/i18n_status.obj		\
            $(SLO)$/i18n_keysym.obj		\
            $(SLO)$/saldata.obj			\
            $(SLO)$/saltimer.obj		\
            $(SLO)$/saldisp.obj			\
            $(SLO)$/randrwrapper.obj	\
            $(SLO)$/salinst.obj			\
            $(SLO)$/salsys.obj			\
            $(SLO)$/soicon.obj			\
            $(SLO)$/sm.obj				\
            $(SLO)$/keysymnames.obj		\
            $(SLO)$/wmadaptor.obj

EXCEPTIONSFILES=\
            $(SLO)$/wmadaptor.obj		\
            $(SLO)$/saldata.obj			\
            $(SLO)$/saldisp.obj			\
            $(SLO)$/i18n_status.obj		\
            $(SLO)$/i18n_cb.obj				\
            $(SLO)$/i18n_ic.obj				\
            $(SLO)$/salsys.obj


.IF "$(ENABLE_RANDR)" != ""
.IF "$(OS)" != "SOLARIS"
CDEFS+=-DUSE_RANDR
.ENDIF
.ENDIF

.IF "$(USE_XINERAMA)" != "NO"
CDEFS+=-DUSE_XINERAMA
.IF "$(USE_XINERAMA_VERSION)" == "Xorg"
CDEFS+=-DUSE_XINERAMA_XORG
.ELIF "$(USE_XINERAMA_VERSION)" == "Xsun"
CDEFS+=-DUSE_XINERAMA_XSUN
.ELSE
# provide sensible default
.IF "$(OS)" != "SOLARIS"
CDEFS+=-DUSE_XINERAMA_XORG
.ELSE
CDEFS+=-DUSE_XINERAMA_XSUN
.ENDIF
.ENDIF
.ENDIF

.IF "$(OS)$(CPU)" == "SOLARISS"
.IF "$(COM)"!="GCC"
SLOFILES+=$(SLO)$/getfpsols.obj
.ENDIF			# "$(COM)"!="GCC"
.ELIF "$(OS)$(CPU)" == "SOLARISI"
SLOFILES+=$(SLO)$/getfpsoli.obj
.ENDIF

.ENDIF		# "$(GUIBASE)"!="unx"

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

.INCLUDE :  $(PRJ)$/util$/target.pmk

# local rule to create the additional object file for SOLARIS
$(SLO)$/%.obj: %.s
    $(CXX) -c -o $(@:s/.obj/.o/) $< && touch $@

