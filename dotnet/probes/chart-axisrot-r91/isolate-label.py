import os, sys, zipfile
SRC = "/home/user/sample-files/slides/chartset-008/pptx/038_Competitive_Advantage_Card_for_PowerPoint_and_Google_Slides_373720f6.pptx"
OUT = sys.argv[1]; os.makedirs(OUT, exist_ok=True)
CATS = ["Product Quality","Innovation","Brand Reputation","Cost Efficiency","Customer Service"]
def variant(path, labels):
    zin = zipfile.ZipFile(SRC)
    with zipfile.ZipFile(path,"w",zipfile.ZIP_DEFLATED) as zout:
        for item in zin.infolist():
            data = zin.read(item.filename)
            if item.filename == "ppt/charts/chart1.xml":
                s = data.decode("utf-8")
                for i,(c,new) in enumerate(zip(CATS, labels)):
                    s = s.replace("<c:v>%s</c:v>" % c, "<c:v>%s</c:v>" % new)
                data = s.encode("utf-8")
            zout.writestr(item, data)
    zin.close()
# all five slots set to one real label
for i,c in enumerate(CATS):
    variant(os.path.join(OUT,"all%d.pptx"%i), [c]*5)
# the real set with ONE label shortened to "x"
for i in range(5):
    labs = list(CATS); labs[i] = "x"
    variant(os.path.join(OUT,"drop%d.pptx"%i), labs)
# the real set with every label's space removed (one long unbreakable word)
variant(os.path.join(OUT,"nospace.pptx"), [c.replace(" ","") for c in CATS])
print("ok")
