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
python3_EXTENSION_MODULES= \
	PCbuild/$(python_arch_subdir)_asyncio$(if $(MSVC_USE_DEBUG_RUNTIME),_d).pyd \
	PCbuild/$(python_arch_subdir)_bz2$(if $(MSVC_USE_DEBUG_RUNTIME),_d).pyd \
	PCbuild/$(python_arch_subdir)_ctypes$(if $(MSVC_USE_DEBUG_RUNTIME),_d).pyd \
	PCbuild/$(python_arch_subdir)_decimal$(if $(MSVC_USE_DEBUG_RUNTIME),_d).pyd \
	PCbuild/$(python_arch_subdir)_elementtree$(if $(MSVC_USE_DEBUG_RUNTIME),_d).pyd \
	PCbuild/$(python_arch_subdir)_msi$(if $(MSVC_USE_DEBUG_RUNTIME),_d).pyd \
	PCbuild/$(python_arch_subdir)_multiprocessing$(if $(MSVC_USE_DEBUG_RUNTIME),_d).pyd \
	PCbuild/$(python_arch_subdir)_overlapped$(if $(MSVC_USE_DEBUG_RUNTIME),_d).pyd \
	PCbuild/$(python_arch_subdir)_queue$(if $(MSVC_USE_DEBUG_RUNTIME),_d).pyd \
	PCbuild/$(python_arch_subdir)_socket$(if $(MSVC_USE_DEBUG_RUNTIME),_d).pyd \
	PCbuild/$(python_arch_subdir)_ssl$(if $(MSVC_USE_DEBUG_RUNTIME),_d).pyd \
	PCbuild/$(python_arch_subdir)_uuid$(if $(MSVC_USE_DEBUG_RUNTIME),_d).pyd \
	PCbuild/$(python_arch_subdir)_zoneinfo$(if $(MSVC_USE_DEBUG_RUNTIME),_d).pyd \
	PCbuild/$(python_arch_subdir)pyexpat$(if $(MSVC_USE_DEBUG_RUNTIME),_d).pyd \
	PCbuild/$(python_arch_subdir)select$(if $(MSVC_USE_DEBUG_RUNTIME),_d).pyd \
	PCbuild/$(python_arch_subdir)unicodedata$(if $(MSVC_USE_DEBUG_RUNTIME),_d).pyd \
	PCbuild/$(python_arch_subdir)winsound$(if $(MSVC_USE_DEBUG_RUNTIME),_d).pyd \

$(eval $(call gb_ExternalPackage_add_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib,\
	$(python3_EXTENSION_MODULES) \
))
else
$(eval $(call gb_ExternalPackage_add_file,python3,$(LIBO_BIN_FOLDER)/python.bin,python))
$(eval $(call gb_ExternalPackage_add_file,python3,$(LIBO_BIN_FOLDER)/libpython$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)$(if $(ENABLE_DBGUTIL),d).so,libpython$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)$(if $(ENABLE_DBGUTIL),d).so))
$(eval $(call gb_ExternalPackage_add_file,python3,$(LIBO_BIN_FOLDER)/libpython$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)$(if $(ENABLE_DBGUTIL),d).so.1.0,libpython$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)$(if $(ENABLE_DBGUTIL),d).so))
$(eval $(call gb_ExternalPackage_add_file,python3,$(LIBO_BIN_FOLDER)/libpython$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)$(if $(ENABLE_DBGUTIL),d).so.1.0-gdb.py,Tools/gdb/libpython.py))

# Unfortunately the python build system does not allow to explicitly enable or
# disable these, it just tries to build them and then prints which did not
# build successfully without stopping; that's why ExternalProject_python3 explicitly checks for the
# existence of all the files on the python3_EXTENSION_MODULES list at the end of the build.
# Obviously this list should not contain stuff with external dependencies
# that may not be available on baseline systems.

python3_EXTENSION_MODULE_SUFFIX=cpython-$(PYTHON_VERSION_MAJOR)$(PYTHON_VERSION_MINOR)$(if $(ENABLE_DBGUTIL),d)-x86_64-linux-gnu
python3_EXTENSION_MODULES= \
	LO_lib/array.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/_asyncio.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/audioop.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/binascii.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/_bisect.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/_blake2.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/_bz2.$(python3_EXTENSION_MODULE_SUFFIX).so \
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
	$(if $(ENABLE_OPENSSL), \
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
	$(if $(ENABLE_OPENSSL), \
		LO_lib/_ssl.$(python3_EXTENSION_MODULE_SUFFIX).so \
	) \
	LO_lib/_statistics.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/_struct.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/syslog.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/termios.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/unicodedata.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/_uuid.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/xxlimited_35.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/xxlimited.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/_xxsubinterpreters.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/_xxtestfuzz.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/zlib.$(python3_EXTENSION_MODULE_SUFFIX).so \
	LO_lib/_zoneinfo.$(python3_EXTENSION_MODULE_SUFFIX).so \


$(eval $(call gb_ExternalPackage_add_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/lib-dynload,\
	$(python3_EXTENSION_MODULES) \
))
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
ifeq ($(HOST_PLATFORM),powerpc64le-unknown-linux-gnu)
$(eval $(call gb_ExternalPackage_add_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib,\
        LO_lib/_sysconfigdata_$(if $(ENABLE_DBGUTIL),d)_linux_powerpc64le-linux-gnu.py \
))
else
ifeq ($(HOST_PLATFORM),aarch64-unknown-linux-gnu)
$(eval $(call gb_ExternalPackage_add_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib,\
        LO_lib/_sysconfigdata_$(if $(ENABLE_DBGUTIL),d)_linux_aarch64-linux-gnu.py \
))
else
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
endif
endif


# packages not shipped:
# dbm, sqlite3 - need some database stuff
# curses - need curses to build the C module
# idlelib, tkinter, turtledemo - need Tk to build the C module
# test - probably unnecessary? was explicitly removed #i116738#
# venv - why would we need virtual environments
#
# Call generateExternalPackage.py to update the lists below
#

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib,\
	Lib/__future__.py \
	Lib/__hello__.py \
	Lib/_aix_support.py \
	Lib/_bootsubprocess.py \
	Lib/_collections_abc.py \
	Lib/_compat_pickle.py \
	Lib/_compression.py \
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
	Lib/enum.py \
	Lib/filecmp.py \
	Lib/fileinput.py \
	Lib/fnmatch.py \
	Lib/fractions.py \
	Lib/ftplib.py \
	Lib/functools.py \
	Lib/genericpath.py \
	Lib/getopt.py \
	Lib/getpass.py \
	Lib/gettext.py \
	Lib/glob.py \
	Lib/graphlib.py \
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

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/__phello__,\
	Lib/__phello__/__init__.py \
	Lib/__phello__/spam.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/__phello__/ham,\
	Lib/__phello__/ham/__init__.py \
	Lib/__phello__/ham/eggs.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/_distutils_hack,\
	Lib/_distutils_hack/__init__.py \
	Lib/_distutils_hack/override.py \
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
	Lib/asyncio/mixins.py \
	Lib/asyncio/proactor_events.py \
	Lib/asyncio/protocols.py \
	Lib/asyncio/queues.py \
	Lib/asyncio/runners.py \
	Lib/asyncio/selector_events.py \
	Lib/asyncio/sslproto.py \
	Lib/asyncio/staggered.py \
	Lib/asyncio/streams.py \
	Lib/asyncio/subprocess.py \
	Lib/asyncio/taskgroups.py \
	Lib/asyncio/tasks.py \
	Lib/asyncio/threads.py \
	Lib/asyncio/timeouts.py \
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
	Lib/ctypes/macholib/__init__.py \
	Lib/ctypes/macholib/dyld.py \
	Lib/ctypes/macholib/dylib.py \
	Lib/ctypes/macholib/framework.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/distutils,\
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
	Lib/distutils/command/bdist_rpm.py \
	Lib/distutils/command/build.py \
	Lib/distutils/command/build_clib.py \
	Lib/distutils/command/build_ext.py \
	Lib/distutils/command/build_py.py \
	Lib/distutils/command/build_scripts.py \
	Lib/distutils/command/check.py \
	Lib/distutils/command/clean.py \
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
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/email,\
	Lib/email/__init__.py \
	Lib/email/_encoded_words.py \
	Lib/email/_header_value_parser.py \
	Lib/email/_parseaddr.py \
	Lib/email/_policybase.py \
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

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/ensurepip,\
	Lib/ensurepip/__init__.py \
	Lib/ensurepip/__main__.py \
	Lib/ensurepip/_uninstall.py \
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
	Lib/importlib/_abc.py \
	Lib/importlib/_bootstrap.py \
	Lib/importlib/_bootstrap_external.py \
	Lib/importlib/abc.py \
	Lib/importlib/machinery.py \
	Lib/importlib/readers.py \
	Lib/importlib/simple.py \
	Lib/importlib/util.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/importlib/metadata,\
	Lib/importlib/metadata/__init__.py \
	Lib/importlib/metadata/_adapters.py \
	Lib/importlib/metadata/_collections.py \
	Lib/importlib/metadata/_functools.py \
	Lib/importlib/metadata/_itertools.py \
	Lib/importlib/metadata/_meta.py \
	Lib/importlib/metadata/_text.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/importlib/resources,\
	Lib/importlib/resources/__init__.py \
	Lib/importlib/resources/_adapters.py \
	Lib/importlib/resources/_common.py \
	Lib/importlib/resources/_itertools.py \
	Lib/importlib/resources/_legacy.py \
	Lib/importlib/resources/abc.py \
	Lib/importlib/resources/readers.py \
	Lib/importlib/resources/simple.py \
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

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/lib2to3/pgen2,\
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

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip,\
	Lib/pip/__init__.py \
	Lib/pip/__main__.py \
	Lib/pip/__pip-runner__.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_internal,\
	Lib/pip/_internal/__init__.py \
	Lib/pip/_internal/build_env.py \
	Lib/pip/_internal/cache.py \
	Lib/pip/_internal/configuration.py \
	Lib/pip/_internal/exceptions.py \
	Lib/pip/_internal/main.py \
	Lib/pip/_internal/pyproject.py \
	Lib/pip/_internal/self_outdated_check.py \
	Lib/pip/_internal/wheel_builder.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_internal/cli,\
	Lib/pip/_internal/cli/__init__.py \
	Lib/pip/_internal/cli/autocompletion.py \
	Lib/pip/_internal/cli/base_command.py \
	Lib/pip/_internal/cli/cmdoptions.py \
	Lib/pip/_internal/cli/command_context.py \
	Lib/pip/_internal/cli/main.py \
	Lib/pip/_internal/cli/main_parser.py \
	Lib/pip/_internal/cli/parser.py \
	Lib/pip/_internal/cli/progress_bars.py \
	Lib/pip/_internal/cli/req_command.py \
	Lib/pip/_internal/cli/spinners.py \
	Lib/pip/_internal/cli/status_codes.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_internal/commands,\
	Lib/pip/_internal/commands/__init__.py \
	Lib/pip/_internal/commands/cache.py \
	Lib/pip/_internal/commands/check.py \
	Lib/pip/_internal/commands/completion.py \
	Lib/pip/_internal/commands/configuration.py \
	Lib/pip/_internal/commands/debug.py \
	Lib/pip/_internal/commands/download.py \
	Lib/pip/_internal/commands/freeze.py \
	Lib/pip/_internal/commands/hash.py \
	Lib/pip/_internal/commands/help.py \
	Lib/pip/_internal/commands/index.py \
	Lib/pip/_internal/commands/inspect.py \
	Lib/pip/_internal/commands/install.py \
	Lib/pip/_internal/commands/list.py \
	Lib/pip/_internal/commands/search.py \
	Lib/pip/_internal/commands/show.py \
	Lib/pip/_internal/commands/uninstall.py \
	Lib/pip/_internal/commands/wheel.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_internal/distributions,\
	Lib/pip/_internal/distributions/__init__.py \
	Lib/pip/_internal/distributions/base.py \
	Lib/pip/_internal/distributions/installed.py \
	Lib/pip/_internal/distributions/sdist.py \
	Lib/pip/_internal/distributions/wheel.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_internal/index,\
	Lib/pip/_internal/index/__init__.py \
	Lib/pip/_internal/index/collector.py \
	Lib/pip/_internal/index/package_finder.py \
	Lib/pip/_internal/index/sources.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_internal/locations,\
	Lib/pip/_internal/locations/__init__.py \
	Lib/pip/_internal/locations/_distutils.py \
	Lib/pip/_internal/locations/_sysconfig.py \
	Lib/pip/_internal/locations/base.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_internal/metadata,\
	Lib/pip/_internal/metadata/__init__.py \
	Lib/pip/_internal/metadata/_json.py \
	Lib/pip/_internal/metadata/base.py \
	Lib/pip/_internal/metadata/pkg_resources.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_internal/metadata/importlib,\
	Lib/pip/_internal/metadata/importlib/__init__.py \
	Lib/pip/_internal/metadata/importlib/_compat.py \
	Lib/pip/_internal/metadata/importlib/_dists.py \
	Lib/pip/_internal/metadata/importlib/_envs.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_internal/models,\
	Lib/pip/_internal/models/__init__.py \
	Lib/pip/_internal/models/candidate.py \
	Lib/pip/_internal/models/direct_url.py \
	Lib/pip/_internal/models/format_control.py \
	Lib/pip/_internal/models/index.py \
	Lib/pip/_internal/models/installation_report.py \
	Lib/pip/_internal/models/link.py \
	Lib/pip/_internal/models/scheme.py \
	Lib/pip/_internal/models/search_scope.py \
	Lib/pip/_internal/models/selection_prefs.py \
	Lib/pip/_internal/models/target_python.py \
	Lib/pip/_internal/models/wheel.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_internal/network,\
	Lib/pip/_internal/network/__init__.py \
	Lib/pip/_internal/network/auth.py \
	Lib/pip/_internal/network/cache.py \
	Lib/pip/_internal/network/download.py \
	Lib/pip/_internal/network/lazy_wheel.py \
	Lib/pip/_internal/network/session.py \
	Lib/pip/_internal/network/utils.py \
	Lib/pip/_internal/network/xmlrpc.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_internal/operations,\
	Lib/pip/_internal/operations/__init__.py \
	Lib/pip/_internal/operations/check.py \
	Lib/pip/_internal/operations/freeze.py \
	Lib/pip/_internal/operations/prepare.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_internal/operations/build,\
	Lib/pip/_internal/operations/build/__init__.py \
	Lib/pip/_internal/operations/build/build_tracker.py \
	Lib/pip/_internal/operations/build/metadata.py \
	Lib/pip/_internal/operations/build/metadata_editable.py \
	Lib/pip/_internal/operations/build/metadata_legacy.py \
	Lib/pip/_internal/operations/build/wheel.py \
	Lib/pip/_internal/operations/build/wheel_editable.py \
	Lib/pip/_internal/operations/build/wheel_legacy.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_internal/operations/install,\
	Lib/pip/_internal/operations/install/__init__.py \
	Lib/pip/_internal/operations/install/editable_legacy.py \
	Lib/pip/_internal/operations/install/wheel.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_internal/req,\
	Lib/pip/_internal/req/__init__.py \
	Lib/pip/_internal/req/constructors.py \
	Lib/pip/_internal/req/req_file.py \
	Lib/pip/_internal/req/req_install.py \
	Lib/pip/_internal/req/req_set.py \
	Lib/pip/_internal/req/req_uninstall.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_internal/resolution,\
	Lib/pip/_internal/resolution/__init__.py \
	Lib/pip/_internal/resolution/base.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_internal/resolution/legacy,\
	Lib/pip/_internal/resolution/legacy/__init__.py \
	Lib/pip/_internal/resolution/legacy/resolver.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_internal/resolution/resolvelib,\
	Lib/pip/_internal/resolution/resolvelib/__init__.py \
	Lib/pip/_internal/resolution/resolvelib/base.py \
	Lib/pip/_internal/resolution/resolvelib/candidates.py \
	Lib/pip/_internal/resolution/resolvelib/factory.py \
	Lib/pip/_internal/resolution/resolvelib/found_candidates.py \
	Lib/pip/_internal/resolution/resolvelib/provider.py \
	Lib/pip/_internal/resolution/resolvelib/reporter.py \
	Lib/pip/_internal/resolution/resolvelib/requirements.py \
	Lib/pip/_internal/resolution/resolvelib/resolver.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_internal/utils,\
	Lib/pip/_internal/utils/__init__.py \
	Lib/pip/_internal/utils/_jaraco_text.py \
	Lib/pip/_internal/utils/_log.py \
	Lib/pip/_internal/utils/appdirs.py \
	Lib/pip/_internal/utils/compat.py \
	Lib/pip/_internal/utils/compatibility_tags.py \
	Lib/pip/_internal/utils/datetime.py \
	Lib/pip/_internal/utils/deprecation.py \
	Lib/pip/_internal/utils/direct_url_helpers.py \
	Lib/pip/_internal/utils/egg_link.py \
	Lib/pip/_internal/utils/encoding.py \
	Lib/pip/_internal/utils/entrypoints.py \
	Lib/pip/_internal/utils/filesystem.py \
	Lib/pip/_internal/utils/filetypes.py \
	Lib/pip/_internal/utils/glibc.py \
	Lib/pip/_internal/utils/hashes.py \
	Lib/pip/_internal/utils/logging.py \
	Lib/pip/_internal/utils/misc.py \
	Lib/pip/_internal/utils/models.py \
	Lib/pip/_internal/utils/packaging.py \
	Lib/pip/_internal/utils/setuptools_build.py \
	Lib/pip/_internal/utils/subprocess.py \
	Lib/pip/_internal/utils/temp_dir.py \
	Lib/pip/_internal/utils/unpacking.py \
	Lib/pip/_internal/utils/urls.py \
	Lib/pip/_internal/utils/virtualenv.py \
	Lib/pip/_internal/utils/wheel.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_internal/vcs,\
	Lib/pip/_internal/vcs/__init__.py \
	Lib/pip/_internal/vcs/bazaar.py \
	Lib/pip/_internal/vcs/git.py \
	Lib/pip/_internal/vcs/mercurial.py \
	Lib/pip/_internal/vcs/subversion.py \
	Lib/pip/_internal/vcs/versioncontrol.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_vendor,\
	Lib/pip/_vendor/__init__.py \
	Lib/pip/_vendor/six.py \
	Lib/pip/_vendor/typing_extensions.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_vendor/cachecontrol,\
	Lib/pip/_vendor/cachecontrol/__init__.py \
	Lib/pip/_vendor/cachecontrol/_cmd.py \
	Lib/pip/_vendor/cachecontrol/adapter.py \
	Lib/pip/_vendor/cachecontrol/cache.py \
	Lib/pip/_vendor/cachecontrol/controller.py \
	Lib/pip/_vendor/cachecontrol/filewrapper.py \
	Lib/pip/_vendor/cachecontrol/heuristics.py \
	Lib/pip/_vendor/cachecontrol/serialize.py \
	Lib/pip/_vendor/cachecontrol/wrapper.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_vendor/cachecontrol/caches,\
	Lib/pip/_vendor/cachecontrol/caches/__init__.py \
	Lib/pip/_vendor/cachecontrol/caches/file_cache.py \
	Lib/pip/_vendor/cachecontrol/caches/redis_cache.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_vendor/certifi,\
	Lib/pip/_vendor/certifi/__init__.py \
	Lib/pip/_vendor/certifi/__main__.py \
	Lib/pip/_vendor/certifi/cacert.pem \
	Lib/pip/_vendor/certifi/core.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_vendor/chardet,\
	Lib/pip/_vendor/chardet/__init__.py \
	Lib/pip/_vendor/chardet/big5freq.py \
	Lib/pip/_vendor/chardet/big5prober.py \
	Lib/pip/_vendor/chardet/chardistribution.py \
	Lib/pip/_vendor/chardet/charsetgroupprober.py \
	Lib/pip/_vendor/chardet/charsetprober.py \
	Lib/pip/_vendor/chardet/codingstatemachine.py \
	Lib/pip/_vendor/chardet/codingstatemachinedict.py \
	Lib/pip/_vendor/chardet/cp949prober.py \
	Lib/pip/_vendor/chardet/enums.py \
	Lib/pip/_vendor/chardet/escprober.py \
	Lib/pip/_vendor/chardet/escsm.py \
	Lib/pip/_vendor/chardet/eucjpprober.py \
	Lib/pip/_vendor/chardet/euckrfreq.py \
	Lib/pip/_vendor/chardet/euckrprober.py \
	Lib/pip/_vendor/chardet/euctwfreq.py \
	Lib/pip/_vendor/chardet/euctwprober.py \
	Lib/pip/_vendor/chardet/gb2312freq.py \
	Lib/pip/_vendor/chardet/gb2312prober.py \
	Lib/pip/_vendor/chardet/hebrewprober.py \
	Lib/pip/_vendor/chardet/jisfreq.py \
	Lib/pip/_vendor/chardet/johabfreq.py \
	Lib/pip/_vendor/chardet/johabprober.py \
	Lib/pip/_vendor/chardet/jpcntx.py \
	Lib/pip/_vendor/chardet/langbulgarianmodel.py \
	Lib/pip/_vendor/chardet/langgreekmodel.py \
	Lib/pip/_vendor/chardet/langhebrewmodel.py \
	Lib/pip/_vendor/chardet/langhungarianmodel.py \
	Lib/pip/_vendor/chardet/langrussianmodel.py \
	Lib/pip/_vendor/chardet/langthaimodel.py \
	Lib/pip/_vendor/chardet/langturkishmodel.py \
	Lib/pip/_vendor/chardet/latin1prober.py \
	Lib/pip/_vendor/chardet/macromanprober.py \
	Lib/pip/_vendor/chardet/mbcharsetprober.py \
	Lib/pip/_vendor/chardet/mbcsgroupprober.py \
	Lib/pip/_vendor/chardet/mbcssm.py \
	Lib/pip/_vendor/chardet/resultdict.py \
	Lib/pip/_vendor/chardet/sbcharsetprober.py \
	Lib/pip/_vendor/chardet/sbcsgroupprober.py \
	Lib/pip/_vendor/chardet/sjisprober.py \
	Lib/pip/_vendor/chardet/universaldetector.py \
	Lib/pip/_vendor/chardet/utf1632prober.py \
	Lib/pip/_vendor/chardet/utf8prober.py \
	Lib/pip/_vendor/chardet/version.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_vendor/chardet/cli,\
	Lib/pip/_vendor/chardet/cli/__init__.py \
	Lib/pip/_vendor/chardet/cli/chardetect.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_vendor/chardet/metadata,\
	Lib/pip/_vendor/chardet/metadata/__init__.py \
	Lib/pip/_vendor/chardet/metadata/languages.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_vendor/colorama,\
	Lib/pip/_vendor/colorama/__init__.py \
	Lib/pip/_vendor/colorama/ansi.py \
	Lib/pip/_vendor/colorama/ansitowin32.py \
	Lib/pip/_vendor/colorama/initialise.py \
	Lib/pip/_vendor/colorama/win32.py \
	Lib/pip/_vendor/colorama/winterm.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_vendor/distlib,\
	Lib/pip/_vendor/distlib/__init__.py \
	Lib/pip/_vendor/distlib/compat.py \
	Lib/pip/_vendor/distlib/database.py \
	Lib/pip/_vendor/distlib/index.py \
	Lib/pip/_vendor/distlib/locators.py \
	Lib/pip/_vendor/distlib/manifest.py \
	Lib/pip/_vendor/distlib/markers.py \
	Lib/pip/_vendor/distlib/metadata.py \
	Lib/pip/_vendor/distlib/resources.py \
	Lib/pip/_vendor/distlib/scripts.py \
	Lib/pip/_vendor/distlib/util.py \
	Lib/pip/_vendor/distlib/version.py \
	Lib/pip/_vendor/distlib/wheel.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_vendor/distro,\
	Lib/pip/_vendor/distro/__init__.py \
	Lib/pip/_vendor/distro/__main__.py \
	Lib/pip/_vendor/distro/distro.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_vendor/idna,\
	Lib/pip/_vendor/idna/__init__.py \
	Lib/pip/_vendor/idna/codec.py \
	Lib/pip/_vendor/idna/compat.py \
	Lib/pip/_vendor/idna/core.py \
	Lib/pip/_vendor/idna/idnadata.py \
	Lib/pip/_vendor/idna/intranges.py \
	Lib/pip/_vendor/idna/package_data.py \
	Lib/pip/_vendor/idna/uts46data.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_vendor/msgpack,\
	Lib/pip/_vendor/msgpack/__init__.py \
	Lib/pip/_vendor/msgpack/exceptions.py \
	Lib/pip/_vendor/msgpack/ext.py \
	Lib/pip/_vendor/msgpack/fallback.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_vendor/packaging,\
	Lib/pip/_vendor/packaging/__about__.py \
	Lib/pip/_vendor/packaging/__init__.py \
	Lib/pip/_vendor/packaging/_manylinux.py \
	Lib/pip/_vendor/packaging/_musllinux.py \
	Lib/pip/_vendor/packaging/_structures.py \
	Lib/pip/_vendor/packaging/markers.py \
	Lib/pip/_vendor/packaging/requirements.py \
	Lib/pip/_vendor/packaging/specifiers.py \
	Lib/pip/_vendor/packaging/tags.py \
	Lib/pip/_vendor/packaging/utils.py \
	Lib/pip/_vendor/packaging/version.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_vendor/pkg_resources,\
	Lib/pip/_vendor/pkg_resources/__init__.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_vendor/platformdirs,\
	Lib/pip/_vendor/platformdirs/__init__.py \
	Lib/pip/_vendor/platformdirs/__main__.py \
	Lib/pip/_vendor/platformdirs/android.py \
	Lib/pip/_vendor/platformdirs/api.py \
	Lib/pip/_vendor/platformdirs/macos.py \
	Lib/pip/_vendor/platformdirs/unix.py \
	Lib/pip/_vendor/platformdirs/version.py \
	Lib/pip/_vendor/platformdirs/windows.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_vendor/pygments,\
	Lib/pip/_vendor/pygments/__init__.py \
	Lib/pip/_vendor/pygments/__main__.py \
	Lib/pip/_vendor/pygments/cmdline.py \
	Lib/pip/_vendor/pygments/console.py \
	Lib/pip/_vendor/pygments/filter.py \
	Lib/pip/_vendor/pygments/formatter.py \
	Lib/pip/_vendor/pygments/lexer.py \
	Lib/pip/_vendor/pygments/modeline.py \
	Lib/pip/_vendor/pygments/plugin.py \
	Lib/pip/_vendor/pygments/regexopt.py \
	Lib/pip/_vendor/pygments/scanner.py \
	Lib/pip/_vendor/pygments/sphinxext.py \
	Lib/pip/_vendor/pygments/style.py \
	Lib/pip/_vendor/pygments/token.py \
	Lib/pip/_vendor/pygments/unistring.py \
	Lib/pip/_vendor/pygments/util.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_vendor/pygments/filters,\
	Lib/pip/_vendor/pygments/filters/__init__.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_vendor/pygments/formatters,\
	Lib/pip/_vendor/pygments/formatters/__init__.py \
	Lib/pip/_vendor/pygments/formatters/_mapping.py \
	Lib/pip/_vendor/pygments/formatters/bbcode.py \
	Lib/pip/_vendor/pygments/formatters/groff.py \
	Lib/pip/_vendor/pygments/formatters/html.py \
	Lib/pip/_vendor/pygments/formatters/img.py \
	Lib/pip/_vendor/pygments/formatters/irc.py \
	Lib/pip/_vendor/pygments/formatters/latex.py \
	Lib/pip/_vendor/pygments/formatters/other.py \
	Lib/pip/_vendor/pygments/formatters/pangomarkup.py \
	Lib/pip/_vendor/pygments/formatters/rtf.py \
	Lib/pip/_vendor/pygments/formatters/svg.py \
	Lib/pip/_vendor/pygments/formatters/terminal.py \
	Lib/pip/_vendor/pygments/formatters/terminal256.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_vendor/pygments/lexers,\
	Lib/pip/_vendor/pygments/lexers/__init__.py \
	Lib/pip/_vendor/pygments/lexers/_mapping.py \
	Lib/pip/_vendor/pygments/lexers/python.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_vendor/pygments/styles,\
	Lib/pip/_vendor/pygments/styles/__init__.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_vendor/pyparsing,\
	Lib/pip/_vendor/pyparsing/__init__.py \
	Lib/pip/_vendor/pyparsing/actions.py \
	Lib/pip/_vendor/pyparsing/common.py \
	Lib/pip/_vendor/pyparsing/core.py \
	Lib/pip/_vendor/pyparsing/exceptions.py \
	Lib/pip/_vendor/pyparsing/helpers.py \
	Lib/pip/_vendor/pyparsing/results.py \
	Lib/pip/_vendor/pyparsing/testing.py \
	Lib/pip/_vendor/pyparsing/unicode.py \
	Lib/pip/_vendor/pyparsing/util.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_vendor/pyparsing/diagram,\
	Lib/pip/_vendor/pyparsing/diagram/__init__.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_vendor/pyproject_hooks,\
	Lib/pip/_vendor/pyproject_hooks/__init__.py \
	Lib/pip/_vendor/pyproject_hooks/_compat.py \
	Lib/pip/_vendor/pyproject_hooks/_impl.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_vendor/pyproject_hooks/_in_process,\
	Lib/pip/_vendor/pyproject_hooks/_in_process/__init__.py \
	Lib/pip/_vendor/pyproject_hooks/_in_process/_in_process.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_vendor/requests,\
	Lib/pip/_vendor/requests/__init__.py \
	Lib/pip/_vendor/requests/__version__.py \
	Lib/pip/_vendor/requests/_internal_utils.py \
	Lib/pip/_vendor/requests/adapters.py \
	Lib/pip/_vendor/requests/api.py \
	Lib/pip/_vendor/requests/auth.py \
	Lib/pip/_vendor/requests/certs.py \
	Lib/pip/_vendor/requests/compat.py \
	Lib/pip/_vendor/requests/cookies.py \
	Lib/pip/_vendor/requests/exceptions.py \
	Lib/pip/_vendor/requests/help.py \
	Lib/pip/_vendor/requests/hooks.py \
	Lib/pip/_vendor/requests/models.py \
	Lib/pip/_vendor/requests/packages.py \
	Lib/pip/_vendor/requests/sessions.py \
	Lib/pip/_vendor/requests/status_codes.py \
	Lib/pip/_vendor/requests/structures.py \
	Lib/pip/_vendor/requests/utils.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_vendor/resolvelib,\
	Lib/pip/_vendor/resolvelib/__init__.py \
	Lib/pip/_vendor/resolvelib/providers.py \
	Lib/pip/_vendor/resolvelib/reporters.py \
	Lib/pip/_vendor/resolvelib/resolvers.py \
	Lib/pip/_vendor/resolvelib/structs.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_vendor/resolvelib/compat,\
	Lib/pip/_vendor/resolvelib/compat/__init__.py \
	Lib/pip/_vendor/resolvelib/compat/collections_abc.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_vendor/rich,\
	Lib/pip/_vendor/rich/__init__.py \
	Lib/pip/_vendor/rich/__main__.py \
	Lib/pip/_vendor/rich/_cell_widths.py \
	Lib/pip/_vendor/rich/_emoji_codes.py \
	Lib/pip/_vendor/rich/_emoji_replace.py \
	Lib/pip/_vendor/rich/_export_format.py \
	Lib/pip/_vendor/rich/_extension.py \
	Lib/pip/_vendor/rich/_fileno.py \
	Lib/pip/_vendor/rich/_inspect.py \
	Lib/pip/_vendor/rich/_log_render.py \
	Lib/pip/_vendor/rich/_loop.py \
	Lib/pip/_vendor/rich/_null_file.py \
	Lib/pip/_vendor/rich/_palettes.py \
	Lib/pip/_vendor/rich/_pick.py \
	Lib/pip/_vendor/rich/_ratio.py \
	Lib/pip/_vendor/rich/_spinners.py \
	Lib/pip/_vendor/rich/_stack.py \
	Lib/pip/_vendor/rich/_timer.py \
	Lib/pip/_vendor/rich/_win32_console.py \
	Lib/pip/_vendor/rich/_windows.py \
	Lib/pip/_vendor/rich/_windows_renderer.py \
	Lib/pip/_vendor/rich/_wrap.py \
	Lib/pip/_vendor/rich/abc.py \
	Lib/pip/_vendor/rich/align.py \
	Lib/pip/_vendor/rich/ansi.py \
	Lib/pip/_vendor/rich/bar.py \
	Lib/pip/_vendor/rich/box.py \
	Lib/pip/_vendor/rich/cells.py \
	Lib/pip/_vendor/rich/color.py \
	Lib/pip/_vendor/rich/color_triplet.py \
	Lib/pip/_vendor/rich/columns.py \
	Lib/pip/_vendor/rich/console.py \
	Lib/pip/_vendor/rich/constrain.py \
	Lib/pip/_vendor/rich/containers.py \
	Lib/pip/_vendor/rich/control.py \
	Lib/pip/_vendor/rich/default_styles.py \
	Lib/pip/_vendor/rich/diagnose.py \
	Lib/pip/_vendor/rich/emoji.py \
	Lib/pip/_vendor/rich/errors.py \
	Lib/pip/_vendor/rich/file_proxy.py \
	Lib/pip/_vendor/rich/filesize.py \
	Lib/pip/_vendor/rich/highlighter.py \
	Lib/pip/_vendor/rich/json.py \
	Lib/pip/_vendor/rich/jupyter.py \
	Lib/pip/_vendor/rich/layout.py \
	Lib/pip/_vendor/rich/live.py \
	Lib/pip/_vendor/rich/live_render.py \
	Lib/pip/_vendor/rich/logging.py \
	Lib/pip/_vendor/rich/markup.py \
	Lib/pip/_vendor/rich/measure.py \
	Lib/pip/_vendor/rich/padding.py \
	Lib/pip/_vendor/rich/pager.py \
	Lib/pip/_vendor/rich/palette.py \
	Lib/pip/_vendor/rich/panel.py \
	Lib/pip/_vendor/rich/pretty.py \
	Lib/pip/_vendor/rich/progress.py \
	Lib/pip/_vendor/rich/progress_bar.py \
	Lib/pip/_vendor/rich/prompt.py \
	Lib/pip/_vendor/rich/protocol.py \
	Lib/pip/_vendor/rich/region.py \
	Lib/pip/_vendor/rich/repr.py \
	Lib/pip/_vendor/rich/rule.py \
	Lib/pip/_vendor/rich/scope.py \
	Lib/pip/_vendor/rich/screen.py \
	Lib/pip/_vendor/rich/segment.py \
	Lib/pip/_vendor/rich/spinner.py \
	Lib/pip/_vendor/rich/status.py \
	Lib/pip/_vendor/rich/style.py \
	Lib/pip/_vendor/rich/styled.py \
	Lib/pip/_vendor/rich/syntax.py \
	Lib/pip/_vendor/rich/table.py \
	Lib/pip/_vendor/rich/terminal_theme.py \
	Lib/pip/_vendor/rich/text.py \
	Lib/pip/_vendor/rich/theme.py \
	Lib/pip/_vendor/rich/themes.py \
	Lib/pip/_vendor/rich/traceback.py \
	Lib/pip/_vendor/rich/tree.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_vendor/tenacity,\
	Lib/pip/_vendor/tenacity/__init__.py \
	Lib/pip/_vendor/tenacity/_asyncio.py \
	Lib/pip/_vendor/tenacity/_utils.py \
	Lib/pip/_vendor/tenacity/after.py \
	Lib/pip/_vendor/tenacity/before.py \
	Lib/pip/_vendor/tenacity/before_sleep.py \
	Lib/pip/_vendor/tenacity/nap.py \
	Lib/pip/_vendor/tenacity/retry.py \
	Lib/pip/_vendor/tenacity/stop.py \
	Lib/pip/_vendor/tenacity/tornadoweb.py \
	Lib/pip/_vendor/tenacity/wait.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_vendor/tomli,\
	Lib/pip/_vendor/tomli/__init__.py \
	Lib/pip/_vendor/tomli/_parser.py \
	Lib/pip/_vendor/tomli/_re.py \
	Lib/pip/_vendor/tomli/_types.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_vendor/truststore,\
	Lib/pip/_vendor/truststore/__init__.py \
	Lib/pip/_vendor/truststore/_api.py \
	Lib/pip/_vendor/truststore/_macos.py \
	Lib/pip/_vendor/truststore/_openssl.py \
	Lib/pip/_vendor/truststore/_ssl_constants.py \
	Lib/pip/_vendor/truststore/_windows.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_vendor/urllib3,\
	Lib/pip/_vendor/urllib3/__init__.py \
	Lib/pip/_vendor/urllib3/_collections.py \
	Lib/pip/_vendor/urllib3/_version.py \
	Lib/pip/_vendor/urllib3/connection.py \
	Lib/pip/_vendor/urllib3/connectionpool.py \
	Lib/pip/_vendor/urllib3/exceptions.py \
	Lib/pip/_vendor/urllib3/fields.py \
	Lib/pip/_vendor/urllib3/filepost.py \
	Lib/pip/_vendor/urllib3/poolmanager.py \
	Lib/pip/_vendor/urllib3/request.py \
	Lib/pip/_vendor/urllib3/response.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_vendor/urllib3/contrib,\
	Lib/pip/_vendor/urllib3/contrib/__init__.py \
	Lib/pip/_vendor/urllib3/contrib/_appengine_environ.py \
	Lib/pip/_vendor/urllib3/contrib/appengine.py \
	Lib/pip/_vendor/urllib3/contrib/ntlmpool.py \
	Lib/pip/_vendor/urllib3/contrib/pyopenssl.py \
	Lib/pip/_vendor/urllib3/contrib/securetransport.py \
	Lib/pip/_vendor/urllib3/contrib/socks.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_vendor/urllib3/contrib/_securetransport,\
	Lib/pip/_vendor/urllib3/contrib/_securetransport/__init__.py \
	Lib/pip/_vendor/urllib3/contrib/_securetransport/bindings.py \
	Lib/pip/_vendor/urllib3/contrib/_securetransport/low_level.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_vendor/urllib3/packages,\
	Lib/pip/_vendor/urllib3/packages/__init__.py \
	Lib/pip/_vendor/urllib3/packages/six.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_vendor/urllib3/packages/backports,\
	Lib/pip/_vendor/urllib3/packages/backports/__init__.py \
	Lib/pip/_vendor/urllib3/packages/backports/makefile.py \
	Lib/pip/_vendor/urllib3/packages/backports/weakref_finalize.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_vendor/urllib3/util,\
	Lib/pip/_vendor/urllib3/util/__init__.py \
	Lib/pip/_vendor/urllib3/util/connection.py \
	Lib/pip/_vendor/urllib3/util/proxy.py \
	Lib/pip/_vendor/urllib3/util/queue.py \
	Lib/pip/_vendor/urllib3/util/request.py \
	Lib/pip/_vendor/urllib3/util/response.py \
	Lib/pip/_vendor/urllib3/util/retry.py \
	Lib/pip/_vendor/urllib3/util/ssl_.py \
	Lib/pip/_vendor/urllib3/util/ssl_match_hostname.py \
	Lib/pip/_vendor/urllib3/util/ssltransport.py \
	Lib/pip/_vendor/urllib3/util/timeout.py \
	Lib/pip/_vendor/urllib3/util/url.py \
	Lib/pip/_vendor/urllib3/util/wait.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pip/_vendor/webencodings,\
	Lib/pip/_vendor/webencodings/__init__.py \
	Lib/pip/_vendor/webencodings/labels.py \
	Lib/pip/_vendor/webencodings/mklabels.py \
	Lib/pip/_vendor/webencodings/tests.py \
	Lib/pip/_vendor/webencodings/x_user_defined.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pkg_resources,\
	Lib/pkg_resources/__init__.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pkg_resources/_vendor,\
	Lib/pkg_resources/_vendor/__init__.py \
	Lib/pkg_resources/_vendor/appdirs.py \
	Lib/pkg_resources/_vendor/zipp.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pkg_resources/_vendor/importlib_resources,\
	Lib/pkg_resources/_vendor/importlib_resources/__init__.py \
	Lib/pkg_resources/_vendor/importlib_resources/_adapters.py \
	Lib/pkg_resources/_vendor/importlib_resources/_common.py \
	Lib/pkg_resources/_vendor/importlib_resources/_compat.py \
	Lib/pkg_resources/_vendor/importlib_resources/_itertools.py \
	Lib/pkg_resources/_vendor/importlib_resources/_legacy.py \
	Lib/pkg_resources/_vendor/importlib_resources/abc.py \
	Lib/pkg_resources/_vendor/importlib_resources/readers.py \
	Lib/pkg_resources/_vendor/importlib_resources/simple.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pkg_resources/_vendor/jaraco,\
	Lib/pkg_resources/_vendor/jaraco/__init__.py \
	Lib/pkg_resources/_vendor/jaraco/context.py \
	Lib/pkg_resources/_vendor/jaraco/functools.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pkg_resources/_vendor/jaraco/text,\
	Lib/pkg_resources/_vendor/jaraco/text/__init__.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pkg_resources/_vendor/more_itertools,\
	Lib/pkg_resources/_vendor/more_itertools/__init__.py \
	Lib/pkg_resources/_vendor/more_itertools/more.py \
	Lib/pkg_resources/_vendor/more_itertools/recipes.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pkg_resources/_vendor/packaging,\
	Lib/pkg_resources/_vendor/packaging/__about__.py \
	Lib/pkg_resources/_vendor/packaging/__init__.py \
	Lib/pkg_resources/_vendor/packaging/_manylinux.py \
	Lib/pkg_resources/_vendor/packaging/_musllinux.py \
	Lib/pkg_resources/_vendor/packaging/_structures.py \
	Lib/pkg_resources/_vendor/packaging/markers.py \
	Lib/pkg_resources/_vendor/packaging/requirements.py \
	Lib/pkg_resources/_vendor/packaging/specifiers.py \
	Lib/pkg_resources/_vendor/packaging/tags.py \
	Lib/pkg_resources/_vendor/packaging/utils.py \
	Lib/pkg_resources/_vendor/packaging/version.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pkg_resources/_vendor/pyparsing,\
	Lib/pkg_resources/_vendor/pyparsing/__init__.py \
	Lib/pkg_resources/_vendor/pyparsing/actions.py \
	Lib/pkg_resources/_vendor/pyparsing/common.py \
	Lib/pkg_resources/_vendor/pyparsing/core.py \
	Lib/pkg_resources/_vendor/pyparsing/exceptions.py \
	Lib/pkg_resources/_vendor/pyparsing/helpers.py \
	Lib/pkg_resources/_vendor/pyparsing/results.py \
	Lib/pkg_resources/_vendor/pyparsing/testing.py \
	Lib/pkg_resources/_vendor/pyparsing/unicode.py \
	Lib/pkg_resources/_vendor/pyparsing/util.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pkg_resources/_vendor/pyparsing/diagram,\
	Lib/pkg_resources/_vendor/pyparsing/diagram/__init__.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pkg_resources/extern,\
	Lib/pkg_resources/extern/__init__.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/pydoc_data,\
	Lib/pydoc_data/__init__.py \
	Lib/pydoc_data/topics.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/re,\
	Lib/re/__init__.py \
	Lib/re/_casefix.py \
	Lib/re/_compiler.py \
	Lib/re/_constants.py \
	Lib/re/_parser.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/setuptools,\
	Lib/setuptools/__init__.py \
	Lib/setuptools/_deprecation_warning.py \
	Lib/setuptools/_entry_points.py \
	Lib/setuptools/_imp.py \
	Lib/setuptools/_importlib.py \
	Lib/setuptools/_itertools.py \
	Lib/setuptools/_path.py \
	Lib/setuptools/_reqs.py \
	Lib/setuptools/archive_util.py \
	Lib/setuptools/build_meta.py \
	Lib/setuptools/dep_util.py \
	Lib/setuptools/depends.py \
	Lib/setuptools/discovery.py \
	Lib/setuptools/dist.py \
	Lib/setuptools/errors.py \
	Lib/setuptools/extension.py \
	Lib/setuptools/glob.py \
	Lib/setuptools/installer.py \
	Lib/setuptools/launch.py \
	Lib/setuptools/logging.py \
	Lib/setuptools/monkey.py \
	Lib/setuptools/msvc.py \
	Lib/setuptools/namespaces.py \
	Lib/setuptools/package_index.py \
	Lib/setuptools/py34compat.py \
	Lib/setuptools/sandbox.py \
	Lib/setuptools/unicode_utils.py \
	Lib/setuptools/version.py \
	Lib/setuptools/wheel.py \
	Lib/setuptools/windows_support.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/setuptools/_distutils,\
	Lib/setuptools/_distutils/__init__.py \
	Lib/setuptools/_distutils/_collections.py \
	Lib/setuptools/_distutils/_functools.py \
	Lib/setuptools/_distutils/_macos_compat.py \
	Lib/setuptools/_distutils/_msvccompiler.py \
	Lib/setuptools/_distutils/archive_util.py \
	Lib/setuptools/_distutils/bcppcompiler.py \
	Lib/setuptools/_distutils/ccompiler.py \
	Lib/setuptools/_distutils/cmd.py \
	Lib/setuptools/_distutils/config.py \
	Lib/setuptools/_distutils/core.py \
	Lib/setuptools/_distutils/cygwinccompiler.py \
	Lib/setuptools/_distutils/debug.py \
	Lib/setuptools/_distutils/dep_util.py \
	Lib/setuptools/_distutils/dir_util.py \
	Lib/setuptools/_distutils/dist.py \
	Lib/setuptools/_distutils/errors.py \
	Lib/setuptools/_distutils/extension.py \
	Lib/setuptools/_distutils/fancy_getopt.py \
	Lib/setuptools/_distutils/file_util.py \
	Lib/setuptools/_distutils/filelist.py \
	Lib/setuptools/_distutils/log.py \
	Lib/setuptools/_distutils/msvc9compiler.py \
	Lib/setuptools/_distutils/msvccompiler.py \
	Lib/setuptools/_distutils/py38compat.py \
	Lib/setuptools/_distutils/py39compat.py \
	Lib/setuptools/_distutils/spawn.py \
	Lib/setuptools/_distutils/sysconfig.py \
	Lib/setuptools/_distutils/text_file.py \
	Lib/setuptools/_distutils/unixccompiler.py \
	Lib/setuptools/_distutils/util.py \
	Lib/setuptools/_distutils/version.py \
	Lib/setuptools/_distutils/versionpredicate.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/setuptools/_distutils/command,\
	Lib/setuptools/_distutils/command/__init__.py \
	Lib/setuptools/_distutils/command/_framework_compat.py \
	Lib/setuptools/_distutils/command/bdist.py \
	Lib/setuptools/_distutils/command/bdist_dumb.py \
	Lib/setuptools/_distutils/command/bdist_rpm.py \
	Lib/setuptools/_distutils/command/build.py \
	Lib/setuptools/_distutils/command/build_clib.py \
	Lib/setuptools/_distutils/command/build_ext.py \
	Lib/setuptools/_distutils/command/build_py.py \
	Lib/setuptools/_distutils/command/build_scripts.py \
	Lib/setuptools/_distutils/command/check.py \
	Lib/setuptools/_distutils/command/clean.py \
	Lib/setuptools/_distutils/command/config.py \
	Lib/setuptools/_distutils/command/install.py \
	Lib/setuptools/_distutils/command/install_data.py \
	Lib/setuptools/_distutils/command/install_egg_info.py \
	Lib/setuptools/_distutils/command/install_headers.py \
	Lib/setuptools/_distutils/command/install_lib.py \
	Lib/setuptools/_distutils/command/install_scripts.py \
	Lib/setuptools/_distutils/command/py37compat.py \
	Lib/setuptools/_distutils/command/register.py \
	Lib/setuptools/_distutils/command/sdist.py \
	Lib/setuptools/_distutils/command/upload.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/setuptools/_vendor,\
	Lib/setuptools/_vendor/__init__.py \
	Lib/setuptools/_vendor/ordered_set.py \
	Lib/setuptools/_vendor/typing_extensions.py \
	Lib/setuptools/_vendor/zipp.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/setuptools/_vendor/importlib_metadata,\
	Lib/setuptools/_vendor/importlib_metadata/__init__.py \
	Lib/setuptools/_vendor/importlib_metadata/_adapters.py \
	Lib/setuptools/_vendor/importlib_metadata/_collections.py \
	Lib/setuptools/_vendor/importlib_metadata/_compat.py \
	Lib/setuptools/_vendor/importlib_metadata/_functools.py \
	Lib/setuptools/_vendor/importlib_metadata/_itertools.py \
	Lib/setuptools/_vendor/importlib_metadata/_meta.py \
	Lib/setuptools/_vendor/importlib_metadata/_text.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/setuptools/_vendor/importlib_resources,\
	Lib/setuptools/_vendor/importlib_resources/__init__.py \
	Lib/setuptools/_vendor/importlib_resources/_adapters.py \
	Lib/setuptools/_vendor/importlib_resources/_common.py \
	Lib/setuptools/_vendor/importlib_resources/_compat.py \
	Lib/setuptools/_vendor/importlib_resources/_itertools.py \
	Lib/setuptools/_vendor/importlib_resources/_legacy.py \
	Lib/setuptools/_vendor/importlib_resources/abc.py \
	Lib/setuptools/_vendor/importlib_resources/readers.py \
	Lib/setuptools/_vendor/importlib_resources/simple.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/setuptools/_vendor/jaraco,\
	Lib/setuptools/_vendor/jaraco/__init__.py \
	Lib/setuptools/_vendor/jaraco/context.py \
	Lib/setuptools/_vendor/jaraco/functools.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/setuptools/_vendor/jaraco/text,\
	Lib/setuptools/_vendor/jaraco/text/__init__.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/setuptools/_vendor/more_itertools,\
	Lib/setuptools/_vendor/more_itertools/__init__.py \
	Lib/setuptools/_vendor/more_itertools/more.py \
	Lib/setuptools/_vendor/more_itertools/recipes.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/setuptools/_vendor/packaging,\
	Lib/setuptools/_vendor/packaging/__about__.py \
	Lib/setuptools/_vendor/packaging/__init__.py \
	Lib/setuptools/_vendor/packaging/_manylinux.py \
	Lib/setuptools/_vendor/packaging/_musllinux.py \
	Lib/setuptools/_vendor/packaging/_structures.py \
	Lib/setuptools/_vendor/packaging/markers.py \
	Lib/setuptools/_vendor/packaging/requirements.py \
	Lib/setuptools/_vendor/packaging/specifiers.py \
	Lib/setuptools/_vendor/packaging/tags.py \
	Lib/setuptools/_vendor/packaging/utils.py \
	Lib/setuptools/_vendor/packaging/version.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/setuptools/_vendor/pyparsing,\
	Lib/setuptools/_vendor/pyparsing/__init__.py \
	Lib/setuptools/_vendor/pyparsing/actions.py \
	Lib/setuptools/_vendor/pyparsing/common.py \
	Lib/setuptools/_vendor/pyparsing/core.py \
	Lib/setuptools/_vendor/pyparsing/exceptions.py \
	Lib/setuptools/_vendor/pyparsing/helpers.py \
	Lib/setuptools/_vendor/pyparsing/results.py \
	Lib/setuptools/_vendor/pyparsing/testing.py \
	Lib/setuptools/_vendor/pyparsing/unicode.py \
	Lib/setuptools/_vendor/pyparsing/util.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/setuptools/_vendor/pyparsing/diagram,\
	Lib/setuptools/_vendor/pyparsing/diagram/__init__.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/setuptools/_vendor/tomli,\
	Lib/setuptools/_vendor/tomli/__init__.py \
	Lib/setuptools/_vendor/tomli/_parser.py \
	Lib/setuptools/_vendor/tomli/_re.py \
	Lib/setuptools/_vendor/tomli/_types.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/setuptools/command,\
	Lib/setuptools/command/__init__.py \
	Lib/setuptools/command/alias.py \
	Lib/setuptools/command/bdist_egg.py \
	Lib/setuptools/command/bdist_rpm.py \
	Lib/setuptools/command/build.py \
	Lib/setuptools/command/build_clib.py \
	Lib/setuptools/command/build_ext.py \
	Lib/setuptools/command/build_py.py \
	Lib/setuptools/command/develop.py \
	Lib/setuptools/command/dist_info.py \
	Lib/setuptools/command/easy_install.py \
	Lib/setuptools/command/editable_wheel.py \
	Lib/setuptools/command/egg_info.py \
	Lib/setuptools/command/install.py \
	Lib/setuptools/command/install_egg_info.py \
	Lib/setuptools/command/install_lib.py \
	Lib/setuptools/command/install_scripts.py \
	Lib/setuptools/command/py36compat.py \
	Lib/setuptools/command/register.py \
	Lib/setuptools/command/rotate.py \
	Lib/setuptools/command/saveopts.py \
	Lib/setuptools/command/sdist.py \
	Lib/setuptools/command/setopt.py \
	Lib/setuptools/command/test.py \
	Lib/setuptools/command/upload.py \
	Lib/setuptools/command/upload_docs.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/setuptools/config,\
	Lib/setuptools/config/__init__.py \
	Lib/setuptools/config/_apply_pyprojecttoml.py \
	Lib/setuptools/config/expand.py \
	Lib/setuptools/config/pyprojecttoml.py \
	Lib/setuptools/config/setupcfg.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/setuptools/config/_validate_pyproject,\
	Lib/setuptools/config/_validate_pyproject/__init__.py \
	Lib/setuptools/config/_validate_pyproject/error_reporting.py \
	Lib/setuptools/config/_validate_pyproject/extra_validations.py \
	Lib/setuptools/config/_validate_pyproject/fastjsonschema_exceptions.py \
	Lib/setuptools/config/_validate_pyproject/fastjsonschema_validations.py \
	Lib/setuptools/config/_validate_pyproject/formats.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/setuptools/extern,\
	Lib/setuptools/extern/__init__.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/tomllib,\
	Lib/tomllib/__init__.py \
	Lib/tomllib/_parser.py \
	Lib/tomllib/_re.py \
	Lib/tomllib/_types.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/unittest,\
	Lib/unittest/__init__.py \
	Lib/unittest/__main__.py \
	Lib/unittest/_log.py \
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
	Lib/wsgiref/types.py \
	Lib/wsgiref/util.py \
	Lib/wsgiref/validate.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/xml,\
	Lib/xml/__init__.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/xml/dom,\
	Lib/xml/dom/NodeFilter.py \
	Lib/xml/dom/__init__.py \
	Lib/xml/dom/domreg.py \
	Lib/xml/dom/expatbuilder.py \
	Lib/xml/dom/minicompat.py \
	Lib/xml/dom/minidom.py \
	Lib/xml/dom/pulldom.py \
	Lib/xml/dom/xmlbuilder.py \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/xml/etree,\
	Lib/xml/etree/ElementInclude.py \
	Lib/xml/etree/ElementPath.py \
	Lib/xml/etree/ElementTree.py \
	Lib/xml/etree/__init__.py \
	Lib/xml/etree/cElementTree.py \
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

$(eval $(call gb_ExternalPackage_add_unpacked_files,python3,$(LIBO_BIN_FOLDER)/python-core-$(PYTHON_VERSION)/lib/zoneinfo,\
	Lib/zoneinfo/__init__.py \
	Lib/zoneinfo/_common.py \
	Lib/zoneinfo/_tzpath.py \
	Lib/zoneinfo/_zoneinfo.py \
))

# vim: set noet sw=4 ts=4:
