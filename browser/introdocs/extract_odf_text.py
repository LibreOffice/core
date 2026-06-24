#!/usr/bin/env python3
"""
Extract all text starting with "_" from an ODF file and generate a .pot file.
"""

import zipfile
import xml.etree.ElementTree as ET
from typing import List
from datetime import datetime

import polib


def extract_underscore_text(odf_file_path: str) -> List[str]:
    """
    Extract all text starting with '_' from an ODF file.

    Args:
        odf_file_path: Path to the ODF file

    Returns:
        List of text strings that start with '_'
    """
    texts = []

    try:
        with zipfile.ZipFile(odf_file_path, "r") as zip_ref:
            if "content.xml" not in zip_ref.namelist():
                print("Error: content.xml not found in ODF file")
                return texts

            content_xml = zip_ref.read("content.xml")
            root = ET.fromstring(content_xml)

            namespaces = {
                "text": "urn:oasis:names:tc:opendocument:xmlns:text:1.0",
            }

            # Find all text:span elements
            for span in root.findall(".//text:span", namespaces):
                if span.text and span.text.startswith("_") and span.text not in texts:
                    texts.append(span.text)

            # Also check text:p (paragraphs) and text:h (headings)
            for element in root.findall(".//text:p", namespaces):
                if element.text and element.text.startswith("_") and element.text not in texts:
                    texts.append(element.text)

            for element in root.findall(".//text:h", namespaces):
                if element.text and element.text.startswith("_") and element.text not in texts:
                    texts.append(element.text)

    except FileNotFoundError:
        print(f"Error: File '{odf_file_path}' not found")
    except zipfile.BadZipFile:
        print(f"Error: '{odf_file_path}' is not a valid ZIP/ODF file")
    except ET.ParseError as e:
        print(f"Error parsing XML: {e}")

    marker_only = [t for t in texts if not t.lstrip("_")]
    for t in marker_only:
        print(
            f"Warning: skipping marker-only text {t!r}; keep the '_' in the"
            " same text span as the string it marks"
        )
    return [t for t in texts if t.lstrip("_")]


def create_po_file(
    texts: List[str],
    output_file: str,
    source_location: str,
    project_name: str = "PACKAGE VERSION",
) -> None:
    """
    Create a .pot file matching the project format.

    Args:
        texts: List of text strings to translate
        output_file: Path to output .pot file
        source_location: Source location string
        project_name: Project name for the POT file header
    """
    now = datetime.now()
    pot_creation_date = now.strftime("%Y-%m-%d %H:%M+0000")

    # polib takes care of gettext escaping (quotes, backslashes, newlines).
    pot = polib.POFile()
    pot.metadata = {
        "Project-Id-Version": project_name,
        "Report-Msgid-Bugs-To": "",
        "POT-Creation-Date": pot_creation_date,
        "PO-Revision-Date": "YEAR-MO-DA HO:MI+ZONE",
        "Last-Translator": "FULL NAME <EMAIL@ADDRESS>",
        "Language-Team": "LANGUAGE <LL@li.org>",
        "Language": "",
        "MIME-Version": "1.0",
        "Content-Type": "text/plain; charset=UTF-8",
        "Content-Transfer-Encoding": "8bit",
    }

    seen = set()
    for i, text in enumerate(texts, 1):
        # Remove leading underscore for msgid
        msgid_text = text.lstrip("_")
        if msgid_text in seen:
            continue
        seen.add(msgid_text)
        pot.append(
            polib.POEntry(msgid=msgid_text, msgstr="", occurrences=[(source_location, i)])
        )

    try:
        pot.save(output_file)
        print(f"POT file created: {output_file}")
        print(f"Total entries: {len(pot)}")
    except IOError as e:
        print(f"Error writing POT file: {e}")


if __name__ == "__main__":
    import sys

    if len(sys.argv) < 3:
        print("usage: extract_odf_text.py <odf_file> <output.pot>")
        sys.exit(1)

    odf_path = sys.argv[1]
    pot_path = sys.argv[2]

    print(f"Extracting text starting with '_' from: {odf_path}\n")
    texts = extract_underscore_text(odf_path)

    if texts:
        print(f"Found {len(texts)} text(s) starting with '_':")
        for i, text in enumerate(texts, 1):
            print(f"  {i}. {text}")

        print("\nCreating POT file...")
        create_po_file(texts, pot_path, odf_path)
    else:
        print("No text starting with '_' found.")
