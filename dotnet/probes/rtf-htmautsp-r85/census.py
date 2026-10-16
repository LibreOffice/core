#!/usr/bin/env python3
r"""Where a `\super` stands relative to `\htmautsp`, over a directory of RTF.

    python3 census.py /home/user/corpus-odf/words

`RTFTokenizer::dispatchKeyword` returns before dispatching anything at all when
the destination is `Destination::SKIP` (`rtftokenizer.cxx`:2156-2164), and
`RTFDocumentImpl::dispatchFlag`'s `SUPER` case skips `checkFirstRun` for a
style-sheet entry (`rtfdispatchflag.cxx`:891-895).  Everywhere else -- a list
level included -- `\super` sends the settings table.  For each file this prints
the offset of `\htmautsp`, the offset of the first `\super` that is neither
skipped nor in the style sheet, and the destination it sits in.
"""
import pathlib
import sys

# Destinations the RTF importer marks SKIP, so that nothing inside them is
# dispatched at all.  Keyed on the control word that opens the group.
SKIPPED = {
    "header", "headerl", "headerr", "headerf", "footer", "footerl", "footerr", "footerf",
    "info", "generator", "filetbl", "themedata", "colorschememapping", "datastore",
    "latentstyles", "xmlnstbl", "pgptbl", "objdata", "result", "object", "pict",
    "userprops", "docvar", "template", "operator", "company", "keywords", "comment",
    "fname", "mail", "pgdsctbl", "rsidtbl", "ftnsep", "ftnsepc", "ftncn", "aftnsep",
    "aftnsepc", "aftncn", "atnid", "atnauthor", "annotation", "falt", "listtext",
    "pnseclvl", "protusertbl", "wgrffmtfilter", "themedata",
}
GUARDED = {"stylesheet"}


def opening_word(d: bytes, i: int) -> str:
    """The control word a group opens with, `\\*` skipped."""
    j = i + 1
    while j < len(d) and d[j:j + 1] in (b"\\", b"*", b"\n", b"\r", b" "):
        if d[j:j + 1] == b"\\" and d[j + 1:j + 2] not in (b"*",):
            break
        j += 1
    if d[j:j + 1] != b"\\":
        return ""
    k = j + 1
    while k < len(d) and (65 <= d[k] <= 90 or 97 <= d[k] <= 122):
        k += 1
    return d[j + 1:k].decode("latin-1")


def scan(d: bytes) -> tuple[int, int, str]:
    """(offset of `\\htmautsp`, offset of the first live `\\super`, its destination)."""
    stack: list[str] = []
    word = -1
    sup = -1
    supdest = ""
    i, n = 0, len(d)
    while i < n:
        c = d[i]
        if c == 0x5C:
            if i + 1 < n and d[i + 1] in b"\\{}'":
                i += 2 if d[i + 1] != 0x27 else 4
                continue
            j = i + 1
            while j < n and (65 <= d[j] <= 90 or 97 <= d[j] <= 122):
                j += 1
            kw = d[i + 1:j].decode("latin-1")
            live = not any(s in SKIPPED for s in stack)
            if kw == "htmautsp" and word < 0 and live:
                word = i
            if kw == "super" and sup < 0 and live and not any(s in GUARDED for s in stack):
                sup = i
                supdest = "/".join(stack[-2:]) or "(body)"
            i = j
            continue
        if c == 0x7B:
            stack.append(opening_word(d, i))
            i += 1
            continue
        if c == 0x7D:
            if stack:
                stack.pop()
            i += 1
            continue
        i += 1
    return word, sup, supdest


def main(root: pathlib.Path) -> None:
    files = sorted(p for p in root.rglob("*.rtf"))
    states = before = 0
    print(f"{'document':60s} {'htmautsp':>9s} {'super':>9s}  destination")
    for f in files:
        word, sup, dest = scan(f.read_bytes())
        if word < 0:
            continue
        states += 1
        if 0 <= sup < word:
            before += 1
            print(f"{f.name[:60]:60s} {word:9d} {sup:9d}  {dest}")
    print(f"\n{len(files)} .rtf, {states} state the word, "
          f"{before} have a live \\super before it")


if __name__ == "__main__":
    main(pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else "/home/user/corpus-odf/words"))
