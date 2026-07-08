# Orchestrator for the CODA-W (Collabora Office Desktop App - Windows) build.
# Copied verbatim into the build root as its Makefile by
# windows/coda/build/autogen.sh, which also writes the coda-config.mk included
# below.
#
# Run "make" from Git Bash. In order it builds: the engine (natively), then -
# once its instdir exists - online configure and the browser bundle (dispatched
# to WSL), and finally the Visual Studio solution (native MSBuild). Each step's
# own tool tracks dependencies, so a bare "make" rebuilds only what changed in
# the engine, the bundle or the app.

include coda-config.mk

# Visual Studio solution configuration. Debug matches an engine built with
# --enable-dbgutil; for a Release app (with a non-dbgutil engine) run
# "make CONFIG=Release".
CONFIG   ?= Debug
PLATFORM ?= x64

# Online CODA-W configure flags whose values are not derived from paths.
APP_NAME ?= Collabora Office
INFO_URL ?= https://example.com/coda/info.html

# Native (Git Bash) paths for the steps that run natively.
ENGINE_BUILD := $(CURDIR)/engine
ONLINE_BUILD := $(CURDIR)/online

.PHONY: all engine browser app clean-app

all: app

# 1. Engine: native make (the engine build uses native compilers and only calls
#    wsl.exe as a helper). Phony so the engine's own make handles incrementality
#    and re-runs config.status when its configure inputs change.
engine: $(ENGINE_BUILD)/autogen.input
	$(MAKE) -C $(ENGINE_BUILD)

# Keep the engine build's autogen.input in sync with the single one in the build
# root, so editing the latter triggers the engine's own reconfigure (its
# config_host.mk lists <builddir>/autogen.input as a prerequisite). A copy, not
# a symlink: native make compares a symlink's own mtime rather than its target's,
# so a symlink would hide edits to the build-root file.
$(ENGINE_BUILD)/autogen.input: autogen.input
	cp $< $@

# 2. Online configure, in WSL. It requires the built engine's instdir, so it
#    runs here at make time rather than in autogen.sh. Order-only on engine: it
#    runs after the engine is built but is not redone on every engine rebuild.
#    Once created, online's own config.status re-runs itself when online
#    configure inputs change. --with-lo-builddir is a WSL path; --with-lo-path a
#    Windows one.
$(ONLINE_BUILD)/config.status: | engine
	mkdir -p $(ONLINE_BUILD)
	wsl.exe --exec bash -c "cd '$(ONLINE_BUILD_WSL)' && '$(SRC_ROOT_WSL)/autogen.sh' --enable-windowsapp --with-app-name='$(APP_NAME)' --with-lo-builddir='$(ENGINE_BUILD_WSL)' --with-lo-path='$(LO_PATH_WIN)' --with-info-url='$(INFO_URL)'"

# 3. Browser bundle, in WSL. Phony; online's make rebuilds only changed parts.
browser: $(ONLINE_BUILD)/config.status
	wsl.exe --exec bash -c "cd '$(ONLINE_BUILD_WSL)' && make"

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
