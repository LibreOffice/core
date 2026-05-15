#
# Copyright the Collabora Online contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
export NODE_PATH=$(abs_builddir)/node_modules

V = $(if $(verbose),,@)
abs_dir = $(if $(filter $(abs_builddir),$(abs_srcdir)),.,$(abs_srcdir))
CYPRESS_BINARY = $(abs_builddir)/node_modules/cypress/bin/cypress
ESLINT_BINARY = $(abs_builddir)/node_modules/eslint/bin/eslint.js
WAIT_ON_BINARY = $(abs_builddir)/node_modules/wait-on/bin/wait-on
GET_PORT_BINARY = $(abs_builddir)/node_modules/get-port-cli/cli.js
NPM_INSTALLED = $(abs_builddir)/workdir/npm_installed
CYPRESS_CONFIG = $(abs_srcdir)/cypress.config.ts
SERVER_ARG = $(subst :, ,$(shell basename $(if $(coolwsd),$(coolwsd),http://localhost:9980)))
COOLWSD_SERVER = $(firstword $(SERVER_ARG))
FREE_PORT = $(lastword $(SERVER_ARG))

export COOLWSD_SERVER
export FREE_PORT
export COOLWSD_VERSION_HASH

BUFFERED_SCRIPT = $(abs_srcdir)/run_buffered.sh

PID_FILE=$(abs_builddir)/workdir/coolwsd.pid
ERROR_LOG=$(abs_builddir)/workdir/error.log
COOLWSD_LOG=$(abs_builddir)/cypress/wsd_logs/coolwsd.log
COOLWSD_OUTPUT=$(abs_builddir)/cypress/wsd_logs/coolwsd_output.log

SUPPORT_FILE_ABS = $(abs_srcdir)/support/index.js
SUPPORT_FILE = $(if $(findstring $(abs_srcdir),$(abs_builddir)),support/index.js,$(SUPPORT_FILE_ABS))

LIGHTHOUSE_USER_AGENT = "cypress"
LIGHTHOUSE_TEST_FOLDER = $(abs_srcdir)/integration_tests/lighthouse
LIGHTHOUSE_DATA_FOLDER = $(DESKTOP_DATA_FOLDER)
LIGHTHOUSE_DATA_WORKDIR = $(DESKTOP_DATA_WORKDIR)
LIGHTHOUSE_TRACK_FOLDER = $(abs_builddir)/workdir/track/lighthouse

DESKTOP_USER_AGENT = "cypress"
DESKTOP_TEST_FOLDER = $(abs_srcdir)/integration_tests/desktop
DESKTOP_DATA_FOLDER = $(abs_srcdir)/data/desktop/
DESKTOP_DATA_WORKDIR = $(if $(coolwsd),$(DESKTOP_DATA_FOLDER),$(abs_builddir)/workdir/data/desktop/)
DESKTOP_TRACK_FOLDER = $(abs_builddir)/workdir/track/desktop

IDLE_USER_AGENT = "cypress"
IDLE_TEST_FOLDER = $(abs_srcdir)/integration_tests/idle
IDLE_DATA_FOLDER = $(abs_srcdir)/data/idle/
IDLE_DATA_WORKDIR = $(if $(coolwsd),$(IDLE_DATA_FOLDER),$(abs_builddir)/workdir/data/idle/)
IDLE_TRACK_FOLDER = $(abs_builddir)/workdir/track/idle

MOBILE_USER_AGENT = "cypress-mobile"
MOBILE_TEST_FOLDER = $(abs_srcdir)/integration_tests/mobile
MOBILE_DATA_FOLDER = $(abs_srcdir)/data/mobile/
MOBILE_DATA_WORKDIR = $(if $(coolwsd),$(MOBILE_DATA_FOLDER),$(abs_builddir)/workdir/data/mobile/)
MOBILE_TRACK_FOLDER = $(abs_builddir)/workdir/track/mobile

MULTIUSER_TEST_FOLDER = $(abs_srcdir)/integration_tests/multiuser
MULTIUSER_DATA_FOLDER = $(abs_srcdir)/data/multiuser/
MULTIUSER_DATA_WORKDIR = $(if $(coolwsd),$(MULTIUSER_DATA_FOLDER),$(abs_builddir)/workdir/data/multiuser/)
MULTIUSER_TRACK_FOLDER=$(abs_builddir)/workdir/track/multiuser

USER_INTERFACE = classic
UPDATE_SCREENSHOT = false

PHP_PROXY_PORT = 9983
NEXTCLOUD_PORT = 9980
ALLOWED_PORTS = $(if $(findstring php-proxy, $(CYPRESS_INTEGRATION)),$(PHP_PROXY_PORT),\
			$(if $(findstring nextcloud, $(CYPRESS_INTEGRATION)),$(NEXTCLOUD_PORT),$(shell seq 9900 1 9980)))

KILL_COMMAND=$(if $(coolwsd),true,pkill -F $(PID_FILE) || pkill --signal SIGKILL -F $(PID_FILE))
LOAD_PID_FILE=$(abs_builddir)/workdir/load.pids
PARALLEL_BUILD = $(findstring -j,$(MAKEFLAGS))
# use setsid --wait when available so Ctrl-C propagates to the cypress process group
SETSID := setsid $(shell setsid --wait true 2>/dev/null && echo --wait)

COMMA :=,
EMPTY :=
SPACE :=$(EMPTY) $(EMPTY)

CORE_VERSION := $(subst $(SPACE),_,$(shell "@LO_PATH@"/program/soffice.bin --version 2> /dev/null))

BROWSER:=$(if $(CYPRESS_BROWSER),$(CYPRESS_BROWSER),$(CHROME))
A11Y_ENABLE:=$(if $(CYPRESS_A11Y),$(CYPRESS_A11Y),true)
A11Y_TAG:=$(if $(findstring true,$(A11Y_ENABLE)),taga11yenabled,taga11ydisabled)

if ENABLE_DEBUG
FILTER_DEBUG=cypress:electron,cypress:launcher
export DEBUG:=$(or $(DEBUG),$(if $(ENABLE_LOGGING),$(FILTER_DEBUG)))
endif

if HAVE_LO_PATH

MOBILE_TEST_FILES=$(subst $(MOBILE_TEST_FOLDER)/,,$(wildcard $(MOBILE_TEST_FOLDER)/*_spec.js) $(wildcard $(MOBILE_TEST_FOLDER)/*/*_spec.js))

LIGHTHOUSE_TEST_FILES=$(subst $(LIGHTHOUSE_TEST_FOLDER)/,,$(wildcard $(LIGHTHOUSE_TEST_FOLDER)/*_spec.js) $(wildcard $(LIGHTHOUSE_TEST_FOLDER)/*/*_spec.js))

DESKTOP_TEST_FILES=$(subst $(DESKTOP_TEST_FOLDER)/,,$(wildcard $(DESKTOP_TEST_FOLDER)/*_spec.js) $(wildcard $(DESKTOP_TEST_FOLDER)/*/*_spec.js))

IDLE_TEST_FILES=$(subst $(IDLE_TEST_FOLDER)/,,$(wildcard $(IDLE_TEST_FOLDER)/*_spec.js) $(wildcard $(IDLE_TEST_FOLDER)/*/*_spec.js))

MULTIUSER_TEST_FILES=$(subst $(MULTIUSER_TEST_FOLDER)/,,$(wildcard $(MULTIUSER_TEST_FOLDER)/*_spec.js) $(wildcard $(MULTIUSER_TEST_FOLDER)/*/*_spec.js))

MOBILE_TEST_FILES_DONE= \
	$(foreach test_file,$(MOBILE_TEST_FILES),$(MOBILE_TRACK_FOLDER)/$(test_file).done)

DESKTOP_TEST_FILES_DONE= \
	$(foreach test_file,$(DESKTOP_TEST_FILES),$(DESKTOP_TRACK_FOLDER)/$(test_file).done)

IDLE_TEST_FILES_DONE= \
	$(foreach test_file,$(IDLE_TEST_FILES),$(IDLE_TRACK_FOLDER)/$(test_file).done)

MULTIUSER_TESTS_DONE= \
	$(MULTIUSER_TRACK_FOLDER)/multiuser_tests.done

check-local: do-check
	$(if $(wildcard $(ERROR_LOG)),$(error CypressError: some tests failed!))

# TODO: enable multiuser tests $(MULTIUSER_TESTS_DONE)
do-check: $(DESKTOP_TEST_FILES_DONE) $(MOBILE_TEST_FILES_DONE) $(IDLE_TEST_FILES_DONE)
	$(V)$(KILL_COMMAND) || true
	$(if $(wildcard $(ERROR_LOG)),$(V)cat $(ERROR_LOG))

$(PID_FILE): @JAILS_PATH@ @CACHE_PATH@ $(NODE_BINS)
	$(call cleanup_before_run)
	$(call run_JS_error_check)
	$(call start_coolwsd)

setup_short_idle:
	$(eval SHORT_IDLE=1)

setup_regular_idle:
	$(eval SHORT_IDLE=)

start_short_idle_coolwsd: @JAILS_PATH@ @CACHE_PATH@ $(NODE_BINS) setup_short_idle
	$(V)$(KILL_COMMAND) || true
	$(call cleanup_before_run)
	$(call run_JS_error_check)
	$(call start_coolwsd)

# SUITE_ONLY drops the desktop ordering dep so check-mobile can run independently
$(MOBILE_TEST_FILES_DONE): $(PID_FILE) $(if $(SUITE_ONLY),,$(DESKTOP_TEST_FILES_DONE))
	$(if $(PARALLEL_BUILD),\
		$(call run_mobile_tests,$(subst $(MOBILE_TRACK_FOLDER)/,,$(basename $@)),$(basename $@).log),\
		$(call run_mobile_tests))
	$(if $(PARALLEL_BUILD),\
		$(V)mkdir -p $(dir $@) && touch $@\
		,\
		$(V)$(foreach done_file,$(MOBILE_TEST_FILES_DONE),mkdir -p $(dir $(done_file)) && touch $(done_file) &&) true\
	)

$(DESKTOP_TEST_FILES_DONE): $(PID_FILE)
	$(if $(PARALLEL_BUILD),\
		$(call run_desktop_tests,$(subst $(DESKTOP_TRACK_FOLDER)/,,$(basename $@)),$(basename $@).log),\
		$(call run_desktop_tests))
	$(if $(PARALLEL_BUILD),\
		$(V)mkdir -p $(dir $@) && touch $@\
		,\
		$(V)$(foreach done_file,$(DESKTOP_TEST_FILES_DONE),mkdir -p $(dir $(done_file)) && touch $(done_file) &&) true\
	)

$(MULTIUSER_TESTS_DONE): $(PID_FILE) $(MOBILE_TEST_FILES_DONE) $(IDLE_TEST_FILES_DONE)
	$(call run_all_multiuser_tests)
	$(V)mkdir -p $(dir $@) && touch $@

$(IDLE_TEST_FILES_DONE): start_short_idle_coolwsd $(PID_FILE)
	$(if $(PARALLEL_BUILD),\
		$(call run_idle_tests,$(subst $(IDLE_TRACK_FOLDER)/,,$(basename $@)),$(basename $@).log),\
		$(call run_idle_tests))
	$(if $(PARALLEL_BUILD),\
		$(V)mkdir -p $(dir $@) && touch $@\
		,\
		$(V)$(foreach done_file,$(IDLE_TEST_FILES_DONE),mkdir -p $(dir $(done_file)) && touch $(done_file) &&) true\
	)

@JAILS_PATH@:
	mkdir -p $@

@CACHE_PATH@:
	mkdir -p $@

define run_JS_error_check
	$(V)echo "Checking for JS errors in test code..."
	$(V)echo
	$(V)$(NODE) $(ESLINT_BINARY) $(abs_srcdir) \
		--ignore-path $(abs_srcdir)/.eslintignore --config .eslintrc
	$(V)echo
endef

define start_coolwsd
	$(if $(coolwsd),true,$(call start_coolwsd_instance))
endef

# Use 'ulimit' to intentionally reduce the maximum number of open file descriptors: if 1024 is not
# enough, then probably we leak them.
define start_coolwsd_instance
	$(eval FREE_PORT:=$(shell $(GET_PORT_BINARY) --host=127.0.0.1 $(ALLOWED_PORTS)))
	$(if $(findstring php-proxy, $(CYPRESS_INTEGRATION)),\
		$(if $(findstring $(PHP_PROXY_PORT),$(FREE_PORT)),,$(error CypressError: $(PHP_PROXY_PORT) port is unavailable!)))
	$(if $(findstring nextcloud, $(CYPRESS_INTEGRATION)),\
		$(if $(findstring $(NEXTCLOUD_PORT),$(FREE_PORT)),,$(error CypressError: $(NEXTCLOUD_PORT) port is unavailable!)))
	$(V)echo "Found available port for testing: $(FREE_PORT)"
	$(V)echo
	$(V)echo "Launching coolwsd..."
	$(V)fc-cache "@LO_PATH@"/share/fonts/truetype
	$(V)echo
	$(V)mkdir -p $(dir $(COOLWSD_OUTPUT))
	ulimit -n 1024; ../coolwsd --o:sys_template_path="@SYSTEMPLATE_PATH@" \
			--o:child_root_path="@JAILS_PATH@" \
			--o:cache_files.path="@CACHE_PATH@" \
			--o:storage.filesystem[@allow]=true \
			--disable-ssl --forcecaching \
			--o:ssl.cert_file_path="$(abs_top_srcdir)/etc/cert.pem" \
			--o:ssl.key_file_path="$(abs_top_srcdir)/etc/key.pem" \
			--o:ssl.ca_file_path="$(abs_top_srcdir)/etc/ca-chain.cert.pem" \
			--o:admin_console.username=admin --o:admin_console.password=admin \
			--o:logging.file[@enable]=true --o:logging.level=debug \
			--o:logging.disabled_areas="" --o:welcome.enable=false \
			--o:user_interface.mode=$(USER_INTERFACE) \
			--o:accessibility.enable=$(A11Y_ENABLE) \
			--o:security.enable_macros_execution=true \
			$(if $(SHORT_IDLE),--o:per_view.out_of_focus_timeout_secs=1) \
			$(if $(SHORT_IDLE),--o:per_view.idle_timeout_secs=7) \
			--port=$(FREE_PORT) \
			--pidfile=$(PID_FILE) \
			--o:logging.file.property[0]=$(COOLWSD_LOG) \
			--unattended \
			$(if $(findstring php-proxy, $(CYPRESS_INTEGRATION)),--o:net.proxy_prefix=true) \
			 > $(COOLWSD_OUTPUT) 2>&1 &
	$(V)$(WAIT_ON_BINARY) http://localhost:$(FREE_PORT) --timeout 60000 || (echo ==dump coolwsd console output== && cat $(COOLWSD_OUTPUT) && exit 1)
	$(V)echo
endef

define cleanup_before_run
	$(V)rm -f $(ERROR_LOG)
	$(V)rm -f $(PID_FILE)
	$(V)rm -rf cypress
	$(V)rm -rf integration_tests/snapshots/actual
	$(V)rm -rf $(abs_top_srcdir)/test/data/presets
	$(V)cp -r $(abs_top_srcdir)/test/data/presets-default $(abs_top_srcdir)/test/data/presets
endef

NODE_BINS = \
	$(CYPRESS_BINARY) \
	$(ESLINT_BINARY) \
	$(WAIT_ON_BINARY) \
	$(GET_PORT_BINARY)

$(NODE_BINS): $(NPM_INSTALLED);

$(NPM_INSTALLED): package.json eslint_plugin/index.js eslint_plugin/package.json $(wildcard patches/*.patch)
#	patch-package cannot apply updated patches over already-patched files
#	remove patched packages so npm install restores clean originals first.
#	https://github.com/ds300/patch-package/issues/227
	$(V)rm -rf node_modules/cypress node_modules/cypress-log-to-output
	$(V)UV_USE_IO_URING=0 npm install --no-audit
	$(V)echo node_modules:
	$(V)ls node_modules
	$(V)mkdir -p $(dir $(NPM_INSTALLED))
	$(V)touch $(NPM_INSTALLED)

clean-local:
	rm -rf node_modules
	rm -rf workdir
	rm -rf cypress
	rm -rf integration_tests/snapshots/actual
	rm -rf package-lock.json
	rm -rf .nyc_output
	rm -rf coverage

# Used on jenkins to run all the tests in parallel as much as possible
check-desktop-mobile-multi:
	check-desktop
	check-mobile
	check-multi

###############
# Mobile tests.
###############

check-mobile: @JAILS_PATH@ @CACHE_PATH@ $(NODE_BINS)
	$(call cleanup_before_run)
	$(call run_JS_error_check)
# coolwsd is already running via the parent make for the parallel sub-make case
	$(if $(and $(PARALLEL_BUILD), $(if $(spec),,yes)),,$(call start_coolwsd))
# with -jN and no single spec, dispatch to per-file .done targets via sub-make;
# SUITE_ONLY=1 suppresses the mobile -> desktop ordering dependency
	$(if $(and $(PARALLEL_BUILD), $(if $(spec),,yes)),\
		$(MAKE) SUITE_ONLY=1 $(MOBILE_TEST_FILES_DONE),\
		$(call run_mobile_tests,$(spec)))
	$(V)$(KILL_COMMAND) || true
# run_buffered.sh always exits 0 and records failures in ERROR_LOG
	$(V)test ! -f $(ERROR_LOG) || (cat $(ERROR_LOG) && false)

run-mobile: @JAILS_PATH@ @CACHE_PATH@ $(NODE_BINS)
	$(call cleanup_before_run)
	$(call run_JS_error_check)
	$(call start_coolwsd)
	$(call run_interactive_mobile,$(spec)) || true
	$(V)$(KILL_COMMAND) || true

MOBILE_CONFIG = \
	specPattern=$(MOBILE_TEST_FOLDER)/**/*_spec.js,supportFile=$(SUPPORT_FILE),userAgent=$(MOBILE_USER_AGENT)

MOBILE_ENV = \
	CYPRESS_INCLUDE_TAGS=tagmobile,DATA_FOLDER=$(MOBILE_DATA_FOLDER),DATA_WORKDIR=$(MOBILE_DATA_WORKDIR),SERVER=$(COOLWSD_SERVER),SERVER_PORT=$(FREE_PORT),LO_CORE_VERSION="$(CORE_VERSION)",INTEGRATION=$(CYPRESS_INTEGRATION)

# Run one mobile test / all mobile tests in interactive test runner.
# Parameters:
# 1 - spec (optional):	test file to run, if not specified all mobile tests are run.
define run_interactive_mobile
	$(V)echo "Open cypress with mobile tests..."
	$(V)echo
	$(if $(1),\
		$(eval SPEC_FILE=$(MOBILE_TEST_FOLDER)/$(1))\
		$(call check_spec_existence,$(1),mobile)\
		$(call run_interactive_single,$(MOBILE_CONFIG),$(MOBILE_ENV),$(SPEC_FILE))\
	,\
		$(call run_interactive_all,$(MOBILE_CONFIG),$(MOBILE_ENV))\
	)
endef

# Run one mobile test / all mobile tests in headless mode.
# Parameters:
# 1 - spec (optional):	test file to run, if not specified all mobile tests are run.
# 2 - log (optional):	log file for cypress run, needed for parallel build only.
# 3 - +env (optional):	additional env variables (will be appended to $(MOBILE_ENV).
define run_mobile_tests
	$(V)echo $(if $(1),"Running cypress mobile test: $(1)","Running cypress mobile tests...")
	$(V)echo
	$(eval ENV_EXTENDED=$(MOBILE_ENV)$(if $(3),$(COMMA)$(3)))
	$(if $(and $(PARALLEL_BUILD), $(1)),\
		$(V)$(call run_test_buffered,\
			$(MOBILE_CONFIG),$(ENV_EXTENDED),$(1),mobile,$(2))\
	,\
		$(eval SPEC_FILE=$(if $(1),$(MOBILE_TEST_FOLDER)/$(1)))\
		$(if $(1),$(call check_spec_existence,$(1),mobile))\
		$(call run_test_headless,\
			$(MOBILE_CONFIG),$(ENV_EXTENDED),$(SPEC_FILE))\
	)
endef

################
# Lighthouse tests.
################

check-lighthouse: @JAILS_PATH@ @CACHE_PATH@ $(NODE_BINS)
	$(call cleanup_before_run)
	$(call run_JS_error_check)
	$(call start_coolwsd)
	$(call run_lighthouse_tests,$(spec))
	$(V)$(KILL_COMMAND) || true

LIGHTHOUSE_CONFIG = \
	specPattern=$(LIGHTHOUSE_TEST_FOLDER)/**/*_spec.js,supportFile=$(SUPPORT_FILE),userAgent=$(LIGHTHOUSE_USER_AGENT)

LIGHTHOUSE_ENV = \
	DATA_FOLDER=$(LIGHTHOUSE_DATA_FOLDER),DATA_WORKDIR=$(LIGHTHOUSE_DATA_WORKDIR),SERVER=$(COOLWSD_SERVER),SERVER_PORT=$(FREE_PORT),LO_CORE_VERSION="$(CORE_VERSION)",INTEGRATION=$(CYPRESS_INTEGRATION),IMAGES_FOLDER=$(abs_top_srcdir)/browser/images/help/en/

LIGHTHOUSE_INCLUDE_TAGS = \
	taglighthouse,$(A11Y_TAG)

# Run lighthouse accessibility tests with Chrome browser.
# Parameters:
# 1 - spec (optional):	test file to run, if not specified all lighthouse tests are run.
define run_lighthouse_tests
	$(V)echo $(if $(1),"Running lighthouse test: $(1)","Running all lighthouse accessibility tests...")
	$(V)echo
	$(eval CYPRESS_INCLUDE_TAGS=$(LIGHTHOUSE_INCLUDE_TAGS))
	$(V)echo Cypress tags: $(CYPRESS_INCLUDE_TAGS)
	$(V)echo
	$(eval export CYPRESS_INCLUDE_TAGS)
	$(eval LIGHTHOUSE_BROWSER=chrome)
	$(eval SPEC_FILE=$(if $(1),$(LIGHTHOUSE_TEST_FOLDER)/$(1)))
	$(if $(1),$(call check_spec_existence,$(1),lighthouse))
	$(SETSID) $(CYPRESS_BINARY) run \
		--browser $(LIGHTHOUSE_BROWSER) \
		--headless \
		--config-file $(CYPRESS_CONFIG) \
		--config $(LIGHTHOUSE_CONFIG) \
		--env $(LIGHTHOUSE_ENV) \
		$(if $(SPEC_FILE),--spec=$(SPEC_FILE)) \
		|| ($(KILL_COMMAND) && false)
endef

################
# Desktop tests.
################

check-desktop: @JAILS_PATH@ @CACHE_PATH@ $(NODE_BINS)
	$(call cleanup_before_run)
	$(call run_JS_error_check)
# coolwsd is already running via the parent make for the parallel sub-make case
	$(if $(and $(PARALLEL_BUILD), $(if $(spec),,yes)),,$(call start_coolwsd))
# with -jN and no single spec, dispatch to per-file .done targets via sub-make
	$(if $(and $(PARALLEL_BUILD), $(if $(spec),,yes)),\
		$(MAKE) $(DESKTOP_TEST_FILES_DONE),\
		$(call run_desktop_tests,$(spec)))
	$(V)$(KILL_COMMAND) || true
# run_buffered.sh always exits 0 and records failures in ERROR_LOG
	$(V)test ! -f $(ERROR_LOG) || (cat $(ERROR_LOG) && false)

run-desktop: @JAILS_PATH@ @CACHE_PATH@ $(NODE_BINS)
	$(call cleanup_before_run)
	$(call run_JS_error_check)
	$(call start_coolwsd)
	$(call run_interactive_desktop,$(spec)) || true
	$(V)$(KILL_COMMAND) || true

DESKTOP_CONFIG = \
	specPattern=$(DESKTOP_TEST_FOLDER)/**/*_spec.js,supportFile=$(SUPPORT_FILE),userAgent=$(DESKTOP_USER_AGENT)

DESKTOP_ENV = \
	DATA_FOLDER=$(DESKTOP_DATA_FOLDER),DATA_WORKDIR=$(DESKTOP_DATA_WORKDIR),SERVER=$(COOLWSD_SERVER),SERVER_PORT=$(FREE_PORT),LO_CORE_VERSION="$(CORE_VERSION)",INTEGRATION=$(CYPRESS_INTEGRATION),IMAGES_FOLDER=$(abs_top_srcdir)/browser/images/help/en/

DESKTOP_INCLUDE_TAGS = \
	tagdesktop,$(A11Y_TAG)

# Run one desktop test / all desktop tests in interactive test runner.
# Parameters:
# 1 - spec (optional):	test file to run, if not specified all desktop tests are run.
define run_interactive_desktop
	$(V)echo "Open cypress with desktop tests..."
	$(V)echo
	$(eval CYPRESS_INCLUDE_TAGS=$(DESKTOP_INCLUDE_TAGS))
	$(V)echo Cypress tags: $(CYPRESS_INCLUDE_TAGS)
	$(V)echo
	$(eval export CYPRESS_INCLUDE_TAGS)
	$(if $(1),\
		$(eval SPEC_FILE=$(DESKTOP_TEST_FOLDER)/$(1))\
		$(call check_spec_existence,$(1),desktop)\
		$(call run_interactive_single,$(DESKTOP_CONFIG),$(DESKTOP_ENV),$(SPEC_FILE))\
	,\
		$(call run_interactive_all,$(DESKTOP_CONFIG),$(DESKTOP_ENV))\
	)
endef

# Run one desktop test / all desktop tests in headless mode.
# Parameters:
# 1 - spec (optional):	test file to run, if not specified all desktop tests are run.
# 2 - log (optional):	log file for cypress run, needed for parallel build only.
# 3 - +env (optional):	additional env variables (will be appended to $(DESKTOP_ENV).
define run_desktop_tests
	$(V)echo $(if $(1),"Running cypress desktop mode test: $(1)","Running cypress desktop mode tests...")
	$(V)echo
	$(eval CYPRESS_INCLUDE_TAGS=$(DESKTOP_INCLUDE_TAGS))
	$(V)echo Cypress tags: $(CYPRESS_INCLUDE_TAGS)
	$(V)echo
	$(eval export CYPRESS_INCLUDE_TAGS)
	$(eval ENV_EXTENDED=$(DESKTOP_ENV)$(if $(3),$(COMMA)$(3)))
	$(if $(and $(PARALLEL_BUILD), $(1)),\
		$(V)$(call run_test_buffered,\
			$(DESKTOP_CONFIG),$(ENV_EXTENDED),$(1),desktop,$(2))\
	,\
		$(eval SPEC_FILE=$(if $(1),$(DESKTOP_TEST_FOLDER)/$(1)))\
		$(if $(1),$(call check_spec_existence,$(1),desktop))\
		$(call run_test_headless,\
			$(DESKTOP_CONFIG),$(ENV_EXTENDED),$(SPEC_FILE))\
	)
endef

################
# Idle tests.
################

check-idle: @JAILS_PATH@ @CACHE_PATH@ $(NODE_BINS) start_short_idle_coolwsd $(PID_FILE)
	$(call run_idle_tests,$(spec))
	$(V)$(KILL_COMMAND) || true

run-idle: @JAILS_PATH@ @CACHE_PATH@ $(NODE_BINS) start_short_idle_coolwsd $(PID_FILE)
	$(call run_interactive_idle,$(spec)) || true
	$(V)$(KILL_COMMAND) || true

IDLE_CONFIG = \
	specPattern=$(IDLE_TEST_FOLDER)/**/*_spec.js,supportFile=$(SUPPORT_FILE),userAgent=$(IDLE_USER_AGENT)

IDLE_ENV = \
	DATA_FOLDER=$(IDLE_DATA_FOLDER),DATA_WORKDIR=$(IDLE_DATA_WORKDIR),SERVER=$(COOLWSD_SERVER),SERVER_PORT=$(FREE_PORT),LO_CORE_VERSION="$(CORE_VERSION)",INTEGRATION=$(CYPRESS_INTEGRATION),IMAGES_FOLDER=$(abs_top_srcdir)/browser/images/help/en/

IDLE_INCLUDE_TAGS = \
	tagdesktop,$(A11Y_TAG)

# Run one idle test / all idle tests in interactive test runner.
# Parameters:
# 1 - spec (optional):	test file to run, if not specified all idle tests are run.
define run_interactive_idle
	$(V)echo "Open cypress with idle tests..."
	$(V)echo
	$(eval CYPRESS_INCLUDE_TAGS=$(IDLE_INCLUDE_TAGS))
	$(V)echo Cypress tags: $(CYPRESS_INCLUDE_TAGS)
	$(V)echo
	$(eval export CYPRESS_INCLUDE_TAGS)
	$(if $(1),\
		$(eval SPEC_FILE=$(IDLE_TEST_FOLDER)/$(1))\
		$(call check_spec_existence,$(1),idle)\
		$(call run_interactive_single,$(IDLE_CONFIG),$(IDLE_ENV),$(SPEC_FILE))\
	,\
		$(call run_interactive_all,$(IDLE_CONFIG),$(IDLE_ENV))\
	)
endef

# Run one idle test / all idle tests in headless mode.
# Parameters:
# 1 - spec (optional):	test file to run, if not specified all idle tests are run.
# 2 - log (optional):	log file for cypress run, needed for parallel build only.
# 3 - +env (optional):	additional env variables (will be appended to $(IDLE_ENV).
define run_idle_tests
	$(V)echo $(if $(1),"Running cypress idle test: $(1)","Running cypress idle tests...")
	$(V)echo
	$(eval CYPRESS_INCLUDE_TAGS=$(IDLE_INCLUDE_TAGS))
	$(V)echo Cypress tags: $(CYPRESS_INCLUDE_TAGS)
	$(V)echo
	$(eval export CYPRESS_INCLUDE_TAGS)
	$(eval ENV_EXTENDED=$(IDLE_ENV)$(if $(3),$(COMMA)$(3)))
	$(if $(and $(PARALLEL_BUILD), $(1)),\
		$(V)$(call run_test_buffered,\
			$(IDLE_CONFIG),$(ENV_EXTENDED),$(1),idle,$(2))\
	,\
		$(eval SPEC_FILE=$(if $(1),$(IDLE_TEST_FOLDER)/$(1)))\
		$(if $(1),$(call check_spec_existence,$(1),idle))\
		$(call run_test_headless,\
			$(IDLE_CONFIG),$(ENV_EXTENDED),$(SPEC_FILE))\
	)
endef

###################
# Multi-user tests.
###################
check-multi: @JAILS_PATH@ @CACHE_PATH@ $(NODE_BINS)
	$(call cleanup_before_run)
	$(call run_JS_error_check)
	$(call start_coolwsd)
	$(if $(spec), \
		$(call run_multiuser_test,$(spec)), \
		$(call run_all_multiuser_tests))
	$(V)$(KILL_COMMAND) || true

run-multi: @JAILS_PATH@ @CACHE_PATH@ $(NODE_BINS)
	$(call cleanup_before_run)
	$(call run_JS_error_check)
	$(call start_coolwsd)
	$(call run_interactive_multiuser,$(spec)) || true
	$(V)$(KILL_COMMAND) || true

MULTIUSER_CONFIG = \
	specPattern=$(MULTIUSER_TEST_FOLDER)/**/*_spec.js,supportFile=$(SUPPORT_FILE),userAgent=$(DESKTOP_USER_AGENT),defaultCommandTimeout=30000

MULTIUSER_ENV = \
	CYPRESS_INCLUDE_TAGS=tagmultiuser,DATA_FOLDER=$(MULTIUSER_DATA_FOLDER),DATA_WORKDIR=$(MULTIUSER_DATA_WORKDIR),SERVER=$(COOLWSD_SERVER),SERVER_PORT=$(FREE_PORT),LO_CORE_VERSION="$(CORE_VERSION)",INTEGRATION=$(CYPRESS_INTEGRATION)

# Run all multi-user tests in headless mode.
# Parameters:
# 1 - +env (optional):	additional env variables (will be appended to $(MULTIUSER_ENV).
define run_all_multiuser_tests
	$(V)echo "Running cypress multi-user tests..."
	$(V)echo
	$(eval ENV_EXTENDED=$(MULTIUSER_ENV)$(if $(1),$(COMMA)$(1)))
	$(call run_test_headless,\
		$(MULTIUSER_CONFIG),$(ENV_EXTENDED),)
endef

# Run one multiuser test / all multiuser tests in interactive test runner.
# Parameters:
# 1 - spec (optional):	test file to run, if not specified all desktop tests are run.
define run_interactive_multiuser
	$(V)echo "Open cypress with multiuser tests..."
	$(V)echo
	$(if $(1),\
		$(eval SPEC_FILE=$(MULTIUSER_TEST_FOLDER)/$(1))\
		$(call check_spec_existence,$(1),multi)\
		$(call run_interactive_single,$(MULTIUSER_CONFIG),$(MULTIUSER_ENV),$(SPEC_FILE))\
	,\
		$(call run_interactive_all,$(MULTIUSER_CONFIG),$(MULTIUSER_ENV))\
	)
endef

# Run one multi-user test in headless mode.
# Parameters:
# 1 - test name:	test name to run. This test name means to test files (user1, user2).
# 2 - +env (optional):	additional env variables (will be appended to $(MULTIUSER_ENV).
define run_multiuser_test
	$(call check_spec_existence,$(1),multi)
	$(V)echo "Running cypress multi-user test: $(1)"
	$(V)echo
	$(eval ENV_EXTENDED=$(MULTIUSER_ENV)$(if $(2),$(COMMA)$(2)))
	$(eval SPEC_FILE=$(if $(1),$(MULTIUSER_TEST_FOLDER)/$(1)))
	$(call run_test_headless,\
		$(MULTIUSER_CONFIG),$(ENV_EXTENDED),$(SPEC_FILE))
endef

############################
# Mobile interference tests.
############################

check-interfer-mobile: do-interfer-mobile-check-log
	$(if $(wildcard $(ERROR_LOG)),$(error CypressError: some tests failed!))

do-interfer-mobile-check-log: do-interfer-mobile-check
	$(if $(wildcard $(ERROR_LOG)),$(V)cat $(ERROR_LOG))

do-interfer-mobile-check: @JAILS_PATH@ @CACHE_PATH@ $(NODE_BINS)
	$(call cleanup_before_run)
	$(call run_JS_error_check)
	$(call start_coolwsd)
	$(if $(spec), \
		$(call run_mobile_interference_test,$(spec)), \
		$(call run_mobile_interference_tests))
	$(V)$(KILL_COMMAND) || true

run-interfer-mobile: @JAILS_PATH@ @CACHE_PATH@ $(NODE_BINS)
	$(call cleanup_before_run)
	$(call run_JS_error_check)
	$(call start_coolwsd)
	$(V)echo
	$(V)echo "Running interfer mobile test in interactive test runner..."
	$(V)echo
	$(call check_spec_existence,$(spec),mobile)
	$(eval USER1_SPEC=$(MOBILE_TEST_FOLDER)/$(spec))
	$(eval USER2_SPEC=interference_user_spec.js)
	$(eval USER2_LOG=$(MULTIUSER_TRACK_FOLDER)/$(USER2_SPEC).log)
	$(call run_interactive_single,\
		$(INTERFER_MOBILE_CONFIG1),$(INTERFER_MOBILE_ENV1),$(USER1_SPEC)) &
	$(V)sleep 5
	$(V)$(call run_test_buffered,\
		$(INTERFER_MOBILE_CONFIG2),$(INTERFER_MOBILE_ENV2),$(USER2_SPEC),interfer,$(USER2_LOG)) || true
	$(V)$(KILL_COMMAND) || true

INTERFER_MOBILE_CONFIG1 = \
	specPattern=$(MOBILE_TEST_FOLDER)/**/*_spec.js,supportFile=$(SUPPORT_FILE),userAgent=$(MOBILE_USER_AGENT),defaultCommandTimeout=30000

INTERFER_MOBILE_ENV1 = \
	CYPRESS_INCLUDE_TAGS=tagmobile,DATA_FOLDER=$(MOBILE_DATA_FOLDER),DATA_WORKDIR=$(MULTIUSER_DATA_WORKDIR),SERVER=$(COOLWSD_SERVER),SERVER_PORT=$(FREE_PORT),LO_CORE_VERSION="$(CORE_VERSION)",INTEGRATION=$(CYPRESS_INTEGRATION),INTERFERENCE_TEST=true

INTERFER_MOBILE_CONFIG2 = \
	supportFile=$(SUPPORT_FILE),userAgent=$(MOBILE_USER_AGENT),defaultCommandTimeout=30000,retries=0

INTERFER_MOBILE_ENV2 = \
	CYPRESS_INCLUDE_TAGS=tagmobile,DATA_FOLDER=$(MOBILE_DATA_FOLDER),DATA_WORKDIR=$(MULTIUSER_DATA_WORKDIR),SERVER=$(COOLWSD_SERVER),SERVER_PORT=$(FREE_PORT),LO_CORE_VERSION="$(CORE_VERSION)",INTEGRATION=$(CYPRESS_INTEGRATION)

# Run all mobile interference tests in headless mode.
define run_mobile_interference_tests
	$(foreach test,$(MOBILE_TEST_FILES),$(call run_mobile_interference_test,$(test)))
endef

# Run one mobile interference test in headless mode.
# Parameters:
# 1 - mobile test file:	  test file to run parallel with the interference user spec file.
define run_mobile_interference_test
	$(V)echo "Running cypress mobile interference test: $(1)"
	$(V)echo
	$(call check_spec_existence,$(1),mobile)
	$(eval USER1_SPEC=$(1))
	$(eval USER2_SPEC=interference_user_spec.js)
	$(eval USER1_LOG=$(MULTIUSER_TRACK_FOLDER)/$(USER1_SPEC).log)
	$(eval USER2_LOG=$(MULTIUSER_TRACK_FOLDER)/$(USER2_SPEC).log)
	$(V)$(call run_test_buffered,\
		$(INTERFER_MOBILE_CONFIG1),$(INTERFER_MOBILE_ENV1),$(USER1_SPEC),interfer-mobile,$(USER1_LOG)) & \
	sleep 5 && \
	$(call run_test_buffered,\
		$(INTERFER_MOBILE_CONFIG2),$(INTERFER_MOBILE_ENV2),$(USER2_SPEC),interfer,$(USER2_LOG)) && \
	wait # waits the background process to be finished
endef

#########################
# proxy.php desktop tests
#########################

check-php-proxy:
	@mkdir -p workdir
	@rm -rf workdir/proxy.php
	curl -L -o workdir/proxy.php https://raw.githubusercontent.com/CollaboraOnline/richdocumentscode/master/proxy.php
	@# necessary for comunication of proxy.php in docker with locally running coolwsd
	sed -i 's/localhost/172.17.0.1/g' workdir/proxy.php
	-docker stop php-proxy 2>/dev/null || true
	-docker rm php-proxy 2>/dev/null || true
	docker run -d --name php-proxy -p 80:80 -v $(PWD)/workdir/proxy.php:/var/www/html/proxy.php php:8.2-apache
	@echo "PHP proxy server starting on http://localhost"
	sleep 3
	-$(MAKE) CYPRESS_INTEGRATION="php-proxy" check-desktop
	$(MAKE) cleanup-php-proxy

cleanup-php-proxy:
	-docker stop php-proxy
	-docker rm php-proxy

#############################
# Desktop interference tests.
#############################

check-interfer-desktop: do-interfer-desktop-check-log
	$(if $(wildcard $(ERROR_LOG)),$(error CypressError: some tests failed!))

do-interfer-desktop-check-log: do-interfer-desktop-check
	$(if $(wildcard $(ERROR_LOG)),$(V)cat $(ERROR_LOG))

do-interfer-desktop-check: @JAILS_PATH@ @CACHE_PATH@ $(NODE_BINS)
	$(call cleanup_before_run)
	$(call run_JS_error_check)
	$(call start_coolwsd)
	$(if $(spec), \
		$(call run_desktop_interference_test,$(spec)), \
		$(call run_desktop_interference_tests))
	$(V)$(KILL_COMMAND) || true

run-interfer-desktop: @JAILS_PATH@ @CACHE_PATH@ $(NODE_BINS)
	$(call cleanup_before_run)
	$(call run_JS_error_check)
	$(call start_coolwsd)
	$(V)echo
	$(V)echo "Running interfer desktop test in interactive test runner..."
	$(V)echo
	$(call check_spec_existence,$(spec),desktop)
	$(eval USER1_SPEC=$(DESKTOP_TEST_FOLDER)/$(spec))
	$(eval USER2_SPEC=interference_user_spec.js)
	$(eval USER2_LOG=$(MULTIUSER_TRACK_FOLDER)/$(USER2_SPEC).log)
	$(call run_interactive_single,\
		$(INTERFER_DESKTOP_CONFIG1),$(INTERFER_DESKTOP_ENV1),$(USER1_SPEC)) &
	$(V)sleep 5
	$(V)$(call run_test_buffered,\
		$(INTERFER_DESKTOP_CONFIG2),$(INTERFER_DESKTOP_ENV2),$(USER2_SPEC),interfer,$(USER2_LOG)) || true
	$(V)$(KILL_COMMAND) || true

INTERFER_DESKTOP_CONFIG1 = \
	specPattern=$(DESKTOP_TEST_FOLDER)/**/*_spec.js,supportFile=$(SUPPORT_FILE),userAgent=$(DESKTOP_USER_AGENT),defaultCommandTimeout=30000

INTERFER_DESKTOP_ENV1 = \
	DATA_FOLDER=$(DESKTOP_DATA_FOLDER),DATA_WORKDIR=$(MULTIUSER_DATA_WORKDIR),SERVER=$(COOLWSD_SERVER),SERVER_PORT=$(FREE_PORT),LO_CORE_VERSION="$(CORE_VERSION)",INTEGRATION=$(CYPRESS_INTEGRATION),INTERFERENCE_TEST=true

INTERFER_DESKTOP_CONFIG2 = \
	supportFile=$(SUPPORT_FILE),userAgent=$(DESKTOP_USER_AGENT),defaultCommandTimeout=30000,retries=0

INTERFER_DESKTOP_ENV2 = \
	DATA_FOLDER=$(DESKTOP_DATA_FOLDER),DATA_WORKDIR=$(MULTIUSER_DATA_WORKDIR),SERVER=$(COOLWSD_SERVER),SERVER_PORT=$(FREE_PORT),LO_CORE_VERSION="$(CORE_VERSION)",INTEGRATION=$(CYPRESS_INTEGRATION)

# Run all desktop interference test in headless mode.
define run_desktop_interference_tests
	$(foreach test,$(DESKTOP_TEST_FILES),$(call run_desktop_interference_test,$(test)))
endef

# Run one desktop interference test in headless mode.
# Parameters:
# 1 - desktop test file:    test file to run parallel with the interference user spec file.
define run_desktop_interference_test
	$(V)echo "Running cypress desktop interference test: $(1)"
	$(V)echo
	$(call check_spec_existence,$(1),desktop)
	$(eval USER1_SPEC=$(1))
	$(eval USER2_SPEC=interference_user_spec.js)
	$(eval USER1_LOG=$(MULTIUSER_TRACK_FOLDER)/$(USER1_SPEC).log)
	$(eval USER2_LOG=$(MULTIUSER_TRACK_FOLDER)/$(USER2_SPEC).log)
	$(V)$(call run_test_buffered,\
		$(INTERFER_DESKTOP_CONFIG1),$(INTERFER_DESKTOP_ENV1),$(USER1_SPEC),interfer-desktop,$(USER1_LOG)) & \
	sleep 5 && \
	$(call run_test_buffered,\
		$(INTERFER_DESKTOP_CONFIG2),$(INTERFER_DESKTOP_ENV2),$(USER2_SPEC),interfer,$(USER2_LOG)) && \
	wait # waits the background process to be finished
endef

################
# Test coverage.
################

run-cov: do-run-cov
	$(if $(wildcard $(ERROR_LOG)),$(V)cat $(ERROR_LOG))

do-run-cov: @JAILS_PATH@ @CACHE_PATH@ $(NODE_BINS)
	$(V)echo
	$(V)echo "Setup coverage tools..."
	$(V)echo
	rm -rf .nyc_output
	rm -rf coverage
	UV_USE_IO_URING=0 npm install @cypress/code-coverage --no-save
	$(V)echo "import '@cypress/code-coverage/support';" >> $(SUPPORT_FILE_ABS)
	$(V)echo "" >> $(SUPPORT_FILE_ABS)
	cd .. && npx nyc instrument --compact=false browser/src browser/dist/src && cd cypress_test
	$(V)echo
	$(V)echo "Run all tests..."
	$(V)echo
	$(call cleanup_before_run)
	$(call start_coolwsd)
	$(call run_desktop_tests,,,COVERAGE_RUN="1")
	$(call run_mobile_tests,,,COVERAGE_RUN="1")
	$(call run_all_multiuser_tests,COVERAGE_RUN="1")
	$(V)$(KILL_COMMAND) || true
	$(call run_idle_tests,,,COVERAGE_RUN="1")
	$(V)$(KILL_COMMAND) || true

####################
# General functions.
####################

# Open cypress interactive test runner window with all tests in it.
#
# Parameters:
# 1 - config-file: cypress configuration file (e.g. cypress.config.ts).
# 2 - config: cypress configuration (e.g. cypress' --config argument).
# 3 - env:    list of environment variables (e.g. cypress' --env argument).
define run_interactive_all
	DEBUG='cypress:*' \
		$(CYPRESS_BINARY) open \
		--config-file $(CYPRESS_CONFIG) \
		--config $(1) \
		--env $(2)
endef

# Run single test in cypress interactive test runner.
#
# Parameters:
# 1 - config-file: cypress configuration file (e.g. cypress.config.ts).
# 2 - config: cypress configuration (e.g. cypress' --config argument).
# 3 - env:    list of environment variables (e.g. cypress' --env argument).
# 4 - spec:   test file path (e.g. cypress' --spec argument)
define run_interactive_single
	DEBUG='cypress:*' \
		$(SETSID) $(CYPRESS_BINARY) run \
		--browser $(BROWSER) \
		--headed --no-exit \
		--config-file $(CYPRESS_CONFIG) \
		--config $(1),retries=0 \
		--env $(2) \
		--spec=$(3)
endef

# Run one test in parallel mode, using parallel running script.
#
# Parameters:
# 1 - config: cypress configuration (e.g. cypress' --config argument).
# 2 - env:    list of environment variables (e.g. cypress' --env argument).
# 3 - spec:   test file path (e.g. cypress' --spec argument)
# 4 - type:   test type (e.g. mobile, desktop, multi-user)
# 5 - log:    log file path
# 6 - config-file: cypress configuration file (e.g. cypress.config.ts)
define run_test_buffered
	$(BUFFERED_SCRIPT) \
		--browser $(BROWSER) \
		--config $(1) \
		--env $(2) \
		--spec $(3) \
		--type $(4) \
		--log-file $(5) \
		--config-file $(CYPRESS_CONFIG)
endef

# Shell commands for CPU load generation (expanded conditionally based on ENABLE_CPULOAD)
LOAD_START_CMD = $(if $(ENABLE_CPULOAD),\
	echo "Starting CPU load generators..."; \
	rm -f $(LOAD_PID_FILE); \
	for i in $$(seq 1 $$(nproc)); do \
		(while true; do :; done) & echo $$! >> $(LOAD_PID_FILE); \
	done; \
	echo "Load generators started";)
LOAD_STOP_CMD = $(if $(ENABLE_CPULOAD),\
	echo "Stopping CPU load generators..."; \
	if [ -f $(LOAD_PID_FILE) ]; then \
		while read pid; do kill $$pid 2>/dev/null || true; done < $(LOAD_PID_FILE); \
		rm -f $(LOAD_PID_FILE); \
	fi; \
	echo "Load generators stopped";)

# Run one test \ tests in headless mode.
#
# Parameters:
# 1 - config-file:	 cypress configuration file (e.g. cypress.config.ts).
# 2 - config:		 cypress configuration (e.g. cypress' --config argument).
# 3 - env:		 list of environment variables (e.g. cypress' --env argument).
# 4 - spec (optional):   test file path (e.g. cypress' --spec argument)
define run_test_headless
	$(LOAD_START_CMD) \
	$(SETSID) $(CYPRESS_BINARY) run \
		--browser $(BROWSER) \
		--headless \
		--config-file $(CYPRESS_CONFIG) \
		--config $(1) \
		--env $(2) \
		$(if $(3),--spec=$(3)); \
		rc=$$?; $(LOAD_STOP_CMD) $(KILL_COMMAND) || true; exit $$rc
endef

# Checks whether the given spec file exists.
# We run this check when the spec file was specified by
# the user so we can give clearer error message.
#
# Parameters:
# 1 - spec:	      test file path (e.g. make check-mobile spec=...)
# 2 - test type:      mobile, desktop, idle or multi test.
define check_spec_existence
	$(eval TEST_FILES=$(if $(filter mobile,$(2)),$(MOBILE_TEST_FILES),\
				$(if $(filter desktop,$(2)),$(DESKTOP_TEST_FILES),\
				$(if $(filter idle,$(2)),$(IDLE_TEST_FILES),\
				$(if $(filter lighthouse,$(2)),$(LIGHTHOUSE_TEST_FILES),\
				$(if $(filter multi,$(2)),$(MULTIUSER_TEST_FILES)))))))
	$(if $(filter $(1),$(TEST_FILES)),,$(error CypressError: Can't find the given spec file!))

endef

else

check-local:
	$(error CypressError: Can't find LibreOffice core installation!)

endif
