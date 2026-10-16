import os, sys, zipfile
SRC = "/home/user/sample-files/slides/chartset-008/pptx/038_Competitive_Advantage_Card_for_PowerPoint_and_Google_Slides_373720f6.pptx"
OUT = sys.argv[1]; os.makedirs(OUT, exist_ok=True)
CATS = ["Product Quality","Innovation","Brand Reputation","Cost Efficiency","Customer Service"]
CASES = {
 # a second, independent hyphenation pair with a LONGER first word
 "q_quality_eff": "Quality Efficiency",   # "Quality " 35.47 + "Ef-" 12.37 = 47.84 <= limit -> TURN
 "q_quality_str": "Quality Stretched",    # no hyphenation point -> "Quality"/"Stretched" -> WRAP
 "q_effcost":     "Efficiency Cost",      # hyphenatable word FIRST and it fits -> WRAP
}
def variant(path, label):
    zin = zipfile.ZipFile(SRC)
    with zipfile.ZipFile(path,"w",zipfile.ZIP_DEFLATED) as zout:
        for item in zin.infolist():
            data = zin.read(item.filename)
            if item.filename == "ppt/charts/chart1.xml":
                s = data.decode("utf-8")
                for c in CATS: s = s.replace("<c:v>%s</c:v>" % c, "<c:v>%s</c:v>" % label)
                data = s.encode("utf-8")
            zout.writestr(item, data)
    zin.close()
for t,l in CASES.items(): variant(os.path.join(OUT,t+".pptx"), l)
