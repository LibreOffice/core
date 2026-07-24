#!/usr/bin/env python3
"""The "_"-marker localization spec for the intro documents.

The engine's translation utility (engine desktop/source/lib/
l10ntranslate.cxx) derives keys from the same XML by the same rules; the
parity cases live in tests/test_l10n_tooling.py here and in the engine's
test_l10ntranslate.cxx, and a rule change must touch both test files.

The rules:

- A translation unit is a text:p / text:h whose first text segment starts
  with "_", a table:table sheet name starting with "_", or a
  table:help-message title starting with "_".
- Runs are one per non-empty text:span child and one per stretch of bare
  text.  Nodes that render nothing (comments, PIs, empty spans) do not
  split bare text, so a multi-run unit always contains a span.  Empty text
  contributes no run.  Exactly one leading "_" is stripped from the first
  run, dropping it if it becomes empty.
- One run: the key is the bare text (an untagged translation takes that
  run's formatting).  Two or more: spans are tagged in span order as
  <1>..</1><2>..</2>; bare text stays untagged.
- Marked paragraphs may contain only spans and bare text (no text:s, tabs,
  line breaks, fields, links, nested spans).
"""

import re
import xml.etree.ElementTree as ET
import zipfile

TEXT_NS = "urn:oasis:names:tc:opendocument:xmlns:text:1.0"
TABLE_NS = "urn:oasis:names:tc:opendocument:xmlns:table:1.0"
STYLE_NS = "urn:oasis:names:tc:opendocument:xmlns:style:1.0"
CALCEXT_NS = "urn:org:documentfoundation:names:experimental:calc:xmlns:calcext:1.0"
DRAW_NS = "urn:oasis:names:tc:opendocument:xmlns:drawing:1.0"
XLINK_NS = "http://www.w3.org/1999/xlink"

TEXT_P = f"{{{TEXT_NS}}}p"
TEXT_H = f"{{{TEXT_NS}}}h"
TEXT_SPAN = f"{{{TEXT_NS}}}span"
TABLE_TABLE = f"{{{TABLE_NS}}}table"
TABLE_NAME = f"{{{TABLE_NS}}}name"
TABLE_HELP_MESSAGE = f"{{{TABLE_NS}}}help-message"
TABLE_TITLE = f"{{{TABLE_NS}}}title"
TABLE_FORMULA = f"{{{TABLE_NS}}}formula"
TABLE_NAMED_EXPRESSIONS = f"{{{TABLE_NS}}}named-expressions"
DRAW_PAGE = f"{{{DRAW_NS}}}page"
DRAW_PAGE_NAME = f"{{{DRAW_NS}}}name"
STYLE_MASTER_PAGE = f"{{{STYLE_NS}}}master-page"
STYLE_NAME_ATTR = f"{{{STYLE_NS}}}name"

# The closed set of attributes that may reference a sheet by name.  The
# engine utility rewrites exactly these on sheet rename; the lints below
# reject marked sheet names anywhere else.
SHEET_REF_ATTRS = (
    f"{{{TABLE_NS}}}base-cell-address",
    f"{{{STYLE_NS}}}base-cell-address",
    f"{{{CALCEXT_NS}}}target-range-address",
    f"{{{CALCEXT_NS}}}base-cell-address",
)

NOT_MARKED = "not-marked"
MARKED = "marked"
FORBIDDEN = "forbidden"

KEY_MAX_LENGTH = 2000

# document text that would parse as a run tag is forbidden (key grammar)
TAG_LOOKALIKE_RE = re.compile(r"</?[0-9]+>")


class _CommentAndPITreeBuilder(ET.TreeBuilder):
    """Keeps comments and PIs as nodes.  TreeBuilder does that itself from
    Python 3.8 on, through insert_comments= and insert_pis=; the build runs
    this on 3.6 too, where the handlers have to build the nodes."""

    def comment(self, text):
        self.start(ET.Comment, {})
        self.data(text)
        self.end(ET.Comment)

    def pi(self, target, text=None):
        self.start(ET.ProcessingInstruction, {})
        self.data(target if text is None else target + " " + text)
        self.end(ET.ProcessingInstruction)


def parse_xml_bytes(data):
    """Parse XML keeping comments/PIs as nodes, mirroring the engine's
    libxml2 view; collect_runs treats them as invisible and merges the bare
    text around them, exactly like the engine's collectRuns."""
    parser = ET.XMLParser(target=_CommentAndPITreeBuilder())
    return ET.fromstring(data, parser=parser)


def _is_comment_or_pi(node):
    return node.tag in (ET.Comment, ET.ProcessingInstruction)


def _pretty_tag(tag):
    if not isinstance(tag, str):
        return "comment/PI"
    if tag.startswith("{"):
        ns, local = tag[1:].split("}", 1)
        prefix = {
            TEXT_NS: "text",
            TABLE_NS: "table",
            STYLE_NS: "style",
            CALCEXT_NS: "calcext",
            DRAW_NS: "draw",
            XLINK_NS: "xlink",
        }.get(ns)
        return f"{prefix}:{local}" if prefix else tag
    return tag


class Run:
    def __init__(self, text, is_span):
        self.text = text
        self.is_span = is_span


class ParagraphRuns:
    def __init__(self, state, runs=None, segments=None, forbidden=""):
        self.state = state
        self.runs = runs if runs is not None else []  # merged, marker-stripped
        self.segments = segments if segments is not None else []  # raw: pre-merge
        self.forbidden = forbidden  # what made the paragraph FORBIDDEN


def collect_runs(para):
    """The §3.1 run list of one text:p / text:h element."""
    segments = []
    forbidden = ""

    def note_forbidden(what):
        nonlocal forbidden
        if not forbidden:
            forbidden = what

    if para.text:
        segments.append(Run(para.text, False))
    for child in para:
        if _is_comment_or_pi(child):
            pass  # invisible; its tail below is a separate bare text node
        elif child.tag == TEXT_SPAN:
            # a span's text is its direct text nodes: leading text plus the
            # tail of every (comment-only) child; element children are banned
            text = child.text or ""
            for grand in child:
                if not _is_comment_or_pi(grand):
                    note_forbidden(f"{_pretty_tag(grand.tag)} inside a span")
                text += grand.tail or ""
            if text:
                segments.append(Run(text, True))
        else:
            note_forbidden(_pretty_tag(child.tag))
        if child.tail:
            segments.append(Run(child.tail, False))

    if not segments or not segments[0].text.startswith("_"):
        return ParagraphRuns(NOT_MARKED, [], segments)
    if forbidden:
        return ParagraphRuns(FORBIDDEN, [], segments, forbidden)

    runs = []
    for segment in segments:
        if not segment.is_span and runs and not runs[-1].is_span:
            runs[-1].text += segment.text
        else:
            runs.append(Run(segment.text, segment.is_span))
    runs[0].text = runs[0].text[1:]
    if not runs[0].text:
        runs.pop(0)
    return ParagraphRuns(MARKED, runs, segments)


def build_key(runs):
    if len(runs) == 1:
        return runs[0].text
    parts = []
    span_index = 0
    for run in runs:
        if run.is_span:
            span_index += 1
            parts.append(f"<{span_index}>{run.text}</{span_index}>")
        else:
            parts.append(run.text)
    return "".join(parts)


def derive_key_from_paragraph_xml(snippet):
    """Test helper mirroring the engine's deriveKeyFromParagraphXml():
    returns (state, key) for a paragraph given as a standalone XML snippet."""
    wrapped = (
        f'<root xmlns:text="{TEXT_NS}" xmlns:xlink="{XLINK_NS}">{snippet}</root>'
    )
    root = parse_xml_bytes(wrapped.encode("utf-8"))
    para = next(child for child in root if not _is_comment_or_pi(child))
    result = collect_runs(para)
    key = build_key(result.runs) if result.state == MARKED and result.runs else ""
    return result.state, key


def msgid_tag_count(msgid):
    """How many <N> tags a translation of this msgid may use: the msgid's
    highest span tag.  A tagless msgid allows no tags at all."""
    tags = [int(m) for m in re.findall(r"<([0-9]+)>", msgid)]
    return max(tags) if tags else 0


def parse_msgstr_tokens(msgstr, tag_count):
    """Parse a msgstr into a flat token list [(span, text), ...]; 0 is
    untagged literal text (bare paragraph text, or the run's own formatting
    in a single-run paragraph).  Tags may be reordered, repeated or omitted.
    Returns None when invalid: stray or unclosed tags, nesting, or a span
    index outside [1, tag_count].

    Mirrors parseMsgstrTokens() in the engine utility."""
    n = len(msgstr)

    def parse_tag(pos):
        i = pos + 1
        closing = i < n and msgstr[i] == "/"
        if closing:
            i += 1
        start = i
        while i < n and msgstr[i] in "0123456789" and i - start < 9:
            i += 1
        if i == start or i >= n or msgstr[i] != ">":
            return None
        return int(msgstr[start:i]), closing, i + 1

    tokens = []
    literal = []
    pos = 0
    while pos < n:
        if msgstr[pos] != "<":
            literal.append(msgstr[pos])
            pos += 1
            continue
        tag = parse_tag(pos)
        if tag is None:  # a lone "<" is literal text
            literal.append(msgstr[pos])
            pos += 1
            continue
        num, closing, after = tag
        if closing or num < 1 or num > tag_count:
            return None  # stray close, or span index out of range
        if literal:
            tokens.append((0, "".join(literal)))
            literal = []
        content = []
        closed = False
        scan = after
        while scan < n:
            if msgstr[scan] != "<":
                content.append(msgstr[scan])
                scan += 1
                continue
            inner = parse_tag(scan)
            if inner is None:
                content.append(msgstr[scan])
                scan += 1
                continue
            inum, iclosing, iend = inner
            if not iclosing or inum != num:
                return None  # nesting, or mismatched close
            closed = True
            scan = iend
            break
        if not closed:
            return None
        tokens.append((num, "".join(content)))
        pos = scan
    if literal:
        tokens.append((0, "".join(literal)))
    return tokens


def is_xml_safe_text(text):
    """True when every character may appear in an XML 1.0 document (its Char
    production).

    The serializers on both sides escape "&", "<" and ">" and nothing else,
    so any other character we write lands in content.xml verbatim and the
    package no longer parses.  0x7F-0x9F stay allowed: only XML 1.1 restricts
    them, and ODF is 1.0.

    Mirrors isXmlSafeUtf8() in the engine utility; the encoding half of that
    check has no counterpart here, since polib already decoded the .po."""
    for ch in text:
        cp = ord(ch)
        if cp in (0x09, 0x0A, 0x0D):
            continue
        if 0x20 <= cp <= 0xD7FF or 0xE000 <= cp <= 0xFFFD or 0x10000 <= cp <= 0x10FFFF:
            continue
        return False
    return True


def escape_sheet_name(name):
    return name.replace("'", "''")


def is_simple_sheet_name(name):
    return re.fullmatch(r"[A-Za-z0-9_]+", name) is not None


def rewrite_sheet_refs(value, renames):
    """Rewrite sheet-name occurrences in one referencing-attribute value.
    renames is [(old_with_marker, new_name), ...].  Quoted occurrences
    ('old', with '' escaping) are replaced first; a remaining bare
    occurrence gets the new name, quoted unless it needs no quoting.
    Longest old name first, so a name that is a prefix of another cannot
    corrupt the longer one.

    Mirrors rewriteSheetRefs() in the engine utility."""
    for old, new in sorted(renames, key=lambda rename: -len(rename[0])):
        quoted_old = "'" + escape_sheet_name(old) + "'"
        quoted_new = "'" + escape_sheet_name(new) + "'"
        value = value.replace(quoted_old, quoted_new)
        if old in value:
            value = value.replace(old, new if is_simple_sheet_name(new) else quoted_new)
    return value


class Unit:
    def __init__(self, key, kind, where):
        self.key = key
        self.kind = kind  # "paragraph" | "sheet-name" | "help-title"
        self.where = where  # human-readable location, used for "#." comments


def _key_lint(key, run_texts, where, errors):
    for text in run_texts:
        if "\n" in text or "\t" in text:
            errors.append(f"{where}: marked text contains a newline or tab")
            break
    for text in run_texts:
        if TAG_LOOKALIKE_RE.search(text):
            errors.append(
                f"{where}: marked text contains literal '<N>' or '</N>', which "
                "collides with the run-tag grammar"
            )
            break
    if key.startswith("#"):
        errors.append(f"{where}: key starts with '#' (reserved for stream comments)")
    if len(key) > KEY_MAX_LENGTH:
        errors.append(f"{where}: key longer than {KEY_MAX_LENGTH} characters")


def _analyze_paragraph(para, where, units, errors, warnings):
    result = collect_runs(para)

    # an underscore is a marker only as the paragraph's first character;
    # anywhere else it is literal text - warn in case a marker was meant
    for segment in result.segments[1:]:
        if segment.text.startswith("_"):
            warnings.append(
                f"{where}: '_' at the start of a non-first run is literal "
                "text; if it was meant as a marker, move it to the very "
                "start of the paragraph"
            )

    if result.state == FORBIDDEN:
        errors.append(
            f"{where}: {result.forbidden} inside a marked paragraph - marked "
            "paragraphs may only contain plain text and text:span runs"
        )
        return
    if result.state != MARKED:
        return
    if not result.runs:
        errors.append(f"{where}: marker-only paragraph ('_' with no text)")
        return

    if result.runs[0].text[:1] in (" ", "\t"):
        errors.append(f"{where}: whitespace directly after the '_' marker")
    if result.runs[-1].text != result.runs[-1].text.rstrip():
        warnings.append(f"{where}: trailing whitespace at the end of the paragraph")

    key = build_key(result.runs)
    _key_lint(key, [run.text for run in result.runs], where, errors)
    if key.startswith("_"):
        warnings.append(
            f"{where}: text begins with a double underscore; old engine builds "
            "will show a leftover '_'"
        )
    units.append(Unit(key, "paragraph", where))


def _context_for(el, context):
    if el.tag == TABLE_TABLE:
        name = el.get(TABLE_NAME, "")
        return f"sheet '{name.lstrip('_')}'"
    if el.tag == DRAW_PAGE:
        return f"slide '{el.get(DRAW_PAGE_NAME, '?')}'"
    if el.tag == STYLE_MASTER_PAGE:
        return f"master page '{el.get(STYLE_NAME_ATTR, '?')}'"
    if el.tag == TABLE_HELP_MESSAGE:
        return "validation help message"
    return context


def _walk(el, member, context, units, errors, warnings):
    for child in el:
        if _is_comment_or_pi(child):
            continue
        if child.tag in (TEXT_P, TEXT_H):
            _analyze_paragraph(
                child, f"{member} ({context})" if context else member,
                units, errors, warnings)
            continue  # paragraphs are leaves of the walk, like in the engine
        child_context = _context_for(child, context)
        where = f"{member} ({child_context})" if child_context else member

        if child.tag == TABLE_TABLE:
            name = child.get(TABLE_NAME, "")
            if name.startswith("_"):
                key = name[1:]
                if not key:
                    errors.append(f"{where}: marker-only sheet name")
                else:
                    _key_lint(key, [key], where + " [sheet name]", errors)
                    units.append(Unit(key, "sheet-name", f"{member} (sheet name)"))
        elif child.tag == TABLE_HELP_MESSAGE:
            title = child.get(TABLE_TITLE, "")
            if title.startswith("_"):
                key = title[1:]
                if not key:
                    errors.append(f"{where}: marker-only validation title")
                else:
                    _key_lint(key, [key], where + " [validation title]", errors)
                    units.append(Unit(key, "help-title", f"{member} (validation title)"))

        _walk(child, member, child_context, units, errors, warnings)


def _iter_elements(root):
    yield root
    for el in root.iter():
        if el is not root and not _is_comment_or_pi(el):
            yield el


def _lint_marked_attributes(root, member, warnings):
    """Only table:name on table:table and table:title on table:help-message
    are translated; a "_" starting any other attribute value is literal text
    the mechanism will neither translate nor strip - warn in case a marker
    was meant."""
    for el in _iter_elements(root):
        for attr, value in el.attrib.items():
            if not value.startswith("_"):
                continue
            if el.tag == TABLE_TABLE and attr == TABLE_NAME:
                continue
            if el.tag == TABLE_HELP_MESSAGE and attr == TABLE_TITLE:
                continue
            if attr.endswith("style-name"):
                continue  # style references are internal names, not text
            warnings.append(
                f"{member}: '_' at the start of untranslated attribute "
                f"{_pretty_tag(attr)} of {_pretty_tag(el.tag)} is literal "
                f"text and will be shown as-is (value {value!r})"
            )


def _lint_sheet_rename_envelope(root, member, errors):
    """The utility renames sheets textually: table:name plus the closed
    SHEET_REF_ATTRS set, quoted-form occurrences only.  Reject anything that
    rename could miss or corrupt: formulas, named expressions, marked names
    in other attributes, or unquoted references."""
    marked = [
        el.get(TABLE_NAME)
        for el in _iter_elements(root)
        if el.tag == TABLE_TABLE and el.get(TABLE_NAME, "").startswith("_")
    ]
    if not marked:
        return

    for el in _iter_elements(root):
        if el.tag == TABLE_NAMED_EXPRESSIONS and len(el):
            errors.append(
                f"{member}: named expressions present while sheet names are "
                "marked - the textual rename cannot update them"
            )
        for attr, value in el.attrib.items():
            if attr == TABLE_FORMULA:
                errors.append(
                    f"{member}: table:formula present while sheet names are "
                    "marked - the textual rename cannot safely update formulas"
                )
                continue
            hits = [name for name in marked if name in value]
            if not hits:
                continue
            if el.tag == TABLE_TABLE and attr == TABLE_NAME and value in marked:
                continue
            if attr not in SHEET_REF_ATTRS:
                errors.append(
                    f"{member}: marked sheet name referenced in unsupported "
                    f"attribute {_pretty_tag(attr)} of {_pretty_tag(el.tag)} "
                    f"(value {value!r})"
                )
                continue
            # inside the closed set only quoted 'name' occurrences survive
            residue = value
            for name in sorted(marked, key=len, reverse=True):
                residue = residue.replace("'" + escape_sheet_name(name) + "'", "\x00")
            for name in hits:
                if name in residue:
                    errors.append(
                        f"{member}: unquoted reference to marked sheet name "
                        f"{name!r} in {_pretty_tag(attr)} (value {value!r})"
                    )


def analyze_document(odf_path):
    """Extract translation units and lint findings from an ODF file.

    Returns (units, errors, warnings); errors mean the document violates the
    marker spec and must be fixed before the tooling can process it."""
    units, errors, warnings = [], [], []
    with zipfile.ZipFile(odf_path) as zf:
        names = zf.namelist()
        if "content.xml" not in names:
            errors.append("content.xml missing from the document")
            return units, errors, warnings
        for member in ("content.xml", "styles.xml"):
            if member not in names:
                continue
            root = parse_xml_bytes(zf.read(member))
            _walk(root, member, "", units, errors, warnings)
            _lint_marked_attributes(root, member, warnings)
            _lint_sheet_rename_envelope(root, member, errors)

    by_normalized = {}
    for unit in units:
        normalized = " ".join(unit.key.split())
        other = by_normalized.setdefault(normalized, unit.key)
        if other != unit.key:
            warnings.append(
                "near-duplicate msgids (differ only in whitespace): "
                f"{other!r} vs {unit.key!r}"
            )
    return units, errors, warnings
