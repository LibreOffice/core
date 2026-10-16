import zipfile, re, sys, collections, pathlib
root = pathlib.Path('/home/user/corpus-odf/words')
docs = sorted(root.glob('*/odt/*.odt'))
counts = collections.Counter()
percontext = collections.Counter()
docsets = collections.defaultdict(set)
for d in docs:
    try:
        z = zipfile.ZipFile(d)
    except Exception as e:
        print('ERR', d, e); continue
    for part in ('content.xml','styles.xml'):
        try: s = z.read(part).decode('utf-8','replace')
        except KeyError: continue
        # find elements carrying a writing-mode attribute
        for m in re.finditer(r'<(style:[a-z-]+-properties)\b([^>]*)>', s):
            elem, attrs = m.group(1), m.group(2)
            for am in re.finditer(r'(\w[\w-]*):writing-mode="([^"]*)"', attrs):
                ns, val = am.group(1), am.group(2)
                counts[(elem, ns, val)] += 1
                if val not in ('lr-tb','rl-tb','page','lr','rl'):
                    docsets[(elem,ns,val)].add(d.name)
for k,v in sorted(counts.items(), key=lambda kv:-kv[1]):
    print(f'{v:6d}  {k[0]:34s} {k[1]}:writing-mode={k[2]}')
print()
for k,s in sorted(docsets.items()):
    print(k, len(s))
    for n in sorted(s): print('    ', n)
