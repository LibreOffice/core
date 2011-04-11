# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original Code is
#       Red Hat, Inc.
# Portions created by the Initial Developer are Copyright (C) 2010 the
# Initial Developer. All Rights Reserved.
#
# Contributor(s): David Tardon <dtardon@redhat.com>
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

PRJ=..$/..$/..

PRJNAME=scripting
TARGET=jsprov

.INCLUDE : settings.mk

.IF "$(L10N_framework)"=="" && "$(SOLAR_JAVA)"!=""

EXTENSIONNAME:=ScriptProviderForJavaScript
EXTENSION_ZIPNAME:=script-provider-for-javascript

COMPONENT_JARFILES=$(EXTENSIONDIR)$/$(EXTENSIONNAME).jar
EXTENSION_PACKDEPS=$(SOLARBINDIR)$/js.jar

.IF "$(ENABLE_SCRIPTING_JAVASCRIPT)" != "YES"
@all:
    @echo "Script Provider for JavaScript build disabled."
.ENDIF

.INCLUDE : extension_pre.mk
.INCLUDE : target.mk
.INCLUDE : extension_post.mk

ALLTAR : $(EXTENSIONDIR)$/js.jar

$(EXTENSIONDIR)$/js.jar : $(SOLARBINDIR)$/js.jar
    @@-$(MKDIRHIER) $(@:d)
    $(COMMAND_ECHO)$(COPY) $< $@

.ELSE

.INCLUDE : target.mk

.ENDIF
