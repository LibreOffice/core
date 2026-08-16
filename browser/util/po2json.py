#!/usr/bin/env python3
"""Convert .po to .json."""

import json
import optparse
import os
import sys
import errno
import re
import polib

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import l10n_html_check

parser = optparse.OptionParser(usage="usage: %prog [options] pofile...")
parser.add_option("--quiet",
                  action="store_false",
                  default=True,
                  dest="verbose",
                  help="don't print status messages to stdout")
parser.add_option("-o",
                  type="string",
                  default="",
                  dest="destfile",
                  help="output file name (if there is exactly one input file)")

(options, args) = parser.parse_args()

if args is None or len(args) == 0:
    print("ERROR: you must specify at least one po file to translate")
    sys.exit(1)

if options.destfile != '' and len(args) != 1:
    print("ERROR: when -o is provided, there has to be exactly 1 input file")
    sys.exit(1)

paramFix = re.compile("(\\(([0-9])\\))")

# Plural entries are keyed the way a MO file keys them - the two source strings
# joined by a NUL - and hold the translated forms joined by NUL as well. The
# language's plural rule rides along under a reserved NUL key. A NUL can never
# occur in a msgid, so neither key can collide with a real string. See
# browser/js/plural.js, which does the lookup for _n().
NUL = "\u0000"
RULE_KEY = NUL + "plural-forms"

for srcfile in args:

    destfile = os.path.splitext(srcfile)[0] + ".json"
    if options.destfile != '':
        destfile = options.destfile

    if options.verbose:
        print("INFO: converting %s to %s" % (srcfile, destfile))

    xlate_map = {}

    po = polib.pofile(srcfile,
                      autodetect_encoding=False,
                      encoding="utf-8",
                      wrapwidth=-1)
    for entry in po.translated_entries():
        if entry.msgid_plural:
            forms = [entry.msgstr_plural[key]
                     for key in sorted(entry.msgstr_plural, key=int)]

            # Every form is checked against the source it is a form of, so a
            # bad one drops the whole entry back to the English forms.
            problems = []
            for index, form in enumerate(forms):
                source = entry.msgid if index == 0 else entry.msgid_plural
                problems += l10n_html_check.check_string(source, form)
            if problems:
                sys.stderr.write(
                    "WARNING: dropping translation with unexpected HTML in %s: %s\n"
                    "  msgid : %r\n  msgstr: %r\n"
                    % (srcfile, "; ".join(problems), entry.msgid, forms))
                continue

            xlate_map[entry.msgid + NUL + entry.msgid_plural] = NUL.join(forms)
            # Keep a plain lookup of the singular working, in case the same
            # string is also passed to _() somewhere.
            xlate_map.setdefault(entry.msgid, forms[0])
            continue

        if entry.msgstr == '':
            continue

        # Some translated strings are rendered as HTML. If a translation uses
        # HTML markup that its source string does not (see l10n_html_check),
        # warn and drop just that string - it falls back to the English
        # original - rather than failing the whole conversion.
        problems = l10n_html_check.check_string(entry.msgid, entry.msgstr)
        if problems:
            sys.stderr.write(
                "WARNING: dropping translation with unexpected HTML in %s: %s\n"
                "  msgid : %r\n  msgstr: %r\n"
                % (srcfile, "; ".join(problems), entry.msgid, entry.msgstr))
            continue

        xlate_map[entry.msgid] = entry.msgstr

    plural_forms = po.metadata.get("Plural-Forms", "").strip()
    if plural_forms:
        xlate_map[RULE_KEY] = plural_forms

    if not os.path.exists(os.path.dirname(destfile)):
        try:
            os.makedirs(os.path.dirname(destfile))
        except OSError as exc:  # Guard against race condition
            if exc.errno != errno.EEXIST:
                raise

    dest = open(destfile, "w")

    dest.write(json.dumps(xlate_map, sort_keys=True))

    dest.close()
