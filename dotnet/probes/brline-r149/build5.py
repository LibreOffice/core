#!/usr/bin/env python3
"""Batch 5: what 26.2.4.2 does with a shape body taller than its shape.

The shape is 200 pt wide and 40 pt tall with zero insets; the body is one break run at a swept
size and then a tail of eight distinct words, so whatever survives can be read word by word.
`-tall` twins of every arm hold the same body in a 288 pt shape and are the control that says
the loss is the height and nothing else.
"""
import pathlib
import sys

import build

TAIL = 'alpha beta gamma delta epsilon zeta eta theta'
NARROW_CX = 2540000        # 200 pt
SHORT_CY = 508000          # 40 pt


def main():
    out = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else 'fixtures5')
    out.mkdir(parents=True, exist_ok=True)
    build.SHAPE_CX = NARROW_CX
    made = []
    for sz in (4, 20, 30, 34, 36, 40, 44, 50, 60, 80, 120):
        for tag, cy in (('', SHORT_CY), ('-tall', build.SHAPE_CY)):
            name = 'of%03d%s' % (sz, tag)
            build.write(out / (name + '.docx'), build.body(1, sz, tail=TAIL), cy=cy)
            made.append(name)
    # anchor and vertOverflow, at a size that overflows
    for anc in ('t', 'ctr', 'b'):
        name = 'oa-%s' % anc
        build.write(out / (name + '.docx'), build.body(1, 60, tail=TAIL), cy=SHORT_CY, anchor=anc)
        made.append(name)
    for tag, vof in (('overflow', 'vertOverflow="overflow"'), ('clip', 'vertOverflow="clip"'),
                     ('ellipsis', 'vertOverflow="ellipsis"'), ('absent', '')):
        name = 'ov-%s' % tag
        build.write(out / (name + '.docx'), build.body(1, 60, tail=TAIL), cy=SHORT_CY, vof=vof)
        made.append(name)
    # several paragraphs, so whole-paragraph loss can be told from line loss
    multi = ''.join(build.body(0, 4, tail=w) for w in
                    ('alpha', 'beta', 'gamma', 'delta', 'epsilon', 'zeta', 'eta', 'theta'))
    for tag, cy in (('multi', SHORT_CY), ('multi-tall', build.SHAPE_CY)):
        build.write(out / (tag + '.docx'), multi, cy=cy)
        made.append(tag)
    print('\n'.join(made), '\n%d fixtures' % len(made))


if __name__ == '__main__':
    main()
