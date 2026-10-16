#!/usr/bin/env python3
"""Batch 4: proportional line spacing against the break line, and the witness's own 259/240."""
import pathlib
import sys

import build


def main():
    out = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else 'fixtures4')
    out.mkdir(parents=True, exist_ok=True)
    made = []
    for tag, ln in (('259', '259'), ('360', '360'), ('480', '480')):
        for n in range(0, 5):
            name = 'pr%s-n%d' % (tag, n)
            build.write(out / (name + '.docx'),
                        build.body(n, 4, line=ln, rule='auto'), line=ln, rule='auto')
            made.append(name)
    # the same sweep in the page body rather than in a shape
    for n in range(0, 4):
        name = 'pb259-n%d' % n
        after = build.body(n, 4, line='259', rule='auto').replace('Hxy', 'Pgy')
        build.write(out / (name + '.docx'), build.body(0, 4, line='259', rule='auto'),
                    after=after, line='259', rule='auto')
        made.append(name)
    print('\n'.join(made), '\n%d fixtures' % len(made))


if __name__ == '__main__':
    main()
