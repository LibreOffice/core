#
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
#       Kálmán Szalai - KAMI <kami911@gmail.com>
# Portions created by the Initial Developer are Copyright (C) 2010 the
# Initial Developer. All Rights Reserved.
#
# Contributor(s): Kálmán Szalai - KAMI <kami911@gmail.com>
#
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.
#

PRJ=..$/..

PRJPCH=

PRJNAME=scp2
TARGET=accessories
TARGETTYPE=CUI

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

SCP_PRODUCT_TYPE=osl
PARFILES= \
        module_accessories.par
.IF "$(WITH_EXTRA_GALLERY)" != ""
PARFILES += \
        module_gallery_accessories.par     \
        file_gallery_accessories.par
.ENDIF
.IF "$(WITH_EXTRA_TEMPLATE)" != ""
PARFILES += \
        module_templates_accessories.par   \
        file_templates_accessories.par
.ENDIF
.IF "$(WITH_EXTRA_SAMPLE)" != ""
PARFILES += \
        module_samples_accessories.par     \
        file_samples_accessories.par
.ENDIF
.IF "$(WITH_EXTRA_FONT)" != ""
PARFILES += \
        module_font_accessories.par        \
        file_font_accessories.par
.ENDIF

ULFFILES= \
        module_accessories.ulf
.IF "$(WITH_EXTRA_GALLERY)" != ""
ULFFILES += \
        module_gallery_accessories.ulf
.ENDIF
.IF "$(WITH_EXTRA_TEMPLATE)" != ""
ULFFILES += \
        module_templates_accessories.ulf
.ENDIF
.IF "$(WITH_EXTRA_SAMPLE)" != ""
ULFFILES += \
        module_samples_accessories.ulf
.ENDIF
.IF "$(WITH_EXTRA_FONT)" != ""
ULFFILES += \
        module_font_accessories.ulf
.ENDIF


# --- File ---------------------------------------------------------
.INCLUDE :  target.mk
