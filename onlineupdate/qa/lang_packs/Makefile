gb_Side := host
include ../../../config_host.mk

UPDATER_DIR := $(WORKDIR)/updater/lang_pack

all : call

unpack:
	@echo "Unpacking the updater test project"
	@rm -r $(UPDATER_DIR) || true
	@mkdir -p $(WORKDIR)/updater/
	@unzip updater_lang.zip -d $(WORKDIR)/updater > /dev/null
	@cp update_signed.mar $(UPDATER_DIR)/user/patch/update.mar
	@cp update_en_signed.mar $(UPDATER_DIR)/user/patch/update_en_signed.mar

call: unpack
	@echo "Update the test project"
	@$(INSTDIR)/program/updater $(UPDATER_DIR)/user/patch $(UPDATER_DIR) $(UPDATER_DIR)/user/update -1

call-gdb: unpack
	gdb --args $(INSTDIR)/program/updater $(UPDATER_DIR)/user/patch $(UPDATER_DIR) $(UPDATER_DIR)/user/update -1
