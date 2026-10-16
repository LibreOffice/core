import subprocess, re, os, sys, glob
PAT = re.compile(r'(?<![\d.])(\d{1,3})\s+of\s+(\d{1,3})(?![\d.])', re.I)
refdir = sys.argv[1]
out = open(sys.argv[2], 'w')
out.write("file\tpages\tverdict\tdetail\n")
files = sorted(glob.glob(os.path.join(refdir, '*.pdf')))
for i, f in enumerate(files):
    try:
        txt = subprocess.run(['pdftotext', '-layout', f, '-'], capture_output=True,
                             timeout=120).stdout.decode('utf-8', 'replace')
    except Exception as e:
        out.write("%s\t?\terror\t%s\n" % (os.path.basename(f), e)); continue
    pages = txt.split('\f')
    if pages and pages[-1].strip() == '': pages = pages[:-1]
    n = len(pages)
    if n < 3:
        out.write("%s\t%d\tskip-short\t\n" % (os.path.basename(f), n)); continue
    # first "N of M" match per page
    vals = []
    for p in pages:
        m = PAT.search(p)
        vals.append(m.group(1) if m else None)
    present = [v for v in vals if v is not None]
    if len(present) < max(3, n * 0.6):
        out.write("%s\t%d\tno-field\t\n" % (os.path.basename(f), n)); continue
    uniq = sorted(set(present), key=int)
    seq_ok = sum(1 for idx, v in enumerate(vals, 1) if v is not None and int(v) == idx)
    if len(uniq) == 1:
        out.write("%s\t%d\tFROZEN\tvalue=%s on %d/%d pages\n" % (os.path.basename(f), n, uniq[0], len(present), n))
    elif seq_ok >= len(present) * 0.8:
        out.write("%s\t%d\tsequential\t%d/%d match index\n" % (os.path.basename(f), n, seq_ok, len(present)))
    else:
        out.write("%s\t%d\tother\tvalues=%s\n" % (os.path.basename(f), n, ','.join(uniq[:12])))
    out.flush()
out.close()
print("done", len(files))
