import zipfile, re, sys, os, glob
docs = sorted(glob.glob('/workspace/sample-files/sheets/batch-*/*/*'))
docs = [d for d in docs if os.path.isfile(d)]
zipn = merge = grid = both = 0
notzip = 0
for d in docs:
    try:
        z = zipfile.ZipFile(d)
    except Exception:
        notzip += 1
        continue
    names = [n for n in z.namelist() if re.match(r'xl/worksheets/sheet.*\.xml$', n)]
    if not names:
        notzip += 1
        continue
    zipn += 1
    m = g = False
    for n in names:
        try: x = z.read(n).decode('utf-8', 'replace')
        except Exception: continue
        if '<mergeCell ' in x: m = True
        if re.search(r'<printOptions[^>]*gridLines="(1|true)"', x): g = True
    merge += m; grid += g; both += (m and g)
print("documents:", len(docs), "OOXML-readable:", zipn, "not-zip/other:", notzip)
print("with mergeCells:", merge, " printing grid:", grid, " both:", both)
