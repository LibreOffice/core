#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.13 $
#
#   last change: $Author: kz $ $Date: 2003-10-15 10:08:48 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************
PRJ=..$/..

PRJNAME=officecfg
TARGET=schema

.INCLUDE :  settings.mk

# --- Targets ------------------------------------------------------

XCSFILES= \
    org$/openoffice$/Inet.xcs \
    org$/openoffice$/Office$/Addons.xcs \
    org$/openoffice$/Office$/Calc.xcs \
    org$/openoffice$/Office$/Chart.xcs \
    org$/openoffice$/Office$/Commands.xcs \
    org$/openoffice$/Office$/Common.xcs \
    org$/openoffice$/Office$/Compatibility.xcs \
    org$/openoffice$/Office$/DataAccess.xcs \
    org$/openoffice$/Office$/Draw.xcs \
    org$/openoffice$/Office$/Impress.xcs \
    org$/openoffice$/Office$/Java.xcs \
    org$/openoffice$/Office$/Jobs.xcs \
    org$/openoffice$/Office$/Labels.xcs \
    org$/openoffice$/Office$/Linguistic.xcs \
    org$/openoffice$/Office$/Math.xcs \
    org$/openoffice$/Office$/ProtocolHandler.xcs \
    org$/openoffice$/Office$/SFX.xcs \
    org$/openoffice$/Office$/Substitution.xcs \
    org$/openoffice$/Office$/TypeDetection.xcs \
    org$/openoffice$/Office$/UI.xcs \
    org$/openoffice$/Office$/Views.xcs \
    org$/openoffice$/Office$/Writer.xcs \
    org$/openoffice$/Office$/WriterWeb.xcs \
    org$/openoffice$/Office$/Security.xcs \
    org$/openoffice$/Setup.xcs \
    org$/openoffice$/UserProfile.xcs \
    org$/openoffice$/VCL.xcs \
    org$/openoffice$/ucb$/Configuration.xcs \
    org$/openoffice$/ucb$/Hierarchy.xcs \
    org$/openoffice$/ucb$/Store.xcs

# transform xcs format to properties format
RESOURCES1=$(foreach,j,$(foreach,i,$(XCSFILES) $(MISC)$/registry$/res$/$i) $(foreach,k,$(alllangext) $(subst,$/res,$/res$/{$(iso_$(k))} $j)))
RESOURCES=$(foreach,i,$(RESOURCES1) $(subst,.xcs,.properties $i))

# remove unnecessary info from the component schemas
TRIMXSL=$(foreach,i,$(XCSFILES) $(MISC)$/registry$/schema$/{$(subst,.xcs,.xcs $i)})

UNIXTEXT= \
    $(MISC)$/oo-org-map.properties \
    $(MISC)$/oo-ldap-attr-map.properties \
    $(MISC)$/oo-common.conf \
    $(MISC)$/oo-common.ldif	\
    $(MISC)$/oo-common-ad.ldf

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

ALLTAR: \
    $(TRIMXSL) \
    $(UNIXTEXT) \
    $(RESOURCES)
