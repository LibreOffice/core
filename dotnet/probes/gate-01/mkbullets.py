"""The known-answer probe: a document whose real word count I fixed myself.

Five flat-ODT variants carrying **identical body text** -- 12 items x 5 words = 60 alphanumeric
words, plus a 4-word heading, 64 in all -- differing only in how the 12 items are labelled:

    none        plain paragraphs, no list at all          (the control)
    bullet      U+2022 BULLET
    pua         U+F0A7, a Symbol/Wingdings private-use bullet
    dash        U+2013 EN DASH
    numbered    an arabic numbering, `1.` .. `12.`

This is the probe `words-rebase-01` said was needed and did not run: it varies **only** the
presence and identity of a list label, so anything that moves is the label and nothing else.

Two answers are known in advance and neither is fitted:
  * the corrected count must be **64 on all five variants**;
  * the raw `wc -w` count must be **64 + 12 on the four labelled variants** and 64 on `none`
    -- except for `numbered`, whose labels carry a digit and are therefore real words under
    the corrected definition too, so it must read 76 under *both* metrics.  The numbered
    variant is the one that can catch a filter that is merely "drop short tokens".
"""
import os, subprocess, sys

OUT = sys.argv[1] if len(sys.argv) > 1 else "/tmp/bullets"
os.makedirs(OUT, exist_ok=True)

ITEMS = 12
WORDS_PER_ITEM = 5
HEADING = "Known answer probe document"          # 4 words
BODY = "alpha bravo charlie delta echo"          # 5 words
KNOWN = len(HEADING.split()) + ITEMS * WORDS_PER_ITEM

HEAD = """<?xml version="1.0" encoding="UTF-8"?>
<office:document xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0"
 xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0"
 xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0"
 xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0"
 office:version="1.3" office:mimetype="application/vnd.oasis.opendocument.text">
 <office:styles>
  <style:style style:name="Standard" style:family="paragraph"/>
%s
 </office:styles>
 <office:body><office:text>
  <text:p text:style-name="Standard">%s</text:p>
"""
TAIL = """ </office:text></office:body>
</office:document>
"""

BULLET_STYLE = """  <text:list-style style:name="L1">
   <text:list-level-style-bullet text:level="1" text:bullet-char="%s"
      style:num-suffix="" text:style-name="Standard">
    <style:list-level-properties text:space-before="0.25in" text:min-label-width="0.25in"/>
   </text:list-level-style-bullet>
  </text:list-style>"""

NUMBER_STYLE = """  <text:list-style style:name="L1">
   <text:list-level-style-number text:level="1" style:num-format="1" style:num-suffix="."
      text:style-name="Standard">
    <style:list-level-properties text:space-before="0.25in" text:min-label-width="0.25in"/>
   </text:list-level-style-number>
  </text:list-style>"""

VARIANTS = {
    "none":     None,
    "bullet":   BULLET_STYLE % "&#x2022;",
    "pua":      BULLET_STYLE % "&#xF0A7;",
    "dash":     BULLET_STYLE % "&#x2013;",
    "numbered": NUMBER_STYLE,
}

for name, style in VARIANTS.items():
    body = []
    if style is None:
        for _ in range(ITEMS):
            body.append(f'  <text:p text:style-name="Standard">{BODY}</text:p>')
    else:
        body.append('  <text:list text:style-name="L1">')
        for _ in range(ITEMS):
            body.append(f'   <text:list-item><text:p text:style-name="Standard">{BODY}'
                        f'</text:p></text:list-item>')
        body.append('  </text:list>')
    doc = HEAD % (style or "", HEADING) + "\n".join(body) + "\n" + TAIL
    with open(os.path.join(OUT, f"{name}.fodt"), "w", encoding="utf-8") as fh:
        fh.write(doc)

print(f"known alphanumeric word count: {KNOWN}; list labels: {ITEMS}")
prof = os.path.join(OUT, "prof")
os.makedirs(prof, exist_ok=True)
env = dict(os.environ, SOURCE_DATE_EPOCH="1700000000", TZ="UTC")
for name in VARIANTS:
    subprocess.run(["soffice", f"-env:UserInstallation=file://{prof}", "--headless",
                    "--convert-to", "pdf", "--outdir", OUT,
                    os.path.join(OUT, f"{name}.fodt")],
                   capture_output=True, env=env, timeout=240)

print(f"{'variant':10s} {'raw wc -w':>10s} {'corrected':>10s} {'non-alnum':>10s}   labels seen")
for name in VARIANTS:
    pdf = os.path.join(OUT, f"{name}.pdf")
    txt = subprocess.run(["pdftotext", pdf, "-"], capture_output=True).stdout.decode("utf-8", "replace")
    toks = txt.split()
    alnum = [t for t in toks if any(c.isalnum() for c in t)]
    non = [t for t in toks if not any(c.isalnum() for c in t)]
    seen = sorted({t for t in non})
    print(f"{name:10s} {len(toks):10d} {len(alnum):10d} {len(non):10d}   "
          + " ".join(f"{t!r}=U+{ord(t[0]):04X}" for t in seen[:4]))
