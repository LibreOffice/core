# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-

.PHONY : all bootstrap build check clean clean-build clean-host dev-install distclean distro-pack-install docs fetch findunusedcode id install subsequentcheck tags

ifeq ($(MAKECMDGOALS),)
MAKECMDGOALS:=all
endif

all: build

SHELL=/usr/bin/env bash
SRCDIR:=$(patsubst %/,%,$(dir $(realpath $(firstword $(MAKEFILE_LIST)))))

ifeq ($(gb_Side),)
gb_Side:=host
endif

include $(SRCDIR)/config_$(gb_Side).mk

ifeq ($(verbose),)
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
apple_remote\
avmedia\
basctl\
basebmp\
basegfx\
basic\
bean\
binaryurp\
canvas\
chart2\
clucene\
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
embeddedobj\
embedserv\
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
io\
javaunohelper\
jurt\
jvmaccess\
jvmfwk\
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
packimages\
padmin\
psprint_config\
qadevOOo\
regexp\
reportdesign\
ridljar\
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
sdext\
sfx2\
slideshow\
smoketest\
sot\
starmath\
stoc\
svl\
svtools\
svx\
sw\
swext\
tail_build\
test\
toolkit\
tools\
translations\
twain\
ucb\
ucbhelper\
udkapi\
unixODBC\
unoil\
unotest\
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
helpcontent2 \
hsqldb\
hunspell\
hyphen\
icc\
icu\
idlc\
instsetoo_native\
jfreereport\
jpeg\
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
lcms2\
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
saxon\
scp2\
setup_native\
shell\
solenv\
soltools\
stax\
stlport\
store\
sysui\
testtools\
tomcat\
udm\
unodevtools\
vigra\
x11_extensions\
xmlhelp\
xpdf\
xsltml\
zlib\

define gbuild_module_rules
.PHONY: $(1) $(1).all $(1).clean $(1).deliver

$(1): bootstrap fetch
	cd $(1) && $(GNUMAKE) -j $(GMAKE_PARALLELISM) $(GMAKE_OPTIONS) gb_PARTIALBUILD=T

$(1).all: bootstrap fetch
	cd $(1) && unset MAKEFLAGS && \
        $(SOLARENV)/bin/build.pl -P$(BUILD_NCPUS) --all -- -P$(GMAKE_PARALLELISM)

$(1).clean:
	cd $(1) && $(GNUMAKE) -j $(GMAKE_PARALLELISM) $(GMAKE_OPTIONS) clean gb_PARTIALBUILD=T

$(1).subsequentcheck:
	cd $(1) && $(GNUMAKE) -j $(GMAKE_PARALLELISM) $(GMAKE_OPTIONS) subsequentcheck gb_PARTIALBUILD=T

$(1).deliver:
	@true

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

$(1).deliver: $(1)
	cd $(1) && $(SOLARENV)/bin/deliver.pl

$(1).clean:
	rm -fr $(1)/$(INPATH)

endef

define dmake_modules_rules
$(foreach m,$(1),$(call dmake_module_rules,$(m)))
endef

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
	ooo.lst post_download post_download.log \
	config_host.mk.last set_soenv.stamp src.downloaded warn
	find $(SOLARENV)/gdb -name "*.pyc" -exec rm {} \;

#
# custom command
#
ifneq ( $(MAKECMDGOALS),cmd)
cmd:
	echo "custom cmd" && ( $(cmd) )

endif

ifneq ($(filter-out clean distclean,$(MAKECMDGOALS)),)
#
# Makefile
#

# I don't like to touch stuff that are supposed to be
# in the source tree, hence read-only
# but I couldn't find a way to get make to
# restart after an autogen. and we _have_ to
# restart since autogen can have changed
# config_host.mk which is included in this
# Makefile

ifeq ($(OS_FOR_BUILD),WNT)
CONFIG_HOST_MK=$(shell cygpath -u $(SRCDIR))/config_host.mk
else
CONFIG_HOST_MK=$(SRCDIR)/config_host.mk
endif

Makefile: $(CONFIG_HOST_MK)
	touch $@

$(CONFIG_HOST_MK) : config_host.mk.in bin/repo-list.in ooo.lst.in configure.in autogen.lastrun
	./autogen.sh

autogen.lastrun:
	@true

#
# Fetch
#
fetch: src.downloaded

src.downloaded : Makefile ooo.lst download
ifeq ($(DO_FETCH_TARBALLS),YES)
	@./download $(SRCDIR)/ooo.lst && touch $@
else
	@echo "Automatic fetching of external tarballs is disabled."
endif

#
# Bootstap
#
ifeq ($(OS_FOR_BUILD),WNT)
WORKDIR_BOOTSTRAP=$(shell cygpath -u $(WORKDIR))/bootstrap
else
WORKDIR_BOOTSTRAP=$(WORKDIR)/bootstrap
endif

$(WORKDIR_BOOTSTRAP):
	@cd $(SRCDIR) && ./bootstrap
	@mkdir -p $(dir $@) && touch $@

bootstrap: $(WORKDIR_BOOTSTRAP)

#
# Build
#
build-packimages: bootstrap fetch $(if $(filter $(INPATH),$(INPATH_FOR_BUILD)),,cross-toolset)
ifeq ($(DISABLE_SCRIPTING),TRUE)
# We must get the headers from vbahelper "delivered" because
# as we don't link to any libs from there they won't otherwise be, or
# something. And we still do include those headers always even if the
# libs aren't built in the --disable-scripting case. (Ifdefs for
# DISABLE_SCRIPTING will be added to the code later as necessary.)
	$(GNUMAKE) vbahelper
endif
ifeq ($(DISABLE_DBCONNECTIVITY),TRUE)
# Ditto for dbconnectivity in the --disable-database-connectivity case
	cd connectivity && sed -e 's/^\(export [A-Z0-9_]*=\)\(.*\)$$/\1"\2"/' <../config_host.mk >conftmp.sh && . conftmp.sh && rm conftmp.sh && $(SOLARENV)/bin/deliver.pl
endif
	cd packimages && unset MAKEFLAGS && \
        $(SOLARENV)/bin/build.pl -P$(BUILD_NCPUS) --all -- -P$(GMAKE_PARALLELISM)

build: build-packimages
ifeq ($(OS_FOR_BUILD),WNT)
	cd instsetoo_native && unset MAKEFLAGS && $(SOLARENV)/bin/build.pl
else
	cd instsetoo_native && unset MAKEFLAGS && \
        $(SOLARENV)/bin/build.pl -P$(BUILD_NCPUS) -- -P$(GMAKE_PARALLELISM)
endif

cross-toolset: bootstrap fetch
	cd cross_toolset && $(GNUMAKE) -j $(GMAKE_PARALLELISM) $(GMAKE_OPTIONS)


#
# Install
#

install:
	echo "Installing in $(INSTALLDIR)..." && \
	ooinstall "$(INSTALLDIR)" && \
	echo "" && \
	echo "Installation finished, you can now execute:" && \
	echo "$(INSTALLDIR)/program/soffice"

dev-install: build
	@rm -rf $(OUTDIR)/installation
	@mkdir $(OUTDIR)/installation
ifeq ($(DISABLE_LINKOO),TRUE)
	@ooinstall $(OUTDIR)/installation/opt
	@install-gdb-printers -L
else
	@ooinstall -l $(OUTDIR)/installation/opt
endif
	@rm -f $(SRCDIR)/install && ln -s $(OUTDIR)/installation/opt/ $(SRCDIR)/install

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
	@grep ::.*\( unusedcode.all \
              | grep -v ^Atom \
              | grep -v ^boost:: \
              | grep -v ^CIcc \
              | grep -v ^CLuceneError:: \
              | grep -v ^cppu:: \
              | grep -v ^Dde \
              | grep -v ^graphite2:: \
              | grep -v ^jvmaccess:: \
              | grep -v ^libcdr:: \
              | grep -v ^libcmis:: \
              | grep -v ^libvisio:: \
              | grep -v ^libwpg:: \
              | grep -v ^lucene:: \
              | grep -v ^salhelper:: \
              | grep -v ^WP1 \
              | grep -v ^WP3 \
              | grep -v ^WP42 \
              | grep -v ^WP6 \
              | grep -v ^WPG \
              | grep -v ^WPS \
              | grep -v WPXBinaryData \
              > unusedcode.easy

check: dev-install subsequentcheck

subsequentcheck :| $(if $(filter-out subsequentcheck,$(MAKECMDGOALS)),dev-install)
	$(GNUMAKE) -j $(GMAKE_PARALLELISM) $(GMAKE_OPTIONS) -f Makefile.post $@

debugrun:
	$(GNUMAKE) -j $(GMAKE_PARALLELISM) $(GMAKE_OPTIONS) -f Makefile.post $@

endif # not clean or distclean


#########################
# help
#
.PHONY: help
help:
	@cat $(SRCDIR)/solenv/gbuild/gbuild.help.txt
	@true


# vim: set noet sw=4 ts=4:
