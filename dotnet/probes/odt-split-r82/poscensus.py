#!/usr/bin/env python3
"""The positioning every splittable floating table states, so the mapping is written to the corpus."""
import collections, pathlib, re, sys, zipfile
from xml.etree import ElementTree as ET
NS = {'office':'urn:oasis:names:tc:opendocument:xmlns:office:1.0',
      'style':'urn:oasis:names:tc:opendocument:xmlns:style:1.0',
      'draw':'urn:oasis:names:tc:opendocument:xmlns:drawing:1.0',
      'text':'urn:oasis:names:tc:opendocument:xmlns:text:1.0',
      'table':'urn:oasis:names:tc:opendocument:xmlns:table:1.0',
      'fo':'urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0',
      'svg':'urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0',
      'loext':'urn:org:documentfoundation:names:experimental:office:xmlns:loext:1.0'}
Q = lambda p: '{%s}%s' % (NS[p.split(':')[0]], p.split(':')[1])
counts = collections.Counter()
for doc in sorted(pathlib.Path('/home/user/corpus-odf/words').glob('*/odt/*.odt')):
    z = zipfile.ZipFile(doc)
    root = ET.fromstring(z.read('content.xml'))
    styles = {}
    for auto in root.iter(Q('office:automatic-styles')):
        for s in auto:
            if s.get(Q('style:name')): styles[s.get(Q('style:name'))] = s
    for frame in root.iter(Q('draw:frame')):
        if frame.get(Q('loext:may-break-between-pages')) != 'true': continue
        g = styles.get(frame.get(Q('draw:style-name')))
        gp = g.find(Q('style:graphic-properties')) if g is not None else None
        get = (lambda n: gp.get(Q(n)) if gp is not None else None)
        counts['anchor=' + str(frame.get(Q('text:anchor-type')))] += 1
        counts['vpos=' + str(get('style:vertical-pos'))] += 1
        counts['vrel=' + str(get('style:vertical-rel'))] += 1
        counts['hpos=' + str(get('style:horizontal-pos'))] += 1
        counts['hrel=' + str(get('style:horizontal-rel'))] += 1
        counts['wrap=' + str(get('style:wrap'))] += 1
        counts['runthrough=' + str(get('style:run-through'))] += 1
        counts['parent=' + str(g.get(Q('style:parent-style-name')) if g is not None else None)] += 1
        counts['svgx=' + ('set' if frame.get(Q('svg:x')) else 'none')] += 1
        counts['svgy=' + ('set' if frame.get(Q('svg:y')) else 'none')] += 1
        counts['svgwidth=' + ('set' if frame.get(Q('svg:width')) else 'none')] += 1
        counts['marginleft=' + str(get('fo:margin-left'))] += 1
for k in sorted(counts): print(f'{counts[k]:5d}  {k}')
