#!/usr/bin/env python3
"""Extract the "_"-marked translatable strings from an ODF template into a
.pot file.

Key derivation and the <N> span-tag msgid format are defined in
l10n_odf.py.  Marker spec violations reject the document with exit code 1.
--check compares the fresh .pot against the committed one instead of
writing it; a stale file prints a diff and exits 2.  Only the entries are
compared: polib versions order the header fields differently.
"""

import argparse
import difflib
import os
import sys

import polib

from l10n_odf import analyze_document


def build_pot(units, source_name):
    pot = polib.POFile()
    # no POT-Creation-Date: regenerating an unchanged document must be
    # byte-stable (--check), and the tooling embeds no timestamps anywhere
    pot.metadata = {
        "Project-Id-Version": source_name,
        "Report-Msgid-Bugs-To": "",
        "PO-Revision-Date": "YEAR-MO-DA HO:MI+ZONE",
        "Last-Translator": "FULL NAME <EMAIL@ADDRESS>",
        "Language-Team": "LANGUAGE <LL@li.org>",
        "Language": "",
        "MIME-Version": "1.0",
        "Content-Type": "text/plain; charset=UTF-8",
        "Content-Transfer-Encoding": "8bit",
    }

    # document order, duplicates merged with their contexts combined
    entries = {}
    for unit in units:
        entry = entries.get(unit.key)
        if entry is None:
            entry = polib.POEntry(msgid=unit.key, msgstr="", comment=unit.where)
            entries[unit.key] = entry
            pot.append(entry)
        elif unit.where not in entry.comment.split("; "):
            entry.comment += "; " + unit.where
    return pot


def entry_keys(pot):
    return [(entry.msgctxt, entry.msgid, entry.comment) for entry in pot]


def main():
    parser = argparse.ArgumentParser(
        description="Extract '_'-marked strings from an ODF file into a .pot")
    parser.add_argument("--check", action="store_true",
                        help="compare against the existing .pot instead of "
                             "writing; exit 2 when it is stale")
    parser.add_argument("odf_file")
    parser.add_argument("pot_file")
    args = parser.parse_args()

    units, errors, warnings = analyze_document(args.odf_file)
    for warning in warnings:
        print(f"warning: {warning}", file=sys.stderr)
    if errors:
        for error in errors:
            print(f"error: {error}", file=sys.stderr)
        print(f"{len(errors)} lint error(s) in {args.odf_file}; not writing "
              "a .pot", file=sys.stderr)
        return 1

    pot = build_pot(units, os.path.basename(args.odf_file))
    generated = str(pot)

    if args.check:
        try:
            with open(args.pot_file, encoding="utf-8") as existing_file:
                existing = existing_file.read()
        except FileNotFoundError:
            print(f"{args.pot_file} does not exist; run "
                  f"extract_odf_text.py {args.odf_file} {args.pot_file}",
                  file=sys.stderr)
            return 2
        if (entry_keys(polib.pofile(existing))
                != entry_keys(polib.pofile(generated))):
            sys.stdout.writelines(difflib.unified_diff(
                existing.splitlines(keepends=True),
                generated.splitlines(keepends=True),
                fromfile=f"{args.pot_file} (committed)",
                tofile=f"{args.pot_file} (regenerated)"))
            print(f"{args.pot_file} is stale; regenerate it with "
                  f"extract_odf_text.py {args.odf_file} {args.pot_file}",
                  file=sys.stderr)
            return 2
        print(f"{args.pot_file} is up to date "
              f"({len(pot)} entries from {len(units)} units)")
        return 0

    with open(args.pot_file, "w", encoding="utf-8") as output_file:
        output_file.write(generated)
    print(f"POT file created: {args.pot_file}")
    print(f"Total entries: {len(pot)} (from {len(units)} units)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
