#!/usr/bin/env python3
"""One-attribute variants of `features/sheet-shape-text.fods`, for §2a of results.md.

Render each through 26.2.4.2 and read the spans out of the PDF. b, c, d and f come back
identical to a, which is what says that neither the graphic style's style:text-properties nor
the draw:text-style-name paragraph style reaches a run on a Calc sheet; g is the control that
says the file is not simply being ignored, and e and h show the two text-area adjustments are.
"""
import re, os, subprocess, sys, hashlib
FIXTURE = '../../tests/corpus/features/sheet-shape-text.fods'
base = open(FIXTURE, encoding='utf-8').read()
V = {}
V['a_base'] = base
V['b_no_gr_size'] = base.replace('   <style:text-properties fo:font-size="18pt"/>\n', '')
V['c_gr_size8'] = base.replace('fo:font-size="18pt"', 'fo:font-size="8pt"')
V['d_para_size14'] = base.replace(
    '<style:style style:name="P1" style:family="paragraph">\n   <style:paragraph-properties fo:text-align="center"/>',
    '<style:style style:name="P1" style:family="paragraph">\n   <style:text-properties fo:font-size="14pt"/>\n   <style:paragraph-properties fo:text-align="center"/>')
V['e_horz_center'] = base.replace('draw:textarea-horizontal-align="justify"', 'draw:textarea-horizontal-align="center"')
V['f_no_textstyle'] = base.replace(' draw:text-style-name="P1"', '')
V['g_para_named'] = base.replace('<text:p>Inherits', '<text:p text:style-name="P1">Inherits')
V['h_vert_middle'] = base.replace('draw:textarea-vertical-align="top"', 'draw:textarea-vertical-align="bottom"')
os.makedirs('var', exist_ok=True)
for k, v in V.items():
    open(f'var/{k}.fods', 'w', encoding='utf-8').write(v)
print(' '.join(V))
