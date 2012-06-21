#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

PRJ=..$/..

PRJNAME=sysui
TARGET=mandriva

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Product Version Information ----------------------------------

.INCLUDE :  ../productversion.mk

# --- Files --------------------------------------------------------
 
.IF "$(PKGFORMAT)"!="$(PKGFORMAT:s/rpm//)"

SPECFILE=$(TARGET)-menus.spec
RPMFLAGFILES=$(foreach,i,{$(PRODUCTLIST)} $(PKGDIR)$/$i-$(SPECFILE:b)-$(PKGVERSION.$i)-$(PKGREV).noarch.flag)

.ENDIF

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE : ../tg_rpm.mk

.IF "$(PKGFORMAT)"!="$(PKGFORMAT:s/rpm//)"

$(RPMFLAGFILES) : $(COMMONMISC)$/{$(PRODUCTLIST)}$/build.flag

.ENDIF
