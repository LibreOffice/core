# few needed variables

stampdir = $(top_builddir)/build/stamp
pkgconfigdir = $(libdir)/pkgconfig
ooo_prefix = @OOO_PREFIX@
solverdir = $(ooo_prefix)/solver

# what to add to the package

EXTRA_DIST = build dmake guw instsetoo_native scp2 soldep solenv soltools \
	     stlport \
	     bootstrap.1 download makefile.rc oowintool set_soenv.in

# TODO more in dist-hook, likeremove .o's in dmake, stuff in build/stamp, etc.
# [basically everything that we have in .gitignores ;-)]
dist-hook:
	git log --date=short --pretty="format:@%cd  %an  <%ae>  [%H]%n%n%s%n%n%e%b" | sed -e "s|^\([^@]\)|\t\1|" -e "s|^@||" >$(distdir)/ChangeLog
	rm -rf $(distdir)/*/unxlng*.pro

# rules to execute

all-local: $(stampdir)/all

bootstrap: $(stampdir)/bootstrap

clean-local:
	( . ./Linux*Env.Set.sh ; dmake clean )

distclean-local:
	( . ./Linux*Env.Set.sh ; dmake distclean )

install-data-local: $(stampdir)/all
	$(MKDIR_P) $(DESTDIR)$(ooo_prefix)
	$(MKDIR_P) $(DESTDIR)$(pkgconfigdir)
	cp -r $(top_builddir)/solenv $(DESTDIR)$(ooo_prefix)/
	cp -r $(top_builddir)/solver $(DESTDIR)$(ooo_prefix)/
	$(INSTALL_DATA) $(top_builddir)/Linux*Env.Set.sh $(DESTDIR)$(solverdir)/
	$(INSTALL_DATA) $(top_builddir)/build/src/buildenv-common $(DESTDIR)$(solverdir)/
	$(INSTALL_DATA) $(top_builddir)/build/src/OpenOffice.org-bootstrap.pc $(DESTDIR)$(pkgconfigdir)/

$(stampdir)/all: $(stampdir)/bootstrap
	( . ./Linux*Env.Set.sh ; \
	  export ULFEX=$(abs_srcdir)/build/bin/noulf ; \
	  export ULFCONV=$(abs_srcdir)/build/bin/noulfconv ; \
	  cd build ; build.pl --all ) && touch $@

$(stampdir)/bootstrap: Linux*Env.Set.sh
	( . ./Linux*Env.Set.sh ; ./bootstrap ) && touch $@
