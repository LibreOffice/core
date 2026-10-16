# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,united))

# gb_UNITEDLIBS is defined in solenv/gbuild/extensions/pre_UnitedLibsList.mk
$(eval $(call gb_Library_use_library_objects,united,\
	$(gb_UNITEDLIBS) \
))

# We cannot merge these libraries because:
#  emser - contains a DllMain function that will conflict with the DllMain in sal/
#  inprocserv - contains a DllMain function that will conflict with the DllMain in sal/
#  oleautobridge - contains a DllMain function that will conflict with the DllMain in sal/
#
$(eval $(call gb_Library_use_libraries,united,\
    $(if $(filter WNT,$(OS)),emser) \
    $(if $(filter WNT,$(OS)),inprocserv) \
    $(if $(filter WNT,$(OS)),oleautobridge) \
))

$(eval $(call gb_Library_use_static_libraries,united,\
	findsofficepath \
	ulingu \
	$(if $(ENABLE_CURL),ulinguremote) \
))

$(eval $(call gb_Library_use_externals,united,\
	abw \
	$(if $(filter AFDKO,$(BUILD_TYPE)),afdko) \
	argon2 \
	boost_headers \
	boost_date_time \
	boost_locale \
    $(if $(USE_HEADLESS_CODE), cairo) \
	cdr \
	$(if $(filter OPENCL,$(BUILD_TYPE)),clew) \
	$(call gb_Helper_optionals_and,DESKTOP XMLHELP,clucene) \
	$(if $(ENABLE_COINMP),coinmp) \
	$(if $(filter TRUE-TRUE,$(USE_HEADLESS_CODE)-$(ENABLE_CPDB)), cpdb) \
	$(if $(filter TRUE-TRUE,$(USE_HEADLESS_CODE)-$(ENABLE_CUPS)), cups) \
	curl \
	$(if $(USING_X11),dbus) \
	dragonbox \
	ebook \
	epubgen \
	etonyek \
	expat \
	fast_float \
	$(if $(USE_HEADLESS_CODE), fontconfig) \
	$(if $(USE_HEADLESS_CODE), freetype) \
	freehand \
	frozen \
	gio \
	graphite \
	$(if $(ENABLE_GPGMEPP),gpgmepp) \
	harfbuzz \
	hunspell \
	hyphen \
	icu_headers \
	icui18n \
	icuuc \
	libcmis \
	lcms2 \
	libeot \
	libexslt \
	libexttextcat \
	libjpeg \
	liblangtag \
	libnumbertext \
	libpng \
	librdf \
	libtiff \
	libwebp \
	libxml2 \
	libxslt \
	md4c \
	mspub \
	mwaw \
	mythes \
	odfgen \
	$(if $(ENABLE_OPENSSL),openssl) \
	orcus-parser \
	orcus \
	pagemaker \
	$(if $(filter PDFIUM,$(BUILD_TYPE)),pdfium) \
	$(call gb_Helper_optional,PYTHON,python) \
	$(call gb_Helper_optional,QUICKJS,qjs) \
	qxp \
	redland_headers \
	raptor_headers \
	rasqal_headers \
	revenge \
	$(if $(DISABLE_GUI),,$(if $(filter SKIA,$(BUILD_TYPE)),skia)) \
	staroffice \
	$(if $(USING_X11),valgrind) \
	visio \
	wpd \
	wpg \
	wps \
	$(if $(ENABLE_GEN),Xrender) \
	yrs \
	zlib \
	zmf \
	zstd \
	zxcvbn-c \
	zxing \
	$(if $(ENABLE_NSS),nss3) \
	$(if $(ENABLE_NSS),nssutil3) \
	$(if $(ENABLE_NSS),plc4) \
))

$(eval $(call gb_Library_add_libs,united,\
	$(if $(ENABLE_GTK3),$(GTK3_LIBS)) \
	$(if $(ENABLE_GTK3),-lm) \
	$(if $(ENABLE_GTK3),-ldl) \
	$(if $(ENABLE_GEN),-lX11) \
	$(if $(ENABLE_GEN),-lXext) \
	$(if $(ENABLE_GEN),-lSM) \
	$(if $(ENABLE_GEN),-lICE) \
	$(if $(filter LINUX,$(OS)),-ldl) \
))


ifeq ($(OS),MACOSX)
$(eval $(call gb_Library_use_system_darwin_frameworks,united, \
    AppKit \
    ApplicationServices \
    CoreFoundation \
    CoreServices \
    Cocoa \
    Carbon \
    Foundation \
    Metal \
    $(if $(filter SKIA,$(BUILD_TYPE)),QuartzCore) \
    Security \
    SystemConfiguration \
))
endif # OS == MACOSX

ifeq ($(ENABLE_GEN)-$(USING_X11),TRUE-TRUE)
ifneq ($(ENABLE_RANDR),)
$(eval $(call gb_Library_use_externals,united,\
	Xrandr \
))
endif
ifeq ($(XINERAMA_LINK),dynamic)
$(eval $(call gb_Library_add_libs,united,\
    -lXinerama \
))
else
$(eval $(call gb_Library_add_libs,united,\
    -Wl$(COMMA)-Bstatic -lXinerama -Wl$(COMMA)-Bdynamic \
))
endif
endif # ENABLE_GEN && USING_X11

# Add no-relax because the sanitized binary is so large we get 32-bit-offset-relocation problems
ifeq ($(COM_IS_CLANG),TRUE)
ifneq ($(filter -fsanitize=%,$(gb_CC)),)
$(eval $(call gb_Library_add_ldflags,united,\
    -Wl$(COMMA)--no-relax \
))
endif # -fsanitize
endif # COM_IS_CLANG

$(call gb_Library_get_linktarget_target,united): \
    $(gb_CustomTarget_workdir)/desktop/united.map

ifneq ($(OS),WNT)
# Add export-dynamic so we can do dlsym() on symbols in the static main binary
# Add version-script to limit the set of symbols that are exported, to only
# the ones that we need to dlsym() on.
$(eval $(call gb_Library_add_ldflags,united,\
    -Wl$(COMMA)--export-dynamic \
    -Wl$(COMMA)--version-script=$(gb_CustomTarget_workdir)/desktop/united.map \
))
endif # ifneq ($(OS),WNT)


$(eval $(call gb_Library_add_exception_objects,united,\
    desktop/source/app/unitedinit \
))


#---------------------------------------------------------------------------
# The below is the insanely complicated makefile code extracted from
# xmlsecurity/*.mk
#---------------------------------------------------------------------------


ifeq ($(OS),WNT)
    $(eval $(call gb_Library_use_system_win32_libs,united,\
        crypt32 \
        Ole32 \
        Shell32 \
    ))
else
    ifneq (,$(filter DESKTOP,$(BUILD_TYPE))$(filter ANDROID,$(OS)))
        ifeq (TRUE,$(ENABLE_NSS))
            $(eval $(call gb_Library_use_externals,united,\
                nss3 \
                plc4 \
            ))
        endif
    endif # BUILD_TYPE=DESKTOP
endif

ifneq ($(ENABLE_NSS)$(ENABLE_OPENSSL),)

    ifeq ($(SYSTEM_XMLSEC),)
        $(eval $(call gb_Library_use_packages,united,\
            xmlsec \
        ))
    endif

    $(eval $(call gb_Library_use_externals,united,\
        xmlsec \
    ))

    ifeq ($(OS),WNT)

        $(eval $(call gb_Library_add_libs,united,\
            $(gb_UnpackedTarball_workdir)/xmlsec/win32/binaries/libxmlsec-mscng.lib \
            $(gb_UnpackedTarball_workdir)/xmlsec/win32/binaries/libxmlsec.lib \
        ))

        $(eval $(call gb_Library_use_system_win32_libs,united,\
            crypt32 \
            advapi32 \
            ncrypt \
        ))

        ifeq ($(ENABLE_NSS),TRUE)
            $(eval $(call gb_Library_use_externals,united,\
                nss3 \
            ))
        endif

    else # !$(OS),WNT

        ifeq ($(SYSTEM_XMLSEC),)
            $(eval $(call gb_Library_add_libs,united,\
                $(gb_UnpackedTarball_workdir)/xmlsec/src/.libs/libxmlsec1.a \
            ))
        endif

        ifeq ($(ENABLE_NSS),TRUE)

            ifeq ($(SYSTEM_XMLSEC),)
                $(eval $(call gb_Library_add_libs,united,\
                    $(gb_UnpackedTarball_workdir)/xmlsec/src/nss/.libs/libxmlsec1-nss.a \
                ))
            endif


            $(eval $(call gb_Library_use_externals,united,\
                plc4 \
            ))
            # nss3 after static libs to appease --as-needed linkers
            $(eval $(call gb_Library_use_externals,united,\
                nss3 \
                nssutil3 \
            ))

            ifeq ($(OS),MACOSX)
                $(eval $(call gb_Library_use_system_darwin_frameworks,united,\
                    CoreFoundation \
                    Security \
                ))
            endif

        else # ! $(ENABLE_NSS)

            ifeq ($(ENABLE_OPENSSL),TRUE)
                $(eval $(call gb_Library_use_external,united,openssl))
            endif

        endif # !$(ENABLE_NSS)

    endif # !$(OS),WNT

endif # $(ENABLE_NSS) || $(ENABLE_OPENSSL)

# vim: set ts=4 sw=4 et:
