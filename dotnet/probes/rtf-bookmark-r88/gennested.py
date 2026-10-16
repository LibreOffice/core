#!/usr/bin/env python3
r"""A group nested inside a bookmark's destination, which is not a bookmark half.

A `{...}` inside `{\*\bkmkstart …}` inherits the destination and closes with a
collection of its own, so a reader that records a half per closing group records a
second, unnamed one -- and an unnamed half is not free: it takes an id and moves
every name after it. `RTFDocumentImpl::popState` guards exactly this case with
`if (&getDestinationText() != getCurrentDestinationText()) break; // not for nested
group` (rtfdocumentimpl.cxx:2736-2740, :2751-2755).

  k_nested   a nested group inside the start's own destination
  l_control  the same document without it, which must draw the same thing

  gennested.py <outdir>
"""
import pathlib
import sys

OUT = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else '.')
OUT.mkdir(parents=True, exist_ok=True)

HDR = ("{\\rtf1\\ansi\\ansicpg1252\\deff0\n"
       "{\\fonttbl{\\f0\\froman\\fcharset0 Liberation Serif;}}\n"
       "\\paperw12240\\paperh15840\\margl1440\\margr1440\\margt1440\\margb1440\n")

REFS = "\\pard\\plain [A] {\\field{\\*\\fldinst  REF A \\\\h }{\\fldrslt CACHED}} ;\\par\n"

# The nested group goes in the *end* half, where the damage is visible: an unnamed end takes
# `m_aBookmarks[""]`, which std::map default-constructs as 0, so it closes the first bookmark
# of the document under the wrong name and leaves the real end to open a second one.
BODY = "\\pard\\plain {\\*\\bkmkstart A}first{\\*\\bkmkend %s} second\\par\n"

(OUT / 'k_nested.rtf').write_text(HDR + BODY % "{x}A" + REFS + "}\n", encoding='ascii')
(OUT / 'l_control.rtf').write_text(HDR + BODY % "A" + REFS + "}\n", encoding='ascii')

print('written 2 probes to', OUT)
