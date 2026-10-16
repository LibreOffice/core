#!/usr/bin/env python3
"""How far the round's rules reach over a directory of RTF.

    census.py <root>

Counts, per document: whether it states `\htmautsp`; whether any paragraph style inherits an
alignment or a space-before it does not state itself (the shape the paragraph half moves); and
whether any `\sbasedon` names a style the sheet has not reached yet.
"""
import sys
import re
import pathlib

ROOT = pathlib.Path(sys.argv[1])
ENTRY = re.compile(r"\{\\s(\d+)([^;{}]*)")
STATES = re.compile(r"\\(qc|qr|qj|ql|sb-?\d+|sbasedon\d+|keepn)")

htm = inherit = forward = total = 0
for path in sorted(ROOT.rglob("*.rtf")):
    text = path.read_text("latin-1", errors="replace")
    total += 1
    if "\\htmautsp" in text:
        htm += 1

    start = text.find("{\\stylesheet")
    if start < 0:
        continue
    sheet = text[start:start + 400000]
    states, order, seen_forward, seen_inherit = {}, [], False, False
    for m in ENTRY.finditer(sheet):
        sid, words = int(m.group(1)), m.group(2)
        own = {w for w in STATES.findall(words)}
        parent = next((int(w[8:]) for w in own if w.startswith("sbasedon")), None)
        if parent is not None and parent not in states:
            seen_forward = True
        states[sid] = (own, parent)
        order.append(sid)

    def carries(words):
        return any(w in ("qc", "qr", "qj") or (w.startswith("sb") and w[2:3].isdigit())
                   for w in words)

    for sid in order:
        own, parent = states[sid]
        if carries(own) or "ql" in own:
            continue
        while parent is not None and parent in states and parent != sid:
            pown, parent = states[parent]
            if carries(pown):
                seen_inherit = True
                break
    inherit += 1 if seen_inherit else 0
    forward += 1 if seen_forward else 0

print(f"{total} documents under {ROOT}")
print(f"  state \\htmautsp                                   {htm}")
print(f"  a style inherits an alignment or a space before   {inherit}")
print(f"  a \\sbasedon names a style not yet defined         {forward}")
