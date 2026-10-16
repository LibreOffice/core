#!/usr/bin/env python3
"""Why the five documents that state a non-identity scale did not move.

For each, find the style carrying the attribute and every reference to that style's name across
`content.xml` and `styles.xml`. A style nothing applies explains a non-mover outright; a style
only a `text:list-level-style-*` names is the list-label case, which `labels/` measures at the
reference.
"""
import pathlib
import re
import zipfile

CORPUS = pathlib.Path('/home/user/corpus-odf/odt')
PARTS = ('content.xml', 'styles.xml')
OPEN = re.compile(r'<style:style\b[^>]*?/?>', re.S)
NAME = re.compile(r'style:name="([^"]+)"')
FAMILY = re.compile(r'style:family="([^"]+)"')
SCALE = re.compile(r'style:text-scale="([^"]*)"')

NON_MOVERS = ['0126226bc58c-SPA-06_mcar_part-6_and_IS_v2.9',
              '1ef46e226af0-JEMIT_Template',
              '2df5a30320b3-Allegiant_Company_Profile_with_History',
              'bc17eefecac9-DRX-Ascend System Course Description',
              'f30b3e8d2d1e-mde087077~283']


def styles_with_a_scale(text):
    """Each `style:style` whose body states a non-identity scale, as (name, family, value)."""
    for match in OPEN.finditer(text):
        if match.group(0).endswith('/>'):
            continue
        end = text.find('</style:style>', match.end())
        body = text[match.end():end if end >= 0 else match.end()]
        scale = SCALE.search(body)
        if not scale or scale.group(1).strip().rstrip('%') in ('100', ''):
            continue
        name = NAME.search(match.group(0))
        family = FAMILY.search(match.group(0))
        yield name.group(1), family.group(1) if family else '?', scale.group(1)


def main():
    for stem in NON_MOVERS:
        with zipfile.ZipFile(CORPUS / (stem + '.odt')) as package:
            parts = {n: package.read(n).decode('utf8', 'replace')
                     for n in PARTS if n in package.namelist()}
        print('== %s' % stem)
        for part, text in parts.items():
            for name, family, value in styles_with_a_scale(text):
                print('   %-44s %-10s %s' % (name[:44], family, value))
                for where, other in parts.items():
                    for hit in re.finditer(r'"%s"' % re.escape(name), other):
                        start = other.rfind('<', 0, hit.start())
                        stop = other.find('>', hit.end())
                        tag = other[start:stop + 1].replace('\n', ' ')
                        print('        %-12s %s' % (where, tag[:150]))
        print()


if __name__ == '__main__':
    main()
