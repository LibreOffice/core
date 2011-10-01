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
# Tor Lillqvist <tml@iki.fi> (initial developer)
# Jan Holesovsky <kendy@suse.cz>
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

PRJ=..
PRJNAME=external
TARGET=mingw-dlls

.INCLUDE: settings.mk

# ------------------------------------------------------------------

.IF "$(GUI)$(COM)" != "WNTGCC"

dummy:
    @echo "Nothing to do."

.ELSE

MINGW_DLLS:=$(MINGW_EXTERNAL_DLLS)

.IF "$(MINGW_SHARED_GCCLIB)" == "YES"
MINGW_DLLS+=$(MINGW_GCCDLL)
.ENDIF

.IF "$(MINGW_SHARED_GXXLIB)" == "YES"
MINGW_DLLS+=$(MINGW_GXXDLL)
.ENDIF

# Guesstimate where the DLLs can be

POTENTIAL_MINGW_RUNTIME_BINDIRS = \
	$(COMPATH)/i686-w64-mingw32/sys-root/mingw/bin \
	/usr/i686-w64-mingw32/sys-root/mingw/bin \

all:
	@for DLL in $(MINGW_DLLS) ; do \
	    for D in $(POTENTIAL_MINGW_RUNTIME_BINDIRS); do \
	       test -f $$D/$$DLL && $(COPY) -p $$D/$$DLL $(BIN)$/ && break; \
	    done ; \
	    test -f $(BIN)$/$$DLL || { echo Could not find $$DLL in none of $(POTENTIAL_MINGW_RUNTIME_BINDIRS) ; exit 1 ; } ; \
	done

.ENDIF

# ------------------------------------------------------------------

.INCLUDE: target.mk
