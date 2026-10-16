#!/usr/bin/env python3
"""What the intermediate pool styles carry, after an RTF import and after a native ODF one.

The RTF half alone cannot say whether an empty `Header and Footer` means *the pool style has no
tab stops* or *the ODF export does not write pool tab stops*. The native half is the control that
separates them: the same binary, the same export filter, a document that reaches the same styles
without going through writerfilter.

  intermediates.py <rtf-imported .fodt> <natively-imported .fodt>
"""
import sys
import xml.etree.ElementTree as ET

NS = {k: '{%s}' % v for k, v in {
    'office': 'urn:oasis:names:tc:opendocument:xmlns:office:1.0',
    'style': 'urn:oasis:names:tc:opendocument:xmlns:style:1.0',
    'fo': 'urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0',
    'text': 'urn:oasis:names:tc:opendocument:xmlns:text:1.0',
}.items()}

WANTED = ['Standard', 'Heading', 'Caption', 'Figure', 'Text', 'Index', 'Contents_20_1',
          'Header_20_and_20_Footer', 'Header', 'Footer', 'Text_20_body']


def describe(path):
    root = ET.parse(path).getroot()
    out = {}
    for st in root.find(NS['office'] + 'styles').findall(NS['style'] + 'style'):
        name = st.get(NS['style'] + 'name')
        if st.get(NS['style'] + 'family') != 'paragraph' or name not in WANTED:
            continue
        para = st.find(NS['style'] + 'paragraph-properties')
        text = st.find(NS['style'] + 'text-properties')
        bits = [f"parent={st.get(NS['style'] + 'parent-style-name')}"]
        for node, keys in ((text, ('font-size', 'font-style', 'font-weight')),
                           (para, ('margin-top', 'margin-bottom'))):
            for key in keys:
                value = node.get(NS['fo'] + key) if node is not None else None
                if value is not None:
                    bits.append(f'{key}={value}')
        if para is not None:
            stops = para.find(NS['style'] + 'tab-stops')
            if stops is not None:
                bits.append('tabs=' + ','.join(
                    f"{s.get(NS['style'] + 'position')}:{s.get(NS['style'] + 'type') or 'left'}"
                    for s in stops.findall(NS['style'] + 'tab-stop')))
            if para.get(NS['text'] + 'number-lines') is not None:
                bits.append('number-lines=' + para.get(NS['text'] + 'number-lines'))
        out[name] = ' '.join(bits)
    return out


left, right = describe(sys.argv[1]), describe(sys.argv[2])
print(f'{"style":<26} {"after an RTF import":<64} after a native ODF import')
for name in WANTED:
    if name not in left and name not in right:
        continue
    print(f'{name:<26} {left.get(name, "-"):<64} {right.get(name, "-")}')
