# Builds the Comonent Thumbs of the Developers Guide.

PRJ=../../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk
include $(SETTINGS)/dk.mk

include thumbs.mk

SUBDIRS= org/openoffice/test org/openoffice/comp/test

# Targets
.PHONY: ALL
ALL : \
	$(SUBDIRS) \
	ComponentsThumbsExample

include $(SETTINGS)/stdtarget.mk

.PHONY : $(SUBDIRS)
$(SUBDIRS) :
	$(MAKE) -C $@	  

org/openoffice/comp/test : org/openoffice/test

ComponentsThumbsExample : $(COMPONENT1_PACKAGE)
	@echo --------------------------------------------------------------------------------
	@echo The ImageShrink Java component was installed if SDK_AUTO_DEPLOYMENT = YES.
	@echo You can use this component inside your office installation, see the example
	@echo description.
	@echo --------------------------------------------------------------------------------

.PHONY: clean
clean :
	$(MAKE) -C org/openoffice/test clean
	$(MAKE) -C org/openoffice/comp/test clean
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_CLASS))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_GEN))
