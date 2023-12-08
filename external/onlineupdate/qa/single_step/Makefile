gb_Side := host
include ../../../config_host.mk

UPDATER_DIR := $(WORKDIR)/updater/single_step

all : call

unpack:
	@echo "Unpacking the updater test project"
	@rm -r $(UPDATER_DIR) || true
	@mkdir -p $(UPDATER_DIR)
	@unzip single_step.zip -d $(WORKDIR)/updater > /dev/null
	@mkdir -p $(UPDATER_DIR)/patch

call-complete: unpack
	@echo "Update the test project"
	@cp complete_signed.mar $(UPDATER_DIR)/patch/update.mar
	@$(INSTDIR)/program/updater $(UPDATER_DIR)/patch $(UPDATER_DIR) $(UPDATER_DIR) 0

call-incremental: unpack
	@echo "Update the test project"
	@cp incremental_signed.mar $(UPDATER_DIR)/patch/update.mar
	@$(INSTDIR)/program/updater $(UPDATER_DIR)/patch $(UPDATER_DIR) $(UPDATER_DIR) 0

call-gdb: unpack
	@cp incremental_signed.mar $(UPDATER_DIR)/patch/update.mar
	gdb --args $(INSTDIR)/program/updater $(UPDATER_DIR)/patch $(UPDATER_DIR) $(UPDATER_DIR) 0
