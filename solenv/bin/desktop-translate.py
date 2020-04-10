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

#
# Translates multiple .desktop files at once with strings from .ulf
# files; if you add new translatable .ulf files please add them to
# l10ntools/source/localize.cxx
#

import os, sys, argparse, io

parser = argparse.ArgumentParser()
parser.add_argument('-p', dest='productname', default='LibreOffice')
parser.add_argument('-d', dest='workdir', default='.')
parser.add_argument('--key', dest='key')
parser.add_argument('--prefix', dest='prefix', default='')
parser.add_argument('--ext', dest='ext')
parser.add_argument('--template-dir', dest='template_dir', default=None)
parser.add_argument('ifile')

o = parser.parse_args()

if o.template_dir is None:
    template_dir = '{}/{}'.format(o.workdir, o.prefix)
else:
    template_dir = o.template_dir

# hack for unity section
if o.key == "UnityQuickList":
    outkey = "Name"
else:
    outkey = o.key


templates = {}

# open input file
source = io.open(o.ifile, encoding='utf-8')

template = None

# read ulf file
for line in source:
    if line.strip() == '':
        continue
    if line[0] == "[":
        template = line.split(']', 1)[0][1:]
        entry = {}
        # For every section in the specified ulf file there should exist
        # a template file in $workdir ..
        entry['outfile'] = "{}{}.{}".format(template_dir, template, o.ext)
        entry['translations'] = {}
        templates[template] = entry
    else:
        # split locale = "value" into 2 strings
        if ' = ' not in line:
            continue
        locale, value = line.split(' = ')

        if locale != line:
            # replace en-US with en
            locale.replace('en-US', 'en')

            # use just anything inside the ""
            value = value.strip()
            assert(value[0] == '"')
            # Some entries span multiple lines.
            # An entry will always end on a double quote.
            while not value.endswith('"'):
                value += '\n'  + source.readline().rstrip()
            value = value[1:-1]

            # replace resource placeholder
            value = value.replace('%PRODUCTNAME', o.productname)

            locale = locale.replace('-', '_')

            templates[template]['translations'][locale] = value

source.close()

processed = 0
# process templates
for template in templates:
    outfilename = templates[template]['outfile']

    # open the template file - ignore sections for which no
    # templates exist
    try:
        template_file = io.open(outfilename, encoding='utf-8')
    except Exception:
        # string files processed one by one
        if o.ext == 'str':
            continue
        sys.exit("Warning: No template found for item '{}' : '{}' : '{}': $!\n".format(template, outfile, line))
    processed += 1

    # open output file
    tmpfilename = '{}.tmp'.format(outfilename)
    outfile = io.open(tmpfilename, 'w', encoding='utf-8')

    # emit the template to the output file
    for line in template_file:
        keyline = line
        if keyline.startswith(o.key):
            keyline = keyline[len(o.key):] + outkey
        outfile.write(keyline)
        if o.key in line:
            translations = templates[template]['translations']
            for locale in sorted (translations.keys()):
                value = translations.get(locale, None)
                # print "locale is $locale\n";
                # print "value is $value\n";
                if value:
                    if o.ext == "desktop" or o.ext == "str":
                        outfile.write(u"""{}[{}]={}\n""".format(outkey, locale, value))
                    else:
                        outfile.write(u"""\t[{}]{}={}\n""".format(locale, outkey, value))

    template_file.close()

    outfile.close()
    if os.path.exists(outfilename):
        os.unlink(outfilename)
    os.rename(tmpfilename, outfilename)

if o.ext == 'str' and processed == 0:
    sys.exit("Warning: No matching templates processed")
