# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,python3,python3))

$(eval $(call gb_ExternalPackage_use_external_project,python3,python3))

ifeq ($(OS),WNT)
ifeq ($(CPUNAME),X86_64)
python_arch_subdir=amd64/
else ifeq ($(CPUNAME),AARCH64)
python_arch_subdir=arm64/
else
python_arch_subdir=win32/
endif

$(eval $(call gb_ExternalPackage_add_file,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/bin/python.exe,PCbuild/$(python_arch_subdir)python$(if $(MSVC_USE_DEBUG_RUNTIME),_d).exe))
$(eval $(call gb_ExternalPackage_add_file,python3,$(LIBO_BIN_FOLDER)/python$(PYTHON_VERSION_MAJOR)$(PYTHON_VERSION_MINOR)$(if $(MSVC_USE_DEBUG_RUNTIME),_d).dll,PCbuild/$(python_arch_subdir)python$(PYTHON_VERSION_MAJOR)$(PYTHON_VERSION_MINOR)$(if $(MSVC_USE_DEBUG_RUNTIME),_d).dll))
ifeq ($(MSVC_USE_DEBUG_RUNTIME),)
$(eval $(call gb_ExternalPackage_add_file,python3,$(LIBO_BIN_FOLDER)/python$(PYTHON_VERSION_MAJOR).dll,PCbuild/$(python_arch_subdir)python$(PYTHON_VERSION_MAJOR).dll))
endif
$(eval $(call gb_ExternalPackage_add_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib,\
	PCbuild/$(python_arch_subdir)_asyncio$(if $(MSVC_USE_DEBUG_RUNTIME),_d).pyd \
	PCbuild/$(python_arch_subdir)_ctypes$(if $(MSVC_USE_DEBUG_RUNTIME),_d).pyd \
	PCbuild/$(python_arch_subdir)_decimal$(if $(MSVC_USE_DEBUG_RUNTIME),_d).pyd \
	PCbuild/$(python_arch_subdir)_elementtree$(if $(MSVC_USE_DEBUG_RUNTIME),_d).pyd \
	PCbuild/$(python_arch_subdir)_msi$(if $(MSVC_USE_DEBUG_RUNTIME),_d).pyd \
	PCbuild/$(python_arch_subdir)_multiprocessing$(if $(MSVC_USE_DEBUG_RUNTIME),_d).pyd \
	PCbuild/$(python_arch_subdir)_overlapped$(if $(MSVC_USE_DEBUG_RUNTIME),_d).pyd \
	PCbuild/$(python_arch_subdir)_queue$(if $(MSVC_USE_DEBUG_RUNTIME),_d).pyd \
	PCbuild/$(python_arch_subdir)_socket$(if $(MSVC_USE_DEBUG_RUNTIME),_d).pyd \
	PCbuild/$(python_arch_subdir)_ssl$(if $(MSVC_USE_DEBUG_RUNTIME),_d).pyd \
	PCbuild/$(python_arch_subdir)pyexpat$(if $(MSVC_USE_DEBUG_RUNTIME),_d).pyd \
	PCbuild/$(python_arch_subdir)select$(if $(MSVC_USE_DEBUG_RUNTIME),_d).pyd \
	PCbuild/$(python_arch_subdir)unicodedata$(if $(MSVC_USE_DEBUG_RUNTIME),_d).pyd \
	PCbuild/$(python_arch_subdir)winsound$(if $(MSVC_USE_DEBUG_RUNTIME),_d).pyd \
))
else
$(eval $(call gb_ExternalPackage_add_file,python3,$(LIBO_BIN_FOLDER)/python.bin,python))
$(eval $(call gb_ExternalPackage_add_file,python3,$(LIBO_BIN_FOLDER)/libpython$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)$(if $(ENABLE_DBGUTIL),d).so,libpython$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)$(if $(ENABLE_DBGUTIL),d).so))
$(eval $(call gb_ExternalPackage_add_file,python3,$(LIBO_BIN_FOLDER)/libpython$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)$(if $(ENABLE_DBGUTIL),d).so.1.0,libpython$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)$(if $(ENABLE_DBGUTIL),d).so))
$(eval $(call gb_ExternalPackage_add_file,python3,$(LIBO_BIN_FOLDER)/python.bin-gdb.py,Tools/gdb/libpython.py))

# Unfortunately the python build system does not allow to explicitly enable or
# disable these, it just tries to build them and then prints which did not
# build successfully without stopping; so the build will break on delivering if
# one of these failed to build.
# Obviously this list should not contain stuff with external dependencies
# that may not be available on baseline systems.

ifneq ($(OS),AIX)
python3_EXTENSION_MODULE_SUFFIX=cpython-$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)$(if $(ENABLE_DBGUTIL),d)
$(eval $(call gb_ExternalPackage_add_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/lib-dynload,\
	LO_lib/array.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/_asyncio.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/audioop.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/binascii.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/_bisect.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/_blake2.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/cmath.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/_codecs_cn.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/_codecs_hk.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/_codecs_iso2022.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/_codecs_jp.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/_codecs_kr.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/_codecs_tw.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/_contextvars.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/_crypt.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/_csv.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/_ctypes.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/_datetime.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/_decimal.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/_elementtree.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/fcntl.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/grp.$(python3_EXTENSION_MODULE_SUFFIX).so \
	$(if $(DISABLE_OPENSSL),, \
		LO_lib/_hashlib.$(python3_EXTENSION_MODULE_SUFFIX).so \
	) \
	LO_lib/_heapq.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/_json.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/_lsprof.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/math.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/_md5.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/mmap.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/_multibytecodec.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/_multiprocessing.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/_opcode.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/ossaudiodev.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/parser.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/_pickle.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/_posixshmem.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/_posixsubprocess.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/pyexpat.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/_queue.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/_random.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/resource.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/select.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/_sha1.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/_sha256.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/_sha3.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/_sha512.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/_socket.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/spwd.$(python3_EXTENSION_MODULE_SUFFIX).so \
	$(if $(DISABLE_OPENSSL),, \
		LO_lib/_ssl.$(python3_EXTENSION_MODULE_SUFFIX).so \
	) \
	LO_lib/_statistics.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/_struct.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/syslog.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/termios.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/unicodedata.$(python3_EXTENSION_MODULE_SUFFIX).so \
	$(if $(ENABLE_DBGUTIL),, \
		LO_lib/xxlimited.$(python3_EXTENSION_MODULE_SUFFIX).so \
	)\
	LO_lib/_xxsubinterpreters.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/_xxtestfuzz.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/zlib.$(python3_EXTENSION_MODULE_SUFFIX).so \
))
endif
endif

# headers are not delivered, but used from unpacked dir Include/
# (+ toplevel for pyconfig.h)

ifeq ($(OS),LINUX)
python3_MACHDEP=linux
else
ifeq ($(OS),MACOSX)
python3_MACHDEP=darwin
endif
endif

# that one is generated...
# note: python configure overrides config.guess with something that doesn't
# put -pc in its linux platform triplets, so filter that...
ifneq ($(OS),WNT)
ifeq ($(CPUNAME),ARM)
$(eval $(call gb_ExternalPackage_add_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib,\
	LO_lib/_sysconfigdata_$(if $(ENABLE_DBGUTIL),d)_$(python3_MACHDEP)_$(subst i686,i386,$(subst v7l-unknown,,$(HOST_PLATFORM))).py \
))
else
$(eval $(call gb_ExternalPackage_add_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib,\
	LO_lib/_sysconfigdata_$(if $(ENABLE_DBGUTIL),d)_$(python3_MACHDEP)_$(subst i686,i386,$(subst -pc,,$(HOST_PLATFORM))).py \
))
endif
endif


# packages not shipped:
# dbm, sqlite3 - need some database stuff
# curses - need curses to build the C module
# idlelib, tkinter, turtledemo - need Tk to build the C module
# test - probably unnecessary? was explicitly removed #i116738#
# venv - why would we need virtual environments
#
# These lists are now sorted with "LC_COLLATE=C sort", by using
#   find Lib/ -name "*.py" | sort | sed -e 's/^/\t/' -e 's/$/ \\/'
#

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib,\
	LICENSE \
	Lib/__future__.py \
	Lib/__phello__.foo.py \
	Lib/_bootlocale.py \
	Lib/_collections_abc.py \
	Lib/_compat_pickle.py \
	Lib/_compression.py \
	Lib/_dummy_thread.py \
	Lib/_markupbase.py \
	Lib/_osx_support.py \
	Lib/_py_abc.py \
	Lib/_pydecimal.py \
	Lib/_pyio.py \
	Lib/_sitebuiltins.py \
	Lib/_strptime.py \
	Lib/_threading_local.py \
	Lib/_weakrefset.py \
	Lib/abc.py \
	Lib/aifc.py \
	Lib/antigravity.py \
	Lib/argparse.py \
	Lib/ast.py \
	Lib/asynchat.py \
	Lib/asyncore.py \
	Lib/base64.py \
	Lib/bdb.py \
	Lib/binhex.py \
	Lib/bisect.py \
	Lib/bz2.py \
	Lib/cProfile.py \
	Lib/calendar.py \
	Lib/cgi.py \
	Lib/cgitb.py \
	Lib/chunk.py \
	Lib/cmd.py \
	Lib/code.py \
	Lib/codecs.py \
	Lib/codeop.py \
	Lib/colorsys.py \
	Lib/compileall.py \
	Lib/configparser.py \
	Lib/contextlib.py \
	Lib/contextvars.py \
	Lib/copy.py \
	Lib/copyreg.py \
	Lib/crypt.py \
	Lib/csv.py \
	Lib/dataclasses.py \
	Lib/datetime.py \
	Lib/decimal.py \
	Lib/difflib.py \
	Lib/dis.py \
	Lib/doctest.py \
	Lib/dummy_threading.py \
	Lib/enum.py \
	Lib/filecmp.py \
	Lib/fileinput.py \
	Lib/fnmatch.py \
	Lib/formatter.py \
	Lib/fractions.py \
	Lib/ftplib.py \
	Lib/functools.py \
	Lib/genericpath.py \
	Lib/getopt.py \
	Lib/getpass.py \
	Lib/gettext.py \
	Lib/glob.py \
	Lib/gzip.py \
	Lib/hashlib.py \
	Lib/heapq.py \
	Lib/hmac.py \
	Lib/imaplib.py \
	Lib/imghdr.py \
	Lib/imp.py \
	Lib/inspect.py \
	Lib/io.py \
	Lib/ipaddress.py \
	Lib/keyword.py \
	Lib/linecache.py \
	Lib/locale.py \
	Lib/lzma.py \
	Lib/mailbox.py \
	Lib/mailcap.py \
	Lib/mimetypes.py \
	Lib/modulefinder.py \
	Lib/netrc.py \
	Lib/nntplib.py \
	Lib/ntpath.py \
	Lib/nturl2path.py \
	Lib/numbers.py \
	Lib/opcode.py \
	Lib/operator.py \
	Lib/optparse.py \
	Lib/os.py \
	Lib/pathlib.py \
	Lib/pdb.py \
	Lib/pickle.py \
	Lib/pickletools.py \
	Lib/pipes.py \
	Lib/pkgutil.py \
	Lib/platform.py \
	Lib/plistlib.py \
	Lib/poplib.py \
	Lib/posixpath.py \
	Lib/pprint.py \
	Lib/profile.py \
	Lib/pstats.py \
	Lib/pty.py \
	Lib/py_compile.py \
	Lib/pyclbr.py \
	Lib/pydoc.py \
	Lib/queue.py \
	Lib/quopri.py \
	Lib/random.py \
	Lib/re.py \
	Lib/reprlib.py \
	Lib/rlcompleter.py \
	Lib/runpy.py \
	Lib/sched.py \
	Lib/secrets.py \
	Lib/selectors.py \
	Lib/shelve.py \
	Lib/shlex.py \
	Lib/shutil.py \
	Lib/signal.py \
	Lib/site.py \
	Lib/smtpd.py \
	Lib/smtplib.py \
	Lib/sndhdr.py \
	Lib/socket.py \
	Lib/socketserver.py \
	Lib/sre_compile.py \
	Lib/sre_constants.py \
	Lib/sre_parse.py \
	Lib/ssl.py \
	Lib/stat.py \
	Lib/statistics.py \
	Lib/string.py \
	Lib/stringprep.py \
	Lib/struct.py \
	Lib/subprocess.py \
	Lib/sunau.py \
	Lib/symbol.py \
	Lib/symtable.py \
	Lib/sysconfig.py \
	Lib/tabnanny.py \
	Lib/tarfile.py \
	Lib/telnetlib.py \
	Lib/tempfile.py \
	Lib/textwrap.py \
	Lib/this.py \
	Lib/threading.py \
	Lib/timeit.py \
	Lib/token.py \
	Lib/tokenize.py \
	Lib/trace.py \
	Lib/traceback.py \
	Lib/tracemalloc.py \
	Lib/tty.py \
	Lib/turtle.py \
	Lib/types.py \
	Lib/typing.py \
	Lib/uu.py \
	Lib/uuid.py \
	Lib/warnings.py \
	Lib/wave.py \
	Lib/weakref.py \
	Lib/webbrowser.py \
	Lib/xdrlib.py \
	Lib/zipapp.py \
	Lib/zipfile.py \
	Lib/zipimport.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/asyncio,\
	Lib/asyncio/__init__.py \
	Lib/asyncio/__main__.py \
	Lib/asyncio/base_events.py \
	Lib/asyncio/base_futures.py \
	Lib/asyncio/base_subprocess.py \
	Lib/asyncio/base_tasks.py \
	Lib/asyncio/constants.py \
	Lib/asyncio/coroutines.py \
	Lib/asyncio/events.py \
	Lib/asyncio/exceptions.py \
	Lib/asyncio/format_helpers.py \
	Lib/asyncio/futures.py \
	Lib/asyncio/locks.py \
	Lib/asyncio/log.py \
	Lib/asyncio/proactor_events.py \
	Lib/asyncio/protocols.py \
	Lib/asyncio/queues.py \
	Lib/asyncio/runners.py \
	Lib/asyncio/selector_events.py \
	Lib/asyncio/sslproto.py \
	Lib/asyncio/staggered.py \
	Lib/asyncio/streams.py \
	Lib/asyncio/subprocess.py \
	Lib/asyncio/tasks.py \
	Lib/asyncio/transports.py \
	Lib/asyncio/trsock.py \
	Lib/asyncio/unix_events.py \
	Lib/asyncio/windows_events.py \
	Lib/asyncio/windows_utils.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/collections,\
	Lib/collections/__init__.py \
	Lib/collections/abc.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/concurrent,\
	Lib/concurrent/__init__.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/concurrent/futures,\
	Lib/concurrent/futures/__init__.py \
	Lib/concurrent/futures/_base.py \
	Lib/concurrent/futures/process.py \
	Lib/concurrent/futures/thread.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/ctypes,\
	Lib/ctypes/__init__.py \
	Lib/ctypes/_aix.py \
	Lib/ctypes/_endian.py \
	Lib/ctypes/util.py \
	Lib/ctypes/wintypes.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/ctypes/macholib,\
	Lib/ctypes/macholib/README.ctypes \
	Lib/ctypes/macholib/fetch_macholib \
	Lib/ctypes/macholib/fetch_macholib.bat \
	Lib/ctypes/macholib/__init__.py \
	Lib/ctypes/macholib/dyld.py \
	Lib/ctypes/macholib/dylib.py \
	Lib/ctypes/macholib/framework.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/distutils,\
	Lib/distutils/README \
	Lib/distutils/__init__.py \
	Lib/distutils/_msvccompiler.py \
	Lib/distutils/archive_util.py \
	Lib/distutils/bcppcompiler.py \
	Lib/distutils/ccompiler.py \
	Lib/distutils/cmd.py \
	Lib/distutils/config.py \
	Lib/distutils/core.py \
	Lib/distutils/cygwinccompiler.py \
	Lib/distutils/debug.py \
	Lib/distutils/dep_util.py \
	Lib/distutils/dir_util.py \
	Lib/distutils/dist.py \
	Lib/distutils/errors.py \
	Lib/distutils/extension.py \
	Lib/distutils/fancy_getopt.py \
	Lib/distutils/file_util.py \
	Lib/distutils/filelist.py \
	Lib/distutils/log.py \
	Lib/distutils/msvc9compiler.py \
	Lib/distutils/msvccompiler.py \
	Lib/distutils/spawn.py \
	Lib/distutils/sysconfig.py \
	Lib/distutils/text_file.py \
	Lib/distutils/unixccompiler.py \
	Lib/distutils/util.py \
	Lib/distutils/version.py \
	Lib/distutils/versionpredicate.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/distutils/command,\
	Lib/distutils/command/__init__.py \
	Lib/distutils/command/bdist.py \
	Lib/distutils/command/bdist_dumb.py \
	Lib/distutils/command/bdist_msi.py \
	Lib/distutils/command/bdist_rpm.py \
	Lib/distutils/command/bdist_wininst.py \
	Lib/distutils/command/build.py \
	Lib/distutils/command/build_clib.py \
	Lib/distutils/command/build_ext.py \
	Lib/distutils/command/build_py.py \
	Lib/distutils/command/build_scripts.py \
	Lib/distutils/command/check.py \
	Lib/distutils/command/clean.py \
	Lib/distutils/command/command_template \
	Lib/distutils/command/config.py \
	Lib/distutils/command/install.py \
	Lib/distutils/command/install_data.py \
	Lib/distutils/command/install_egg_info.py \
	Lib/distutils/command/install_headers.py \
	Lib/distutils/command/install_lib.py \
	Lib/distutils/command/install_scripts.py \
	Lib/distutils/command/register.py \
	Lib/distutils/command/sdist.py \
	Lib/distutils/command/upload.py \
	Lib/distutils/command/wininst-10.0.exe \
	Lib/distutils/command/wininst-10.0-amd64.exe \
	Lib/distutils/command/wininst-14.0.exe \
	Lib/distutils/command/wininst-14.0-amd64.exe \
	Lib/distutils/command/wininst-6.0.exe \
	Lib/distutils/command/wininst-7.1.exe \
	Lib/distutils/command/wininst-8.0.exe \
	Lib/distutils/command/wininst-9.0.exe \
	Lib/distutils/command/wininst-9.0-amd64.exe \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/email,\
	Lib/email/__init__.py \
	Lib/email/_encoded_words.py \
	Lib/email/_header_value_parser.py \
	Lib/email/_parseaddr.py \
	Lib/email/_policybase.py \
	Lib/email/architecture.rst \
	Lib/email/base64mime.py \
	Lib/email/charset.py \
	Lib/email/contentmanager.py \
	Lib/email/encoders.py \
	Lib/email/errors.py \
	Lib/email/feedparser.py \
	Lib/email/generator.py \
	Lib/email/header.py \
	Lib/email/headerregistry.py \
	Lib/email/iterators.py \
	Lib/email/message.py \
	Lib/email/parser.py \
	Lib/email/policy.py \
	Lib/email/quoprimime.py \
	Lib/email/utils.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/email/mime,\
	Lib/email/mime/__init__.py \
	Lib/email/mime/application.py \
	Lib/email/mime/audio.py \
	Lib/email/mime/base.py \
	Lib/email/mime/image.py \
	Lib/email/mime/message.py \
	Lib/email/mime/multipart.py \
	Lib/email/mime/nonmultipart.py \
	Lib/email/mime/text.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/encodings,\
	Lib/encodings/__init__.py \
	Lib/encodings/aliases.py \
	Lib/encodings/ascii.py \
	Lib/encodings/base64_codec.py \
	Lib/encodings/big5.py \
	Lib/encodings/big5hkscs.py \
	Lib/encodings/bz2_codec.py \
	Lib/encodings/charmap.py \
	Lib/encodings/cp037.py \
	Lib/encodings/cp1006.py \
	Lib/encodings/cp1026.py \
	Lib/encodings/cp1125.py \
	Lib/encodings/cp1140.py \
	Lib/encodings/cp1250.py \
	Lib/encodings/cp1251.py \
	Lib/encodings/cp1252.py \
	Lib/encodings/cp1253.py \
	Lib/encodings/cp1254.py \
	Lib/encodings/cp1255.py \
	Lib/encodings/cp1256.py \
	Lib/encodings/cp1257.py \
	Lib/encodings/cp1258.py \
	Lib/encodings/cp273.py \
	Lib/encodings/cp424.py \
	Lib/encodings/cp437.py \
	Lib/encodings/cp500.py \
	Lib/encodings/cp720.py \
	Lib/encodings/cp737.py \
	Lib/encodings/cp775.py \
	Lib/encodings/cp850.py \
	Lib/encodings/cp852.py \
	Lib/encodings/cp855.py \
	Lib/encodings/cp856.py \
	Lib/encodings/cp857.py \
	Lib/encodings/cp858.py \
	Lib/encodings/cp860.py \
	Lib/encodings/cp861.py \
	Lib/encodings/cp862.py \
	Lib/encodings/cp863.py \
	Lib/encodings/cp864.py \
	Lib/encodings/cp865.py \
	Lib/encodings/cp866.py \
	Lib/encodings/cp869.py \
	Lib/encodings/cp874.py \
	Lib/encodings/cp875.py \
	Lib/encodings/cp932.py \
	Lib/encodings/cp949.py \
	Lib/encodings/cp950.py \
	Lib/encodings/euc_jis_2004.py \
	Lib/encodings/euc_jisx0213.py \
	Lib/encodings/euc_jp.py \
	Lib/encodings/euc_kr.py \
	Lib/encodings/gb18030.py \
	Lib/encodings/gb2312.py \
	Lib/encodings/gbk.py \
	Lib/encodings/hex_codec.py \
	Lib/encodings/hp_roman8.py \
	Lib/encodings/hz.py \
	Lib/encodings/idna.py \
	Lib/encodings/iso2022_jp.py \
	Lib/encodings/iso2022_jp_1.py \
	Lib/encodings/iso2022_jp_2.py \
	Lib/encodings/iso2022_jp_2004.py \
	Lib/encodings/iso2022_jp_3.py \
	Lib/encodings/iso2022_jp_ext.py \
	Lib/encodings/iso2022_kr.py \
	Lib/encodings/iso8859_1.py \
	Lib/encodings/iso8859_10.py \
	Lib/encodings/iso8859_11.py \
	Lib/encodings/iso8859_13.py \
	Lib/encodings/iso8859_14.py \
	Lib/encodings/iso8859_15.py \
	Lib/encodings/iso8859_16.py \
	Lib/encodings/iso8859_2.py \
	Lib/encodings/iso8859_3.py \
	Lib/encodings/iso8859_4.py \
	Lib/encodings/iso8859_5.py \
	Lib/encodings/iso8859_6.py \
	Lib/encodings/iso8859_7.py \
	Lib/encodings/iso8859_8.py \
	Lib/encodings/iso8859_9.py \
	Lib/encodings/johab.py \
	Lib/encodings/koi8_r.py \
	Lib/encodings/koi8_t.py \
	Lib/encodings/koi8_u.py \
	Lib/encodings/kz1048.py \
	Lib/encodings/latin_1.py \
	Lib/encodings/mac_arabic.py \
	Lib/encodings/mac_centeuro.py \
	Lib/encodings/mac_croatian.py \
	Lib/encodings/mac_cyrillic.py \
	Lib/encodings/mac_farsi.py \
	Lib/encodings/mac_greek.py \
	Lib/encodings/mac_iceland.py \
	Lib/encodings/mac_latin2.py \
	Lib/encodings/mac_roman.py \
	Lib/encodings/mac_romanian.py \
	Lib/encodings/mac_turkish.py \
	Lib/encodings/mbcs.py \
	Lib/encodings/oem.py \
	Lib/encodings/palmos.py \
	Lib/encodings/ptcp154.py \
	Lib/encodings/punycode.py \
	Lib/encodings/quopri_codec.py \
	Lib/encodings/raw_unicode_escape.py \
	Lib/encodings/rot_13.py \
	Lib/encodings/shift_jis.py \
	Lib/encodings/shift_jis_2004.py \
	Lib/encodings/shift_jisx0213.py \
	Lib/encodings/tis_620.py \
	Lib/encodings/undefined.py \
	Lib/encodings/unicode_escape.py \
	Lib/encodings/utf_16.py \
	Lib/encodings/utf_16_be.py \
	Lib/encodings/utf_16_le.py \
	Lib/encodings/utf_32.py \
	Lib/encodings/utf_32_be.py \
	Lib/encodings/utf_32_le.py \
	Lib/encodings/utf_7.py \
	Lib/encodings/utf_8.py \
	Lib/encodings/utf_8_sig.py \
	Lib/encodings/uu_codec.py \
	Lib/encodings/zlib_codec.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/html,\
	Lib/html/__init__.py \
	Lib/html/entities.py \
	Lib/html/parser.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/http,\
	Lib/http/__init__.py \
	Lib/http/client.py \
	Lib/http/cookiejar.py \
	Lib/http/cookies.py \
	Lib/http/server.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/importlib,\
	Lib/importlib/__init__.py \
	Lib/importlib/_bootstrap.py \
	Lib/importlib/_bootstrap_external.py \
	Lib/importlib/abc.py \
	Lib/importlib/machinery.py \
	Lib/importlib/metadata.py \
	Lib/importlib/resources.py \
	Lib/importlib/util.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/json,\
	Lib/json/__init__.py \
	Lib/json/decoder.py \
	Lib/json/encoder.py \
	Lib/json/scanner.py \
	Lib/json/tool.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/lib2to3,\
	Lib/lib2to3/__init__.py \
	Lib/lib2to3/__main__.py \
	Lib/lib2to3/Grammar.txt \
	Lib/lib2to3/PatternGrammar.txt \
	Lib/lib2to3/btm_matcher.py \
	Lib/lib2to3/btm_utils.py \
	Lib/lib2to3/fixer_base.py \
	Lib/lib2to3/fixer_util.py \
	Lib/lib2to3/main.py \
	Lib/lib2to3/patcomp.py \
	Lib/lib2to3/pygram.py \
	Lib/lib2to3/pytree.py \
	Lib/lib2to3/refactor.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/lib2to3/fixes,\
	Lib/lib2to3/fixes/__init__.py \
	Lib/lib2to3/fixes/fix_apply.py \
	Lib/lib2to3/fixes/fix_asserts.py \
	Lib/lib2to3/fixes/fix_basestring.py \
	Lib/lib2to3/fixes/fix_buffer.py \
	Lib/lib2to3/fixes/fix_dict.py \
	Lib/lib2to3/fixes/fix_except.py \
	Lib/lib2to3/fixes/fix_exec.py \
	Lib/lib2to3/fixes/fix_execfile.py \
	Lib/lib2to3/fixes/fix_exitfunc.py \
	Lib/lib2to3/fixes/fix_filter.py \
	Lib/lib2to3/fixes/fix_funcattrs.py \
	Lib/lib2to3/fixes/fix_future.py \
	Lib/lib2to3/fixes/fix_getcwdu.py \
	Lib/lib2to3/fixes/fix_has_key.py \
	Lib/lib2to3/fixes/fix_idioms.py \
	Lib/lib2to3/fixes/fix_import.py \
	Lib/lib2to3/fixes/fix_imports.py \
	Lib/lib2to3/fixes/fix_imports2.py \
	Lib/lib2to3/fixes/fix_input.py \
	Lib/lib2to3/fixes/fix_intern.py \
	Lib/lib2to3/fixes/fix_isinstance.py \
	Lib/lib2to3/fixes/fix_itertools.py \
	Lib/lib2to3/fixes/fix_itertools_imports.py \
	Lib/lib2to3/fixes/fix_long.py \
	Lib/lib2to3/fixes/fix_map.py \
	Lib/lib2to3/fixes/fix_metaclass.py \
	Lib/lib2to3/fixes/fix_methodattrs.py \
	Lib/lib2to3/fixes/fix_ne.py \
	Lib/lib2to3/fixes/fix_next.py \
	Lib/lib2to3/fixes/fix_nonzero.py \
	Lib/lib2to3/fixes/fix_numliterals.py \
	Lib/lib2to3/fixes/fix_operator.py \
	Lib/lib2to3/fixes/fix_paren.py \
	Lib/lib2to3/fixes/fix_print.py \
	Lib/lib2to3/fixes/fix_raise.py \
	Lib/lib2to3/fixes/fix_raw_input.py \
	Lib/lib2to3/fixes/fix_reduce.py \
	Lib/lib2to3/fixes/fix_reload.py \
	Lib/lib2to3/fixes/fix_renames.py \
	Lib/lib2to3/fixes/fix_repr.py \
	Lib/lib2to3/fixes/fix_set_literal.py \
	Lib/lib2to3/fixes/fix_standarderror.py \
	Lib/lib2to3/fixes/fix_sys_exc.py \
	Lib/lib2to3/fixes/fix_throw.py \
	Lib/lib2to3/fixes/fix_tuple_params.py \
	Lib/lib2to3/fixes/fix_types.py \
	Lib/lib2to3/fixes/fix_unicode.py \
	Lib/lib2to3/fixes/fix_urllib.py \
	Lib/lib2to3/fixes/fix_ws_comma.py \
	Lib/lib2to3/fixes/fix_xrange.py \
	Lib/lib2to3/fixes/fix_xreadlines.py \
	Lib/lib2to3/fixes/fix_zip.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pgen2,\
	Lib/lib2to3/pgen2/__init__.py \
	Lib/lib2to3/pgen2/conv.py \
	Lib/lib2to3/pgen2/driver.py \
	Lib/lib2to3/pgen2/grammar.py \
	Lib/lib2to3/pgen2/literals.py \
	Lib/lib2to3/pgen2/parse.py \
	Lib/lib2to3/pgen2/pgen.py \
	Lib/lib2to3/pgen2/token.py \
	Lib/lib2to3/pgen2/tokenize.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/logging,\
	Lib/logging/__init__.py \
	Lib/logging/config.py \
	Lib/logging/handlers.py \
))

ifeq (WNT,$(OS))
$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/msilib,\
	Lib/msilib/__init__.py \
	Lib/msilib/schema.py \
	Lib/msilib/sequence.py \
	Lib/msilib/text.py \
))
endif

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/multiprocessing,\
	Lib/multiprocessing/__init__.py \
	Lib/multiprocessing/connection.py \
	Lib/multiprocessing/context.py \
	Lib/multiprocessing/dummy/__init__.py \
	Lib/multiprocessing/dummy/connection.py \
	Lib/multiprocessing/forkserver.py \
	Lib/multiprocessing/heap.py \
	Lib/multiprocessing/managers.py \
	Lib/multiprocessing/pool.py \
	Lib/multiprocessing/popen_fork.py \
	Lib/multiprocessing/popen_forkserver.py \
	Lib/multiprocessing/popen_spawn_posix.py \
	Lib/multiprocessing/popen_spawn_win32.py \
	Lib/multiprocessing/process.py \
	Lib/multiprocessing/queues.py \
	Lib/multiprocessing/reduction.py \
	Lib/multiprocessing/resource_sharer.py \
	Lib/multiprocessing/resource_tracker.py \
	Lib/multiprocessing/shared_memory.py \
	Lib/multiprocessing/sharedctypes.py \
	Lib/multiprocessing/spawn.py \
	Lib/multiprocessing/synchronize.py \
	Lib/multiprocessing/util.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/multiprocessing/dummy,\
	Lib/multiprocessing/dummy/__init__.py \
	Lib/multiprocessing/dummy/connection.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pydoc_data,\
	Lib/pydoc_data/__init__.py \
	Lib/pydoc_data/_pydoc.css \
	Lib/pydoc_data/topics.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/unittest,\
	Lib/unittest/__init__.py \
	Lib/unittest/__main__.py \
	Lib/unittest/async_case.py \
	Lib/unittest/case.py \
	Lib/unittest/loader.py \
	Lib/unittest/main.py \
	Lib/unittest/mock.py \
	Lib/unittest/result.py \
	Lib/unittest/runner.py \
	Lib/unittest/signals.py \
	Lib/unittest/suite.py \
	Lib/unittest/util.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/urllib,\
	Lib/urllib/__init__.py \
	Lib/urllib/error.py \
	Lib/urllib/parse.py \
	Lib/urllib/request.py \
	Lib/urllib/response.py \
	Lib/urllib/robotparser.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/wsgiref,\
	Lib/wsgiref/__init__.py \
	Lib/wsgiref/handlers.py \
	Lib/wsgiref/headers.py \
	Lib/wsgiref/simple_server.py \
	Lib/wsgiref/util.py \
	Lib/wsgiref/validate.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/xml,\
	Lib/xml/__init__.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/xml/dom,\
	Lib/xml/dom/__init__.py \
	Lib/xml/dom/domreg.py \
	Lib/xml/dom/expatbuilder.py \
	Lib/xml/dom/minicompat.py \
	Lib/xml/dom/minidom.py \
	Lib/xml/dom/NodeFilter.py \
	Lib/xml/dom/pulldom.py \
	Lib/xml/dom/xmlbuilder.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/xml/etree,\
	Lib/xml/etree/__init__.py \
	Lib/xml/etree/cElementTree.py \
	Lib/xml/etree/ElementInclude.py \
	Lib/xml/etree/ElementPath.py \
	Lib/xml/etree/ElementTree.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/xml/parsers,\
	Lib/xml/parsers/__init__.py \
	Lib/xml/parsers/expat.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/xml/sax,\
	Lib/xml/sax/__init__.py \
	Lib/xml/sax/_exceptions.py \
	Lib/xml/sax/expatreader.py \
	Lib/xml/sax/handler.py \
	Lib/xml/sax/saxutils.py \
	Lib/xml/sax/xmlreader.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/xmlrpc,\
	Lib/xmlrpc/__init__.py \
	Lib/xmlrpc/client.py \
	Lib/xmlrpc/server.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/site-packages,\
	Lib/site-packages/README.txt \
))

# vim: set noet sw=4 ts=4:
