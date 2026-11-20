#
# Copyright the Collabora Office contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

MAINTAINERCLEANFILES = Makefile.in

NODE_MODULES = $(abs_top_builddir)/browser/node_modules
TSC = $(NODE) $(NODE_MODULES)/typescript/bin/tsc
MOCHA = $(NODE) $(NODE_MODULES)/mocha/bin/mocha

WORKDIR = $(abs_builddir)/workdir
BUILD_MARKER = $(WORKDIR)/.built

SRCS = $(wildcard $(srcdir)/*.ts) $(wildcard $(srcdir)/helper/*.ts)

# The tests that draw what the engine writes build into one script with the browser sources, the
# same way the mocha_tests bundle builds. The rule of the bundle names RENDERER_SRCS for a second
# expansion, so make runs the find only when it checks the bundle.
RENDERER_BUNDLE = $(WORKDIR)/renderer/main.js
RENDERER_SRCS = $(wildcard $(srcdir)/renderer/*.ts) \
	$(shell find $(top_srcdir)/browser/src $(top_srcdir)/browser/js \
		$(top_srcdir)/browser/mocha_tests/refs $(top_srcdir)/browser/mocha_tests/helper \
		$(top_srcdir)/browser/mocha_tests/sources.ts \
		\( -name '*.ts' -o -name '*.tsx' -o -name '*.js' \))

# The port coolwsd listens on. Pass another one when a server already listens on the default.
COOLWSD_BIN = $(abs_top_builddir)/coolwsd
COOLWSD_PORT ?= 9980
COOLWSD_URL ?= ws://localhost:$(COOLWSD_PORT)
COOLWSD_PID = $(WORKDIR)/coolwsd.pid
COOLWSD_LOG = $(WORKDIR)/coolwsd.log
# The tests load copies of the samples from here.
DOCUMENTS_DIR = $(WORKDIR)/documents

# Exits with 0 when a server listens on COOLWSD_PORT, and with 1 otherwise.
PORT_IN_USE = $(NODE) -e "require('net').connect($(COOLWSD_PORT), 'localhost') \
	.on('connect', () => process.exit(0)).on('error', () => process.exit(1))"
# Prints a port that no server listens on. It comes from below the range the kernel picks from
# for port 0, where the servers of unit tests running in parallel get theirs, so none of them can
# take it before coolwsd starts.
FREE_PORT = $(NODE) -e "const fs = require('fs'), net = require('net'); \
	let low = 32768; \
	try { low = parseInt(fs.readFileSync('/proc/sys/net/ipv4/ip_local_port_range', 'utf8')); } \
	catch (e) {} \
	const first = 10000, count = low - first; \
	const tryPort = (attempt) => { \
		const port = count > 0 && attempt < 100 ? first + Math.floor(Math.random() * count) : 0; \
		const server = net.createServer(); \
		server.on('error', () => tryPort(attempt + 1)); \
		server.listen(port, () => { console.log(server.address().port); server.close(); }); \
	}; \
	tryPort(0);"

build-integration: $(BUILD_MARKER) $(RENDERER_BUNDLE)

# The compiled tests go to the build tree, where node finds the modules they load (and tsc finds
# them there through baseUrl, also when the build tree is not the source tree).
$(BUILD_MARKER): $(SRCS) $(srcdir)/tsconfig.json
	$(TSC) --project $(srcdir)/tsconfig.json --outDir $(WORKDIR) \
		--typeRoots $(NODE_MODULES)/@types --baseUrl $(NODE_MODULES)
	touch $(BUILD_MARKER)

# tsc can report a failure and still write the script, so a failure deletes it.
.SECONDEXPANSION:
$(RENDERER_BUNDLE): $$(RENDERER_SRCS)
	$(TSC) $(srcdir)/renderer/main.ts --outfile $@ --module none --moduleResolution node \
		--lib dom,es2016 --target ES5 --downlevelIteration --allowJs --jsx react \
		--jsxFactory h --jsxFragmentFactory Fragment --typeRoots $(NODE_MODULES)/@types \
		|| (rm -f $@ && false)

# Runs the tests against a coolwsd that already listens at COOLWSD_URL.
check-integration: $(BUILD_MARKER) $(RENDERER_BUNDLE)
	COOLWSD_URL="$(COOLWSD_URL)" COOL_TEST_SAMPLES="$(abs_top_srcdir)/test/samples" \
		COOL_TEST_WORKDIR="$(DOCUMENTS_DIR)" \
		$(MOCHA) --config $(abs_srcdir)/.mocharc.json '$(WORKDIR)/**/*.test.js' \
		$(RENDERER_BUNDLE)

# Starts coolwsd in the background and waits until it listens on COOLWSD_PORT. A coolwsd that
# an earlier start left running is stopped first. Another server on the port is an error, since
# the tests would run against it. A coolwsd that does not get ready in time is stopped again. The
# kits answer with the primitive samples of the engine.
start-coolwsd:
	mkdir -p $(WORKDIR)
	-test -f $(COOLWSD_PID) && kill `cat $(COOLWSD_PID)` 2>/dev/null || true
	@for i in `seq 1 10`; do \
		if ! $(PORT_IN_USE); then exit 0; fi; \
		sleep 1; \
	done; \
	echo "Port $(COOLWSD_PORT) is in use. Stop that server or pass COOLWSD_PORT=<free port>."; \
	exit 1
	fc-cache "@LO_PATH@"/share/fonts/truetype >/dev/null 2>&1 || true
	ulimit -n 1024; COOL_VECTOR_PRIMITIVE_SAMPLES=1 $(COOLWSD_BIN) \
		--o:sys_template_path="@SYSTEMPLATE_PATH@" \
		--o:child_root_path="@JAILS_PATH@" \
		--o:cache_files.path="@CACHE_PATH@" \
		--o:storage.filesystem[@allow]=true \
		--disable-ssl --forcecaching \
		--o:welcome.enable=false \
		--o:logging.file[@enable]=true --o:logging.level=warning \
		--port=$(COOLWSD_PORT) \
		--pidfile=$(COOLWSD_PID) \
		--unattended \
		> $(COOLWSD_LOG) 2>&1 & echo $$! > $(COOLWSD_PID)
	@for i in `seq 1 60`; do \
		if $(PORT_IN_USE); then exit 0; fi; \
		sleep 1; \
	done; \
	echo "coolwsd did not become ready, see $(COOLWSD_LOG)"; \
	$(MAKE) stop-coolwsd; \
	exit 1

stop-coolwsd:
	-test -f $(COOLWSD_PID) && kill `cat $(COOLWSD_PID)` 2>/dev/null || true
	-rm -f $(COOLWSD_PID)

# Starts coolwsd, runs the tests and stops coolwsd, also when the start or a test fails.
check-with-server: $(BUILD_MARKER) $(RENDERER_BUNDLE)
	@trap '$(MAKE) stop-coolwsd' EXIT INT TERM; \
		$(MAKE) start-coolwsd && \
		$(MAKE) check-integration COOLWSD_URL=ws://localhost:$(COOLWSD_PORT)

# Runs check-with-server with coolwsd on a port that no server listens on, so the run does not
# clash with another server on the machine. make check runs this from the test folder, which
# comes after coolwsd is built.
check-on-free-port:
	@$(MAKE) check-with-server COOLWSD_PORT=`$(FREE_PORT)`

clean-local:
	-$(MAKE) stop-coolwsd
	rm -rf $(WORKDIR)

.PHONY: build-integration check-integration start-coolwsd stop-coolwsd check-with-server \
	check-on-free-port
