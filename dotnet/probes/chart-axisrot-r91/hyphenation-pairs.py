import os, sys, zipfile
SRC = "/home/user/sample-files/slides/chartset-008/pptx/038_Competitive_Advantage_Card_for_PowerPoint_and_Google_Slides_373720f6.pptx"
OUT = sys.argv[1]; os.makedirs(OUT, exist_ok=True)
CATS = ["Product Quality","Innovation","Brand Reputation","Cost Efficiency","Customer Service"]
# a hyphenation model and a max-line-width model disagree on c vs d: same second word,
# only the FIRST word's length differs, so the widest line is WIDER in d.
CASES = {
 "h_cost_stretched": "Cost Stretched",
 "h_cost_strengths": "Cost Strengths",
 "h_cost_scratched": "Cost Scratched",
 "h_cost_thoughts":  "Cost Thoughts",
 "h_nnnn_stretched": "nnnn Stretched",
 "a_cost_eff":   "Cost Efficiency",        # known: ROTATE
 "b_cost_nnn":   "Cost nnnnnnn",           # unhyphenatable second word of similar width
 "c_short_eff":  "nnnn Efficiency",        # short first word: room on line 1 for "Effi-"
 "d_long_eff":   "nnnnnnnn Efficiency",    # long first word: no room for any fragment
 "e_long_nnn":   "nnnnnnnn nnnnnnn",       # control: neither hyphenatable
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
print("ok")
