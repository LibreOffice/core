"""Does the leading room decide it? Five first words x three second words.

The 22-word sweep left one word disagreeing -- `Service`, which this tree hyphenates and
26.2.4.2 does not -- and the obvious explanation is the room left on the first line, since
EditEngine's leading limit is `nMaxBreakPos - nWordStart - 1` characters
(editeng/source/editeng/impedit3.cxx:2143-2160). This varies exactly that, by lengthening
the FIRST word, and refutes it: `Marketing` (point at 3) hyphenates with LESS room than
`Service` (point at 3) is given, and `Service` never hyphenates at any room where its label
also wraps.

    python3 leading-sweep.py <dir>
    /opt/libreoffice26.2/program/soffice --headless --convert-to pdf --outdir <dir>/ref <dir>/*.pptx
"""
import os
import sys
import zipfile

SRC = ("/home/user/sample-files/slides/chartset-008/pptx/"
       "038_Competitive_Advantage_Card_for_PowerPoint_and_Google_Slides_373720f6.pptx")
CATS = ["Product Quality", "Innovation", "Brand Reputation", "Cost Efficiency",
        "Customer Service"]

FIRST = ["A", "Co", "Cost", "Costly", "Costlier"]
SECOND = ["Service", "Quality", "Efficiency"]

into = sys.argv[1]
os.makedirs(into, exist_ok=True)

for first in FIRST:
    for second in SECOND:
        with zipfile.ZipFile(SRC) as zin, \
             zipfile.ZipFile(os.path.join(into, f"{first}_{second}.pptx"), "w",
                             zipfile.ZIP_DEFLATED) as out:
            for item in zin.infolist():
                data = zin.read(item.filename)
                if item.filename == "ppt/charts/chart1.xml":
                    text = data.decode("utf-8")
                    for category in CATS:
                        text = text.replace("<c:v>%s</c:v>" % category,
                                            "<c:v>%s %s</c:v>" % (first, second))
                    data = text.encode("utf-8")
                out.writestr(item, data)

print("wrote", len(FIRST) * len(SECOND), "variants to", into)
