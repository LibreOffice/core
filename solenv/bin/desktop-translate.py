#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

"""Translates multiple .desktop files at once with strings from .ulf
files; if you add new translatable .ulf files please add them to
l10ntools/source/localize.cxx in case the module is not already listed."""

import os
import sys
import argparse
import io


def encode_desktop_string(s_value):
    """Function encoding strings to be used as values in .desktop files."""
    # <https://specifications.freedesktop.org/desktop-entry-spec/desktop-entry-spec-1.1.html#
    # value-types> says "The escape sequences \s, \n, \t, \r, and \\ are supported for values of
    # type string and localestring, meaning ASCII space, newline, tab, carriage return, and
    # backslash, respectively."  <https://specifications.freedesktop.org/desktop-entry-spec/
    # desktop-entry-spec-1.1.html#basic-format> says "A file is interpreted as a series of lines
    # that are separated by linefeed characters", so it is apparently necessary to escape at least
    # linefeed and backslash characters.  It is unclear why that spec talks about "linefeed" in
    # one place and about "newline" ("\n") and "carriage return" ("\r") in another, and how they are
    # supposed to relate, so just escape any U+000A LINE FEED as "\n" and any U+000D CARRIAGE RETURN
    # as "\r"; it is unclear exactly which occurrences of U+0020 SPACE and U+0009 CHARACTER
    # TABULATION would need to be escaped, so they are mostly left unescaped, for readability:
    s_value = s_value.replace("\\", "\\\\").replace("\n", "\\n").replace("\r", "\\r")
    if s_value.startswith(" "):
        # <https://specifications.freedesktop.org/desktop-entry-spec/desktop-entry-spec-1.1.html#
        # entries> says "Space before and after the equals sign should be ignored", so escape a
        # leading U+0020 SPACE as "\s" (while it is not clear whether "space" there means just
        # U+0020 SPACE or any kind of white space, in which case at least a leading U+0009 CHARACTER
        # TABULATION should similarly be escaped as "\t"; also, it is unclear whether such
        # characters should also be escaped at the end):
        s_value = "\\s" + s_value[1:]
    return s_value


parser = argparse.ArgumentParser()
parser.add_argument("-p", dest="productname", default="LibreOffice")
parser.add_argument("-d", dest="workdir", default=".")
parser.add_argument("--key", dest="key")
parser.add_argument("--prefix", dest="prefix", default="")
parser.add_argument("--ext", dest="ext")
parser.add_argument("--template-dir", dest="template_dir", default=None)
parser.add_argument("ifile")

o = parser.parse_args()

if o.template_dir is None:
    template_dir = f"{o.workdir}/{o.prefix}"
else:
    template_dir = o.template_dir

# hack for unity section
if o.key == "UnityQuickList":
    OUTKEY = "Name"
else:
    OUTKEY = o.key


templates = {}

# open input file
source = io.open(o.ifile, encoding="utf-8")

template = None

# read ulf file
for line in source:
    if line.strip() == "":
        continue
    if line[0] == "[":
        template = line.split("]", 1)[0][1:]
        entry = {}
        # For every section in the specified ulf file there should exist
        # a template file in $workdir ..
        entry["outfile"] = f"{template_dir}{template}.{o.ext}"
        entry["translations"] = {}
        templates[template] = entry
    else:
        # split locale = "value" into 2 strings
        if " = " not in line:
            continue
        locale, value = line.split(" = ")

        if locale != line:
            # replace en-US with en
            locale = locale.replace("en-US", "en")

            # use just anything inside the ""
            assert value[0] == '"'
            # Some entries span multiple lines.
            # An entry will always end on a double quote.
            while not value.endswith('"\n'):
                value += source.readline()
            value = value[1:-2]

            # replace resource placeholder
            value = value.replace("%PRODUCTNAME", o.productname)

            locale = locale.replace("-", "_")

            templates[template]["translations"][locale] = value

source.close()

processed = 0
# process templates
for template, entries in templates.items():
    outfilename = entries["outfile"]

    # open the template file - ignore sections for which no
    # templates exist
    try:
        template_file = io.open(outfilename, encoding="utf-8")
    except OSError:
        # string files processed one by one
        if o.ext == "str":
            continue
        sys.exit(
            f"Warning: No template found for item '{template}' : '{outfilename}'\n"
        )
    processed += 1

    # open output file
    tmpfilename = f"{outfilename}.tmp"
    outfile = io.open(tmpfilename, "w", encoding="utf-8")

    # emit the template to the output file
    for line in template_file:
        keyline = line
        if keyline.startswith(o.key):
            keyline = OUTKEY + keyline[len(o.key) :]
        outfile.write(keyline)
        if o.key in line:
            translations = entries["translations"]
            for locale in sorted(translations.keys()):
                value = translations.get(locale, None)
                # print "locale is $locale\n";
                # print "value is $value\n";
                if value:
                    if o.ext in ("desktop", "str"):
                        if o.ext == "desktop":
                            value = encode_desktop_string(value)
                        outfile.write(f"{OUTKEY}[{locale}]={value}\n")
                    else:
                        outfile.write(f"\t[{locale}]{OUTKEY}={value}\n")

    template_file.close()

    outfile.close()
    if os.path.exists(outfilename):
        os.unlink(outfilename)
    os.rename(tmpfilename, outfilename)

if o.ext == "str" and processed == 0:
    sys.exit("Warning: No matching templates processed")
