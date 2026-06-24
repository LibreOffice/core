#!/usr/bin/env python3

import zipfile
import xml.etree.ElementTree as ET
from io import BytesIO
import re
import os
import sys
from glob import glob
import polib

from extract_odf_text import extract_underscore_text


def locale_for(lang_code):
    return lang_code.replace("_", "-")


def po_prefix_for(odf_file):
    """Calc-Intro-Template.ods -> 'Calc-Intro-Template'."""
    return os.path.splitext(os.path.basename(odf_file))[0]


def build_source_en_us(odf_file):
    """en-US entries are the document's own English source: the "_"-marked text
    with the leading "_" stripped. Keys keep the "_" so they match the runtime
    lookup (and the parse_po_file keys)."""
    return {s: s.lstrip("_") for s in extract_underscore_text(odf_file)}


def parse_po_file(po_file_path):
    """Parse a .po file and return {"_" + msgid: msgstr} for translated entries."""
    translations = {}
    try:
        po = polib.pofile(po_file_path)
        for entry in po:
            if entry.msgid and entry.msgstr:
                translations["_" + entry.msgid] = entry.msgstr
        print(
            f"  Found {len(translations)} translations in {os.path.basename(po_file_path)}"
        )
        return translations
    except Exception as e:
        print(f"  Error parsing {po_file_path}: {e}")
        import traceback

        traceback.print_exc()
        return {}


def extract_language_from_filename(filename, prefix):
    """'Calc-Intro-Template-de.po' with prefix 'Calc-Intro-Template' -> 'de'."""
    match = re.search(rf"{re.escape(prefix)}-(.+?)\.po$", filename)
    return match.group(1) if match else None


def detect_available_locales(po_directory, prefix):
    """Return the sorted list of locales for which a <prefix>-<lang>.po exists."""
    po_files = glob(os.path.join(po_directory, f"{prefix}-*.po"))
    available = []
    for po_file in sorted(po_files):
        lang_code = extract_language_from_filename(os.path.basename(po_file), prefix)
        if lang_code:
            locale = locale_for(lang_code)
            available.append(locale)
            print(f"  Detected: {os.path.basename(po_file)} -> {locale}")
    return sorted(available)


def collect_translations(po_directory, prefix, seed_translations):
    """Build {locale: {key: translation}}, seeded with en-US from the source."""
    locale_translations = dict(seed_translations or {})

    po_files = glob(os.path.join(po_directory, f"{prefix}-*.po"))
    if po_files:
        print(f"Found {len(po_files)} PO files")
        for po_file in sorted(po_files):
            lang_code = extract_language_from_filename(
                os.path.basename(po_file), prefix
            )
            if not lang_code:
                continue
            locale = locale_for(lang_code)
            print(f"Parsing {os.path.basename(po_file)} for locale {locale}")
            locale_translations[locale] = parse_po_file(po_file)
    else:
        print(f"No PO files found for '{prefix}' in {po_directory} (en-US only)")

    return locale_translations


def build_l10n_content(locale_translations, supported_locales):
    """Render the plain-text 'l10n' stream from the collected translations."""
    if not locale_translations:
        print("No translations were loaded")
        return None

    all_msgids = set()
    for translations in locale_translations.values():
        all_msgids.update(translations.keys())
    all_msgids = sorted(all_msgids)

    print(f"Found {len(all_msgids)} unique message IDs across all locales")

    # Format is <opt-prefix|string>\nlocale\t<translated-string>\n<repeat>\n\n
    l10n_lines = [
        "# supported locales",
        ",".join(supported_locales),
        "",
        "# strings and their translations",
    ]

    for msgid in all_msgids:
        if not msgid or len(msgid) > 500:
            continue

        l10n_lines.append("")
        l10n_lines.append(msgid)

        for locale in supported_locales:
            if locale in locale_translations and msgid in locale_translations[locale]:
                translation = locale_translations[locale][msgid]
                translation = translation.replace("\n", " ").replace("\t", " ")
                l10n_lines.append(f"{locale}\t{translation}")

    l10n_content = "\n".join(l10n_lines)
    print(f"Generated l10n content: {len(l10n_content)} bytes, {len(all_msgids)} entries")
    return l10n_content


def add_l10n_to_odf(odf_path, l10n_content):
    """Add (or replace) the 'l10n' stream in an ODF file and register it in the manifest."""
    if not os.path.exists(odf_path):
        print(f"File not found: {odf_path}")
        return False

    try:
        with zipfile.ZipFile(odf_path, "r") as zip_read:
            try:
                manifest_data = zip_read.read("META-INF/manifest.xml")
            except KeyError:
                print("manifest.xml not found in document")
                return False

            ET.register_namespace(
                "manifest", "urn:oasis:names:tc:opendocument:xmlns:manifest:1.0"
            )
            root = ET.fromstring(manifest_data)
            ns = {"manifest": "urn:oasis:names:tc:opendocument:xmlns:manifest:1.0"}

            # Remove existing l10n entry if present
            for entry in root.findall("manifest:file-entry", ns):
                if (
                    entry.get(
                        "{urn:oasis:names:tc:opendocument:xmlns:manifest:1.0}full-path"
                    )
                    == "l10n"
                ):
                    root.remove(entry)
                    print("Removed existing l10n entry from manifest")

            new_entry = ET.Element(
                "{urn:oasis:names:tc:opendocument:xmlns:manifest:1.0}file-entry"
            )
            new_entry.set(
                "{urn:oasis:names:tc:opendocument:xmlns:manifest:1.0}full-path", "l10n"
            )
            new_entry.set(
                "{urn:oasis:names:tc:opendocument:xmlns:manifest:1.0}media-type",
                "text/plain",
            )
            root.append(new_entry)
            print("Added l10n entry to manifest")

            new_manifest = ET.tostring(root, encoding="utf-8")

            temp_data = BytesIO()
            with zipfile.ZipFile(temp_data, "w", zipfile.ZIP_DEFLATED) as zip_write:
                for item in zip_read.infolist():
                    if item.filename not in ["l10n", "META-INF/manifest.xml"]:
                        zip_write.writestr(item, zip_read.read(item.filename))
                zip_write.writestr("META-INF/manifest.xml", new_manifest)
                zip_write.writestr("l10n", l10n_content)
                print(f"Added l10n stream: {len(l10n_content)} bytes")

        with open(odf_path, "wb") as f:
            f.write(temp_data.getvalue())

        print(f"Successfully updated {odf_path}")
        return True

    except Exception as e:
        print(f"Error: {e}")
        import traceback

        traceback.print_exc()
        return False


if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("usage: create_l10n_intro_docs.py <po_directory> <odf_file>")
        sys.exit(1)

    po_directory = sys.argv[1]
    odf_file = sys.argv[2]
    prefix = po_prefix_for(odf_file)

    print("=" * 60)
    print(f"Embedding l10n into: {odf_file}")
    print(f"PO directory: {po_directory}   PO prefix: {prefix}-*.po")
    print("=" * 60)

    po_locales = detect_available_locales(po_directory, prefix)
    # en-US first; it always exists (the document's own source text).
    supported_locales = ["en-US"] + [l for l in po_locales if l != "en-US"]
    print(f"Supported locales: {supported_locales}")

    locale_translations = collect_translations(
        po_directory,
        prefix,
        seed_translations={"en-US": build_source_en_us(odf_file)},
    )

    l10n_content = build_l10n_content(locale_translations, supported_locales)
    if l10n_content:
        add_l10n_to_odf(odf_file, l10n_content)
    else:
        print("Failed to create l10n content")
