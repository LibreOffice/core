gb_Side := host
include ../../../config_host.mk

unpack:
	@echo "Unpacking the updater test project"
	@rm -r $(WORKDIR)/updater || true
	@unzip updater.zip -d $(WORKDIR) > /dev/null

call: unpack
	@echo "Update the test project"
	@$(INSTDIR)/program/updater $(WORKDIR)/updater/user/patch $(WORKDIR)/updater $(WORKDIR)/updater/user/update /replace

call-gdb: unpack
	gdb --args $(INSTDIR)/program/updater $(WORKDIR)/updater/user/patch $(WORKDIR)/updater $(WORKDIR)/updater/user/update /replace
