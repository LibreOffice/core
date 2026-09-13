#!/usr/bin/env python3
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Builds an ODT that exercises the sentence checker, a chapter per language
# and a row per rule, for checking by hand in Writer.
#
# Usage: lpdemo.py <output.odt> [<dictionaries directory>]
#
# Every example is generated from the rule's own pattern and then checked
# against it, so a row is only written if the text really does match. What
# the row cannot promise is that the rule will fire: a condition that asks
# for morphology or for a correctly spelled word will not be satisfied by
# invented text. Those rows say so in the last column.

import argparse
import ast
import os
import re
import sys
import zipfile

# Letters to prefer when a character class has to be sampled. Latin first,
# so English reads sensibly; the rest let the other languages pick something
# from their own alphabet rather than the first character of a range.
LOWER = "aeioubcdfghjklmnpqrstvwxyz"
CYRILLIC = "\u043e\u0435\u0430\u0438\u043d\u0442\u0441\u0440\u0432\u043b\u043a\u043c\u0434\u043f\u0443\u044f\u044b\u044c\u0433\u0437\u0431\u0447\u0439\u0445\u0436\u0448\u044e\u0446\u0449\u044d\u0444\u044a\u0451"
ACCENTED = "\u00e1\u00e9\u00ed\u00f3\u00f6\u00fa\u00fc\u0151\u0171\u00e2\u00e3\u00e7\u00ea\u00f4"

# Letters to try when a character class has to be sampled, in order. Lower
# case first, so a class written [Kk] for case insensitivity gives "k" and the
# examples do not come out in tHiS sHaPe.
PREFERRED = LOWER + ACCENTED + CYRILLIC + LOWER.upper() + "0123456789"

# Russian rules often accept Latin and Cyrillic in the same class; take the
# alphabet the reader expects.
PREFERRED_BY_PACKAGE = {
    "ru_RU": CYRILLIC + LOWER + ACCENTED + CYRILLIC.upper() + LOWER.upper() + "0123456789",
    "hu_HU": LOWER + ACCENTED + LOWER.upper() + ACCENTED.upper() + "0123456789",
}


# Ordinary words to build examples from, before falling back to the spelling
# dictionary, whose shortest entries tend to be abbreviations and oddities.
COMMON = {
    "en": ["apple", "orange", "idea", "hour", "house", "book", "table", "river",
           "city", "animal", "engine", "picture", "effort", "problem", "letter",
           "window", "garden", "island", "evening", "morning"],
    "hu_HU": ["alma", "ablak", "asztal", "ember", "erd\u0151", "id\u0151", "h\u00e1z",
              "k\u00f6nyv", "v\u00e1ros", "vir\u00e1g", "kutya", "macska", "gyerek",
              "munka", "orsz\u00e1g", "iskola", "utca", "level\u00e9l", "reggel", "est\u00e9"],
    "ru_RU": ["\u043e\u043a\u043d\u043e", "\u0441\u0442\u043e\u043b", "\u043a\u043d\u0438\u0433\u0430",
              "\u0433\u043e\u0440\u043e\u0434", "\u0434\u043e\u043c", "\u0440\u0435\u043a\u0430",
              "\u0434\u0435\u0440\u0435\u0432\u043e", "\u043c\u0430\u0448\u0438\u043d\u0430",
              "\u0440\u0430\u0431\u043e\u0442\u0430", "\u0448\u043a\u043e\u043b\u0430",
              "\u0443\u043b\u0438\u0446\u0430", "\u043f\u0438\u0441\u044c\u043c\u043e",
              "\u044f\u0431\u043b\u043e\u043a\u043e", "\u0438\u0434\u0435\u044f", "\u0447\u0430\u0441",
              "\u0432\u0435\u0447\u0435\u0440", "\u0443\u0442\u0440\u043e", "\u0441\u0430\u0434",
              "\u043e\u0441\u0442\u0440\u043e\u0432", "\u043f\u043e\u0435\u0437\u0434"],
    "pt_BR": ["casa", "livro", "cidade", "\u00e1rvore", "janela", "mesa", "ideia",
              "hora", "escola", "trabalho", "crian\u00e7a", "carro", "flor", "animal",
              "problema", "jardim", "ilha", "manh\u00e3", "tarde", "cidade"],
}


_WORDS = {}


def wordlist(dictdir, package):
    """Short, ordinary words from the package's own spelling dictionary."""
    if package in _WORDS:
        return _WORDS[package]
    found = []
    for name in (package, package.split("_")[0] + "_" + package.split("_")[-1],
                 "en_US", "en_GB"):
        path = os.path.join(dictdir, package, name + ".dic")
        if os.path.exists(path):
            break
    else:
        _WORDS[package] = []
        return []
    with open(path, encoding="utf-8", errors="replace") as handle:
        next(handle, None)                      # the entry count
        for line in handle:
            word = re.split(r"[/\t ]", line.strip(), 1)[0]
            if 3 <= len(word) <= 9 and word.isalpha() and word == word.lower():
                found.append(word)
    # Short first, so examples stay readable, behind the common words.
    found.sort(key=lambda w: (len(w), w))
    _WORDS[package] = COMMON.get(package, []) + found
    return found if False else _WORDS[package]


def pick_word(words, items, negated, first_items=None):
    """A real word whose letters all sit in the class, or None."""
    for word in words:
        candidates = [word]
        if first_items is not None and not in_class(word[0], first_items):
            # A class that only takes capitals wants the word capitalised.
            candidates = [word.capitalize()]
        for candidate in candidates:
            if first_items is not None and not in_class(candidate[0], first_items):
                continue
            body = candidate[1:] if first_items is not None else candidate
            if all(in_class(c, items) != negated for c in body):
                return candidate
    return None


class Unsupported(Exception):
    pass


class Sampler:
    """Walks a pattern and builds a string it matches."""

    def __init__(self, pattern, words=(), optional=False, preferred=PREFERRED):
        self.s = pattern
        self.i = 0
        self.groups = {}
        self.n = 0
        self.words = words
        self.optional = optional
        self.preferred = preferred

    def peek(self, k=0):
        return self.s[self.i + k] if self.i + k < len(self.s) else ""

    def parse(self, stop=")"):
        branches = [[]]
        while self.i < len(self.s):
            c = self.peek()
            if c == "|":
                self.i += 1
                branches.append([])
                continue
            if c == ")" and stop == ")":
                break
            branches[-1].append(self.atom())
        for branch in branches:
            try:
                return "".join(part() for part in branch)
            except Unsupported:
                continue
        raise Unsupported("every branch failed")

    def atom(self):
        c = self.peek()
        if c == "(":
            return self.group()
        if c == "[":
            composite = self.try_word_start()
            if composite:
                return composite
            return self.quantify(self.charclass())
        if c == "\\":
            return self.quantify(self.escape())
        if c == ".":
            self.i += 1
            return self.quantify(lambda: "x")
        if c in "^$":
            self.i += 1
            return lambda: ""
        self.i += 1
        return self.quantify(lambda ch=c: ch)

    def group(self):
        self.i += 1
        name = None
        capture = True
        if self.peek() == "?":
            kind = self.peek(1)
            if kind == ":":
                self.i += 2
                capture = False
            elif kind == "P" and self.peek(2) == "<":
                end = self.s.index(">", self.i)
                name = self.s[self.i + 3:end]
                self.i = end + 1
            elif kind in "=!<":
                # A lookaround sits exactly where its text has to sit, so the
                # positive ones can simply be emitted and the negative ones
                # left out.
                negative = kind == "!" or (kind == "<" and self.peek(2) == "!")
                self.i += 3 if kind == "<" else 2
                inner = self.parse()
                self.i += 1
                return (lambda: "") if negative else (lambda inner=inner: inner)
            else:
                self.i = self.s.index(")", self.i) + 1
                return lambda: ""
        number = None
        if capture:
            self.n += 1
            number = self.n
        inner = self.parse()
        self.i += 1

        def emit(inner=inner, number=number, name=name):
            if number:
                self.groups[number] = inner
            if name:
                self.groups[name] = inner
            return inner

        return self.quantify(emit)

    def as_word(self, emit):
        """A real word for a repeated letter class, if one fits."""
        items = getattr(emit, "items", None)
        if items is None or not self.words:
            return None
        chosen = pick_word(self.words, items, emit.negated)
        return (lambda chosen=chosen: chosen) if chosen else None

    def try_word_start(self):
        """A letter class followed by a repeated letter class is one word, not
        a letter and then a word."""
        start = self.i
        first = self.charclass()
        if self.peek() in "?*+{":
            self.i = start
            return None
        if self.peek() == "[":
            rest = self.charclass()
            items, negated = rest.items, rest.negated
        elif self.s[self.i:self.i + 2] == "\\w":
            self.i += 2
            items, negated = ["\\w"], False
        else:
            self.i = start
            return None
        if self.peek() not in "*+":
            self.i = start
            return None
        self.i += 1
        if self.peek() in "?+":
            self.i += 1
        chosen = (pick_word(self.words, items, negated, first_items=first.items)
                  if self.words else None)
        if not chosen:
            self.i = start
            return None
        return lambda chosen=chosen: chosen

    def charclass(self):
        j = self.i + 1
        negated = self.s[j] == "^"
        if negated:
            j += 1
        items = []
        first = True
        while True:
            ch = self.s[j]
            if ch == "]" and not first:
                break
            first = False
            if ch == "\\":
                items.append(self.s[j:j + 2])
                j += 2
                continue
            if self.s[j + 1] == "-" and self.s[j + 2] != "]":
                items.append((ch, self.s[j + 2]))
                j += 3
                continue
            items.append(ch)
            j += 1
        self.i = j + 1

        def emit(items=items, negated=negated):
            if negated:
                for c in self.preferred:
                    if not in_class(c, items):
                        return c
                raise Unsupported("nothing outside the class")
            for c in self.preferred:
                if in_class(c, items):
                    return c
            for preferred in (".", ",", " "):
                if preferred in items:
                    return preferred
            for item in items:
                if isinstance(item, tuple):
                    return item[0]
                if item.startswith("\\"):
                    return {"\\w": "a", "\\d": "1", "\\s": " "}.get(item, item[1])
                return item
            raise Unsupported("empty class")

        emit.items = items
        emit.negated = negated
        return emit

    def escape(self):
        c = self.peek(1)
        self.i += 2
        if c.isdigit():
            return lambda n=int(c): self.groups.get(n, "")
        if c == "g":
            end = self.s.index(">", self.i)
            name = self.s[self.i + 1:end]
            self.i = end + 1
            return lambda name=name: self.groups.get(name, "")
        if c == "u":
            code = self.s[self.i:self.i + 4]
            self.i += 4
            return lambda code=code: chr(int(code, 16))
        if c == "w":
            word_class = lambda: "a"
            word_class.items = ["\\w"]
            word_class.negated = False
            return word_class
        plain = {"d": "1", "s": " ", "b": "", "B": "", "n": "\n", "t": "\t"}
        if c in plain:
            return lambda v=plain[c]: v
        if c in "WDS":
            return lambda: "-"
        return lambda ch=c: ch

    def quantify(self, emit):
        c = self.peek()
        if c in "?*":
            self.i += 1
            if self.peek() in "?+":
                self.i += 1
            # An optional part is usually punctuation around the interesting
            # text, and reads better left out.
            return emit if self.optional else (lambda: "")
        if c == "+":
            self.i += 1
            if self.peek() in "?+":
                self.i += 1
            word = self.as_word(emit)
            if word:
                return word
            # The same text twice, so a backreference to this group lines up.
            return lambda: (lambda t: t + t)(emit())
        if c == "{":
            end = self.s.index("}", self.i)
            spec = self.s[self.i + 1:end]
            self.i = end + 1
            if self.peek() in "?+":
                self.i += 1
            low = spec.split(",")[0]
            count = int(low) if low.isdigit() else 1
            return lambda count=max(count, 1): emit() * count
        return emit


def in_class(c, items):
    for item in items:
        if isinstance(item, tuple):
            if item[0] <= c <= item[1]:
                return True
        elif item.startswith("\\"):
            if item == "\\w" and (c.isalnum() or c == "_"):
                return True
            if item == "\\d" and c.isdigit():
                return True
            if item == "\\s" and c.isspace():
                return True
        elif item == c:
            return True
    return False


# Frames to put around a sampled fragment. A rule may want a word boundary,
# the start of the paragraph, or nothing at all on either side, so try a few
# and keep the first that still matches.
def frames(left, right):
    return [
        lambda s: " " + s + " ",
        # A fragment that carries its own spaces belongs between two words,
        # not at the start of the paragraph, where the checker will not mark it.
        lambda s: left + s + right,
        lambda s: s + " " + right + ".",
        lambda s: left + " " + s + " " + right + ".",
        lambda s: left + " " + s,
        lambda s: s + " ",
        lambda s: " " + s,
        lambda s: s,
        lambda s: left + s,
        lambda s: s + right,
    ]


def example_for(pattern, words=(), preferred=PREFERRED):
    """(text, match) the pattern really matches, or None.

    Tried without the pattern's optional parts first, since those are usually
    punctuation around the text the rule is really about.
    """
    carriers = [w for w in words if len(w) > 3][:2] or ["ab", "cd"]
    fallback = None
    for optional in (False, True):
        try:
            fragment = Sampler(pattern, words, optional, preferred).parse(stop="")
        except Exception:
            continue
        for frame in frames(carriers[0], carriers[-1]):
            text = frame(fragment)
            try:
                found = re.search(pattern, text)
            except re.error:
                return None
            if not found or not found.group(0):
                continue
            # A rule about spacing will match a line of nothing but spaces,
            # which is no use to a reader and which the checker skips.
            if any(c.isalnum() for c in text):
                return text, found
            if fallback is None:
                fallback = (text, found)
    return fallback


def expand(template, match):
    """Numbered and named backreferences, the way the checker expands them."""
    def one(m):
        name = m.group(1) or m.group(2)
        try:
            return (match.group(int(name)) if name.isdigit() else match.group(name)) or ""
        except (IndexError, KeyError, error_type):
            return ""
    return re.sub(r"\\(\d)|\\g<([^>]+)>", one, template)


error_type = re.error


def suggestions_for(rule, match):
    template = rule[1]
    if template.startswith("="):
        return None                      # built by code at run time
    if template == "_":
        return []                        # the rule marks, but offers nothing
    text = expand(template, match).replace("\\n", "\n")
    return [part for part in re.split(r"[|\n]", text)]


def message_for(rule, match):
    text = rule[2]
    if text.startswith("="):
        return None
    return expand(text, match).replace("\\n", " ").strip()


def option_of(condition):
    if condition is False:
        return None
    found = re.search(r'option\(LOCALE,\s*"(\w+)"\)', condition or "")
    return found.group(1) if found else None


def extra_condition(condition):
    """What the rule wants beyond its option, in the words of the last column."""
    if condition is False:
        return "always"
    stripped = re.sub(r'option\(LOCALE,\s*"\w+"\)', "", condition).strip()
    if not stripped.strip("and or ()").strip():
        return "always"
    if re.search(r"\b(morph|spell|word|stem|generate|analyse)\s*\(", condition):
        return "needs real words"
    return "extra condition"


def labels_from_browser(package, dictdir):
    """Labels for a package with no .dlg, taken from the options dialog."""
    path = os.path.join(dictdir, os.pardir, os.pardir, "browser", "admin", "src",
                        "integrator", "AdminIntegratorSettings.ts")
    if not os.path.exists(path):
        return {}
    text = open(path, encoding="utf-8").read()
    pairs = re.findall(r"'%s-(\w+)':\s*_\('((?:[^'\\]|\\.)*)'\)" % re.escape(package), text)
    return {name: label.replace("\\'", "'") for name, label in pairs}


def read_labels(dictdir, package):
    """Option id -> English label, from the package's .dlg if we have one."""
    path = os.path.join(dictdir, package, "lightproof", package + ".dlg")
    if not os.path.exists(path):
        return labels_from_browser(package, dictdir)
    labels = {}
    started = False
    for line in open(path, encoding="utf-8").read().split("\n"):
        line = line.strip()
        if line.startswith("["):
            # Titles follow the groups; take the English block only.
            started = "en_US" in line or "en" == line.strip("[]=")
            continue
        if not started or not line or line.startswith("#") or "=" not in line:
            continue
        name, _, label = line.partition("=")
        # The label may carry a line of help after a \n, and a space before it.
        labels.setdefault(name.strip(), label.split("\\n")[0].strip())
    return labels


def read_package(dictdir, package):
    base = os.path.join(dictdir, package, "pythonpath")
    rules_src = open(os.path.join(base, "lightproof_%s.py" % package),
                     encoding="utf-8-sig").read()
    rules = ast.literal_eval(rules_src.split("dic =", 1)[1].strip())
    scope = {}
    exec(open(os.path.join(base, "lightproof_opts_%s.py" % package),
              encoding="utf-8-sig").read(), scope)
    return rules, scope["lopts"][package], scope["lopts_default"][package]


def esc(text):
    return (text.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;"))


def visible(text):
    """Make runs of spaces countable, and show what is otherwise invisible."""
    text = text.replace("\u00ad", "\u00ad(soft hyphen)")
    return text


LANGS = [
    ("en", "English", "en", "US"),
    ("hu_HU", "Hungarian", "hu", "HU"),
    ("ru_RU", "Russian", "ru", "RU"),
    ("pt_BR", "Portuguese (Brazilian)", "pt", "BR"),
]


def build_rows(dictdir, package, limit=None):
    rules, options, defaults = read_package(dictdir, package)
    labels = read_labels(dictdir, package)
    words = wordlist(dictdir, package)
    preferred = PREFERRED_BY_PACKAGE.get(package, PREFERRED)
    groups = {}
    for index, rule in enumerate(rules):
        made = example_for(rule[0], words, preferred)
        if not made:
            continue
        text, match = made
        option = option_of(rule[3])
        row = {
            "index": index,
            "text": text,
            "message": message_for(rule, match),
            "suggestions": suggestions_for(rule, match),
            "fires": ("spaces only, will not mark"
                      if not any(c.isalnum() for c in text) else extra_condition(rule[3])),
        }
        groups.setdefault(option, []).append(row)
    if limit:
        # One example per distinct message, for the package too big to list.
        seen = set()
        trimmed = {}
        for option, rows in groups.items():
            for row in rows:
                key = row["message"]
                if key in seen or row["fires"] != "always" or not row["suggestions"]:
                    continue
                seen.add(key)
                trimmed.setdefault(option, []).append(row)
                if sum(len(v) for v in trimmed.values()) >= limit:
                    return trimmed, options, defaults, labels
        return trimmed, options, defaults, labels
    return groups, options, defaults, labels


MIMETYPE = "application/vnd.oasis.opendocument.text"

MANIFEST = """<?xml version="1.0" encoding="UTF-8"?>
<manifest:manifest xmlns:manifest="urn:oasis:names:tc:opendocument:xmlns:manifest:1.0" manifest:version="1.3">
 <manifest:file-entry manifest:full-path="/" manifest:version="1.3" manifest:media-type="%s"/>
 <manifest:file-entry manifest:full-path="content.xml" manifest:media-type="text/xml"/>
 <manifest:file-entry manifest:full-path="styles.xml" manifest:media-type="text/xml"/>
 <manifest:file-entry manifest:full-path="meta.xml" manifest:media-type="text/xml"/>
</manifest:manifest>
""" % MIMETYPE

META = """<?xml version="1.0" encoding="UTF-8"?>
<office:document-meta xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0" xmlns:meta="urn:oasis:names:tc:opendocument:xmlns:meta:1.0" xmlns:dc="http://purl.org/dc/elements/1.1/" office:version="1.3">
 <office:meta>
  <dc:title>Sentence checker rule demonstration</dc:title>
  <meta:generator>lpdemo.py</meta:generator>
 </office:meta>
</office:document-meta>
"""

NS = ('xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0" '
      'xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0" '
      'xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0" '
      'xmlns:table="urn:oasis:names:tc:opendocument:xmlns:table:1.0" '
      'xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0" '
      'xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0"')

STYLES = """<?xml version="1.0" encoding="UTF-8"?>
<office:document-styles %s office:version="1.3">
 <office:styles>
  <style:style style:name="Standard" style:family="paragraph">
   <style:paragraph-properties fo:margin-top="0cm" fo:margin-bottom="0.2cm"/>
   <style:text-properties style:font-name="Liberation Sans" fo:font-size="10pt"/>
  </style:style>
  <style:style style:name="Title" style:family="paragraph" style:parent-style-name="Standard">
   <style:paragraph-properties fo:margin-bottom="0.4cm"/>
   <style:text-properties fo:font-size="22pt" fo:font-weight="bold"/>
  </style:style>
  <style:style style:name="Heading_20_1" style:display-name="Heading 1" style:family="paragraph" style:parent-style-name="Standard">
   <style:paragraph-properties fo:margin-top="0.6cm" fo:margin-bottom="0.3cm" fo:break-before="page"/>
   <style:text-properties fo:font-size="18pt" fo:font-weight="bold" fo:color="#2a6099"/>
  </style:style>
  <style:style style:name="Heading_20_2" style:display-name="Heading 2" style:family="paragraph" style:parent-style-name="Standard">
   <style:paragraph-properties fo:margin-top="0.5cm" fo:margin-bottom="0.2cm" fo:keep-with-next="always"/>
   <style:text-properties fo:font-size="13pt" fo:font-weight="bold"/>
  </style:style>
 </office:styles>
 <office:automatic-styles>
  <style:page-layout style:name="pm1">
   <style:page-layout-properties fo:page-width="29.7cm" fo:page-height="21cm" style:print-orientation="landscape" fo:margin-top="1.5cm" fo:margin-bottom="1.5cm" fo:margin-left="1.5cm" fo:margin-right="1.5cm"/>
  </style:page-layout>
 </office:automatic-styles>
 <office:master-styles>
  <style:master-page style:name="Standard" style:page-layout-name="pm1"/>
 </office:master-styles>
</office:document-styles>
""" % NS


def automatic_styles():
    out = ['<office:automatic-styles>']
    # One paragraph style per language, so the checker runs on the examples
    # and stays off everything else.
    for package, _, lang, country in LANGS:
        out.append('<style:style style:name="Ex%s" style:family="paragraph" style:parent-style-name="Standard">'
                   '<style:text-properties fo:language="%s" fo:country="%s" '
                   'style:font-name="Liberation Serif" fo:font-size="11pt"/></style:style>'
                   % (package, lang, country))
    out.append('<style:style style:name="Note" style:family="paragraph" style:parent-style-name="Standard">'
               '<style:text-properties fo:language="zxx" fo:country="none" fo:font-size="9pt"/></style:style>')
    out.append('<style:style style:name="NoteHead" style:family="paragraph" style:parent-style-name="Standard">'
               '<style:text-properties fo:language="zxx" fo:country="none" fo:font-size="9pt" '
               'fo:font-weight="bold"/></style:style>')
    out.append('<style:style style:name="Intro" style:family="paragraph" style:parent-style-name="Standard">'
               '<style:text-properties fo:language="zxx" fo:country="none"/></style:style>')
    out.append('<style:style style:name="Tbl" style:family="table">'
               '<style:table-properties style:width="26.7cm" table:align="left"/></style:style>')
    for name, width in (("c1", "1.2cm"), ("c2", "8.5cm"), ("c3", "7.5cm"),
                        ("c4", "6.0cm"), ("c5", "3.5cm")):
        out.append('<style:style style:name="%s" style:family="table-column">'
                   '<style:table-column-properties style:column-width="%s"/></style:style>' % (name, width))
    out.append('<style:style style:name="Cell" style:family="table-cell">'
               '<style:table-cell-properties fo:border="0.05pt solid #b0b0b0" '
               'fo:padding="0.08cm" style:vertical-align="top"/></style:style>')
    out.append('<style:style style:name="HeadCell" style:family="table-cell">'
               '<style:table-cell-properties fo:border="0.05pt solid #808080" '
               'fo:padding="0.08cm" fo:background-color="#e8e8e8"/></style:style>')
    out.append('</office:automatic-styles>')
    return "".join(out)


def cell(style, paragraph_style, text):
    return ('<table:table-cell table:style-name="%s" office:value-type="string">'
            '<text:p text:style-name="%s">%s</text:p></table:table-cell>'
            % (style, paragraph_style, text))


def table_for(rows, package):
    out = ['<table:table table:name="t%s%d" table:style-name="Tbl">' % (package, id(rows) % 100000)]
    for name in ("c1", "c2", "c3", "c4", "c5"):
        out.append('<table:table-column table:style-name="%s"/>' % name)
    out.append('<table:table-header-rows><table:table-row>')
    for title in ("Rule", "Example (expect a blue underline)", "What the rule reports",
                  "Suggestions offered", "Fires when"):
        out.append(cell("HeadCell", "NoteHead", esc(title)))
    out.append('</table:table-row></table:table-header-rows>')
    for row in rows:
        message = row["message"]
        message = esc(message) if message is not None else "<text:span>(built at run time)</text:span>"
        if row["suggestions"] is None:
            offered = "(built at run time)"
        elif not row["suggestions"]:
            offered = "(none, the rule only marks)"
        else:
            offered = "  \u2022  ".join(row["suggestions"])
        out.append('<table:table-row>')
        out.append(cell("Cell", "Note", str(row["index"])))
        out.append(cell("Cell", "Ex" + package, esc(visible(row["text"]))))
        out.append(cell("Cell", "Note", message))
        out.append(cell("Cell", "Note", esc(offered)))
        out.append(cell("Cell", "Note", esc(row["fires"])))
        out.append('</table:table-row>')
    out.append('</table:table>')
    return "".join(out)


INTRO = [
    "This document exercises the built-in sentence checker. Every example below was generated from a rule's "
    "own pattern and then checked against it, so the text really does match the rule.",
    "Before you start, turn on the options named in the headings. In the browser that is Options \u25b8 "
    "Lightproof Sentence Checker; in the desktop engine, Tools \u25b8 Options \u25b8 Language Settings \u25b8 "
    "Sentence Checking. A rule whose option is off marks nothing, and that is correct behaviour, not a bug. "
    "Turn AutoCorrect while typing off as well, or it will rewrite the examples as you scroll.",
    "The last column says what a rule wants beyond its option. \"always\" means the example is enough on its "
    "own. \"needs real words\" means the rule also asks the spelling dictionary or the morphology about what "
    "it matched, which invented text will not satisfy, so no underline there is expected. \"extra condition\" "
    "means the rule tests something else again, such as the country of the locale or a list of exceptions, "
    "and may or may not mark.",
    "Examples are deliberately nonsense words in places. The checker matches patterns, not meaning, and "
    "nonsense keeps each example to the one rule it is meant to show.",
    "Only the example column carries a language. Everything else is set to no language so it is never "
    "underlined itself.",
]


def build(dictdir, out_path):
    body = ['<?xml version="1.0" encoding="UTF-8"?>',
            '<office:document-content %s office:version="1.3">' % NS,
            automatic_styles(),
            '<office:body><office:text>']
    body.append('<text:p text:style-name="Title">Sentence checker rule demonstration</text:p>')
    for line in INTRO:
        body.append('<text:p text:style-name="Intro">%s</text:p>' % esc(line))

    summary = []
    for package, title, _, _ in LANGS:
        limit = 30 if package == "pt_BR" else None
        groups, options, defaults, labels = build_rows(dictdir, package, limit)
        count = sum(len(v) for v in groups.values())
        summary.append((title, count))
        body.append('<text:h text:outline-level="1" text:style-name="Heading_20_1">%s</text:h>'
                    % esc(title))
        if limit:
            body.append('<text:p text:style-name="Intro">%s</text:p>' % esc(
                "This package has far too many rules to list, so here is one example for each kind of "
                "report it makes."))
        body.append('<text:p text:style-name="Intro">%s</text:p>' % esc(
            "%d examples. Options on by default: %s."
            % (count, ", ".join(sorted(defaults)) or "none")))
        for option in sorted(groups, key=lambda o: (o is not None, o or "")):
            rows = groups[option]
            if option is None:
                heading = "Rules with no option (%d)" % len(rows)
            else:
                label = labels.get(option)
                state = "on by default" if option in defaults else "off by default"
                heading = "Option \u201c%s\u201d%s \u2014 %s (%d)" % (
                    option, " (%s)" % label if label else "", state, len(rows))
            body.append('<text:h text:outline-level="2" text:style-name="Heading_20_2">%s</text:h>'
                        % esc(heading))
            body.append(table_for(rows, package))
    body.append('</office:text></office:body></office:document-content>')
    content = "".join(body)

    with zipfile.ZipFile(out_path, "w") as odt:
        # The mimetype entry has to come first and be stored, not deflated.
        info = zipfile.ZipInfo("mimetype")
        info.compress_type = zipfile.ZIP_STORED
        odt.writestr(info, MIMETYPE)
        odt.writestr("META-INF/manifest.xml", MANIFEST, zipfile.ZIP_DEFLATED)
        odt.writestr("meta.xml", META, zipfile.ZIP_DEFLATED)
        odt.writestr("styles.xml", STYLES, zipfile.ZIP_DEFLATED)
        odt.writestr("content.xml", content, zipfile.ZIP_DEFLATED)
    return summary


def main():
    parser = argparse.ArgumentParser(description="Build the sentence checker demonstration document")
    parser.add_argument("output", help="the .odt to write")
    parser.add_argument("dictdir", nargs="?", default="dictionaries",
                        help="the dictionaries directory (default: dictionaries)")
    args = parser.parse_args()
    for title, count in build(args.dictdir, args.output):
        sys.stderr.write("lpdemo: %-24s %d examples\n" % (title, count))
    sys.stderr.write("lpdemo: wrote %s\n" % args.output)


if __name__ == "__main__":
    main()
