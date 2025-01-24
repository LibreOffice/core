#!/usr/bin/env bash
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# This script prints lines from .ui files, where the translatable="yes" attribute
# was not set -- presumably by mistake. It prints a few false positives though.

for i in `git ls-files *.ui`; do
    for j in "label" "title" "text" "format" "copyright" "comments" "preview_text" "tooltip" "message" ; do
        grep -Hns "\<property name\=\"$j\"" $i | grep -v "translatable\=\"yes" | grep -v "translatable\=\"no" | grep -v gtk\- | grep ">.*[A-Za-z].*<";
    done
    grep -Hns "<item" $i | grep -v "translatable\=\"yes" | grep -v "translatable\=\"no" | grep ">.*[A-Za-z].*<";
done
