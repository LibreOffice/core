"""The decisive experiment: hold every glyph fixed and vary only the run's language.

026.2.4.2 ships hyphenation patterns for exactly four locales -- en_US, en_GB, fr and es.
Tagging a chart label with a locale that has NO installed pattern file switches the
hyphenator off for that run without changing one character, one advance or one line of the
document.  If the axis arrangement flips, the arrangement is decided by the hyphenator.

Writes 16 variants of the 038 witness: {Cost Efficiency, Cost Stretched} x eight language
tags.  Render them with /opt/libreoffice26.2/program/soffice and classify with classify.py.
"""
import zipfile, os, sys

SRC = ("/home/user/sample-files/slides/chartset-008/pptx/"
       "038_Competitive_Advantage_Card_for_PowerPoint_and_Google_Slides_373720f6.pptx")
OUT = sys.argv[1] if len(sys.argv) > 1 else "langvar"
os.makedirs(OUT, exist_ok=True)

CATS = ["Product Quality", "Innovation", "Brand Reputation", "Cost Efficiency",
        "Customer Service"]
# 038's category axis states its run properties once, and states no lang of its own.
CAT_DEFRPR = ('<a:defRPr sz="1100" b="0" i="0" u="none" strike="noStrike" '
              'kern="1200" baseline="0">')

def variant(name, label, lang):
    zin = zipfile.ZipFile(SRC)
    with zipfile.ZipFile(os.path.join(OUT, name + ".pptx"), "w", zipfile.ZIP_DEFLATED) as z:
        for it in zin.infolist():
            d = zin.read(it.filename)
            if it.filename == "ppt/charts/chart1.xml":
                s = d.decode("utf-8")
                for c in CATS:
                    s = s.replace("<c:v>%s</c:v>" % c, "<c:v>%s</c:v>" % label)
                if lang:
                    assert s.count(CAT_DEFRPR) == 1
                    s = s.replace(CAT_DEFRPR,
                                  CAT_DEFRPR.replace('<a:defRPr ', '<a:defRPr lang="%s" ' % lang))
                d = s.encode("utf-8")
            z.writestr(it, d)
    zin.close()

for lang in [None, "en-US", "en-GB", "fr-FR", "de-DE", "ru-RU", "zxx", "es-ES"]:
    variant("eff_" + (lang or "none"), "Cost Efficiency", lang)
    variant("str_" + (lang or "none"), "Cost Stretched", lang)
print("wrote 16 variants to", OUT)
