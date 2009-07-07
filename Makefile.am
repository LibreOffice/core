# few needed variables

stampdir = $(top_builddir)/build/stamp
pkgconfigdir = $(libdir)/pkgconfig
ooo_prefix = @OOO_PREFIX@
solenvdir = $(ooo_prefix)/solenv
solverdir = $(ooo_prefix)/solver
buildsolverdir = "@abs_builddir@/solver"

# the predefined rules

include $(solverdir)/Makefile.common

# what to add to the package

# TODO
EXTRA_DIST = 

dist-hook: dist-hook-common

# rules to execute

all-local: $(stampdir)/all

# TODO
clean-local:

# TODO
distclean-local:

install-data-local: $(stampdir)/all install-data-common
	$(INSTALL_DATA) $(top_builddir)/build/src/OpenOffice.org-libs-extern-sys.pc $(DESTDIR)$(pkgconfigdir)/

$(stampdir)/all: buildenv all-common
