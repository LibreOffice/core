# Orchestrator for the CODA-W (Collabora Office Desktop App - Windows) build.
# Copied verbatim into the build root as its Makefile by
# windows/coda/build/autogen.sh, which also writes the coda-config.mk included
# below.
#
# Run "make" from Git Bash. In order it builds: the engine (natively), then -
# once its instdir exists - online configure and the browser bundle (dispatched
# to MSYS2), and finally the Visual Studio solution (native MSBuild). Each
# step's own tool tracks dependencies, so a bare "make" rebuilds only what
# changed in the engine, the bundle or the app.

include coda-config.mk

# Visual Studio solution configuration. Debug matches an engine built with
# --enable-dbgutil; for a Release app (with a non-dbgutil engine) run
# "make CONFIG=Release".
CONFIG   ?= Debug
PLATFORM ?= x64

# Online CODA-W configure flags whose values are not derived from paths.
# Override on the make command line, e.g.:
#   make CONFIG=Release APP_NAME='Collabora Office Preview' VENDOR='Collabora Productivity Limited'
# ONLINE_CONFIGURE_ARGS is appended verbatim for anything else. Changing any
# of them re-runs the online configure on the next make.
APP_NAME ?= Collabora Office
INFO_URL ?= https://example.com/coda/info.html
VENDOR ?=
ONLINE_CONFIGURE_ARGS ?=

ONLINE_CONFIGURE_FLAGS = --enable-windowsapp --with-app-name='$(APP_NAME)' $(if $(VENDOR),--with-vendor='$(VENDOR)') --with-lo-builddir='$(ENGINE_BUILD)' --with-lo-path='$(LO_PATH_WIN)' --with-info-url='$(INFO_URL)' $(ONLINE_CONFIGURE_ARGS)

# Native (Git Bash) paths. MSYS2 shares the /c/... convention, so the same
# paths work in the steps dispatched there.
ENGINE_BUILD := $(CURDIR)/engine
ONLINE_BUILD := $(CURDIR)/online

# Run a command line inside MSYS2 ($(call run_msys2,<command line>)). Not a
# login shell: MSYS2's /etc/profile would replace the caller's PATH with its
# own minimal one (hiding e.g. the native node and ~/bin) and point
# CONFIG_SITE at /etc/config.site, which presets the build triplet to cygwin.
# Instead keep the caller's PATH - it survives the realm hop correctly - and
# just prepend MSYS2's own bin directories so its unix tools come first.
# MSYS=winsymlinks:nativestrict: the configures create real symlinks; force it
# so the MSYS2 side does not depend on the user's environment.
run_msys2 = MSYSTEM=MSYS MSYS=winsymlinks:nativestrict "$(MSYS2_BASH)" --noprofile --norc -c "export PATH=\"/usr/local/bin:/usr/bin:/bin:/opt/bin:\$$PATH\" CONFIG_SITE=/dev/null; $(1)"

.PHONY: all engine browser app clean-app

all: app

# 1. Engine: native make (the engine build uses native compilers and only calls
#    the MSYS2 tools as helpers). Phony so the engine's own make handles
#    incrementality and re-runs config.status when its configure inputs change.
engine: $(ENGINE_BUILD)/autogen.input
	$(MAKE) -C $(ENGINE_BUILD)

# Keep the engine build's autogen.input in sync with the single one in the build
# root, so editing the latter triggers the engine's own reconfigure (its
# config_host.mk lists <builddir>/autogen.input as a prerequisite). A copy, not
# a symlink: native make compares a symlink's own mtime rather than its target's,
# so a symlink would hide edits to the build-root file.
$(ENGINE_BUILD)/autogen.input: autogen.input
	cp $< $@

# Keep the online configure flags in a file that is rewritten only when their
# content changes, so editing APP_NAME & co. reconfigures online on the next
# make while an unchanged flag set stays incremental.
.PHONY: online-configure-flags
$(ONLINE_BUILD)/online-configure.flags: online-configure-flags
	@mkdir -p $(ONLINE_BUILD)
	@printf '%s\n' "$(ONLINE_CONFIGURE_FLAGS)" | cmp -s - $@ || printf '%s\n' "$(ONLINE_CONFIGURE_FLAGS)" > $@

# 2. Online configure, in MSYS2. It requires the built engine's instdir, so it
#    runs here at make time rather than in autogen.sh. Order-only on engine: it
#    runs after the engine is built but is not redone on every engine rebuild.
#    Once created, online's own config.status re-runs itself when online
#    configure inputs change. --with-lo-builddir is a Unix path; --with-lo-path
#    a Windows one.
$(ONLINE_BUILD)/config.status: $(ONLINE_BUILD)/online-configure.flags | engine
	mkdir -p $(ONLINE_BUILD)
	$(call run_msys2,cd '$(ONLINE_BUILD)' && '$(SRC_ROOT)/autogen.sh' $(ONLINE_CONFIGURE_FLAGS))

# 3. Browser bundle, in MSYS2 (python3, m4 and friends come from there; node is
#    the native one configure detected). Phony; online's make rebuilds only
#    changed parts.
browser: $(ONLINE_BUILD)/config.status
	$(call run_msys2,cd '$(ONLINE_BUILD)' && make)

# 4. App: native MSBuild (incremental). SolutionDir points at the online build
#    tree so the generated config.props is picked up and the output lands
#    out-of-tree; the .sln itself is read from the source tree. The trailing
#    backslash is added here because make would read it as a line continuation
#    in the variable value.
#    Switches use the '-' prefix, not '/': Git Bash's MSYS argument conversion
#    rewrites a leading-slash argument as a path (/nologo -> C:/Program Files/
#    Git/nologo), which MSBuild then rejects.
app: browser
	"$(MSBUILD)" -restore -nologo -p:Configuration=$(CONFIG) -p:Platform=$(PLATFORM) -p:SolutionDir='$(SOLUTION_DIR)\' '$(SLN)'

clean-app:
	"$(MSBUILD)" -t:Clean -nologo -p:Configuration=$(CONFIG) -p:Platform=$(PLATFORM) -p:SolutionDir='$(SOLUTION_DIR)\' '$(SLN)'
