#!/usr/bin/env python3
"""Embed the translations for an intro template as an "l10n" zip stream.

Stream format ("#" starts a comment, blank lines separate blocks):

    # supported locales
    de,fr
    # strings and their translations

    <1>3. Press </1><2>Enter</2><3> to calculate</3>
    de\t<1>3. Druecken Sie </1><2>Enter</2><3> zum Rechnen</3>

Keys are the .pot msgids verbatim, without the "_" marker; a key the
stream misses just gets its marker stripped, so en-US needs no entries.
The stream is embedded even with no translations yet - its presence is
what marks the file as translatable - and the locale list is then the
placeholder "-", which matches no real locale.

Every member with markers is also embedded a second time, unchanged, as an
l10n template (content.xml -> l10n_template.xml, styles.xml ->
l10n_template_styles.xml) for the engine to rewrite from, so the file
re-translates on later opens until the first user save drops the machinery.
"""

import os
import re
import sys
import zipfile
import xml.etree.ElementTree as ET
from glob import glob
from io import BytesIO

import polib

from l10n_odf import (
    KEY_MAX_LENGTH,
    analyze_document,
    is_xml_safe_text,
    msgid_tag_count,
    parse_msgstr_tokens,
)

SHEET_NAME_FORBIDDEN = set("[]*?:/\\")

# the l10n template embedded for each member that carries markers
TEMPLATE_MEMBERS = {
    "content.xml": "l10n_template.xml",
    "styles.xml": "l10n_template_styles.xml",
}


def locale_for(lang_code):
    return lang_code.replace("_", "-")


def po_prefix_for(odf_file):
    """Calc-Intro-Template.ods -> 'Calc-Intro-Template'."""
    return os.path.splitext(os.path.basename(odf_file))[0]


def extract_language_from_filename(filename, prefix):
    """'Calc-Intro-Template-de.po' with prefix 'Calc-Intro-Template' -> 'de'."""
    match = re.search(rf"{re.escape(prefix)}-(.+?)\.po$", filename)
    return match.group(1) if match else None


def parse_po_file(po_file_path):
    """Return {msgid: msgstr} for the translated entries, skipping fuzzy and
    obsolete ones (polib iteration includes both)."""
    translations = {}
    po = polib.pofile(po_file_path)
    skipped = 0
    for entry in po:
        if not entry.msgid or not entry.msgstr:
            continue
        if "fuzzy" in entry.flags or entry.obsolete:
            skipped += 1
            continue
        translations[entry.msgid] = entry.msgstr
    print(f"  {len(translations)} translations in {os.path.basename(po_file_path)}"
          + (f" ({skipped} fuzzy/obsolete skipped)" if skipped else ""))
    return translations


def validate_translations(locale, translations, units, problems):
    """Drop invalid entries loudly; append hard failures to problems."""
    # one msgid can serve several kinds at once: the same text is both a
    # sheet name and a paragraph in the Calc template
    unit_kinds = {}
    for unit in units:
        unit_kinds.setdefault(unit.key, set()).add(unit.kind)

    valid = {}
    for msgid, msgstr in translations.items():
        where = f"[{locale}] {msgid!r}"
        if msgid not in unit_kinds:
            print(f"  warning: {where}: not in the document (stale entry?); "
                  "skipped", file=sys.stderr)
            continue
        if len(msgid) > KEY_MAX_LENGTH or len(msgstr) > KEY_MAX_LENGTH:
            problems.append(f"{where}: longer than {KEY_MAX_LENGTH} characters")
            continue
        if "\n" in msgstr or "\t" in msgstr:
            print(f"  warning: {where}: translation contains a newline or "
                  "tab; flattened to spaces", file=sys.stderr)
            msgstr = msgstr.replace("\n", " ").replace("\t", " ")
        if "  " in msgstr:
            print(f"  warning: {where}: translation contains consecutive "
                  "spaces", file=sys.stderr)
        # a control character here would be written into content.xml verbatim
        # and the document would stop parsing; the engine drops such an entry
        # too, this only makes it visible before anything ships
        if not is_xml_safe_text(msgstr):
            print(f"  warning: {where}: translation contains a character "
                  "that is illegal in XML; skipped", file=sys.stderr)
            continue
        tokens = parse_msgstr_tokens(msgstr, msgid_tag_count(msgid))
        if tokens is None:
            print(f"  warning: {where}: translation's <N> tags do not match "
                  "the msgid's spans; skipped", file=sys.stderr)
            continue
        # "<1></1>" is not an empty msgstr, but it holds no text either, and
        # the engine would rebuild the paragraph as an empty one
        if not any(text for _, text in tokens):
            print(f"  warning: {where}: translation holds no text, only "
                  "<N> tags; skipped", file=sys.stderr)
            continue
        if "sheet-name" in unit_kinds[msgid] and set(msgstr) & SHEET_NAME_FORBIDDEN:
            if unit_kinds[msgid] == {"sheet-name"}:
                print(f"  warning: {where}: sheet name translation contains "
                      "one of []*?:/\\; skipped", file=sys.stderr)
                continue
            # the text is also used elsewhere, so the translation still ships
            # and the engine keeps the English name for the sheet itself
            print(f"  warning: {where}: this text names a sheet too and the "
                  "translation contains one of []*?:/\\; the sheet keeps "
                  "its English name", file=sys.stderr)
        valid[msgid] = msgstr
    return valid


def build_l10n_content(locales, translations_by_locale):
    """Render the stream text; locales in the given order."""
    lines = [
        "# supported locales",
        ",".join(locales) if locales else "-",
        "# strings and their translations",
    ]
    all_msgids = sorted({msgid
                         for translations in translations_by_locale.values()
                         for msgid in translations})
    for msgid in all_msgids:
        lines.append("")
        lines.append(msgid)
        for locale in locales:
            msgstr = translations_by_locale.get(locale, {}).get(msgid)
            if msgstr is not None:
                lines.append(f"{locale}\t{msgstr}")
    lines.append("")
    return "\n".join(lines)


def add_l10n_to_odf(odf_path, l10n_content, template_members):
    """Add (or replace) the 'l10n' stream and the l10n templates of
    template_members ({member: template_name}) in the ODF file, registering
    everything in the manifest and preserving all other members in order."""
    added = dict(template_members)
    replaced = {"l10n", "META-INF/manifest.xml", *added.values()}

    with zipfile.ZipFile(odf_path, "r") as zip_read:
        manifest_ns = "urn:oasis:names:tc:opendocument:xmlns:manifest:1.0"
        ET.register_namespace("manifest", manifest_ns)
        root = ET.fromstring(zip_read.read("META-INF/manifest.xml"))
        new_paths = {"l10n": "text/plain"}
        new_paths.update((name, "text/xml") for name in added.values())
        for entry in root.findall(f"{{{manifest_ns}}}file-entry"):
            if entry.get(f"{{{manifest_ns}}}full-path") in new_paths:
                root.remove(entry)
        for path, media_type in new_paths.items():
            new_entry = ET.Element(f"{{{manifest_ns}}}file-entry")
            new_entry.set(f"{{{manifest_ns}}}full-path", path)
            new_entry.set(f"{{{manifest_ns}}}media-type", media_type)
            root.append(new_entry)
        new_manifest = ET.tostring(root, encoding="utf-8")

        # snapshot before the copy loop: writestr(item, ...) mutates the
        # ZipInfo objects zip_read uses to locate its own members
        template_data = {template_name: zip_read.read(member)
                         for member, template_name in added.items()}

        def epoch_entry(name):
            """A ZipInfo dated to the zip epoch (1980-01-01), so members we
            create carry no build timestamp and embedding is deterministic."""
            info = zipfile.ZipInfo(name)
            info.compress_type = zipfile.ZIP_DEFLATED
            return info

        temp_data = BytesIO()
        with zipfile.ZipFile(temp_data, "w", zipfile.ZIP_DEFLATED) as zip_write:
            for item in zip_read.infolist():
                if item.filename not in replaced:
                    zip_write.writestr(item, zip_read.read(item.filename))
            zip_write.writestr(epoch_entry("META-INF/manifest.xml"), new_manifest)
            zip_write.writestr(epoch_entry("l10n"), l10n_content)
            for template_name, data in template_data.items():
                zip_write.writestr(epoch_entry(template_name), data)

    with open(odf_path, "wb") as output_file:
        output_file.write(temp_data.getvalue())


def main():
    if len(sys.argv) < 3:
        print("usage: create_l10n_intro_docs.py <po_directory> <odf_file>")
        return 1

    po_directory, odf_file = sys.argv[1], sys.argv[2]
    prefix = po_prefix_for(odf_file)
    print(f"Embedding l10n into {odf_file} "
          f"(po files: {po_directory}/{prefix}-*.po)")

    units, errors, warnings = analyze_document(odf_file)
    for warning in warnings:
        print(f"warning: {warning}", file=sys.stderr)
    if errors:
        for error in errors:
            print(f"error: {error}", file=sys.stderr)
        print(f"{len(errors)} lint error(s) in {odf_file}; fix the document "
              "first", file=sys.stderr)
        return 1

    problems = []
    translations_by_locale = {}
    for po_file in sorted(glob(os.path.join(po_directory, f"{prefix}-*.po"))):
        lang_code = extract_language_from_filename(os.path.basename(po_file), prefix)
        if not lang_code:
            continue
        locale = locale_for(lang_code)
        print(f"Parsing {os.path.basename(po_file)} for locale {locale}")
        translations = validate_translations(
            locale, parse_po_file(po_file), units, problems)
        if translations:
            translations_by_locale[locale] = translations

    if problems:
        for problem in problems:
            print(f"error: {problem}", file=sys.stderr)
        return 1

    locales = sorted(translations_by_locale)
    content = build_l10n_content(locales, translations_by_locale)
    marked_members = {unit.where.split(" ")[0] for unit in units}
    template_members = {member: TEMPLATE_MEMBERS[member]
                        for member in sorted(marked_members)
                        if member in TEMPLATE_MEMBERS}
    add_l10n_to_odf(odf_file, content, template_members)
    print(f"Embedded l10n stream: {len(content)} bytes, "
          f"locales: {','.join(locales) if locales else '(none)'}; "
          f"template copies: {', '.join(template_members.values()) or '(none)'}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
