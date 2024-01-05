#!/bin/sh
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# Generate an external tarball from upstream git repo $1 (optionally using $2 in the tarball name):

set -ex

source=${1?}
tag=${2-$(git -C "${source?}" describe --abbrev=40 --always)}

tmpdir=$(mktemp -d)

copyto() {
    (cd "${tmpdir?}" && mkdir -p $(dirname "${1?}"))
    cp "${source?}"/"${2?}" "${tmpdir?}"/"${1?}"
}

# Copy sources according to the old onlineupdate/ target layout:

copyto include/onlineupdate/mar.h modules/libmar/src/mar.h
copyto include/onlineupdate/mar_cmdline.h modules/libmar/src/mar_cmdline.h
copyto include/onlineupdate/mar_private.h modules/libmar/src/mar_private.h
copyto include/onlineupdate/mozilla/Assertions.h mfbt/Assertions.h
copyto include/onlineupdate/mozilla/Attributes.h mfbt/Attributes.h
copyto include/onlineupdate/mozilla/Compiler.h mfbt/Compiler.h
copyto include/onlineupdate/mozilla/Likely.h mfbt/Likely.h
copyto include/onlineupdate/mozilla/MacroArgs.h mfbt/MacroArgs.h
copyto include/onlineupdate/mozilla/Types.h mfbt/Types.h
copyto include/onlineupdate/mozilla/nsTraceRefcnt.h xpcom/base/nsTraceRefcnt.h
copyto include/mozilla/Char16.h mfbt/Char16.h
copyto onlineupdate/inc/bspatch.h toolkit/mozapps/update/updater/bspatch/bspatch.h
copyto onlineupdate/source/libmar/README modules/libmar/README
copyto onlineupdate/source/libmar/sign/mar_sign.c modules/libmar/sign/mar_sign.c
copyto onlineupdate/source/libmar/sign/nss_secutil.c modules/libmar/sign/nss_secutil.c
copyto onlineupdate/source/libmar/sign/nss_secutil.h modules/libmar/sign/nss_secutil.h
copyto onlineupdate/source/libmar/src/mar_create.c modules/libmar/src/mar_create.c
copyto onlineupdate/source/libmar/src/mar_extract.c modules/libmar/src/mar_extract.c
copyto onlineupdate/source/libmar/src/mar_read.c modules/libmar/src/mar_read.c
copyto onlineupdate/source/libmar/tool/mar.c modules/libmar/tool/mar.c
copyto onlineupdate/source/libmar/verify/MacVerifyCrypto.cpp modules/libmar/verify/MacVerifyCrypto.cpp
copyto onlineupdate/source/libmar/verify/cryptox.c modules/libmar/verify/cryptox.c
copyto onlineupdate/source/libmar/verify/cryptox.h modules/libmar/verify/cryptox.h
copyto onlineupdate/source/libmar/verify/mar_verify.c modules/libmar/verify/mar_verify.c
copyto onlineupdate/source/mbsdiff/bsdiff.c other-licenses/bsdiff/bsdiff.c
copyto onlineupdate/source/service/certificatecheck.cpp toolkit/mozapps/update/common/certificatecheck.cpp
copyto onlineupdate/source/service/certificatecheck.h toolkit/mozapps/update/common/certificatecheck.h
copyto onlineupdate/source/service/maintenanceservice.cpp toolkit/components/maintenanceservice/maintenanceservice.cpp
copyto onlineupdate/source/service/maintenanceservice.h toolkit/components/maintenanceservice/maintenanceservice.h
copyto onlineupdate/source/service/registrycertificates.cpp toolkit/mozapps/update/common/registrycertificates.cpp
copyto onlineupdate/source/service/registrycertificates.h toolkit/mozapps/update/common/registrycertificates.h
copyto onlineupdate/source/service/resource.h toolkit/components/maintenanceservice/resource.h
copyto onlineupdate/source/service/servicebase.cpp toolkit/components/maintenanceservice/servicebase.cpp
copyto onlineupdate/source/service/servicebase.h toolkit/components/maintenanceservice/servicebase.h
copyto onlineupdate/source/service/serviceinstall.cpp toolkit/components/maintenanceservice/serviceinstall.cpp
copyto onlineupdate/source/service/serviceinstall.h toolkit/components/maintenanceservice/serviceinstall.h
copyto onlineupdate/source/service/workmonitor.cpp toolkit/components/maintenanceservice/workmonitor.cpp
copyto onlineupdate/source/service/workmonitor.h toolkit/components/maintenanceservice/workmonitor.h
# Renamed target from onlineupdate/source/update/common/errors.h:
copyto onlineupdate/source/update/common/updatererrors.h toolkit/mozapps/update/common/updatererrors.h
copyto onlineupdate/source/update/common/pathhash.cpp toolkit/mozapps/update/common/pathhash.cpp
copyto onlineupdate/source/update/common/pathhash.h toolkit/mozapps/update/common/pathhash.h
copyto onlineupdate/source/update/common/readstrings.cpp toolkit/mozapps/update/common/readstrings.cpp
copyto onlineupdate/source/update/common/readstrings.h toolkit/mozapps/update/common/readstrings.h
copyto onlineupdate/source/update/common/uachelper.cpp toolkit/mozapps/update/common/uachelper.cpp
copyto onlineupdate/source/update/common/uachelper.h toolkit/mozapps/update/common/uachelper.h
copyto onlineupdate/source/update/common/updatedefines.h toolkit/mozapps/update/common/updatedefines.h
copyto onlineupdate/source/update/common/updatehelper.cpp toolkit/mozapps/update/common/updatehelper.cpp
copyto onlineupdate/source/update/common/updatehelper.h toolkit/mozapps/update/common/updatehelper.h
copyto onlineupdate/source/update/updater/Makefile.in toolkit/mozapps/update/updater/Makefile.in
copyto onlineupdate/source/update/updater/archivereader.cpp toolkit/mozapps/update/updater/archivereader.cpp
copyto onlineupdate/source/update/updater/archivereader.h toolkit/mozapps/update/updater/archivereader.h
copyto onlineupdate/source/update/updater/bspatch.cpp toolkit/mozapps/update/updater/bspatch/bspatch.cpp
copyto onlineupdate/source/update/updater/gen_cert_header.py toolkit/mozapps/update/updater/gen_cert_header.py
copyto onlineupdate/source/update/updater/launchchild_osx.mm toolkit/mozapps/update/updater/launchchild_osx.mm
copyto onlineupdate/source/update/updater/loaddlls.cpp toolkit/mozapps/update/updater/loaddlls.cpp
# Renamed target from onlineupdate/source/update/updater/macbuild/Contents/Info.plist:
copyto onlineupdate/source/update/updater/macbuild/Contents/Info.plist.in toolkit/mozapps/update/updater/macbuild/Contents/Info.plist.in
copyto onlineupdate/source/update/updater/macbuild/Contents/PkgInfo toolkit/mozapps/update/updater/macbuild/Contents/PkgInfo
copyto onlineupdate/source/update/updater/macbuild/Contents/Resources/English.lproj/InfoPlist.strings.in toolkit/mozapps/update/updater/macbuild/Contents/Resources/English.lproj/InfoPlist.strings.in
copyto onlineupdate/source/update/updater/macbuild/Contents/Resources/English.lproj/MainMenu.nib/classes.nib toolkit/mozapps/update/updater/macbuild/Contents/Resources/English.lproj/MainMenu.nib/classes.nib
copyto onlineupdate/source/update/updater/macbuild/Contents/Resources/English.lproj/MainMenu.nib/info.nib toolkit/mozapps/update/updater/macbuild/Contents/Resources/English.lproj/MainMenu.nib/info.nib
copyto onlineupdate/source/update/updater/macbuild/Contents/Resources/English.lproj/MainMenu.nib/keyedobjects.nib toolkit/mozapps/update/updater/macbuild/Contents/Resources/English.lproj/MainMenu.nib/keyedobjects.nib
copyto onlineupdate/source/update/updater/macbuild/Contents/Resources/updater.icns toolkit/mozapps/update/updater/macbuild/Contents/Resources/updater.icns
copyto onlineupdate/source/update/updater/progressui-unused/progressui_osx.mm toolkit/mozapps/update/updater/progressui_osx.mm
copyto onlineupdate/source/update/updater/progressui.h toolkit/mozapps/update/updater/progressui.h
copyto onlineupdate/source/update/updater/progressui_gtk.cpp toolkit/mozapps/update/updater/progressui_gtk.cpp
copyto onlineupdate/source/update/updater/progressui_null.cpp toolkit/mozapps/update/updater/progressui_null.cpp
copyto onlineupdate/source/update/updater/progressui_win.cpp toolkit/mozapps/update/updater/progressui_win.cpp
copyto onlineupdate/source/update/updater/resource.h toolkit/mozapps/update/updater/resource.h
copyto onlineupdate/source/update/updater/updater-common.build toolkit/mozapps/update/updater/updater-common.build
copyto onlineupdate/source/update/updater/updater-xpcshell/Makefile.in toolkit/mozapps/update/updater/updater-xpcshell/Makefile.in
copyto onlineupdate/source/update/updater/updater-xpcshell/moz.build toolkit/mozapps/update/updater/updater-xpcshell/moz.build
copyto onlineupdate/source/update/updater/updater.cpp toolkit/mozapps/update/updater/updater.cpp
copyto onlineupdate/source/update/updater/updater.exe.comctl32.manifest toolkit/mozapps/update/updater/updater.exe.comctl32.manifest
copyto onlineupdate/source/update/updater/updater.exe.manifest toolkit/mozapps/update/updater/updater.exe.manifest
copyto onlineupdate/source/update/updater/updater.ico toolkit/mozapps/update/updater/updater.ico
copyto onlineupdate/source/update/updater/updater.png toolkit/mozapps/update/updater/updater.png
copyto onlineupdate/source/update/updater/updater.rc toolkit/mozapps/update/updater/updater.rc
copyto onlineupdate/source/update/updater/xpcom/glue/nsVersionComparator.cpp xpcom/base/nsVersionComparator.cpp
copyto onlineupdate/source/update/updater/xpcom/glue/nsVersionComparator.h xpcom/base/nsVersionComparator.h

# Missing source for include/onlineupdate/mozilla/TypeTraits.h
# Missing source for onlineupdate/source/libmar/sign/Makefile.in
# Missing source for onlineupdate/source/libmar/src/Makefile.in
# Missing source for onlineupdate/source/libmar/tool/Makefile.in
# Missing source for onlineupdate/source/service/windowsHelper.hxx
# Missing source for onlineupdate/source/update/common/sources.mozbuild
# Missing source for onlineupdate/source/update/common/updatelogging.cxx
# Missing source for onlineupdate/source/update/common/updatelogging.h
# Missing source for onlineupdate/source/update/updater/progressui-unused/progressui_gonk.cxx
# Missing source for onlineupdate/source/update/updater/progressui_gtk_icon.h
# Missing source for onlineupdate/source/update/updater/updater.svg

# Copy additional sources according to the actual source layout:

copyto include/mozilla/Alignment.h mfbt/Alignment.h
copyto include/mozilla/AllocPolicy.h mfbt/AllocPolicy.h
copyto include/mozilla/ArrayUtils.h mfbt/ArrayUtils.h
copyto include/mozilla/Atomics.h mfbt/Atomics.h
copyto include/mozilla/Casting.h mfbt/Casting.h
copyto include/mozilla/CheckedInt.h mfbt/CheckedInt.h
copyto include/mozilla/CmdLineAndEnvUtils.h toolkit/xre/CmdLineAndEnvUtils.h
copyto include/mozilla/CompactPair.h mfbt/CompactPair.h
copyto include/mozilla/DebugOnly.h mfbt/DebugOnly.h
copyto include/mozilla/DynamicallyLinkedFunctionPtr.h mozglue/misc/DynamicallyLinkedFunctionPtr.h
copyto include/mozilla/EndianUtils.h mfbt/EndianUtils.h
copyto include/mozilla/Fuzzing.h mfbt/Fuzzing.h
copyto include/mozilla/HashFunctions.h mfbt/HashFunctions.h
copyto include/mozilla/IntegerPrintfMacros.h mozglue/misc/IntegerPrintfMacros.h
copyto include/mozilla/IntegerTypeTraits.h mfbt/IntegerTypeTraits.h
copyto include/mozilla/MathAlgorithms.h mfbt/MathAlgorithms.h
copyto include/mozilla/Maybe.h mfbt/Maybe.h
copyto include/mozilla/MaybeStorageBase.h mfbt/MaybeStorageBase.h
copyto include/mozilla/MemoryChecking.h mfbt/MemoryChecking.h
copyto include/mozilla/MemoryReporting.h mfbt/MemoryReporting.h
copyto include/mozilla/OperatorNewExtensions.h mfbt/OperatorNewExtensions.h
copyto include/mozilla/Poison.h mfbt/Poison.h
copyto include/mozilla/Printf.h mozglue/misc/Printf.h
copyto include/mozilla/ReentrancyGuard.h mfbt/ReentrancyGuard.h
copyto include/mozilla/Result.h mfbt/Result.h
copyto include/mozilla/ResultVariant.h mfbt/ResultVariant.h
copyto include/mozilla/Span.h mfbt/Span.h
copyto include/mozilla/Sprintf.h mozglue/misc/Sprintf.h
copyto include/mozilla/StaticAnalysisFunctions.h mfbt/StaticAnalysisFunctions.h
copyto include/mozilla/TemplateLib.h mfbt/TemplateLib.h
copyto include/mozilla/ThreadSafety.h mfbt/ThreadSafety.h
copyto include/mozilla/TypedEnumBits.h mfbt/TypedEnumBits.h
copyto include/mozilla/UniquePtr.h mfbt/UniquePtr.h
copyto include/mozilla/UniquePtrExtensions.h mfbt/UniquePtrExtensions.h
copyto include/mozilla/Variant.h mfbt/Variant.h
copyto include/mozilla/Vector.h mfbt/Vector.h
copyto include/mozilla/WinHeaderOnlyUtils.h widget/windows/WinHeaderOnlyUtils.h
copyto include/mozilla/WrappingOperations.h mfbt/WrappingOperations.h
copyto include/mozilla/fallible.h mfbt/fallible.h
copyto mfbt/double-conversion/double-conversion/bignum-dtoa.cc mfbt/double-conversion/double-conversion/bignum-dtoa.cc
copyto mfbt/double-conversion/double-conversion/bignum-dtoa.h mfbt/double-conversion/double-conversion/bignum-dtoa.h
copyto mfbt/double-conversion/double-conversion/bignum.cc mfbt/double-conversion/double-conversion/bignum.cc
copyto mfbt/double-conversion/double-conversion/bignum.h mfbt/double-conversion/double-conversion/bignum.h
copyto mfbt/double-conversion/double-conversion/cached-powers.cc mfbt/double-conversion/double-conversion/cached-powers.cc
copyto mfbt/double-conversion/double-conversion/cached-powers.h mfbt/double-conversion/double-conversion/cached-powers.h
copyto mfbt/double-conversion/double-conversion/diy-fp.h mfbt/double-conversion/double-conversion/diy-fp.h
copyto mfbt/double-conversion/double-conversion/double-to-string.cc mfbt/double-conversion/double-conversion/double-to-string.cc
copyto mfbt/double-conversion/double-conversion/double-to-string.h mfbt/double-conversion/double-conversion/double-to-string.h
copyto mfbt/double-conversion/double-conversion/fast-dtoa.cc mfbt/double-conversion/double-conversion/fast-dtoa.cc
copyto mfbt/double-conversion/double-conversion/fast-dtoa.h mfbt/double-conversion/double-conversion/fast-dtoa.h
copyto mfbt/double-conversion/double-conversion/fixed-dtoa.cc mfbt/double-conversion/double-conversion/fixed-dtoa.cc
copyto mfbt/double-conversion/double-conversion/fixed-dtoa.h mfbt/double-conversion/double-conversion/fixed-dtoa.h
copyto mfbt/double-conversion/double-conversion/ieee.h mfbt/double-conversion/double-conversion/ieee.h
copyto mfbt/double-conversion/double-conversion/utils.h mfbt/double-conversion/double-conversion/utils.h
copyto modules/xz-embedded/src/xz.h modules/xz-embedded/src/xz.h
copyto modules/xz-embedded/src/xz_config.h modules/xz-embedded/src/xz_config.h
copyto modules/xz-embedded/src/xz_crc32.c modules/xz-embedded/src/xz_crc32.c
copyto modules/xz-embedded/src/xz_crc64.c modules/xz-embedded/src/xz_crc64.c
copyto modules/xz-embedded/src/xz_dec_bcj.c modules/xz-embedded/src/xz_dec_bcj.c
copyto modules/xz-embedded/src/xz_dec_lzma2.c modules/xz-embedded/src/xz_dec_lzma2.c
copyto modules/xz-embedded/src/xz_dec_stream.c modules/xz-embedded/src/xz_dec_stream.c
copyto modules/xz-embedded/src/xz_lzma2.h modules/xz-embedded/src/xz_lzma2.h
copyto modules/xz-embedded/src/xz_private.h modules/xz-embedded/src/xz_private.h
copyto modules/xz-embedded/src/xz_stream.h modules/xz-embedded/src/xz_stream.h
copyto mozglue/misc/Printf.cpp mozglue/misc/Printf.cpp
copyto nsprpub/lib/libc/src/strdup.c nsprpub/lib/libc/src/strdup.c
copyto nsprpub/lib/libc/src/strlen.c nsprpub/lib/libc/src/strlen.c
copyto other-licenses/nsis/Contrib/CityHash/cityhash/city.cpp other-licenses/nsis/Contrib/CityHash/cityhash/city.cpp
copyto other-licenses/nsis/Contrib/CityHash/cityhash/city.h other-licenses/nsis/Contrib/CityHash/cityhash/city.h
copyto toolkit/mozapps/update/common/commonupdatedir.h toolkit/mozapps/update/common/commonupdatedir.h
copyto toolkit/mozapps/update/common/updatecommon.cpp toolkit/mozapps/update/common/updatecommon.cpp
copyto toolkit/mozapps/update/common/updatecommon.h toolkit/mozapps/update/common/updatecommon.h
copyto toolkit/mozapps/update/common/updateutils_win.cpp toolkit/mozapps/update/common/updateutils_win.cpp
copyto toolkit/mozapps/update/common/updateutils_win.h toolkit/mozapps/update/common/updateutils_win.h
copyto toolkit/mozapps/update/updater/crctable.h toolkit/mozapps/update/updater/crctable.h
copyto toolkit/xre/nsWindowsRestart.cpp toolkit/xre/nsWindowsRestart.cpp
copyto tools/update-packaging/common.sh tools/update-packaging/common.sh
copyto tools/update-packaging/make_full_update.sh tools/update-packaging/make_full_update.sh
copyto tools/update-packaging/make_incremental_update.sh tools/update-packaging/make_incremental_update.sh
copyto xpcom/base/nsAlgorithm.h xpcom/base/nsAlgorithm.h
copyto xpcom/base/nsAutoRef.h xpcom/base/nsAutoRef.h
copyto xpcom/base/nsWindowsHelpers.h xpcom/base/nsWindowsHelpers.h
copyto xpcom/string/nsCharTraits.h xpcom/string/nsCharTraits.h
copyto xpcom/string/nsUTF8Utils.h xpcom/string/nsUTF8Utils.h

LC_ALL=C tar -c --format=gnu --sort=name --owner=0 --group=0 --mode=go=rX,u=rwX \
 --mtime '2023-12-11 00:00:00+0' --xz -f onlineupdate-"${tag?}".tar.xz -C "${tmpdir?}" .
rm -r "${tmpdir?}"
