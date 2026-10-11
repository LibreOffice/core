#!/usr/bin/env python3
"""Render the start-center preview of each intro document, per language.

For every document here and every language its embedded l10n stream offers,
plus English, the script embeds the stream the way the build does, has the engine's
l10ntranslate tool apply the translation, re-saves the result headless with
soffice so that the file gets a fresh embedded thumbnail rendered from the
translated text, and stores that thumbnail as

    preview/<Document>.png           English, markers stripped
    preview/<Document>-<bcp47>.png   one per translated locale: -sv, -en-GB, ...

The backstage shows the file of the UI language and falls back to the
English one.  The previews are committed: run this again after a document
or its translations change, and commit the images with that change.

usage: render_previews.py [--engine <instdir>] [document ...]

The engine defaults to the in-tree engine/instdir; it has to be built, on
Linux, where it has the l10ntranslate tool.  The engine renders with the
fonts of this machine next to its own, so the machine needs a font for
every script the translations use, e.g. Noto Sans CJK for Chinese.
"""

import argparse
import os
import shutil
import subprocess
import sys
import tempfile
import zipfile
from glob import glob

from create_l10n_intro_docs import po_prefix_for

HERE = os.path.dirname(os.path.abspath(__file__))
PO_DIR = os.path.join(HERE, "po")
PREVIEW_DIR = os.path.join(HERE, "preview")
EMBED_SCRIPT = os.path.join(HERE, "create_l10n_intro_docs.py")
DEFAULT_ENGINE = os.path.normpath(
    os.path.join(HERE, "..", "..", "engine", "instdir"))
SOURCE_LANGUAGE = "en-US"


def languages_of(marked_document):
    """The locales the embedded l10n stream offers, e.g. ['de', 'en-GB'].

    They are the first non-comment line of the stream; "-" stands for none.
    A .po without a single translation is not among them.
    """
    with zipfile.ZipFile(marked_document) as package:
        stream = package.read("l10n").decode("utf-8")
    for line in stream.splitlines():
        if line and not line.startswith("#"):
            return [locale for locale in line.split(",") if locale != "-"]
    return []


def run(command):
    """Run a tool, with its stderr in the error message if it fails."""
    completed = subprocess.run(command, capture_output=True, text=True)
    if completed.returncode != 0:
        sys.exit(f"{os.path.basename(command[0])} failed:\n{completed.stderr}")
    return completed.stdout


def thumbnail_of(odf_path):
    with zipfile.ZipFile(odf_path) as package:
        return package.read("Thumbnails/thumbnail.png")


def preview_path(document, language):
    suffix = "" if language == SOURCE_LANGUAGE else "-" + language
    return os.path.join(PREVIEW_DIR, po_prefix_for(document) + suffix + ".png")


def render(document, soffice, translate, workdir):
    name = os.path.basename(document)
    extension = os.path.splitext(name)[1][1:]

    # the shipped form: the source document with the l10n stream embedded
    marked = os.path.join(workdir, "marked", name)
    os.makedirs(os.path.dirname(marked), exist_ok=True)
    shutil.copyfile(document, marked)
    run([sys.executable, EMBED_SCRIPT, PO_DIR, marked])
    languages = languages_of(marked)

    profile = "file://" + os.path.join(workdir, "profile")
    english = None
    for language in [SOURCE_LANGUAGE] + languages:
        langdir = os.path.join(workdir, language)
        translated = os.path.join(langdir, name)
        os.makedirs(langdir, exist_ok=True)
        shutil.copyfile(marked, translated)

        result = run([translate, translated, translated, language]).strip()
        expected = "translated" if language in languages else "copied untranslated"
        if result != expected:
            sys.exit(f"{name}, {language}: l10ntranslate reported "
                     f"'{result}', expected '{expected}'")

        # a headless re-save renders a new thumbnail from the translated text
        saved = os.path.join(langdir, "saved")
        run([soffice, "--headless", f"-env:UserInstallation={profile}",
             "--convert-to", extension, "--outdir", saved, translated])

        thumbnail = thumbnail_of(os.path.join(saved, name))
        target = preview_path(document, language)
        if language != SOURCE_LANGUAGE and thumbnail == english:
            # the backstage falls back to the English file anyway
            if os.path.exists(target):
                os.remove(target)
            print(f"{language}: renders like English, no file")
            continue
        if language == SOURCE_LANGUAGE:
            english = thumbnail
        with open(target, "wb") as output:
            output.write(thumbnail)
        print(f"{os.path.relpath(target, HERE)}: {len(thumbnail)} bytes")


def main():
    parser = argparse.ArgumentParser(
        description="Render the per-language previews of the intro documents.")
    parser.add_argument("--engine", default=DEFAULT_ENGINE,
                        help="the engine instdir (default: %(default)s)")
    parser.add_argument("documents", nargs="*",
                        help="the documents to render (default: all here)")
    args = parser.parse_args()

    program = os.path.join(args.engine, "program")
    soffice = os.path.join(program, "soffice")
    translate = os.path.join(program, "l10ntranslate")
    for tool in (soffice, translate):
        if not os.path.exists(tool):
            sys.exit(f"{tool} not found; build the engine first")

    documents = args.documents or sorted(
        path for ext in ("odt", "ods", "odp")
        for path in glob(os.path.join(HERE, f"*.{ext}")))
    if not documents:
        sys.exit("no intro documents found")

    os.makedirs(PREVIEW_DIR, exist_ok=True)
    with tempfile.TemporaryDirectory() as workdir:
        for document in documents:
            render(os.path.abspath(document), soffice, translate, workdir)
    return 0


if __name__ == "__main__":
    sys.exit(main())
