#!/usr/bin/env python3
# Copyright the Collabora Online contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

"""Check that translations only use the HTML markup present in their source.

Some ui-*.po / help-*.po strings are rendered as HTML rather than as plain
text, and some *source* strings legitimately contain markup - keyboard
shortcuts wrapped in <kbd>, help pages with <p>/<a>/<table>, etc. So we cannot
simply forbid all HTML in translations; instead we keep a translation's markup
consistent with its own source.

The rule:

  A translation may only use the HTML tags that already appear in its own
  source string, and must not add <script>/<iframe>/... elements, on*
  event-handler attributes or javascript:/vbscript:/data: URL schemes that the
  source does not have.

Checking this once, when the translations are converted, lets the rest of the
code use the translated strings as-is.

Usable both as a library (check_entries / check_pofile) and as a CLI:

    ./l10n_html_check.py po/ui-*.po
"""

import re
import sys

# A tag is '<' or '</' immediately followed by a name. No whitespace is allowed
# between '<' and the name so that mathematical text such as "a < b" is not
# mistaken for a tag.
_TAG_RE = re.compile(r"<(/?)([a-zA-Z][a-zA-Z0-9-]*)")

# Full opening/closing/self-closing tag, used to inspect attributes.
_FULL_TAG_RE = re.compile(r"<[a-zA-Z/!][^>]*>")

# Active markup a translation must not add regardless of the source string:
# script-like elements, event-handler attributes and script URL schemes.
_ACTIVE_MARKUP_RE = re.compile(
    r"""(?xi)
      <\s*(?:script|iframe|object|embed|form|meta|link|style|svg|math|base|applet)\b  # active elements
    | \son[a-z]+\s*=                            # event-handler attributes: onclick=, onerror=, ...
    | =\s*["']?\s*(?:javascript|vbscript|data)\s*:   # script URL scheme in an attribute value
    """
)


def _tag_names(text):
    """Return the set of lower-cased HTML tag names used in *text*."""
    return {m.group(2).lower() for m in _TAG_RE.finditer(text)}


def check_string(msgid, msgstr):
    """Check one source/translation pair.

    Returns a list of human-readable problem descriptions (empty if the
    translation is fine).
    """
    problems = []

    if not msgstr:
        return problems

    # 1) Tags that the translation introduces but the source does not have.
    src_tags = _tag_names(msgid)
    new_tags = sorted(t for t in _tag_names(msgstr) if t not in src_tags)
    if new_tags:
        problems.append(
            "introduces HTML tag(s) not present in the source string: "
            + ", ".join("<%s>" % t for t in new_tags)
        )

    # 2) Active markup, wherever it appears.
    for m in _ACTIVE_MARKUP_RE.finditer(msgstr):
        if not _ACTIVE_MARKUP_RE.search(msgid):
            problems.append("adds active markup: %r" % m.group(0).strip())
            break

    return problems


def check_entries(entries, srcfile=""):
    """Check translated polib entries. Returns a list of violation dicts."""
    violations = []
    for entry in entries:
        # Both singular and (when present) plural translations are checked.
        pairs = []
        if entry.msgstr:
            pairs.append((entry.msgid, entry.msgstr))
        for idx, plural in (entry.msgstr_plural or {}).items():
            pairs.append((entry.msgid_plural or entry.msgid, plural))

        for msgid, msgstr in pairs:
            for problem in check_string(msgid, msgstr):
                violations.append(
                    {
                        "file": srcfile,
                        "msgid": msgid,
                        "msgstr": msgstr,
                        "problem": problem,
                    }
                )
    return violations


def check_pofile(po, srcfile=""):
    """Check an already-parsed polib.POFile. Returns a list of violation dicts."""
    return check_entries(po.translated_entries(), srcfile)


def format_violations(violations):
    """Render violations as a human-readable, multi-line string."""
    lines = []
    for v in violations:
        lines.append("%s:" % (v["file"] or "<po>"))
        lines.append("  problem: %s" % v["problem"])
        lines.append("  msgid  : %r" % v["msgid"])
        lines.append("  msgstr : %r" % v["msgstr"])
    return "\n".join(lines)


def main(argv):
    import polib

    if len(argv) < 2:
        sys.stderr.write("usage: %s pofile...\n" % argv[0])
        return 2

    all_violations = []
    for srcfile in argv[1:]:
        po = polib.pofile(
            srcfile, autodetect_encoding=False, encoding="utf-8", wrapwidth=-1
        )
        all_violations.extend(check_pofile(po, srcfile))

    if all_violations:
        sys.stderr.write(
            "ERROR: unexpected HTML found in %d translated string(s):\n"
            % len(all_violations)
        )
        sys.stderr.write(format_violations(all_violations) + "\n")
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
