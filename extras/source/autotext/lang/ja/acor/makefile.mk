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

PRJ=..$/..$/..$/..$/..

PRJNAME=extras
TARGET=autotext

.INCLUDE : settings.mk
.INCLUDE : $(PRJ)$/util$/target.pmk

FLAGS = -r
FILES = $(shell ls  | grep -v makefile.mk )
TFILES = $(shell ls *.xml && ls */*.xml)

NAME =acor_ja-JP
EXT = .dat

zip1:
		mkdir $(MISC)/$(NAME) && cp -r $(FILES) $(MISC)/$(NAME)
		$(foreach,i,$(TFILES) $(shell $(XSLTPROC) -o $(MISC)/$(NAME)/$i $(PRJ)/util/compact.xsl $(MISC)/$(NAME)/$i))
		cd $(MISC)/$(NAME) && zip $(FLAGS)  ../$(NAME)$(EXT) $(FILES)
		rm -r $(MISC)/$(NAME)




.INCLUDE : target.mk#
