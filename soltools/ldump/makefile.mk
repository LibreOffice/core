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

PRJ=..

PRJNAME=ldump
TARGET=ldump
TARGETTYPE=CUI

# --- Settings -----------------------------------------------------

.INCLUDE : $(PRJ)$/util$/makefile.pmk
.INCLUDE :  settings.mk

UWINAPILIB=$(0)
LIBSALCPPRT=$(0)

# --- Files --------------------------------------------------------

# ldump only supports windows environment
.IF "$(GUI)"=="WNT"
.IF "$(COM)"!="GCC"
#ldump4 reimplements feature set of ldump2 and ldump3
APP1TARGET=	ldump4
.IF "$(GUI)"=="WNT"
APP1STACK=	16000
.ENDIF
APP1OBJS=   $(OBJ)$/ldump.obj $(OBJ)$/hashtbl.obj

.ENDIF #"$(COM)"!="GCC"
.ENDIF #"$(GUI)"=="WNT"

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
