#!/usr/bin/env python3
"""The style probes this round is decided on.

Each writes one small .rtf; render both ways and read the size, the position and
the page a paragraph lands on.

  ord_*        the same two stylesheet entries in both orders
  fw_*         a forward \sbasedon under twelve different style names
  q_*, sp_*    what a style's own statement costs it, and the pool's spacing
  n_*          the pool's keep-with-next, swept across the page boundary
"""
import pathlib, sys

OUT = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else '.')
OUT.mkdir(parents=True, exist_ok=True)

def doc(styles, body):
    return ("{\\rtf1\\ansi\\ansicpg1252\\deff0\n"
            "{\\fonttbl{\\f0\\froman\\fcharset0 Liberation Serif;}}\n"
            "{\\stylesheet" + styles + "}\n"
            "\\paperw12240\\paperh15840\\margl1440\\margr1440\\margt1440\\margb1440\n"
            + body + "}\n")

NORMAL = "{\\s0\\snext0\\f0\\fs20 Normal;}"
BASE = "{\\s1392\\sbasedon0\\snext1392\\f0\\fs18\\b Notes/Cautions Heading;}"
H4 = "{\\s4\\sbasedon1392\\snext0\\sb120\\sa120 heading 4;}"
ONE = "\\pard\\plain \\s4\\sb120\\sa120{\nCAUTIONS}\\par\n"

(OUT / 'ord_after.rtf').write_text(doc(NORMAL + H4 + BASE, ONE), encoding='ascii')
(OUT / 'ord_before.rtf').write_text(doc(NORMAL + BASE + H4, ONE), encoding='ascii')

NAMES = {'h1': ('heading 1', 1), 'h2': ('heading 2', 2), 'h3': ('heading 3', 3),
         'h4': ('heading 4', 4), 'h5': ('heading 5', 5), 'widget': ('Widget Heading', 7),
         'bodytext': ('Body Text', 8), 'caption': ('caption', 9), 'title': ('Title', 10),
         'subtitle': ('Subtitle', 11), 'quote': ('Quote', 12), 'list': ('List Paragraph', 13)}
for key, (name, sid) in NAMES.items():
    styles = NORMAL + "{\\s%d\\sbasedon1392\\snext0 %s;}" % (sid, name) + BASE
    (OUT / f'fw_{key}.rtf').write_text(
        doc(styles, "\\pard\\plain \\s%d{\nCAUTIONS}\\par\n" % sid), encoding='ascii')

def three(sid):
    return ("\\pard\\plain \\fs20{\nAAA}\\par\n"
            "\\pard\\plain \\s%d{\nHEAD}\\par\n" % sid +
            "\\pard\\plain \\fs20{\nBBB}\\par\n")

for tag, styles, sid in [
        ('q_ownsp', NORMAL + "{\\s7\\snext0\\sb480\\sa480\\fs28 Widget Heading;}", 7),
        ('q_ownsp_p', NORMAL + "{\\s7\\sbasedon0\\snext0\\sb480\\sa480\\fs28 Widget Heading;}", 7),
        ('q_h4_nobase', NORMAL + "{\\s4\\snext0 heading 4;}", 4),
        ('q_h4_fwd', NORMAL + "{\\s4\\sbasedon1392\\snext0 heading 4;}" + BASE, 4),
        ('q_h4_own', NORMAL + "{\\s4\\snext0\\fs20 heading 4;}", 4),
        ('sp_h4', NORMAL + "{\\s4\\sbasedon1392\\snext0 heading 4;}" + BASE, 4),
        ('sp_widget', NORMAL + "{\\s7\\sbasedon1392\\snext0 Widget Heading;}" + BASE, 7)]:
    (OUT / f'{tag}.rtf').write_text(doc(styles, three(sid)), encoding='ascii')

def keep(sid, fill):
    b = ''.join("\\pard\\plain \\fs20{\nline %d}\\par\n" % i for i in range(fill))
    b += "\\pard\\plain \\s%d{\nHEAD}\\par\n" % sid
    b += "\\pard\\plain \\fs20{\nTAILWORD}\\par\n"
    return b

for tag, entry, sid in [('n_h4', "{\\s4\\snext0 heading 4;}", 4),
                        ('n_widget', "{\\s7\\snext0\\sb240\\sa120\\fs28 Widget Heading;}", 7)]:
    for fill in range(50, 60):
        (OUT / f'{tag}_{fill}.rtf').write_text(
            doc(NORMAL + entry, keep(sid, fill)), encoding='ascii')
print('written to', OUT)
