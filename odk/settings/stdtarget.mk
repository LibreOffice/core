.SUFFIXES:

.SECONDARY:

ifeq "$(DEBUG)" "yes"
debug : ALL
endif

.PHONY: install
install: $(REGISTERFLAG)
