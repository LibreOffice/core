
.IF "$(GUI)"=="UNX" || "$(GUI)"=="MAC"
NAMEpurpenv_helper := uno_purpenvhelper$(COMID)

purpenv_helper_LIB := -l$(NAMEpurpenv_helper)

.ELIF "$(GUI)"=="OS2"
NAMEpurpenv_helper := purpeh$(UDK_MAJOR)
purpenv_helper_LIB := i$(NAMEpurpenv_helper)

.ELSE
NAMEpurpenv_helper := purpenvhelper$(UDK_MAJOR)$(COMID)
.IF "$(COM)"=="GCC"
purpenv_helper_LIB := -l$(NAMEpurpenv_helper)
.ELSE
purpenv_helper_LIB := $(LIBPRE) i$(NAMEpurpenv_helper).lib
.ENDIF

.ENDIF
