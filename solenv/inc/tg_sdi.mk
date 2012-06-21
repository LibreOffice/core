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

#######################################################
# instructions for ???
# unroll begin

.IF "$(SDI$(TNR)TARGET)"!=""
$(SDI$(TNR)TARGET): $(SVSDI$(TNR)DEPEND) $(SDI$(TNR)NAME).sdi
    @echo "Making:   " $(@:f)
    @-$(RM) $@
    $(COMMAND_ECHO)$(SVIDL) @$(mktmp \
    $(VERBOSITY) \
    -fs$(INCCOMX)/$(SDI$(TNR)NAME).hxx	\
    -fd$(INCCOMX)/$(SDI$(TNR)NAME).ilb	\
    -fm$(MISC)/$(SDI$(TNR)NAME).don	\
    -fl$(MISC)/$(SDI$(TNR)NAME).lst         \
    -fx$(SDI$(TNR)EXPORT).sdi		\
    -fz$(MISC)/$(SDI$(TNR)NAME).sid	\
    $(SDI$(TNR)NAME).sdi -I$(MISC) -I$(SVSDIINC) -I$(INC) $(INCLUDE))
.ENDIF # "$(SDI$(TNR)TARGET)"!=""

# Instruction for ???
# unroll end
#######################################################

