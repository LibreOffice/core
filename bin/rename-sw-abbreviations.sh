#! /bin/bash
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# This script renames the most annoying abbreviations in Writer (and partially
# in the shared code too).  Just run it in the source directory.

# sw only:

for I in "FrmFmt/FrameFormat" "Fmt/Format" "Cntnt/Content" "Txt/Text" "Tbl/Table" "GotoFld/GotoFormatField" "Fld/Field" "Ftn/Footnote" "Updt/Update" "Fml/Formula" "Hnt/Hint"
do
    S="${I%/*}"
    # change all except the filenames (in the .mk and in #include)
    # also avoid XML_numFmt, that's a token name
    git grep -l "$S" sw/ | grep -v '\.mk' | xargs sed -i '/\(#include\|XML_numFmt\)/ !{ s/'"$I"'/g }'
done

# global:

for I in "SvxSwAutoFmtFlags/SvxSwAutoFormatFlags" "GetCharFmtName/GetCharFormatName" \
    "SvxFmtBreakItem/SvxFormatBreakItem" "SvxFmtKeepItem/SvxFormatKeepItem" \
    "SvxFmtSplitItem/SvxFormatSplitItem" "etTxtLeft/etTextLeft" \
    "etTxtFirstLineOfst/etTextFirstLineOfst" "CntntProtected/ContentProtected" \
    "etTxtColor/etTextColor" "ClearFldColor/ClearFieldColor" \
    "etCntntProtect/etContentProtect" "etPropTxtFirstLineOfst/etPropTextFirstLineOfst" \
    "etCharFmtName/etCharFormatName" "HasMergeFmtTbl/HasMergeFormatTable" \
    "etMergeFmtIndex/etMergeFormatIndex" "bAFmtByInput/bAFormatByInput" \
    "bAFmt/bAFormat" "IsTxtFmt/IsTextFormat" "BuildWhichTbl/BuildWhichTable" \
    "etFld/etField" "IsAutoFmtByInput/IsAutoFormatByInput" \
    "etAutoFmtByInput/etAutoFormatByInput" "etMacroTbl/etMacroTable" \
    "SvxClipboardFmtItem/SvxClipboardFormatItem" "SwFlyFrmFmt/SwFlyFrameFormat" \
    "etTxtSize/etTextSize"
do
    S="${I%/*}"
    git grep -l "$S" | grep -v -e '\.mk' -e 'rename-sw-abbreviations.sh' | xargs sed -i "s/$I/g"
done

# vim: set noet sw=4 ts=4:
