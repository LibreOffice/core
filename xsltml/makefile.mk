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

PRJ=.

PRJNAME=xsltml
TARGET=xsltml

.IF "$(ENABLE_MEDIAWIKI)" == "YES"

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

TARFILE_NAME=xsltml_2.1.2
TARFILE_MD5=a7983f859eafb2677d7ff386a023bc40
PATCH_FILES=$(TARFILE_NAME).patch
TARFILE_IS_FLAT:=TRUE

CONVERTFILES=\
    README \
    entities.xsl \
    glayout.xsl \
    mmltex.xsl \
    cmarkup.xsl \
    scripts.xsl \
    tables.xsl \
    tokens.xsl \

# --- Targets ------------------------------------------------------

.INCLUDE :	set_ext.mk
.INCLUDE :	target.mk
.INCLUDE :	tg_ext.mk

.ELSE
@all:
    @echo "MediaWiki Publisher extension disabled."
.ENDIF

