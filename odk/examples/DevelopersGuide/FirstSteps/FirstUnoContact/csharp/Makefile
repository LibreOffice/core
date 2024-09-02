# Builds the FirstUnoContact example of the Developers Guide.

PRJ = ../../../../..
SETTINGS = $(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk

# Settings
APP_NAME = FirstUnoContact
APP_LANG = cs

APP_SRC_DIR = $(subst /,$(PS),$(CURDIR))
APP_BIN_DIR = $(subst /,$(PS),$(OUT_BIN))
APP_MISC_DIR = $(subst /,$(PS),$(OUT_MISC)/$(APP_NAME))

APP_PROJ_NAME = $(APP_NAME).$(APP_LANG)proj
APP_PROJ_FILE = $(APP_MISC_DIR)/$(APP_PROJ_NAME)

APP_EXE_NAME = $(APP_NAME)$(EXE_EXT)
APP_EXE_FILE = $(APP_BIN_DIR)/$(APP_EXE_NAME)

DOTNET_FLAGS = -c Release
LO_NUPKG_ID = LibreOffice.Bindings
LO_NUPKG_VERSION = 0.1.0
LO_NUPKG_DIR = $(abspath $(PRJ)/dotnet)

# Targets
.PHONY: ALL
ALL : $(APP_NAME)

include $(SETTINGS)/stdtarget.mk

$(APP_PROJ_FILE) :
	-$(MKDIR) $(@D)
	$(ECHO) "<Project Sdk=\"Microsoft.NET.Sdk\">" > $@
	$(ECHO) "    <PropertyGroup>" >> $@
	$(ECHO) "        <AssemblyName>$(APP_NAME)</AssemblyName>" >> $@
	$(ECHO) "        <TargetFramework>net8.0</TargetFramework>" >> $@
	$(ECHO) "        <OutputType>exe</OutputType>" >> $@
	$(ECHO) "        <PublishSingleFile>true</PublishSingleFile>" >> $@
	$(ECHO) "        <SelfContained>false</SelfContained>" >> $@
	$(ECHO) "        <IncludeAllContentForSelfExtract>true</IncludeAllContentForSelfExtract>" >> $@
	$(ECHO) "        <RestoreAdditionalProjectSources>$(LO_NUPKG_DIR)</RestoreAdditionalProjectSources>" >> $@
	$(ECHO) "    </PropertyGroup>" >> $@
	$(ECHO) "    <ItemGroup>" >> $@
	$(ECHO) "        <PackageReference Include=\"$(LO_NUPKG_ID)\" Version=\"$(LO_NUPKG_VERSION)\" />" >> $@
	$(ECHO) "        <Compile Include=\"$(APP_SRC_DIR)/*.$(APP_LANG)\" />" >> $@
	$(ECHO) "    </ItemGroup>" >> $@
	$(ECHO) "</Project>" >> $@
	$(ECHOLINE) >> $@

$(APP_EXE_FILE) : $(APP_PROJ_FILE)
	-$(MKDIR) $(@D)
	$(SDK_DOTNET) publish $< $(DOTNET_FLAGS) -o $(<D)
	$(COPY) $(<D)/$(APP_EXE_NAME) $@

.PHONY: $(APP_NAME)
$(APP_NAME) : $(APP_EXE_FILE)
	$(ECHO) --------------------------------------------------------
	$(ECHO) Use the following commands to run the example:
	$(ECHO) $(MAKE) run OR $(MAKE) $(APP_NAME).run
	$(ECHO)
	$(ECHO) And the following commands to clean the example:
	$(ECHO) $(MAKE) clean OR $(MAKE) $(APP_NAME).clean
	$(ECHO) --------------------------------------------------------

.PHONY: $(APP_NAME).run run
$(APP_NAME).run run : $(APP_EXE_FILE)
	cd $(<D) && ./$(<F)

.PHONY: $(APP_NAME).clean clean
$(APP_NAME).clean clean :
	-$(DELRECURSIVE) $(APP_MISC_DIR)
	-$(DEL) $(APP_EXE_FILE)
