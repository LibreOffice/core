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

PRJNAME=pyuno
PRJ=..

.INCLUDE : settings.mk
.INCLUDE : pyversion.mk

ROOT=$(MISC)/pyuno-doc

FILES=\
    $(ROOT)/python-bridge.html \
    $(ROOT)/customized_setup.png \
    $(ROOT)/mode_component.png \
    $(ROOT)/mode_ipc.png \
    $(ROOT)/modes.sxd \
    $(ROOT)/optional_components.png \
    $(ROOT)/samples/swriter.py \
    $(ROOT)/samples/swritercomp.py \
    $(ROOT)/samples/ooextract.py \
    $(ROOT)/samples/biblioaccess.py \
    $(ROOT)/samples/swritercompclient.py \
    $(ROOT)/samples/hello_world_pyuno.zip


$(MISC)/pyuno-doc.zip : dirs $(FILES)
	-rm -f $@
	cd $(MISC) && zip -r pyuno-doc.zip pyuno-doc

dirs .PHONY :
	-mkdir $(ROOT)
	-mkdir $(ROOT)/samples

$(ROOT)/samples/hello_world_pyuno.zip : hello_world_comp.py Addons.xcu
	-rm -f $@
	zip $@ hello_world_comp.py Addons.xcu

$(ROOT)/samples/% : %
	-rm -f $@
	$(COPY) $? $@

$(ROOT)/% : ../doc/%
	-rm -f $@
	$(COPY) $? $@
