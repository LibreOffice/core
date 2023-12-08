$(eval $(call gb_WinResTarget_WinResTarget,updaterres))

$(call gb_WinResTarget_get_target,updaterres,updaterres): | \
    $(call gb_UnpackedTarball_get_target,onlineupdate)

$(eval $(call gb_WinResTarget_set_include,updaterres,\
	$$(INCLUDE) \
	-I$(call gb_UnpackedTarball_get_dir,onlineupdate)/onlineupdate/source/update/common \
	-I$(call gb_UnpackedTarball_get_dir,onlineupdate)/onlineupdate/source/update/updater \
))

$(call gb_WinResTarget_get_clean_target,updaterres): \
    RCFILE := $(call gb_UnpackedTarball_get_dir,onlineupdate)/onlineupdate/source/update/updater/updater.rc
$(call gb_WinResTarget_get_target,updaterres): \
    RCFILE := $(call gb_UnpackedTarball_get_dir,onlineupdate)/onlineupdate/source/update/updater/updater.rc
