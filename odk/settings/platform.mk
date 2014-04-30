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


# Map target platform (extension cfg) and extension platform (SDK settings)
# See:
# http://wiki.openoffice.org/wiki/Documentation/DevGuide/Extensions/Target_Platform
# odk/settings/settings.mk

# only make version 3.81 or later accepts the multiple else conditions
ifeq "$(UNOPKG_PLATFORM)" "FreeBSD_x86"
    EXTENSION_PLATFORM=freebsd_x86
else
    ifeq "$(UNOPKG_PLATFORM)" "FreeBSD_x86_64"
        EXTENSION_PLATFORM=freebsd_x86_64
    else
    ifeq "$(UNOPKG_PLATFORM)" "Linux_PowerPC"
        EXTENSION_PLATFORM=linux_powerpc
    else
    ifeq "$(UNOPKG_PLATFORM)" "Linux_x86"
        EXTENSION_PLATFORM=linux_x86
    else
    ifeq "$(UNOPKG_PLATFORM)" "Linux_x86_64"
        EXTENSION_PLATFORM=linux_x86_64
    else
    ifeq "$(UNOPKG_PLATFORM)" "MacOSX_PowerPC"
        EXTENSION_PLATFORM=macosx_powerpc
    else
    ifeq "$(UNOPKG_PLATFORM)" "MacOSX_x86"
        EXTENSION_PLATFORM=macosx_x86
    else
    ifeq "$(UNOPKG_PLATFORM)" "MacOSX_x86_64"
        EXTENSION_PLATFORM=macosx_x86_64
    else
    ifeq "$(UNOPKG_PLATFORM)" "Solaris_SPARC"
        EXTENSION_PLATFORM=solaris_sparc
    else
    ifeq "$(UNOPKG_PLATFORM)" "Solaris_x86"
        EXTENSION_PLATFORM=solaris_x86
    else
    ifeq "$(UNOPKG_PLATFORM)" "Windows"
        EXTENSION_PLATFORM=windows_x86
    endif
    endif
    endif
    endif
    endif
    endif
    endif
    endif
    endif
    endif
endif
