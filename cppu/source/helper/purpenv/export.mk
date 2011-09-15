
.IF "$(COM)"=="GCC"
NAMEpurpenv_helper := uno_purpenvhelper$(COMID)
purpenv_helper_LIB := -l$(NAMEpurpenv_helper)
.ELSE
NAMEpurpenv_helper := purpenvhelper$(UDK_MAJOR)$(COMID)
purpenv_helper_LIB := $(LIBPRE) i$(NAMEpurpenv_helper).lib
.ENDIF
