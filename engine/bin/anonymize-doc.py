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
"""Anonymize a DOCX / ODT (also XLSX, PPTX, ODS, ODP, flat .fodt/.fods/.fodp)
while keeping its structure, so a layout bug can be reproduced without the
original content.

  anonymize-doc.py input.docx output.docx [--keep-images] [--keep-embeddings]

What changes:
  - every letter becomes x / X (Hebrew -> alef, Arabic -> beh, CJK -> one
    ideograph, so text direction and script stay the same), every digit -> 0.
    Spaces, punctuation, tabs, line breaks stay.
  - field codes: only text inside "quotes" is anonymized, so PAGE, TOC \\o "1-3"
    and the like keep working.
  - document properties, custom XML, metadata, authors, comment authors,
    alt text, form field defaults, external link targets.
  - the ODF office:meta element is emptied, which also drops the names of
    custom properties, the template path and the editing statistics.
  - links to files outside the package, such as a linked image or a relative
    hyperlink, lose their path, because a path often names a user or a folder.
  - images, image previews and thumbnails become a 1x1 PNG (their displayed
    size is stored in the XML, so layout does not change).
  - binary OLE parts and macros are emptied; embedded ZIP documents are
    anonymized recursively.

What stays: styles, style names, bookmark names, section/table/frame names,
fonts, page setup and compatibility settings. Check the leak report this
script prints at the end before passing the file on.
"""

import base64
import re
import sys
import unicodedata
import zipfile
import io

TINY_PNG = base64.b64decode(
    "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mP8z8BQDwAEhQGAhKmMIQAAAABJRU5ErkJggg==")
IMAGE_EXT = {"png", "jpg", "jpeg", "jfif", "gif", "bmp", "tif", "tiff", "emf",
             "wmf", "emz", "wmz", "svg", "svgz", "wdp", "hdp", "jxr", "pct",
             "pict", "webp", "eps"}

KEEP_IMAGES = "--keep-images" in sys.argv
KEEP_EMBEDDINGS = "--keep-embeddings" in sys.argv

ODF_NS = "urn:oasis:names:tc:opendocument:xmlns:office"


def map_char(c):
    cat = unicodedata.category(c)
    if cat == "Nd":
        return "0"
    if not cat.startswith("L"):
        return c
    name = unicodedata.name(c, "")
    if name.startswith("HEBREW"):
        return "א"
    if name.startswith("ARABIC"):
        return "ب"
    if unicodedata.east_asian_width(c) in "WF":
        return "一"
    return "X" if c.isupper() else "x"


ENTITY = re.compile(r"&(#x[0-9a-fA-F]+|#[0-9]+|[A-Za-z]+);")


def anon(s, digits_only=False):
    """Anonymize XML-escaped text, leaving entities intact where harmless."""
    out = []
    pos = 0
    for m in ENTITY.finditer(s):
        out.append(_anon_plain(s[pos:m.start()], digits_only))
        ent = m.group(1)
        if ent.startswith("#"):
            cp = int(ent[2:], 16) if ent[1] in "xX" else int(ent[1:])
            c = chr(cp)
            r = _anon_plain(c, digits_only)
            out.append(m.group(0) if r == c else r)
        else:
            out.append(m.group(0))
        pos = m.end()
    out.append(_anon_plain(s[pos:], digits_only))
    return "".join(out)


def _anon_plain(s, digits_only):
    if digits_only:
        return "".join("0" if unicodedata.category(c) == "Nd" else c for c in s)
    return "".join(map_char(c) for c in s)


QUOTED = re.compile(r'(&quot;|")(.*?)(&quot;|")')


def anon_quoted(s):
    return QUOTED.sub(lambda m: m.group(1) + anon(m.group(2)) + m.group(3), s)


def is_number(s):
    try:
        float(s.strip())
        return True
    except ValueError:
        return False


TOKEN = re.compile(r"<!--.*?-->|<!\[CDATA\[.*?\]\]>|<[?!].*?>|<[^>]*>|[^<]+", re.S)
ATTR = re.compile(r"""(\s)([\w:.-]+)(\s*=\s*)("[^"]*"|'[^']*')""")
TAGNAME = re.compile(r"</?\s*([^\s/>]+)")


def local(n):
    return n.split(":", 1)[-1]


def prefix(n):
    return n.split(":", 1)[0] if ":" in n else ""


# ---------------------------------------------------------------- OOXML rules

OOXML_ANON_ATTRS = {
    # (element local name or "*", attribute local name)
    ("docPr", "descr"), ("docPr", "title"), ("docPr", "name"),
    ("cNvPr", "descr"), ("cNvPr", "title"), ("cNvPr", "name"),
    ("*", "author"), ("*", "initials"), ("*", "userId"), ("*", "providerId"),
    ("alias", "val"), ("tag", "val"), ("docVar", "val"),
    ("default", "val"), ("helpText", "val"), ("statusText", "val"),
    ("listEntry", "val"), ("listItem", "displayText"), ("listItem", "value"),
    ("hyperlink", "tooltip"), ("textpath", "string"), ("imagedata", "title"),
    ("*", "creator"), ("*", "lastModifiedBy"), ("comment", "authorId"),
    ("property", "name"),  # custom document property names in docProps/custom.xml
}


def ooxml_text(path, stack, text):
    name = local(stack[-1][0]) if stack else ""
    if path.startswith(("docProps/", "customXml/")) or "/customXml/" in path:
        return anon(text)
    if name in ("t", "delText"):
        return anon(text)
    if name in ("instrText", "delInstrText"):
        return anon_quoted(text)
    if name == "v":
        # shared-string index in a worksheet: keep
        if len(stack) > 1 and local(stack[-2][0]) == "c" and re.search(r'\st="s"', stack[-2][1]):
            return text
        return anon(text, digits_only=is_number(text))
    if name in ("lpwstr", "lpstr", "bstr", "title", "subject", "creator",
                "keywords", "description", "lastModifiedBy", "category",
                "Company", "Manager", "HyperlinkBase", "contentStatus"):
        return anon(text)
    return text


def ooxml_attr(tag, tagtext, aname, value):
    tl, al = local(tag), local(aname)
    if tl == "Relationship" and al == "Target" and 'TargetMode="External"' in tagtext:
        return '"http://example.invalid/"'
    if tl == "fldSimple" and al == "instr":
        return anon_quoted(value)
    if (tl, al) in OOXML_ANON_ATTRS or ("*", al) in OOXML_ANON_ATTRS:
        return anon(value)
    return value


# ------------------------------------------------------------------ ODF rules

ODF_ANON_ATTRS = {"office:string-value", "text:string-value", "text:description",
                  "form:value", "form:current-value", "form:label", "form:title",
                  "office:title", "text:author", "dc:creator", "text:initials",
                  "office:name", "xlink:title", "table:content-validation-name"}
ODF_DIGIT_ATTRS = {"office:value", "text:value"}


def odf_text(path, stack, text):
    names = [s[0] for s in stack]
    if any(prefix(n) == "number" for n in names):
        return text
    if "office:binary-data" in names:
        return text  # replaced wholesale in transform()
    if "config:config-item" in names:
        if re.search(r'config:name="Printer(Name|Setup)"', stack[-1][1]):
            return ""
        return text
    if "office:settings" in names:
        return text
    return anon(text)


# Top-level entries of each ZIP package being transformed, innermost last. A link whose first
# path segment is one of these points into the package and keeps its target.
PACKAGE_ENTRIES = []


def is_package_link(v):
    if v.startswith("#"):
        return True
    if not PACKAGE_ENTRIES:
        return False
    v = v[2:] if v.startswith("./") else v
    return v.split("/", 1)[0] in PACKAGE_ENTRIES[-1]


def odf_attr(tag, tagtext, aname, value):
    if aname == "xlink:href":
        v = value[1:-1]
        if "://" in v or v.startswith(("mailto:", "file:")):
            return '"http://example.invalid/"'
        if not is_package_link(v):
            return anon(value)
        return value
    if aname in ODF_ANON_ATTRS:
        return anon(value)
    if aname in ODF_DIGIT_ATTRS:
        return anon(value, digits_only=True)
    return value


# ------------------------------------------------------------------ transform

def transform_xml(path, data):
    try:
        s = data.decode("utf-8")
    except UnicodeDecodeError:
        return data
    odf = ODF_NS in s[:5000] or ODF_NS in s
    text_rule, attr_rule = (odf_text, odf_attr) if odf else (ooxml_text, ooxml_attr)
    out = []
    stack = []
    meta_depth = None  # stack depth of an open office:meta element, whose content is dropped
    for m in TOKEN.finditer(s):
        tok = m.group(0)
        if meta_depth is not None:
            if tok.startswith("</") and len(stack) == meta_depth + 1:
                meta_depth = None
            elif tok.startswith("<") and not tok.startswith(("<?", "<!")):
                if tok.startswith("</"):
                    stack.pop()
                elif not tok.endswith("/>"):
                    stack.append(("", tok))
                continue
            else:
                continue
        if tok.startswith("<"):
            if tok.startswith(("<?", "<!")):
                out.append(tok)
                continue
            tm = TAGNAME.match(tok)
            name = tm.group(1) if tm else ""
            if (odf and name == "office:meta" and not tok.startswith("</")
                    and not tok.endswith("/>")):
                meta_depth = len(stack)
            if tok.startswith("</"):
                if stack:
                    stack.pop()
                out.append(tok)
                continue
            tok = ATTR.sub(lambda a: a.group(1) + a.group(2) + a.group(3)
                           + attr_rule(name, tok, a.group(2), a.group(4)), tok)
            if not tok.endswith("/>"):
                stack.append((name, tok))
            out.append(tok)
        else:
            if odf and stack and stack[-1][0] == "office:binary-data":
                out.append(base64.b64encode(TINY_PNG).decode() if not KEEP_IMAGES else tok)
            else:
                out.append(text_rule(path, stack, tok))
    return "".join(out).encode("utf-8")


def transform_part(path, data, report):
    base = path.rsplit("/", 1)[-1]
    ext = base.rsplit(".", 1)[-1].lower() if "." in base else ""
    if data[:4] == b"PK\x03\x04":
        return transform_zip(data, report, path + "/")
    if (ext in IMAGE_EXT or path.startswith(("Thumbnails/", "ObjectReplacements/"))
            or "/ObjectReplacements/" in path or base.startswith("thumbnail")):
        if KEEP_IMAGES:
            return data
        report["images"] += 1
        return TINY_PNG
    if data[:8] == b"\xd0\xcf\x11\xe0\xa1\xb1\x1a\xe1":  # OLE2: .bin objects, macros
        if KEEP_EMBEDDINGS:
            return data
        report["ole"] += 1
        return b""
    if ext in ("xml", "rels", "vml", "fodt", "fods", "fodp", "fodg") or data.lstrip()[:5] == b"<?xml":
        report["xml"] += 1
        return transform_xml(path, data)
    return data


def transform_zip(data, report, prefix_path=""):
    zin = zipfile.ZipFile(io.BytesIO(data))
    PACKAGE_ENTRIES.append({n.split("/", 1)[0] for n in zin.namelist()})
    try:
        return _transform_zip(zin, report, prefix_path)
    finally:
        PACKAGE_ENTRIES.pop()


def _transform_zip(zin, report, prefix_path):
    buf = io.BytesIO()
    with zipfile.ZipFile(buf, "w") as zout:
        for info in zin.infolist():
            content = zin.read(info.filename)
            if not info.is_dir():
                content = transform_part(info.filename, content, report)
            ni = zipfile.ZipInfo(info.filename, date_time=(1980, 1, 1, 0, 0, 0))
            ni.compress_type = info.compress_type
            ni.external_attr = info.external_attr
            zout.writestr(ni, content)
            if not prefix_path:
                report["parts"].append(info.filename)
    return buf.getvalue()


# ---------------------------------------------------------------- leak report

def leak_report(data):
    """Collect word-like strings still present, so the user can eyeball them."""
    words = {}

    def scan(blob):
        if blob[:4] == b"PK\x03\x04":
            z = zipfile.ZipFile(io.BytesIO(blob))
            for n in z.namelist():
                scan(z.read(n))
            return
        try:
            s = blob.decode("utf-8")
        except UnicodeDecodeError:
            return
        if "<" not in s:
            return
        for m in TOKEN.finditer(s):
            tok = m.group(0)
            if tok.startswith("<"):
                vals = [a.group(4)[1:-1] for a in ATTR.finditer(tok)]
            else:
                vals = [tok]
            for v in vals:
                if v.startswith(("http://schemas.", "urn:", "http://www.w3.org", "http://purl.org")):
                    continue
                for w in re.findall(r"[^\W\d_]{4,}", v):
                    if set(w.lower()) <= {"x"}:
                        continue
                    words[w] = words.get(w, 0) + 1

    scan(data)
    return words


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    if len(args) != 2:
        print(__doc__)
        sys.exit(1)
    src, dst = args
    data = open(src, "rb").read()
    report = {"xml": 0, "images": 0, "ole": 0, "parts": []}
    if data[:4] == b"PK\x03\x04":
        out = transform_zip(data, report)
    elif data.lstrip()[:5] == b"<?xml" or data.lstrip()[:1] == b"<":
        report["xml"] += 1
        out = transform_xml(src, data)
    else:
        sys.exit("Not a ZIP or XML document (.doc/.rtf are not supported).")
    open(dst, "wb").write(out)

    print(f"Wrote {dst}: {report['xml']} XML parts anonymized, "
          f"{report['images']} images replaced, {report['ole']} binary OLE parts emptied.")
    words = leak_report(out)
    print("\nRemaining words of 4+ letters (style names, bookmark names, fonts, "
          "XML vocabulary...). Check that nothing sensitive is left:\n")
    for w, n in sorted(words.items(), key=lambda kv: (-kv[1], kv[0])):
        print(f"  {n:6}  {w}")


if __name__ == "__main__":
    main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
