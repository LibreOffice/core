# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
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

# user notifications and formatting

define gb_Output__format_type
$(call gb_Output_time)[$(word 2,$(1) build clean) $(2)]
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

define gb_Output_info
$(info [info  $(2)] $(1))
endef

define gb_Output_warn
$(warning $(NEWLINE)[WARN  $(2)] !!!$(NEWLINE)[WARN  $(2)] !!! $(1)$(NEWLINE)[WARN  $(2)] !!!)
endef

define gb_Output_time
$(if $(gb_TIME),$(shell date +"%k:%M:%S "))
endef

gb_Output_ESCAPE := $(shell echo|awk 'BEGIN { printf "%c", 27 }' -)
gb_Output_BELL := $(shell echo|awk 'BEGIN { printf "%c", 7 }' -)

# default to color output, if interactive
ifeq ($(origin gb_COLOR),undefined)
ifneq ($(MAKE_TERMOUT),)
gb_COLOR=$(true)
endif
endif

# only enable colorized output if
# - gb_COLOR is set
# - we have a known term
KNOWN_TERM:=Eterm aterm gnome kterm linux putty rxvt rxvt-unicode screen xterm xterm xtermc cygwin
KNOWN_TERM+=$(patsubst %,%-color,$(KNOWN_TERM))
KNOWN_TERM+=$(patsubst %-color,%-256color,$(KNOWN_TERM))
KNOWN_TERM+=$(patsubst %-color,%+256color,$(KNOWN_TERM))
KNOWN_TERM+=$(patsubst %,screen.%,$(KNOWN_TERM))
ifneq ($(strip $(gb_COLOR)),)
ifneq ($(filter $(TERM),$(KNOWN_TERM)),)

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
$(call gb_Output_time)$(subst :, ,$(word 2,$(1) \
	$(gb_Output_COLOR_OUTBUILD_LEVEL$(3))[$(gb_Output_COLOR_INBUILD_LEVEL$(3))$(subst $(WHITESPACE),:,$(2))$(gb_Output_COLOR_OUTBUILD_LEVEL$(3))] \
	$(gb_Output_COLOR_OUTCLEAN_LEVEL$(3))[$(gb_Output_COLOR_INCLEAN_LEVEL$(3))$(subst $(WHITESPACE),:,$(2))$(gb_Output_COLOR_OUTCLEAN_LEVEL$(3))]))$(gb_Output_COLOR_RESET)
endef

define gb_Output_info
$(info $(gb_Output_COLOR_OUTBUILD_LEVEL6)[$(gb_Output_COLOR_INBUILD_LEVEL6)info  $(2)$(gb_Output_COLOR_OUTBUILD_LEVEL6)]$(gb_Output_COLOR_RESET) $(1))
endef

define gb_Output_warn
$(warning $(NEWLINE)$(gb_Output_COLOR_OUTCLEAN_LEVEL6)[$(gb_Output_COLOR_INCLEAN_LEVEL6)WARN  $(2)$(gb_Output_COLOR_OUTCLEAN_LEVEL6)]$(gb_Output_COLOR_RESET) !!!$(NEWLINE)$(gb_Output_COLOR_OUTCLEAN_LEVEL6)[$(gb_Output_COLOR_INCLEAN_LEVEL6)WARN  $(2)$(gb_Output_COLOR_OUTCLEAN_LEVEL6)]$(gb_Output_COLOR_RESET) !!! $(1)$(NEWLINE)$(gb_Output_COLOR_OUTCLEAN_LEVEL6)[$(gb_Output_COLOR_INCLEAN_LEVEL6)WARN  $(2)$(gb_Output_COLOR_OUTCLEAN_LEVEL6)]$(gb_Output_COLOR_RESET) !!!)
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
ifneq ($(filter $(TERM),$(KNOWN_TERM)),)
define gb_Output_announce_title
$(info $(gb_Output_ESCAPE)]2;gbuild: $(1)$(gb_Output_BELL)$(gb_Output_ESCAPE)[A)
endef

$(call gb_Output_announce_title,...)

endif
endif

# only enable bell output if
# - gb_BELL is set
# - gb_TTY is true (not piping to a file)
ifneq ($(strip $(gb_BELL)),)
define gb_Output_announce_bell
$(info $(gb_Output_BELL)$(gb_Output_ESCAPE)[A)
endef
endif

define gb_Output_announce
$(info $(call gb_Output__format_type,$(2),$(3),$(4)) $(call gb_Output__format_target,$(1)))
endef


# vim: set noet sw=4:
