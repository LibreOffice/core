# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# Copy CEF runtime binaries and resources to instdir/program/
# Only active when ENABLE_CEF=TRUE (--with-cef was specified)
#

ifeq ($(ENABLE_CEF),TRUE)

# CEF Release DLLs -> instdir/program/
$(eval $(call gb_Package_Package,cef,$(CEF_DIR)/Release))

$(eval $(call gb_Package_add_files,cef,$(LIBO_BIN_FOLDER),\
    libcef.dll \
    chrome_elf.dll \
    d3dcompiler_47.dll \
    libEGL.dll \
    libGLESv2.dll \
    vk_swiftshader.dll \
    vulkan-1.dll \
    v8_context_snapshot.bin \
))

endif

# vim: set noet sw=4 ts=4:
