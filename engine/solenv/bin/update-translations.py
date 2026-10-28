#!/usr/bin/env python3
# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# Copyright the Collabora Office contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

"""Bring the po files of every language up to date with the pot files.

usage: update-translations.py <pot dir> <translations source dir>

For every language directory under the translations source dir:

  - po files without a pot counterpart are removed,
  - pot files without a po counterpart get a new, untranslated po file,
  - the remaining po files are merged with msgmerge.

The merge keeps the diff down to the real changes. The po files are a mix
of layouts: msgmerge writes long lines, Weblate rewraps the messages it
touches at 77 columns. Any message whose content did not change keeps its
previous lines verbatim, whatever its layout was, and a message that still
lives in the same source files keeps its previous "#:" references even if
the line numbers moved. Obsolete messages are dropped. A po file whose
messages did not change at all is left alone, POT-Creation-Date included.
"""

import concurrent.futures
import os
import subprocess
import sys


def read_blocks(text):
    """Split a po file into blocks of lines separated by blank lines."""
    blocks = []
    current = []
    for line in text.split('\n'):
        if line.strip() == '':
            if current:
                blocks.append(current)
                current = []
        else:
            current.append(line)
    if current:
        blocks.append(current)
    return blocks


def render(blocks):
    return '\n\n'.join('\n'.join(block) for block in blocks) + '\n'


def unquote(text):
    """Return the content of a quoted po string as written, escapes kept."""
    text = text.strip()
    if len(text) >= 2 and text[0] == '"' and text[-1] == '"':
        return text[1:-1]
    return text


class Message:
    def __init__(self, block):
        self.block = block
        self.obsolete = False
        self.extracted = []
        self.translator = []
        self.refs = []
        self.flags = set()
        self.previous = []
        self.strings = {}
        current = None
        for line in block:
            if line.startswith('#~'):
                self.obsolete = True
            elif line.startswith('#,'):
                self.flags.update(f.strip() for f in line[2:].split(',') if f.strip())
            elif line.startswith('#:'):
                self.refs.append(line)
            elif line.startswith('#.'):
                self.extracted.append(line)
            elif line.startswith('#|'):
                self.previous.append(line)
            elif line.startswith('#'):
                self.translator.append(line)
            elif line.startswith('"'):
                if current is not None:
                    self.strings[current] += unquote(line)
            else:
                keyword, _, rest = line.partition(' ')
                current = keyword
                self.strings[current] = unquote(rest)

    def key(self):
        return (self.strings.get('msgctxt'), self.strings.get('msgid'),
                self.strings.get('msgid_plural'))

    def is_header(self):
        return self.strings.get('msgid') == '' and 'msgctxt' not in self.strings

    def ref_files(self):
        files = set()
        for line in self.refs:
            for token in line[2:].split():
                files.add(token.rsplit(':', 1)[0] if ':' in token else token)
        return files

    def content(self):
        return (self.extracted, self.translator, self.refs, self.flags,
                self.previous, self.strings)

    def with_refs(self, refs):
        """Return the lines of this message with its "#:" lines replaced."""
        if not self.refs or self.refs == refs:
            return self.block
        indices = [i for i, line in enumerate(self.block) if line.startswith('#:')]
        return self.block[:indices[0]] + refs + self.block[indices[-1] + 1:]


def header_with_pot_date(old_header, new_header):
    """Keep the header the po file had, with the POT-Creation-Date of the pot."""
    new_date = [line for line in new_header if line.startswith('"POT-Creation-Date:')]
    if not new_date:
        return old_header
    result = []
    for line in old_header:
        if line.startswith('"POT-Creation-Date:'):
            result.append(new_date[0])
        else:
            result.append(line)
    return result


def merge(po_path, pot_path):
    """Merge one po file with its pot and write it back if it changed."""
    with open(po_path, encoding='utf-8') as f:
        old_text = f.read()
    result = subprocess.run(
        ['msgmerge', '--no-wrap', '--previous', '--quiet', po_path, pot_path],
        capture_output=True, text=True, encoding='utf-8')
    if result.returncode != 0:
        raise RuntimeError('msgmerge failed for %s:\n%s' % (po_path, result.stderr))

    old_blocks = [Message(b) for b in read_blocks(old_text)]
    new_blocks = [Message(b) for b in read_blocks(result.stdout)]
    old_by_key = {}
    for message in old_blocks:
        if not message.is_header() and not message.obsolete:
            old_by_key.setdefault(message.key(), message)

    output = []
    header = None
    for message in new_blocks:
        if message.obsolete:
            continue
        if message.is_header():
            old_header = old_blocks[0] if old_blocks and old_blocks[0].is_header() else None
            if old_header is not None:
                header = len(output)
                output.append(old_header.block)
            else:
                output.append(message.block)
            continue
        old = old_by_key.get(message.key())
        if old is None:
            output.append(message.block)
            continue
        refs = message.refs
        if old.refs and old.ref_files() == message.ref_files():
            refs = old.refs
        if old.content() == (message.extracted, message.translator, refs,
                             message.flags, message.previous, message.strings):
            output.append(old.block)
        else:
            output.append(message.with_refs(refs))

    if render(output) == old_text:
        return False
    if header is not None:
        output[header] = header_with_pot_date(output[header], new_blocks[0].block)
    new_text = render(output)
    with open(po_path, 'w', encoding='utf-8') as f:
        f.write(new_text)
    return True


def language_header_lines(lang_dir):
    """Take the Language and Plural-Forms header lines from a sibling po file."""
    for root, _, files in sorted(os.walk(lang_dir)):
        for name in sorted(files):
            if not name.endswith('.po'):
                continue
            with open(os.path.join(root, name), encoding='utf-8') as f:
                blocks = read_blocks(f.read())
            if not blocks:
                continue
            return [line for line in blocks[0]
                    if line.startswith('"Language:') or line.startswith('"Plural-Forms:')]
    return []


def create(po_path, pot_path, lang, lang_header):
    """Create an untranslated po file for a language from the pot."""
    with open(pot_path, encoding='utf-8') as f:
        blocks = read_blocks(f.read())
    header = []
    for line in blocks[0]:
        header.append(line)
        if line.startswith('"Language-Team:'):
            language = [l for l in lang_header if l.startswith('"Language:')]
            header.append(language[0] if language else '"Language: %s\\n"' % lang)
        elif line.startswith('"Content-Transfer-Encoding:'):
            header.extend(l for l in lang_header if l.startswith('"Plural-Forms:'))
    blocks[0] = header
    os.makedirs(os.path.dirname(po_path), exist_ok=True)
    with open(po_path, 'w', encoding='utf-8') as f:
        f.write(render(blocks))


def relative_files(top, suffix):
    result = set()
    for root, _, files in os.walk(top):
        for name in files:
            if name.endswith(suffix):
                path = os.path.relpath(os.path.join(root, name), top)
                result.add(path[:-len(suffix)])
    return result


def remove_empty_dirs(top):
    for root, dirs, files in os.walk(top, topdown=False):
        if root != top and not dirs and not files:
            os.rmdir(root)


def main():
    if len(sys.argv) != 3:
        print(__doc__, file=sys.stderr)
        return 2
    pot_dir, source_dir = sys.argv[1], sys.argv[2]
    pots = relative_files(pot_dir, '.pot')
    if not pots:
        print('no pot files found under %s' % pot_dir, file=sys.stderr)
        return 1

    removed = created = updated = unchanged = 0
    jobs = []
    for lang in sorted(os.listdir(source_dir)):
        lang_dir = os.path.join(source_dir, lang)
        if not os.path.isdir(lang_dir):
            continue
        pos = relative_files(lang_dir, '.po')
        for rel in sorted(pos - pots):
            os.remove(os.path.join(lang_dir, rel + '.po'))
            removed += 1
        lang_header = language_header_lines(lang_dir)
        for rel in sorted(pots - pos):
            create(os.path.join(lang_dir, rel + '.po'),
                   os.path.join(pot_dir, rel + '.pot'), lang, lang_header)
            created += 1
        for rel in sorted(pos & pots):
            jobs.append((os.path.join(lang_dir, rel + '.po'),
                         os.path.join(pot_dir, rel + '.pot')))
        remove_empty_dirs(lang_dir)

    # A broken pot fails for every language alike; report it once.
    failures = {}

    def run(job):
        try:
            return merge(*job)
        except RuntimeError as e:
            failures.setdefault(job[1], str(e))
            return False

    with concurrent.futures.ThreadPoolExecutor(max_workers=os.cpu_count()) as pool:
        for changed in pool.map(run, jobs):
            if changed:
                updated += 1
            else:
                unchanged += 1

    print('po files: %d removed, %d created, %d updated, %d unchanged'
          % (removed, created, updated, unchanged))
    for pot in sorted(failures):
        print(failures[pot], file=sys.stderr)
    return 1 if failures else 0


if __name__ == '__main__':
    sys.exit(main())

# vim: set shiftwidth=4 softtabstop=4 expandtab:
