"""How far the two chart defects of round 66 reach across the corpus.

Counts, per document, the chart parts that carry
  * a reversed category axis (`c:orientation val="maxMin"` on `c:catAx`/`c:dateAx`/`c:serAx`,
    ODF `chart:reverse-direction` on a category axis),
  * a tick-label position other than `nextTo`,
  * an accounting-style number format — one whose code contains a `?` digit placeholder or a
    `*` fill — anywhere a chart axis or a cell style can reach it.

OOXML parts are read straight out of the zip; ODF `content.xml` likewise. A file that is not a
zip, or has no chart part, contributes a zero row and is still counted, so the denominator is the
corpus and not the subset that happened to parse.
"""
import re, sys, zipfile
from pathlib import Path

CORPUS = Path(sys.argv[1] if len(sys.argv) > 1 else "/home/user/sample-files")

CAT = re.compile(rb"<c:(?:cat|date|ser)Ax\b.*?</c:(?:cat|date|ser)Ax>", re.S)
VAL = re.compile(rb"<c:valAx\b.*?</c:valAx>", re.S)
REV = re.compile(rb'<c:orientation val="maxMin"\s*/?>')
LBL = re.compile(rb'<c:tickLblPos val="(\w+)"\s*/?>')
FMT = re.compile(rb'formatCode="([^"]*)"')
FMT2 = re.compile(rb"<c:formatCode>([^<]*)</c:formatCode>")
ODFREV = re.compile(rb'chart:reverse-direction="true"')

def accounting(code: bytes) -> bool:
    # A `?` digit placeholder or a `*` column fill outside a quoted literal. Both are what an
    # accounting format is made of and neither appears in an ordinary numeric code.
    out, quoted = [], False
    i = 0
    while i < len(code):
        c = code[i:i+1]
        if c == b'"':
            quoted = not quoted
        elif c == b"\\":
            i += 2
            continue
        elif not quoted and c in (b"?", b"*"):
            return True
        i += 1
    return False

print("path\tcharts\trevcat\tlblpos\tacctfmt")
for src in sorted(CORPUS.rglob("*")):
    if not src.is_file():
        continue
    rel = src.relative_to(CORPUS).as_posix()
    if "/" not in rel:
        continue
    charts = revcat = lblpos = acct = 0
    try:
        with zipfile.ZipFile(src) as z:
            names = z.namelist()
            parts = [n for n in names if re.search(r"charts?/chart\d*\.xml$", n)]
            for n in parts:
                blob = z.read(n)
                charts += 1
                for m in CAT.finditer(blob):
                    if REV.search(m.group(0)):
                        revcat += 1
                for m in VAL.finditer(blob):
                    p = LBL.search(m.group(0))
                    if p and p.group(1) not in (b"nextTo", b"none"):
                        lblpos += 1
                codes = FMT.findall(blob) + FMT2.findall(blob)
                if any(accounting(c) for c in codes):
                    acct += 1
            if not parts and "content.xml" in names:
                blob = z.read("content.xml")
                if b"<chart:chart" in blob or b"office:chart" in blob:
                    charts += 1
                    if ODFREV.search(blob):
                        revcat += 1
            # A workbook's own styles can carry an accounting format the charts never see.
            for n in names:
                if n.endswith("styles.xml") and n.startswith("xl/"):
                    if any(accounting(c) for c in FMT.findall(z.read(n))):
                        acct += 1
    except Exception:
        pass
    if charts or acct:
        print(f"{rel}\t{charts}\t{revcat}\t{lblpos}\t{acct}")
