import zipfile, re, collections, pathlib
root = pathlib.Path('/home/user/corpus-odf')
for ext, glob in (('ods','sheets/*/ods/*.ods'), ('odp','slides/*/odp/*.odp')):
    counts = collections.Counter(); docs = collections.defaultdict(set)
    for d in sorted(root.glob(glob)):
        try: z = zipfile.ZipFile(d)
        except Exception: continue
        for part in ('content.xml','styles.xml'):
            try: s = z.read(part).decode('utf-8','replace')
            except KeyError: continue
            for m in re.finditer(r'<(style:[a-z-]+-properties)\b([^>]*)>', s):
                for am in re.finditer(r'(\w[\w-]*):writing-mode="([^"]*)"', m.group(2)):
                    counts[(m.group(1), am.group(1), am.group(2))] += 1
                    docs[(m.group(1), am.group(1), am.group(2))].add(d.name)
    print('==', ext)
    for k,v in sorted(counts.items(), key=lambda kv:-kv[1]):
        print(f'  {v:6d}  {k[0]:32s} {k[1]}:writing-mode={k[2]}   docs={len(docs[k])}')
