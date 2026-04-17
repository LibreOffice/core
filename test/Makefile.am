#
# Copyright the Collabora Online contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Cap threadpools to 4 threads.
export MAX_CONCURRENCY=4
AUTOMAKE_OPTION = serial-tests

DIST_FOLDER ?= $(abs_top_builddir)/browser/dist

# unittest: tests that run a captive coolwsd as part of themselves.
if ENABLE_LIBFUZZER
check_PROGRAMS =
noinst_PROGRAMS =
else
check_PROGRAMS = fakesockettest
noinst_PROGRAMS = fakesockettest
endif
noinst_PROGRAMS += unittest unithttplib clienttest

include_paths = ${ZLIB_CFLAGS} ${ZSTD_CFLAGS} ${PNG_CFLAGS}
if ENABLE_SSL
include_paths += ${OPENSSL_CFLAGS}
endif

if ENABLE_MACOS
util_platform_cpp = ../common/Util-macos.cpp
else
util_platform_cpp = ../common/Util-linux.cpp
endif

AM_CXXFLAGS = $(CPPUNIT_CFLAGS) -DTDOC=\"$(abs_top_srcdir)/test/data\" -DTDIST=\"$(DIST_FOLDER)\" \
	-I${top_srcdir}/common -I${top_srcdir}/net -I${top_srcdir}/wsd -I${top_srcdir}/kit \
	-I${top_srcdir} -I${top_srcdir}/test \
	${include_paths}

if !ENABLE_LIBFUZZER
# These are ordered by how long each takes to run.
# The longest-running tests are put first, the
# fastest tests are last. This reduces the
# total time with parallel runs.
# The time each test takes is logged, looked for
# 'Finished in' in the `make check` output.
# When adding new tests, please maintain order.
all_la_unit_tests = \
	unit-tiletest.la \
	unit-wopi-documentconflict.la \
	unit-wopi-stuck-save.la \
	unit-wopi-fail-upload.la \
	unit-wopi-async-slow.la \
	unit-perf.la \
	unit-wopi-lock.la \
	unit-save-torture.la \
	unit-invalidation.la \
	unit-close.la \
	unit-user-presets.la \
	unit-wopi-watermark.la \
	unit-crash.la \
	unit-integration.la \
	unit-timeout.la \
	unit-wopi-save-on-exit.la \
	unit-initial-load-fail.la \
	unit-insert-delete.la \
	unit-uno-command.la \
	unit-quarantine.la \
	unit-wopi.la \
	unit-each-view.la \
	unit-copy-paste.la \
	unit-httpws.la \
	unit-load-torture.la \
	unit-load.la \
	unit-wopi-languages.la \
	unit-session.la \
	unit-wopi-edge-cases.la \
	unit-socket-failure.la \
	unit-kit-child-session.la \
	unit-wopi-fileurl.la \
	unit-cursor.la \
	unit-copy-paste-writer.la \
	unit-convert-csv.la \
	unit-render-shape.la \
	unit-multi-tenant.la \
	unit-large-paste.la \
	unit-join-disconnect.la \
	unit-convert.la \
	unit-wopi-temp.la \
	unit-wopi-transient-connect-fault.la \
	unit-password-protected.la \
	unit-calc.la \
	unit-rendering-options.la \
	unit-prefork.la \
	unit-any-input.la \
	unit-save.la \
	unit-wopi-saveas.la \
	unit-typing.la \
	unit-tiff-load.la \
	unit-synthetic-lok.la \
	unit-paste.la \
	unit-wopi-versionrestore.la \
	unit-oauth.la \
	unit-http.la \
	unit-wopi-renamefile.la \
	unit-render-search-result.la \
	unit-clipboard-close.la \
	unit-proxy.la \
	unit-wopi-httpredirect.la \
	unit-convert-to-password-protected.la \
	unit-bad-doc-load.la \
	unit-hosting.la
# 	unit-fuzz.la
#	unit-admin.la
#	unit-tilecache.la # Empty test.
endif

check_LTLIBRARIES = ${all_la_unit_tests}

MAGIC_TO_FORCE_SHLIB_CREATION = -rpath /dummy
AM_LDFLAGS = -module $(MAGIC_TO_FORCE_SHLIB_CREATION) $(ZLIB_LIBS) $(ZSTD_LIBS) ${PNG_LIBS}

# Clang's linker doesn't like -pthread.
if !HAVE_CLANG
AM_CXXFLAGS += -pthread
AM_LDFLAGS += -pthread
endif

if ENABLE_SSL
AM_LDFLAGS += ${OPENSSL_LIBS}
endif

# We work around some of the mess of using the same sources both on
# the server side and here in unit tests with conditional compilation
# based on BUILDING_TESTS

AM_CPPFLAGS = -pthread -I$(top_srcdir) -DBUILDING_TESTS -DLOK_ABORT_ON_ASSERTION

kit_sources = \
	../kit/Kit.cpp \
	../kit/KitWebSocket.cpp \
	../kit/TestStubs.cpp

wsd_sources = \
	../common/Crypto.cpp \
	../wsd/Admin.cpp \
	../wsd/AdminModel.cpp \
	../wsd/AIChatSession.cpp \
	../wsd/Auth.cpp \
	../wsd/COOLWSD.cpp \
	../wsd/CacheUtil.cpp \
	../wsd/ClientRequestDispatcher.cpp \
	../wsd/ClientSession.cpp \
	../wsd/ContentType.cpp \
	../wsd/DocumentBroker.cpp \
	../wsd/FileServer.cpp \
	../wsd/FileServerUtil.cpp \
	../wsd/HostUtil.cpp \
	../wsd/ProofKey.cpp \
	../wsd/ProxyProtocol.cpp \
	../wsd/ProxyRequestHandler.cpp \
	../wsd/QuarantineUtil.cpp \
	../wsd/RemoteConfig.cpp \
	../wsd/RequestDetails.cpp \
	../wsd/RequestVettingStation.cpp \
	../wsd/ServerAuditUtil.cpp \
	../wsd/SlideCache.cpp \
	../wsd/SpecialBrokers.cpp \
	../wsd/Storage.cpp \
	../wsd/TileCache.cpp \
	../wsd/dumpWsdState.cpp \
	../wsd/wopi/CheckFileInfo.cpp \
	../wsd/wopi/StorageConnectionManager.cpp \
	../wsd/wopi/WopiProxy.cpp \
	../wsd/wopi/WopiStorage.cpp

test_base_sources = \
	DeltaTests.cpp \
	FileServeWhiteBoxTests.cpp \
	HostUtilTests.cpp \
	HttpWhiteBoxTests.cpp \
	KitQueueTests.cpp \
	LoggingWhiteBoxTests.cpp \
	NetUtilWhiteBoxTests.cpp \
	NumUtilWhiteBoxTests.cpp \
	RequestDetailsTests.cpp \
	StringVectorTests.cpp \
	TestGlobals.cpp \
	UriTests.cpp \
	UtilTests.cpp \
	WhiteBoxTests.cpp \
	WopiProofTests.cpp

common_sources = \
	$(util_platform_cpp) \
	../common/Authorization.cpp \
	../common/CommandControl.cpp \
	../common/ConfigUtil.cpp \
	../common/CoolMount.cpp \
	../common/DummyTraceEventEmitter.cpp \
	../common/FileUtil-unix.cpp \
	../common/FileUtil.cpp \
	../common/JailUtil.cpp \
	../common/Log-common.cpp \
	../common/Log-poco.cpp \
	../common/MobileApp.cpp \
	../common/ProcUtil.cpp \
	../common/Protocol.cpp \
	../common/RegexUtil.cpp \
	../common/Seccomp.cpp \
	../common/Session.cpp \
	../common/SigUtil-server.cpp \
	../common/Simd.cpp \
	../common/SpookyV2.cpp \
	../common/StringVector.cpp \
	../common/Syscall.cpp \
	../common/TraceEvent.cpp \
	../common/Unit-server.cpp \
	../common/Unit.cpp \
	../common/Uri.cpp \
	../common/Util-server.cpp \
	../common/Util-unix.cpp \
	../common/Util.cpp \
	../kit/KitQueue.cpp \
	../kit/LogUI.cpp \
	../net/DelaySocket.cpp \
	../net/HttpHelper.cpp \
	../net/HttpRequest.cpp \
	../net/NetUtil.cpp \
	../net/Socket.cpp \
	../wsd/Exceptions.cpp

globals_sources = ../common/Globals.cpp

unithttplib_CPPFLAGS = -I$(top_srcdir) -DBUILDING_TESTS -DSTANDALONE_CPPUNIT -g
unithttplib_SOURCES = $(common_sources) $(globals_sources) test.cpp HttpRequestTests.cpp
unithttplib_LDADD = $(CPPUNIT_LIBS)

unittest_CPPFLAGS = -I$(top_srcdir) -DBUILDING_TESTS -DSTANDALONE_CPPUNIT -g
unittest_SOURCES = \
	$(common_sources) \
	$(test_base_sources) \
	$(kit_sources) \
	../wsd/FileServerUtil.cpp \
	../wsd/HostUtil.cpp \
	../wsd/ProofKey.cpp \
	../wsd/RequestDetails.cpp \
	../wsd/TestStubs.cpp \
	../wsd/TileCache.cpp \
	test.cpp

unittest_LDADD = $(top_builddir)/libsimd.a $(CPPUNIT_LIBS)

clienttest_CPPFLAGS = -I$(top_srcdir) -DBUILDING_TESTS -DSTANDALONE_CPPUNIT -g

clienttest_SOURCES = \
	$(common_sources) \
	$(globals_sources) \
	$(wsd_sources) \
	ClientTestStubs.cpp \
	ClientRequestDispatcherTests.cpp \
	test.cpp

clienttest_LDADD = $(top_builddir)/libsimd.a $(CPPUNIT_LIBS)

# Add SSL dependencies if needed.
if ENABLE_SSL
unittest_SOURCES += ../net/Ssl.cpp
unithttplib_SOURCES += ../net/Ssl.cpp
clienttest_SOURCES += ../net/Ssl.cpp
else
unittest_LDADD += -lssl -lcrypto
unithttplib_LDADD += -lssl -lcrypto
clienttest_LDADD += -lssl -lcrypto
endif
if ENABLE_LIBFUZZER
unit_fuzz_la_SOURCES = UnitFuzz.cpp
unit_fuzz_la_LIBADD = $(CPPUNIT_LIBS)
else

fakesockettest_CPPFLAGS = -g
fakesockettest_SOURCES = fakesockettest.cpp  ../net/FakeSocket.cpp ../common/DummyTraceEventEmitter.cpp ../common/Log-common.cpp ../common/Log-poco.cpp ../common/ProcUtil.cpp ../common/Util.cpp ../common/Util-server.cpp $(util_platform_cpp) ../common/Util-unix.cpp ../common/Globals.cpp
fakesockettest_LDADD = $(CPPUNIT_LIBS)

# old-style unit tests - bootstrapped via UnitClient
unit_tiletest_la_SOURCES = UnitClient.cpp TileCacheTests.cpp KitPidHelpers.cpp
unit_tiletest_la_LIBADD = $(CPPUNIT_LIBS)
unit_integration_la_SOURCES = UnitClient.cpp integration-http-server.cpp KitPidHelpers.cpp
unit_integration_la_LIBADD = $(CPPUNIT_LIBS)
unit_httpws_la_SOURCES = UnitClient.cpp httpwstest.cpp KitPidHelpers.cpp
unit_httpws_la_LIBADD = $(CPPUNIT_LIBS)
unit_crash_la_SOURCES = UnitClient.cpp httpcrashtest.cpp KitPidHelpers.cpp
unit_crash_la_LIBADD = $(CPPUNIT_LIBS)

# unit test modules:
unit_http_la_SOURCES = UnitHTTP.cpp
unit_http_la_LIBADD = $(CPPUNIT_LIBS)
#unit_admin_la_SOURCES = UnitAdmin.cpp
#unit_admin_la_LIBADD = $(CPPUNIT_LIBS)
unit_typing_la_SOURCES = UnitTyping.cpp
unit_typing_la_LIBADD = $(CPPUNIT_LIBS)
unit_copy_paste_la_SOURCES = UnitCopyPaste.cpp
unit_copy_paste_la_LIBADD = $(CPPUNIT_LIBS)
unit_clipboard_close_la_SOURCES = UnitClipboardClose.cpp
unit_clipboard_close_la_LIBADD = $(CPPUNIT_LIBS)
unit_copy_paste_writer_la_SOURCES = UnitCopyPasteWriter.cpp
unit_copy_paste_writer_la_LIBADD = $(CPPUNIT_LIBS)
unit_convert_la_SOURCES = UnitConvert.cpp
unit_convert_csv_la_SOURCES = UnitConvertCSV.cpp
unit_convert_to_password_protected_la_SOURCES = UnitConvertToPasswordProtected.cpp
unit_initial_load_fail_la_SOURCES = UnitInitialLoadFail.cpp
unit_initial_load_fail_la_LIBADD = $(CPPUNIT_LIBS)
unit_join_disconnect_la_SOURCES = UnitJoinDisconnect.cpp
unit_join_disconnect_la_LIBADD = $(CPPUNIT_LIBS)
unit_timeout_la_SOURCES = UnitTimeout.cpp
unit_timeout_la_LIBADD = $(CPPUNIT_LIBS)
unit_socket_failure_la_SOURCES = UnitSocketFailure.cpp
unit_socket_failure_la_LIBADD = $(CPPUNIT_LIBS)
unit_prefork_la_SOURCES = UnitPrefork.cpp
unit_prefork_la_LIBADD = $(CPPUNIT_LIBS)
# unit_tilecache_la_SOURCES = UnitTileCache.cpp
# unit_tilecache_la_LIBADD = $(CPPUNIT_LIBS)
unit_oauth_la_SOURCES = UnitOAuth.cpp
unit_oauth_la_LIBADD = $(CPPUNIT_LIBS)
unit_wopi_la_SOURCES = UnitWOPI.cpp
unit_wopi_la_LIBADD = $(CPPUNIT_LIBS)
unit_wopi_async_slow_la_SOURCES = UnitWOPISlow.cpp
unit_wopi_async_slow_la_LIBADD = $(CPPUNIT_LIBS)
unit_wopi_saveas_la_SOURCES = UnitWOPISaveAs.cpp
unit_wopi_saveas_la_LIBADD = $(CPPUNIT_LIBS)
unit_wopi_edge_cases_la_SOURCES = UnitWopiEdgeCases.cpp
unit_wopi_edge_cases_la_LIBADD = $(CPPUNIT_LIBS)
unit_wopi_versionrestore_la_SOURCES = UnitWOPIVersionRestore.cpp
unit_wopi_versionrestore_la_LIBADD = $(CPPUNIT_LIBS)
unit_wopi_documentconflict_la_SOURCES = UnitWOPIDocumentConflict.cpp
unit_wopi_documentconflict_la_LIBADD = $(CPPUNIT_LIBS)
unit_wopi_save_on_exit_la_SOURCES = UnitWOPISaveOnExit.cpp
unit_wopi_save_on_exit_la_LIBADD = $(CPPUNIT_LIBS)
unit_wopi_stuck_save_la_SOURCES = UnitWOPIStuckSave.cpp
unit_wopi_stuck_save_la_LIBADD = $(CPPUNIT_LIBS)
unit_wopi_transient_connect_fault_la_SOURCES = UnitWOPITransientConnectFault.cpp
unit_wopi_transient_connect_fault_la_LIBADD = $(CPPUNIT_LIBS)
unit_wopi_fail_upload_la_SOURCES = UnitWOPIFailUpload.cpp
unit_wopi_fail_upload_la_LIBADD = $(CPPUNIT_LIBS)
unit_wopi_renamefile_la_SOURCES = UnitWOPIRenameFile.cpp
unit_wopi_renamefile_la_LIBADD = $(CPPUNIT_LIBS)
unit_wopi_languages_la_SOURCES = UnitWOPILanguages.cpp
unit_wopi_languages_la_LIBADD = $(CPPUNIT_LIBS)
unit_wopi_lock_la_SOURCES = UnitWOPILock.cpp
unit_wopi_lock_la_LIBADD = $(CPPUNIT_LIBS)
unit_wopi_watermark_la_SOURCES = UnitWOPIWatermark.cpp
unit_wopi_watermark_la_LIBADD = $(CPPUNIT_LIBS)
unit_wopi_temp_la_SOURCES = UnitWOPITemplate.cpp
unit_wopi_temp_la_LIBADD = $(CPPUNIT_LIBS)
unit_wopi_fileurl_la_SOURCES = UnitWOPIFileUrl.cpp
unit_wopi_fileurl_la_LIBADD = $(CPPUNIT_LIBS)
unit_wopi_httpredirect_la_SOURCES = UnitWOPIHttpRedirect.cpp
unit_wopi_httpredirect_la_LIBADD = $(CPPUNIT_LIBS)
unit_tiff_load_la_SOURCES = UnitTiffLoad.cpp
unit_tiff_load_la_LIBADD = $(CPPUNIT_LIBS)
unit_save_la_SOURCES = UnitSave.cpp
unit_save_la_LIBADD = $(CPPUNIT_LIBS)
unit_any_input_la_SOURCES = UnitAnyInput.cpp
unit_any_input_la_LIBADD = $(CPPUNIT_LIBS)
unit_kit_child_session_la_SOURCES = UnitKitChildSession.cpp
unit_kit_child_session_la_LIBADD = $(CPPUNIT_LIBS)
unit_large_paste_la_SOURCES = UnitLargePaste.cpp
unit_large_paste_la_LIBADD = $(CPPUNIT_LIBS)
unit_paste_la_SOURCES = UnitPaste.cpp
unit_paste_la_LIBADD = $(CPPUNIT_LIBS)
unit_load_torture_la_SOURCES = UnitLoadTorture.cpp
unit_load_torture_la_LIBADD = $(CPPUNIT_LIBS)
unit_save_torture_la_SOURCES = UnitSaveTorture.cpp
unit_save_torture_la_LIBADD = $(CPPUNIT_LIBS)
unit_synthetic_lok_la_SOURCES = UnitSyntheticLok.cpp
unit_synthetic_lok_la_LIBADD = $(CPPUNIT_LIBS)
unit_rendering_options_la_SOURCES = UnitRenderingOptions.cpp
unit_rendering_options_la_LIBADD = $(CPPUNIT_LIBS)
unit_invalidation_la_SOURCES = UnitInvalidation.cpp
unit_invalidation_la_LIBADD = $(CPPUNIT_LIBS)
unit_password_protected_la_SOURCES = UnitPasswordProtected.cpp
unit_password_protected_la_LIBADD = $(CPPUNIT_LIBS)
unit_render_search_result_la_SOURCES = UnitRenderSearchResult.cpp
unit_render_search_result_la_LIBADD = $(CPPUNIT_LIBS)
unit_render_shape_la_SOURCES = UnitRenderShape.cpp
unit_render_shape_la_LIBADD = $(CPPUNIT_LIBS)
unit_each_view_la_SOURCES = UnitEachView.cpp
unit_each_view_la_LIBADD = $(CPPUNIT_LIBS)
unit_session_la_SOURCES = UnitSession.cpp
unit_session_la_LIBADD = $(CPPUNIT_LIBS)
unit_uno_command_la_SOURCES = UnitUNOCommand.cpp
unit_uno_command_la_LIBADD = $(CPPUNIT_LIBS)
unit_load_la_SOURCES = UnitLoad.cpp
unit_load_la_LIBADD = $(CPPUNIT_LIBS)
unit_cursor_la_SOURCES = UnitCursor.cpp
unit_cursor_la_LIBADD = $(CPPUNIT_LIBS)
unit_calc_la_SOURCES = UnitCalc.cpp
unit_calc_la_LIBADD = $(CPPUNIT_LIBS)
unit_insert_delete_la_SOURCES = UnitInsertDelete.cpp
unit_insert_delete_la_LIBADD = $(CPPUNIT_LIBS)
unit_close_la_SOURCES = UnitClose.cpp
unit_close_la_LIBADD = $(CPPUNIT_LIBS)
unit_bad_doc_load_la_SOURCES = UnitBadDocLoad.cpp
unit_bad_doc_load_la_LIBADD = $(CPPUNIT_LIBS)
unit_hosting_la_SOURCES = UnitHosting.cpp
unit_hosting_la_LIBADD = $(CPPUNIT_LIBS)
unit_quarantine_la_SOURCES = UnitQuarantine.cpp
unit_quarantine_la_LIBADD = $(CPPUNIT_LIBS)
unit_multi_tenant_la_SOURCES = UnitMultiTenant.cpp
unit_multi_tenant_la_LIBADD = $(CPPUNIT_LIBS)
unit_user_presets_la_SOURCES = UnitUserPresets.cpp
unit_user_presets_la_LIBADD = $(CPPUNIT_LIBS)
unit_perf_la_SOURCES = UnitPerf.cpp
unit_perf_la_LIBADD = $(CPPUNIT_LIBS) $(LIBPFM_LIBS)
unit_proxy_la_SOURCES = UnitProxy.cpp
unit_proxy_la_LIBADD = $(CPPUNIT_LIBS) $(LIBPFM_LIBS)
endif

if HAVE_LO_PATH
SYSTEM_STAMP = @SYSTEMPLATE_PATH@/system_stamp
else
SYSTEM_STAMP =
endif

CLEANUP_COMMAND=if test -s ../coolwsd; then echo "Cleaning up..." && ../coolwsd --disable-cool-user-checking --cleanup --o:logging.level=trace; fi

if HAVE_LO_PATH
if !ENABLE_LIBFUZZER
check-local:
	@$(CLEANUP_COMMAND)
	@UNITTEST=1 ./fakesockettest 2> test.log || (cat test.log && /bin/false)
	@fc-cache "@LO_PATH@"/share/fonts/truetype
	@echo "Done test check-local"

# unittest and unithttplib are run in all-local (before TESTS).
TESTS = \
	${all_la_unit_tests}

endif
endif

TEST_EXTENSIONS = .la
LA_LOG_DRIVER = ${top_builddir}/test/run_unit.sh
LOG_DRIVER = ${top_builddir}/test/run_unit_standalone.sh

EXTRA_DIST = data/delta-text.png data/delta-text2.png data/delta-graphic.png data/delta-graphic2.png data/hello.odt data/hello.txt $(test_SOURCES) $(unittest_SOURCES) run_unit.sh run_unit_standalone.sh

build-tests: ${all_la_unit_tests}
	@echo "All test libraries built."

# compile_commands.json support: collect all test source files.
all_test_la_sources = $(sort $(foreach t,$(all_la_unit_tests),$($(subst -,_,$(basename $(t)))_SOURCES)))

all_test_compiled_sources = $(sort \
	$(common_sources) \
	$(globals_sources) \
	$(test_base_sources) \
	$(unittest_SOURCES) \
	$(unithttplib_SOURCES) \
	$(fakesockettest_SOURCES) \
	$(all_test_la_sources) \
)

TEST_JSON_COMPILE_SRC = $(patsubst %.cpp,%.cmd,$(filter %.cpp,$(all_test_compiled_sources)))

CLANGXX_TEST_COMPILE_FLAGS=clang++ $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
	$(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS)

JSON_TEST_COMPILE_FLAGS=$(subst ",\",$(subst \,\\,$(CLANGXX_TEST_COMPILE_FLAGS)))

.cpp.cmd:
	@echo -n "{\"directory\":\"$(abs_builddir)\",\"command\":\"" > $@
	@echo -n $(JSON_TEST_COMPILE_FLAGS) >> $@
	@echo -n " -o $(patsubst %.cpp,%.o,$<) -c $<\",\"file\":\"$<\"}" >> $@

compile_commands: $(TEST_JSON_COMPILE_SRC)

check_valgrind: all
	@fc-cache "@LO_PATH@"/share/fonts/truetype
	./run_unit.sh --log-file test.log --trs-file test.trs --valgrind

# Run unittest and unithttplib during the normal build.
all-local: unittest unithttplib clienttest
	@echo
	@echo "Running build-time unit tests.  For more thorough testing, please run 'make check'."
	@echo "Test output is in `pwd`/test.log and is displayed on failure."
	@echo
	@fc-cache "@LO_PATH@"/share/fonts/truetype
	@UNITTEST=1 ${top_builddir}/test/clienttest --cert-path ${abs_top_srcdir}/etc 2> test.log || (cat test.log && /bin/false)
	@UNITTEST=1 ${top_builddir}/test/unittest --cert-path ${abs_top_srcdir}/etc 2> test.log || (cat test.log && /bin/false)
	@UNITTEST=1 ${top_builddir}/test/unithttplib --cert-path ${abs_top_srcdir}/etc 2> test.log || (cat test.log && /bin/false)
	@echo "Done test all-local"
	@$(CLEANUP_COMMAND)

.PRECIOUS: $(TEST_LOGS)
