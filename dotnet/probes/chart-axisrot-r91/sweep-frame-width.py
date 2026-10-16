"""Hold the label; sweep the chart frame's width.

Nine 'n's is one unbreakable word of 53.1 pt at 11 pt Carlito on chart2's 96 dpi device.
The frame's width sets the diagram rectangle at 0.96*frame (2% margin each side) and the
tick pitch at diagram/N -- checked exactly against the unmodified document, whose 282.30 pt
frame gives the 54.20 pt pitch the reference draws.

  model A (this tree):  turns when word > 0.95 * 0.96*F/N              -> F = 291.1 pt at N=5
  model B (maximum-label adjustInnerSize): -> F = 317.6 pt at N=5
"""
import os, sys, zipfile

SRC = "/home/user/sample-files/slides/chartset-008/pptx/038_Competitive_Advantage_Card_for_PowerPoint_and_Google_Slides_373720f6.pptx"
OUT = sys.argv[1]
os.makedirs(OUT, exist_ok=True)

CATS = ["Product Quality", "Innovation", "Brand Reputation", "Cost Efficiency",
        "Customer Service"]
WORD = "n" * 9
EMU = 12700  # per point

def variant(path, frame_pt):
    cx = int(round(frame_pt * EMU))
    zin = zipfile.ZipFile(SRC)
    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as zout:
        for item in zin.infolist():
            data = zin.read(item.filename)
            if item.filename == "ppt/charts/chart1.xml":
                s = data.decode("utf-8")
                for c in CATS:
                    s = s.replace("<c:v>%s</c:v>" % c, "<c:v>%s</c:v>" % WORD)
                data = s.encode("utf-8")
            elif item.filename == "ppt/slides/slide1.xml":
                s = data.decode("utf-8")
                old = '<a:ext cx="3585210" cy="2824797"/>'
                assert s.count(old) == 1
                s = s.replace(old, '<a:ext cx="%d" cy="2824797"/>' % cx)
                data = s.encode("utf-8")
            zout.writestr(item, data)
    zin.close()

for f in range(270, 341, 2):
    variant(os.path.join(OUT, "f%03d.pptx" % f), float(f))
print("done")
