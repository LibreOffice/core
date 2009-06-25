# few needed variables

stampdir = $(top_builddir)/build/stamp
pkgconfigdir = $(libdir)/pkgconfig
ooo_prefix = @OOO_PREFIX@
solverdir = $(ooo_prefix)/solver

# rules to execute

all-local: $(stampdir)/all

bootstrap: $(stampdir)/bootstrap

distclean-local:
	( . ./Linux*Env.Set.sh ; dmake distclean )

clean-local:
	( . ./Linux*Env.Set.sh ; dmake clean )

install-data-local: $(stampdir)/all
	mkdir -p $(ooo_prefix)
	mkdir -p $(pkgconfigdir)
	cp -a $(top_builddir)/solenv $(ooo_prefix)/
	cp -a $(top_builddir)/solver $(ooo_prefix)/
	$(INSTALL_SCRIPT) $(top_builddir)/Linux*Env.Set.sh $(solverdir)/
	$(INSTALL_DATA) $(top_builddir)/build/src/OpenOffice.org-bootstrap.pc $(pkgconfigdir)/

$(stampdir)/all: $(stampdir)/bootstrap
	( . ./Linux*Env.Set.sh ; \
	  export ULFEX=$(abs_srcdir)/build/bin/noulf ; \
	  export ULFCONV=$(abs_srcdir)/build/bin/noulfconv ; \
	  cd build ; build.pl --all ) && touch $@

$(stampdir)/bootstrap: Linux*Env.Set.sh
	( . ./Linux*Env.Set.sh ; ./bootstrap ) && touch $@
