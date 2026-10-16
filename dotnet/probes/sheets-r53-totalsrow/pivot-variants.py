import os, re, shutil, subprocess, zipfile

SRC = "/c/sandbox/workdir/sample-files/sheets/chartset-004/xlsx/DynamicBubbleChart.xlsx"
BASE = os.path.dirname(os.path.abspath(__file__))

def build(name, edits=None, drop=()):
    out = os.path.join(BASE, name + ".xlsx")
    zin = zipfile.ZipFile(SRC)
    with zipfile.ZipFile(out, "w", zipfile.ZIP_DEFLATED) as zo:
        for it in zin.infolist():
            if it.filename in drop: continue
            data = zin.read(it.filename)
            for old, new in (edits or {}).get(it.filename, ()):
                ob, nb = old.encode(), new.encode()
                if ob not in data: raise SystemExit(f"{name}: {it.filename}: missing {old[:60]}")
                data = data.replace(ob, nb)
            zo.writestr(it, data)
    zin.close(); return out

def render(path):
    stem = os.path.splitext(os.path.basename(path))[0]
    d = os.path.join(BASE, "out", stem)
    shutil.rmtree(d, ignore_errors=True); os.makedirs(d, exist_ok=True)
    env = dict(os.environ, SOURCE_DATE_EPOCH="1700000000", TZ="UTC")
    subprocess.run(["soffice", f"-env:UserInstallation=file://{BASE}/prof", "--headless",
                    "--convert-to", "pdf", "--outdir", d, path], capture_output=True, timeout=300, env=env)
    pdf = os.path.join(d, stem + ".pdf")
    if not os.path.exists(pdf): return None
    return subprocess.run(["pdftotext", "-layout", "-f", "1", "-l", "1", pdf, "-"],
                          capture_output=True).stdout.decode("utf8", "replace")

def summary(t):
    if t is None: return "RENDER FAILED"
    return "labels drawn: " + ", ".join(
        f"{d}={len(re.findall(re.escape(d), t))}"
        for d in ["Finance", "Information Technology", "Production", "Purchase"])

print("baseline          ", summary(render(SRC)))
print("no fillDownLabels ", summary(render(build("V_nofill",
    {"xl/pivotTables/pivotTable1.xml": [('fillDownLabels="1"', 'fillDownLabels="0"')]}))))
print("pivot part removed", summary(render(build("V_nopivot", drop=("xl/pivotTables/pivotTable1.xml",),
    edits={"xl/worksheets/sheet1.xml": [('<pivotSelection', '<xxPivotSelection')] if False else ()}))))
