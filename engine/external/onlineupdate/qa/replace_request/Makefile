gb_Side := host
include ../../../config_host.mk

UPDATER_DIR := $(WORKDIR)/updater/replace_request

unpack:
	@echo "Unpacking the updater test project"
	@rm -r $(UPDATER_DIR) || true
	@mkdir -p $(WORKDIR)/updater
	@unzip updater.zip -d $(WORKDIR)/updater > /dev/null

call: unpack
	@echo "Update the test project"
	@$(INSTDIR)/program/updater $(UPDATER_DIR)/user/patch $(UPDATER_DIR) $(UPDATER_DIR)/user/update /replace

call-gdb: unpack
	gdb --args $(INSTDIR)/program/updater $(UPDATER_DIR)/user/patch $(UPDATER_DIR) $(UPDATER_DIR)/user/update /replace
