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

    if not os.path.exists(os.path.dirname(destfile)):
        try:
            os.makedirs(os.path.dirname(destfile))
        except OSError as exc:  # Guard against race condition
            if exc.errno != errno.EEXIST:
                raise

    dest = open(destfile, "w")

    dest.write(json.dumps(xlate_map, sort_keys=True))

    dest.close()
