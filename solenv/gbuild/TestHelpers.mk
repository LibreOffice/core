# -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# This makes sure that more_fonts, opensymbol and fontconfig is installed
define gb_TestHelpers_use_more_fonts

ifneq (,$$(filter MORE_FONTS,$$(BUILD_TYPE)))
ifeq (,$$(filter more_fonts,$$(gb_Module_ALLMODULES)))

# the gbuildtojson / gbuild self-test runs tests "out of scope", but itself depends
# on more fonts, so we must ignore the font tests for these tests.
ifeq (,$$(gb_IGNORE_MORE_FONTS))

# we are in single module mode -> fail test on missing fonts from filelists!
# include Module_more_font.mk fails with strange error messages,
# so we can't check if the more_fonts filelists are up-to-date, so just assume it.

$(1): \
    $$(foreach font,$$(gb_Package_MODULE_ooo_fonts), \
        $$(if $$(wildcard $$(call gb_Package_get_target,$$(font))), \
            $$(foreach file,$$(shell cat $$(call gb_Package_get_target,$$(font))), \
                $$(if $$(wildcard $$(file)),,$$(error Missing font -> run make more_fonts extras))), \
            $$(error Missing font filelist -> run make more_fonts extras)))
endif
else
# we're in global run mode (full knowledge) -> depend directly on the delivered fonts!

$(1): \
    $$(foreach package,$$(subst ExternalPackage_,,$$(filter ExternalPackage_%,$$(call gb_Module_get_classnames,more_fonts))), \
        $$(call gb_Package_get_files,$$(package))) \
    $$(call gb_Package_get_files,extras_font) \
    $$(if $$(filter Package_extras_fontconfig,$$(call gb_Module_get_classnames,extras)), \
        $$(call gb_Package_get_files,extras_fontconfig))
endif
endif

endef

# vim: set noet sw=4:
