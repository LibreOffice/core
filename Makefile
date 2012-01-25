# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-

.PHONY : all bootstrap fetch build clean clean-build clean-host
all: build

SHELL=/usr/bin/env bash
SRCDIR:=$(patsubst %/,%,$(dir $(realpath $(firstword $(MAKEFILE_LIST)))))

ifeq ($(gb_Side),)
gb_Side:=host
endif

include $(SRCDIR)/config_$(gb_Side).mk

ifneq ($(verbose),)
GMAKE_OPTIONS:=-rs
else
GMAKE_OPTIONS:=-r
endif

gbuild_modules:= \
MathMLDTD\
Mesa\
UnoControls\
accessibility\
animations\
avmedia\
basctl\
basebmp\
basegfx\
basic\
bean\
binaryurp\
canvas\
chart2\
comphelper\
configmgr\
cppcanvas\
cppu\
cppuhelper\
cui\
dbaccess\
desktop\
drawinglayer\
dtrans\
editeng\
eventattacher\
extensions\
fileaccess\
filter\
forms\
formula\
fpicker\
framework\
hwpfilter\
i18npool\
i18nutil\
idl\
lingucomponent\
linguistic\
lotuswordpro\
nlpsolver\
np_sdk\
o3tl\
offapi\
officecfg\
oovbaapi\
oox\
package\
padmin\
psprint_config\
qadevOOo\
regexp\
reportdesign\
rsc\
sal\
salhelper\
sane\
sax\
sc\
scaddins\
sccomp\
scripting\
sd\
sfx2\
slideshow\
sot\
starmath\
svl\
svtools\
svx\
sw\
swext\
tail_build\
test\
toolkit\
tools\
twain\
ucb\
ucbhelper\
udkapi\
unixODBC\
unotools\
unoxml\
ure\
uui\
vbahelper\
vcl\
wizards\
writerfilter\
writerperfect\
xmerge\
xml2cmp\
xmloff\
xmlreader\
xmlscript\
xmlsecurity\

dmake_modules:=\
afms\
apache-commons\
apple_remote\
autodoc\
beanshell\
berkeleydb\
binfilter \
boost\
bridges\
cairo\
cli_ure\
codemaker\
connectivity\
cosv\
cppunit\
cpputools\
crashrep\
ct2n\
curl\
dictionaries \
embeddedobj\
embedserv\
epm\
expat\
external\
extras\
fontconfig\
freetype\
gdk-pixbuf\
gettext\
glib\
graphite\
helpconten2 \
hsqldb\
hunspell\
hyphen\
icc\
icu\
idlc\
instsetoo_native\
io\
javaunohelper\
jfreereport\
jpeg\
jurt\
jvmaccess\
jvmfwk\
l10ntools\
languagetool\
libcdr\
libcmis\
libcroco\
libexttextcat\
libgsf\
libpng\
librsvg\
libvisio\
libwpd\
libwpg\
libwps\
libxml2\
libxmlsec\
libxslt\
lpsolve\
lucene\
mdds\
migrationanalysis\
more_fonts\
moz\
mysqlc\
mysqlcppconn\
mythes\
neon\
nss\
odk\
openssl\
packimages\
pango\
postgresql\
postprocess\
python\
pyuno\
rdbmaker\
readlicense_oo\
redland\
registry\
remotebridges\
reportbuilder\
rhino\
ridljar\
saxon\
scp2\
sdext\
setup_native\
shell\
smoketest\
solenv\
soltools\
stax\
stlport\
stoc\
store\
sysui\
tomcat\
udm\
unodevtools\
unoil\
unotest\
vigra\
x11_extensions\
xmlhelp\
xpdf\
xsltml\
zlib\

define gbuild_module_rules
.PHONY: $(1) $(1).clean

$(1): bootstrap fetch
	cd $(1) && $(GNUMAKE) -j $(GMAKE_PARALLELISM) $(GMAKE_OPTIONS)

$(1).clean:
	cd $(1) && $(GNUMAKE) -j $(GMAKE_PARALLELISM) $(GMAKE_OPTIONS) clean

endef

define gbuild_modules_rules
$(foreach m,$(1),$(call gbuild_module_rules,$(m)))
endef

define dmake_module_rules
.PHONY: $(1) $(1).all $(1).deliver $(1).clean

$(1): bootstrap fetch
	cd $(1) && unset MAKEFLAGS && \
        $(SOLARENV)/bin/build.pl -P$(BUILD_NCPUS) -- -P$(GMAKE_PARALLELISM)

$(1).all: bootstrap fetch
	cd $(1) && unset MAKEFLAGS && \
        $(SOLARENV)/bin/build.pl -P$(BUILD_NCPUS) --all -- -P$(GMAKE_PARALLELISM)

$(1).deliver: bootstrap fetch
	cd $(1) && $(SOLARENV)/bin/deliver.pl

$(1).clean:
	rm -fr $(1)/$(INPATH)

endef

define dmake_modules_rules
$(foreach m,$(1),$(call dmake_module_rules,$(m)))
endef

#
# Build
#
build: bootstrap fetch $(if $(filter $(INPATH),$(INPATH_FOR_BUILD)),,cross-toolset)
	cd instsetoo_native && unset MAKEFLAGS && \
	$(SOLARENV)/bin/build.pl -P$(BUILD_NCPUS) --all -- -P$(GMAKE_PARALLELISM)

cross-toolset:
	cd cross_toolset && $(GNUMAKE) -j $(GMAKE_PARALLELISM) $(GMAKE_OPTIONS)

#
# Partial Build
#
$(eval $(call gbuild_modules_rules,$(gbuild_modules)))

$(eval $(call dmake_modules_rules,$(dmake_modules)))

#
# Clean
#
clean: clean-host clean-build

clean-host:
	rm -fr $(SRCDIR)/*/$(INPATH)
	rm -fr $(SRCDIR)/workdir/*
	rm -fr $(SRCDIR)/solver/$(INPATH)
	rm -fr install

clean-build:
ifeq ($(CROSS_COMPILING),YES)
	rm -rf */$(INPATH_FOR_BUILD)
endif

#
# Distclean
#
distclean : clean
ifeq ($(BUILD_DMAKE),YES)
	(if [ -f dmake/Makefile ] ; then $(GNUMAKE) -j $(GMAKE_PARALLELISM) -C dmake distclean; fi) && \
	rm -f solenv/*/bin/dmake*
endif
	rm -fr config_host.mk config_build.mk aclocal.m4 autom4te.cache \
    bin/repo-list config.log config.status configure \
	desktop/scripts/soffice.sh ooo.lst post_download post_download.log \
	config_host.mk.last set_soenv.stamp src.downloaded warn

#
# custum command
#
ifneq ( $(MAKECMDGOALS),cmd)
cmd:
	echo "custum cmd" && ( $(cmd) )

endif

#
# Bootstap
#
$(WORKDIR)/bootstrap:
	@cd $(SRCDIR) && ./bootstrap
	@mkdir -p $(dir $@) && touch $@

bootstrap: $(WORKDIR)/bootstrap

#
# Fetch
#
fetch: src.downloaded

src.downloaded : ooo.lst download
ifeq (@DO_FETCH_TARBALLS@,YES)
	./download $(SRCDIR)/ooo.lst && touch $@
else
	@echo "Automatic fetching of external tarballs is disabled."
endif

#
# Install
#

install:
	echo "Installing in $(INSTALLDIR)..." && \
	ooinstall "$(INSTALLDIR)" && \
	echo "" && \
	echo "Installation finished, you can now execute:" && \
	echo "$(INSTALLDIR)/program/soffice"

#dev-install:  $(WORKDIR)/bootstrap \
#			  $(ROOT_SRC)/src.downloaded \
#			  $(if $(filter $(INPATH),$(INPATH_FOR_BUILD)),,cross_toolset) \
#			| $(filter build,$(MAKECMDGOALS)) \
#			  $(if $(filter check,$(MAKECMDGOALS)),build)
#	@rm -f $(SRCDIR)/install && ln -s $(OUTDIR)/installation/opt/ $(SRCDIR)/install
#	cd smoketestoo_native && \
#	$(SOLARENV)/bin/build.pl -P$(BUILD_NCPUS) --all -- -P$(GMAKE_PARALLELISM)

distro-pack-install: install
	$(SRCDIR)/bin/distro-install-clean-up
	$(SRCDIR)/bin/distro-install-desktop-integration
	$(SRCDIR)/bin/distro-install-sdk
	$(SRCDIR)/bin/distro-install-file-lists

id:
	@create-ids

tags:
	@create-tags

docs:
	@mkdocs.sh $(SRCDIR)/docs $(SOLARENV)/inc/doxygen.cfg

findunusedcode:
	@which callcatcher > /dev/null 2>&1 || \
	    (echo "callcatcher not installed" && false)
	@sed -e s,$$INPATH,callcatcher,g $(SRCDIR)/config_host.mk > $(SRCDIR)/config_host_callcatcher.mk
	@mkdir -p $(SRCDIR)/solenv/callcatcher/bin && \
	    ln -sf $(SRCDIR)/solenv/$(INPATH)/bin/dmake \
		$(SRCDIR)/solenv/callcatcher/bin/dmake
	@$(GNUMAKE) -f $(SOLARENV)/bin/callcatcher.Makefile
	@grep ::.*\( unusedcode.all | grep -v ^cppu:: > unusedcode.easy

ifneq ( $(filter-out check debugrun dev-install subsequentcheck unitcheck,$(MAKECMDGOALS)),$(MAKECMDGOALS))
gb_SourceEnvAndRecurse_STAGE=buildpl
include $(SOLARENV)/gbuild/gbuild.mk
$(eval $(call gb_Module_make_global_targets,$(wildcard $(SRCDIR)/RepositoryModule_*.mk)))
endif

# vim: set noet sw=4 ts=4:
