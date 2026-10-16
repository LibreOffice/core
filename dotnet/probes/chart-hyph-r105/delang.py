"""Rewrite every chart part's a:defRPr/a:rPr/a:endParaRPr @lang to a given tag.

Setting the tag to a language with NO installed hyphenation dictionary (de-DE here)
turns 26.2.4.2's hyphenator off for those runs without changing one glyph, one width
or one line of the document.  If an axis still rotates, its rotation is not hyphenation.
"""
import re, sys, os, zipfile
SRC, DST, LANG = sys.argv[1], sys.argv[2], sys.argv[3]
pat = re.compile(r'<a:(defRPr|rPr|endParaRPr)\b([^>/]*?)(/?)>')
def fix(m):
    tag, attrs, slash = m.group(1), m.group(2), m.group(3)
    attrs = re.sub(r'\slang="[^"]*"', '', attrs)
    return f'<a:{tag} lang="{LANG}"{attrs}{slash}>'
zin = zipfile.ZipFile(SRC); n = 0
with zipfile.ZipFile(DST, "w", zipfile.ZIP_DEFLATED) as z:
    for it in zin.infolist():
        d = zin.read(it.filename)
        if re.search(r"charts/chart[^/]*\.xml$", it.filename):
            s = d.decode("utf-8"); s2, k = pat.subn(fix, s); n += k
            d = s2.encode("utf-8")
        z.writestr(it, d)
zin.close()
print(f"{os.path.basename(DST)}: {n} run-property elements retagged {LANG}")
