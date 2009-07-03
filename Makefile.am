# few needed variables

stampdir = $(top_builddir)/build/stamp
pkgconfigdir = $(libdir)/pkgconfig
ooo_prefix = @OOO_PREFIX@
solenvdir = $(ooo_prefix)/solenv
buildsolverdir = "@abs_builddir@/solver"

# what to add to the package

# TODO
EXTRA_DIST = 

# TODO more in dist-hook, likeremove .o's in dmake, stuff in build/stamp, etc.
# [basically everything that we have in .gitignores ;-)]
dist-hook:
	git log --date=short --pretty="format:@%cd  %an  <%ae>  [%H]%n%n%s%n%n%e%b" | sed -e "s|^\([^@]\)|\t\1|" -e "s|^@||" >$(distdir)/ChangeLog
	rm -rf $(distdir)/*/unxlng*.pro

# rules to execute

all-local: $(stampdir)/all

# TODO
clean-local:

# TODO
distclean-local:

install-data-local: $(stampdir)/all
	( . buildenv -i ; \
	  export PRODUCT="OpenOffice" ; \
	  perl -w $(solenvdir)/bin/make_installer.pl \
	       -f $(ooo_prefix)/instsetoo_native/util/openoffice.lst \
	       -l 'en-US' -p "$$PRODUCT" -buildid "$$BUILD" \
	       -dontstrip -verbose -simple $(DESTDIR)$(ooo_prefix) )
	$(INSTALL_DATA) $(top_builddir)/build/src/OpenOffice.org-ure.pc $(DESTDIR)$(pkgconfigdir)/

$(stampdir)/all: buildenv
	( . buildenv ; cd build ; build.pl --all ) && touch $@
