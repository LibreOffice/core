"""Sweep one category-label width on 038 and read what 26.2.4.2 does with the axis.

Every category is the SAME single word, so there is no blank to wrap at: the only
way the label can break is inside the word, which is lcl_hasWordBreak's own trigger.
"""
import os, re, shutil, subprocess, sys, zipfile

SRC = "/home/user/sample-files/slides/chartset-008/pptx/038_Competitive_Advantage_Card_for_PowerPoint_and_Google_Slides_373720f6.pptx"
OUT = sys.argv[1]
os.makedirs(OUT, exist_ok=True)

CATS = ["Product Quality", "Innovation", "Brand Reputation", "Cost Efficiency",
        "Customer Service"]

def variant(path, word):
    zin = zipfile.ZipFile(SRC)
    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as zout:
        for item in zin.infolist():
            data = zin.read(item.filename)
            if item.filename == "ppt/charts/chart1.xml":
                s = data.decode("utf-8")
                for c in CATS:
                    s = s.replace("<c:v>%s</c:v>" % c, "<c:v>%s</c:v>" % word)
                data = s.encode("utf-8")
            zout.writestr(item, data)
    zin.close()

words = []
for n in range(4, 22):
    words.append(("w%02d" % n, "n" * n))

for tag, word in words:
    variant(os.path.join(OUT, "%s.pptx" % tag), word)
print("\n".join("%s %s" % w for w in words))
