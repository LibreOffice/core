#!/usr/bin/env python3
"""Compare extracted text against a LibreOffice reference extraction.

Normalises away differences that are never real (line endings, BOMs, Unicode
composition, blank-line padding) while preserving those that can be genuine bugs
(case, punctuation, non-breaking spaces). See ../SKILL.md for the rationale per step.

Standard library only.
"""

from __future__ import annotations

import argparse
import difflib
import pathlib
import re
import sys
import unicodedata

NBSP = " "


def load(path: pathlib.Path) -> str:
    """Read a file, tolerating the encodings these filters actually emit."""
    raw = path.read_bytes()
    for encoding in ("utf-8-sig", "utf-8", "utf-16", "cp1252", "latin-1"):
        try:
            return raw.decode(encoding)
        except (UnicodeDecodeError, UnicodeError):
            continue
    # latin-1 cannot fail, so this is unreachable in practice.
    return raw.decode("latin-1", errors="replace")


def normalise(text: str, fold_spaces: bool) -> str:
    text = text.replace("﻿", "")               # BOM: an encoding artefact
    text = text.replace("\r\n", "\n").replace("\r", "\n")
    text = unicodedata.normalize("NFC", text)       # composed == decomposed
    if fold_spaces:
        # Off by default: NBSP vs space can itself be the bug being investigated.
        text = text.replace(NBSP, " ").replace(" ", " ")
    text = "\n".join(line.rstrip() for line in text.split("\n"))
    text = re.sub(r"\n{3,}", "\n\n", text)          # inconsistent block padding
    return text.strip("\n")


def token_similarity(a: str, b: str) -> float:
    """Word-level similarity. More meaningful than character ratio for prose, where a
    one-word change should not look like a large difference."""
    wa, wb = a.split(), b.split()
    if not wa and not wb:
        return 1.0
    return difflib.SequenceMatcher(None, wa, wb, autojunk=False).ratio()


def first_divergence(a: str, b: str) -> tuple[int, int, str] | None:
    """Locate the first differing line, which is where a cascade begins."""
    la, lb = a.split("\n"), b.split("\n")
    for i in range(max(len(la), len(lb))):
        x = la[i] if i < len(la) else None
        y = lb[i] if i < len(lb) else None
        if x != y:
            if x is None:
                return i + 1, 0, f"actual ended; reference still has: {y!r}"
            if y is None:
                return i + 1, 0, f"reference ended; actual still has: {x!r}"
            col = next((c + 1 for c, (p, q) in enumerate(zip(x, y)) if p != q),
                       min(len(x), len(y)) + 1)
            return i + 1, col, f"reference: {y!r}\n           actual:    {x!r}"
    return None


def describe_codepoints(a: str, b: str, limit: int = 24) -> list[str]:
    """Show differing characters with code points.

    A systematic substitution pattern here (every non-ASCII character wrong, ASCII
    perfect) means a codepage was misread, not that parsing failed.
    """
    out: list[str] = []
    matcher = difflib.SequenceMatcher(None, b, a, autojunk=False)
    for tag, i1, i2, j1, j2 in matcher.get_opcodes():
        if tag == "equal":
            continue
        exp, act = b[i1:i2], a[j1:j2]
        for ch, label in ((exp, "reference"), (act, "actual")):
            for c in ch[:8]:
                out.append(f"  {label:9s} {c!r} U+{ord(c):04X} "
                           f"{unicodedata.name(c, '<unnamed>')}")
        if len(out) >= limit:
            out.append("  ... truncated")
            return out
    return out


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--expected", required=True, type=pathlib.Path,
                    help="LibreOffice reference text file.")
    ap.add_argument("--actual", required=True, type=pathlib.Path,
                    help="Paperless extraction to check.")
    ap.add_argument("--fold-spaces", action="store_true",
                    help="Treat NBSP as a normal space. Off by default: the difference "
                         "can itself be the bug.")
    ap.add_argument("--show-codepoints", action="store_true",
                    help="List differing characters with code points (finds codepage "
                         "problems).")
    ap.add_argument("--context", type=int, default=3, help="Diff context lines.")
    ap.add_argument("--min-similarity", type=float, default=None, metavar="S",
                    help="Exit non-zero if token similarity falls below S.")
    args = ap.parse_args()

    for p in (args.expected, args.actual):
        if not p.is_file():
            print(f"not a file: {p}", file=sys.stderr)
            return 3

    exp = normalise(load(args.expected), args.fold_spaces)
    act = normalise(load(args.actual), args.fold_spaces)

    similarity = token_similarity(act, exp)
    exp_words, act_words = len(exp.split()), len(act.split())

    print(f"reference: {args.expected}  ({exp_words} words, {len(exp)} chars)")
    print(f"actual:    {args.actual}  ({act_words} words, {len(act)} chars)")
    print(f"token similarity: {similarity:.4f}")

    if exp == act:
        print("\nIDENTICAL after normalisation.")
        return 0

    # An extraction that finds MORE than the reference is usually correct: these
    # filters drop headers, comments, notes and shape text by design.
    if act_words > exp_words * 1.05 and similarity > 0.75:
        print("\nNOTE: Paperless extracted more text than the reference. This is often "
              "CORRECT -- LibreOffice's text filters drop headers/footers, comments, "
              "notes and shape text. Verify the extra content is real before treating "
              "it as a defect.")

    div = first_divergence(act, exp)
    if div:
        line, col, detail = div
        print(f"\nfirst divergence at line {line}"
              + (f", column {col}" if col else "") + ":")
        print(f"           {detail}")

    print("\nunified diff (reference -> actual):")
    diff = difflib.unified_diff(
        exp.split("\n"), act.split("\n"),
        fromfile="libreoffice-reference", tofile="paperless-actual",
        lineterm="", n=args.context)
    shown = 0
    for row in diff:
        print(row)
        shown += 1
        if shown > 200:
            print("... diff truncated at 200 lines")
            break

    if args.show_codepoints:
        details = describe_codepoints(act, exp)
        if details:
            print("\ndiffering characters:")
            print("\n".join(details))
            print("\nIf every non-ASCII character is wrong while ASCII is perfect, this "
                  "is a CODEPAGE mismatch, not a parsing bug.")

    if args.min_similarity is not None and similarity < args.min_similarity:
        print(f"\nsimilarity {similarity:.4f} below --min-similarity "
              f"{args.min_similarity}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
