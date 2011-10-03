#
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# [ Copyright (C) 2011 SUSE <cbosdonnat@suse.com> (initial developer) ]
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.
#

PRJ=..$/..$/..
PRJNAME=ucb
# Version
UCPCMIS_MAJOR=1
TARGET=ucpcmis
ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE
NO_BSYMBOLIC=TRUE

.INCLUDE: settings.mk
.IF "$(L10N_framework)"==""

.IF "$(ENABLE_CMIS)"!=""

# no "lib" prefix
DLLPRE =

SLOFILES=$(SLO)$/cmis_provider.obj\
         $(SLO)$/cmis_content.obj \
		 $(SLO)$/cmis_url.obj

SHL1TARGET=$(TARGET)$(UCPCMIS_MAJOR).uno
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1LIBS=$(SLB)$/$(TARGET).lib
SHL1IMPLIB=i$(TARGET)
SHL1STDLIBS=\
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(SALLIB)  \
    $(SALHELPERLIB)  \
    $(UCBHELPERLIB) \
	$(TOOLSLIB) \
	$(CMISLIB)

SHL1VERSIONMAP=$(SOLARENV)/src/component.map

.ENDIF          # "$(ENABLE_CMIS)"!=""
.ENDIF # L10N_framework

.INCLUDE: target.mk

ALLTAR : $(MISC)/ucpcmis.component

$(MISC)/ucpcmis.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        ucpcmis.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt ucpcmis.component
