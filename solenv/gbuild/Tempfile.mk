# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-

ifeq ($(HAVE_GNUMAKE_FILE_FUNC),YES)
define var2file
$(file >$(1),$(3))$(1)
endef
else
# Write string to temporary file by chopping into pieces that
# fit the commandline
# parameters: filename, maxitems (for one write), string
# returns: filename
define var2file
$(strip $(1)
$(eval gb_var2file_helpervar := $$(shell printf "%s" "" > $(1) ))\
$(foreach item,$(3),$(eval gb_var2file_curblock += $(item)
	ifeq ($$(words $$(gb_var2file_curblock)),$(2)) 
		gb_var2file_helpervar := $$(shell printf "%s" "$$(gb_var2file_curblock)" >> $(1) )
		gb_var2file_curblock :=
	endif
	))\
	$(eval gb_var2file_helpervar := $(shell echo " "$(gb_var2file_curblock) >> $(1) )
		gb_var2file_curblock :=
	))
endef
endif

# vim: set noet sw=4 ts=4:
