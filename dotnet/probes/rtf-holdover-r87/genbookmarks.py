#!/usr/bin/env python3
r"""The two bookmark probes that reproduce writerfilter's name rotation.

RTF sends a bookmark's name before its id (rtfdocumentimpl.cxx:224-236) and
DomainMapper_Impl::SetBookmarkName (DomainMapper_Impl.cxx:9426-9447) expects the
OOXML order, so each name lands on the previously opened bookmark.

  H_five  the corpus's own shape: five \bkmkstart, three collapsed \bkmkend, a
          {\field}, two more \bkmkend. 26.2.4.2 draws the REF as the whole caption.
  I_two   two starts only, which comes out as two bookmarks with one name --
          `R1` and `R1 Copy 1`.

Render each `--convert-to pdf` for what the REF says and `--convert-to fodt` for
where the bookmarks actually landed.
"""
import pathlib, sys

OUT = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else '.')
OUT.mkdir(parents=True, exist_ok=True)

HDR = ("{\\rtf1\\ansi\\ansicpg1252\\deff0\n"
       "{\\fonttbl{\\f0\\froman\\fcharset0 Liberation Serif;}}\n"
       "\\paperw12240\\paperh15840\\margl1440\\margr1440\\margt1440\\margb1440\n")
TAIL = ("\\pard\\plain See {\\field{\\*\\fldinst  REF R1 \\\\h }{\\fldrslt Table 48}} end.\\par\n}\n")

FIVE = ("\\pard\\plain {\n"
        "{\\*\\bkmkstart T1}{\\*\\bkmkstart T2}{\\*\\bkmkstart R2}{\\*\\bkmkstart T3}{\\*\\bkmkstart R1}"
        "{\\*\\bkmkend T1}{\\*\\bkmkend T2}{\\*\\bkmkend R2}Table }{\n"
        "{\\field{\\*\\fldinst  SEQ Table \\\\* ARABIC }{\\fldrslt 48}}}{\n"
        "{\\*\\bkmkend R1}: Caption Words Here{\\*\\bkmkend T3}}\n"
        "\\par\n")
TWO = ("\\pard\\plain {\n"
       "{\\*\\bkmkstart T3}{\\*\\bkmkstart R1}Table }{\n"
       "{\\field{\\*\\fldinst  SEQ Table \\\\* ARABIC }{\\fldrslt 48}}}{\n"
       "{\\*\\bkmkend R1}: Caption Words Here{\\*\\bkmkend T3}}\n"
       "\\par\n")

(OUT / 'H_five.rtf').write_text(HDR + FIVE + TAIL, encoding='ascii')
(OUT / 'I_two.rtf').write_text(HDR + TWO + TAIL, encoding='ascii')
print('written to', OUT)
