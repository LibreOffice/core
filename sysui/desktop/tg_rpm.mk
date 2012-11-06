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

.IF "$(PKGFORMAT)"!="$(PKGFORMAT:s/rpm//)"

ALLTAR : $(RPMFLAGFILES)

$(RPMFLAGFILES) : $(SPECFILE)
    @-$(MKDIRHIER) $(@:d)
    -$(RM) $(@:d)$(@:b:s/-/ /:1)-$(SPECFILE:b)-*
    $(RPM) -bb $< $(RPMMACROS) \
        --buildroot $(ABSLOCALOUT)$/misc$/$(@:b) \
        --define "_builddir $(shell @cd $(COMMONMISC)$/$(@:b:s/-/ /:1) && pwd)" \
        --define "productname $(PRODUCTNAME.$(@:b:s/-/ /:1))" \
        --define "pkgprefix $(@:b:s/-/ /:1)$(PRODUCTVERSION.$(@:b:s/-/ /:1))" \
        --define "unixfilename $(UNIXFILENAME.$(@:b:s/-/ /:1))" \
        --define "productversion $(PRODUCTVERSION.$(@:b:s/-/ /:1))" \
        --define "iconprefix $(ICONPREFIX.$(@:b:s/-/ /:1))" \
        --define "version $(PKGVERSION.$(@:b:s/-/ /:1))" \
        --define "release $(PKGREV)" \
        --define "__debug_install_post %nil" \
        --define "_unpackaged_files_terminate_build  0" && $(TOUCH) $@
.ENDIF
