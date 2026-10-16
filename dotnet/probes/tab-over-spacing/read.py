import subprocess, sys, xml.etree.ElementTree as ET
NS="{http://www.w3.org/1999/xhtml}"
xml=subprocess.run(["pdftotext","-bbox-layout",sys.argv[1],"-"],capture_output=True,text=True).stdout
root=ET.fromstring(xml)
print(f"{'label':>12} {'xMin':>8} {'xMax':>8} {'xMax_tw':>9}")
for ln in root.iter(NS+"line"):
    ws=[(w.text or "",float(w.get("xMin")),float(w.get("xMax"))) for w in ln.iter(NS+"word")]
    if not ws: continue
    lbl=ws[0][0][:12]
    print(f"{lbl:>12} {ws[0][1]:8.2f} {ws[-1][2]:8.2f} {ws[-1][2]*20:9.1f}")
