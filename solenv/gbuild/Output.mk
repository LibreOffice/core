#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************


define gb_Output__format_type
[ $(word 2,$(1) build clean) $(2) ]
endef

define gb_Output__format_target
$(1)
endef

define gb_Output_error
$(error $(1))
endef

define gb_Output_announce_title
endef

define gb_Output_announce_bell
endef


gb_Output_ESCAPE := $(shell echo -e '\033')
gb_Output_BELL := $(shell echo -e '\033')

# only enable colorized output if
# - gb_COLOR is set
# - we have a known term
ifneq ($(strip $(gb_COLOR)),)
ifneq ($(filter $(TERM),Eterm aterm gnome kterm linux rxvt rxvt-unicode screen xterm xterm-color),)

gb_Output_COLOR_RESET := $(gb_Output_ESCAPE)[0m
gb_Output_COLOR_RESETANDESCAPE := $(gb_Output_COLOR_RESET)$(gb_Output_ESCAPE)

gb_Output_COLOR_OUTBUILD_LEVEL1 := $(gb_Output_COLOR_RESETANDESCAPE)[37;40m
gb_Output_COLOR_OUTBUILD_LEVEL2 := $(gb_Output_COLOR_RESETANDESCAPE)[37;40m
gb_Output_COLOR_OUTBUILD_LEVEL3 := $(gb_Output_COLOR_RESETANDESCAPE)[37;40m
gb_Output_COLOR_OUTBUILD_LEVEL4 := $(gb_Output_COLOR_RESETANDESCAPE)[37;40m
gb_Output_COLOR_OUTBUILD_LEVEL5 := $(gb_Output_COLOR_RESETANDESCAPE)[37;1;46m
gb_Output_COLOR_OUTBUILD_LEVEL6 := $(gb_Output_COLOR_RESETANDESCAPE)[37;1;44m

gb_Output_COLOR_INBUILD_LEVEL1 := $(gb_Output_COLOR_RESETANDESCAPE)[36;40m
gb_Output_COLOR_INBUILD_LEVEL2 := $(gb_Output_COLOR_RESETANDESCAPE)[36;1;40m
gb_Output_COLOR_INBUILD_LEVEL3 := $(gb_Output_COLOR_RESETANDESCAPE)[32;40m
gb_Output_COLOR_INBUILD_LEVEL4 := $(gb_Output_COLOR_RESETANDESCAPE)[32;1;40m
gb_Output_COLOR_INBUILD_LEVEL5 := $(gb_Output_COLOR_RESETANDESCAPE)[37;1;46m
gb_Output_COLOR_INBUILD_LEVEL6 := $(gb_Output_COLOR_RESETANDESCAPE)[37;1;44m

gb_Output_COLOR_OUTCLEAN_LEVEL1 := $(gb_Output_COLOR_RESETANDESCAPE)[37;40m
gb_Output_COLOR_OUTCLEAN_LEVEL2 := $(gb_Output_COLOR_RESETANDESCAPE)[37;40m
gb_Output_COLOR_OUTCLEAN_LEVEL3 := $(gb_Output_COLOR_RESETANDESCAPE)[37;40m
gb_Output_COLOR_OUTCLEAN_LEVEL4 := $(gb_Output_COLOR_RESETANDESCAPE)[37;40m
gb_Output_COLOR_OUTCLEAN_LEVEL5 := $(gb_Output_COLOR_RESETANDESCAPE)[33;1;41m
gb_Output_COLOR_OUTCLEAN_LEVEL6 := $(gb_Output_COLOR_RESETANDESCAPE)[37;1;41m

gb_Output_COLOR_INCLEAN_LEVEL1 := $(gb_Output_COLOR_RESETANDESCAPE)[33;40m
gb_Output_COLOR_INCLEAN_LEVEL2 := $(gb_Output_COLOR_RESETANDESCAPE)[33;1;40m
gb_Output_COLOR_INCLEAN_LEVEL3 := $(gb_Output_COLOR_RESETANDESCAPE)[31;40m
gb_Output_COLOR_INCLEAN_LEVEL4 := $(gb_Output_COLOR_RESETANDESCAPE)[31;1;40m
gb_Output_COLOR_INCLEAN_LEVEL5 := $(gb_Output_COLOR_RESETANDESCAPE)[33;1;41m
gb_Output_COLOR_INCLEAN_LEVEL6 := $(gb_Output_COLOR_RESETANDESCAPE)[37;1;41m

gb_Output_COLOR_ERROR := $(gb_Output_COLOR_RESETANDESCAPE)[37;1;41m

define gb_Output__format_type
$(subst :, ,$(word 2,$(1) \
    $(gb_Output_COLOR_OUTBUILD_LEVEL$(3))[:$(gb_Output_COLOR_INBUILD_LEVEL$(3))_.oO:$(subst $(WHITESPACE),:,$(2))$(gb_Output_COLOR_OUTBUILD_LEVEL$(3)):] \
    $(gb_Output_COLOR_OUTCLEAN_LEVEL$(3))[:$(gb_Output_COLOR_INCLEAN_LEVEL$(3))Xx._:$(subst $(WHITESPACE),:,$(2))$(gb_Output_COLOR_OUTCLEAN_LEVEL$(3)):]))$(gb_Output_COLOR_RESET)
endef

define gb_Output_error
$(error $(gb_Output_COLOR_ERROR)$(1)$(gb_Output_COLOR_RESET))
endef

endif
endif

# only enable title output if
# - gb_TITLES is set
# - we have a known term
ifneq ($(strip $(gb_TITLES)),)
ifneq ($(filter $(TERM),Eterm aterm gnome kterm rxvt rxvt-unicode screen xterm xterm-color),)
define gb_Output_announce_title
$(shell echo -en '\033]2;gbuild: $(1)\07' > `tty`)
endef

$(call gb_Output_announce_title,...)

endif
endif

# only enable bell output if
# - gb_BELL is set
# - gb_TTY is true (not piping to a file)
ifneq ($(strip $(gb_BELL)),)
define gb_Output_announce_bell
$(shell echo -en '\07' > `tty`)
endef
endif

define gb_Output_announce
$(info $(call gb_Output__format_type,$(2),$(3),$(4)) $(call gb_Output__format_target,$(1)))
endef


# vim: set noet sw=4 ts=4:
