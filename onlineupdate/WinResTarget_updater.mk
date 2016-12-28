$(eval $(call gb_WinResTarget_WinResTarget,updaterres))

$(eval $(call gb_WinResTarget_set_include,updaterres,\
	$$(INCLUDE) \
	-I$(SRCDIR)/onlineupdate/source/update/common \
	-I$(SRCDIR)/onlineupdate/source/update/updater \
))

$(eval $(call gb_WinResTarget_add_dependencies,updaterres, \
	onlineupdate/source/update/updater/updater.ico \
))

$(eval $(call gb_WinResTarget_set_rcfile,updaterres, \
	onlineupdate/source/update/updater/updater \
))
