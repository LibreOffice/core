import os, re, subprocess
OPS = "/home/user/libreoffice-core/.claude/skills/render-comparison/scripts/pdf-ops.py"
SOFFICE = "/opt/libreoffice26.2/program/soffice"
CLI = "/home/user/libreoffice-core/dotnet/tools/XRay.Office.Cli/bin/Release/net10.0/linux-x64/XRay.Office.Cli"
def rules(pdf, page):
    out = subprocess.run(["python3", OPS, "dump", pdf, "--page", str(page)],
                         capture_output=True, text=True).stdout
    ys, texts = [], []
    for l in out.splitlines():
        m = re.match(r'stroke\s+p\d+\s+\(\s*([-\d.]+),\s*([-\d.]+)\)-\(\s*([-\d.]+),\s*([-\d.]+)\)', l)
        if m:
            x1,y1,x2,y2 = map(float, m.groups())
            if abs(y1-y2) < 0.01 and x2-x1 > 100: ys.append(round(y1,2))
        m = re.match(r'text\s+p\d+\s+\(\s*([-\d.]+),\s*([-\d.]+)\)', l)
        if m: texts.append(round(float(m.group(2)),2))
    return sorted(set(ys), reverse=True), sorted(set(texts), reverse=True)
def render(src, who, tag, out):
    d = os.path.join(out, who, tag); os.makedirs(d, exist_ok=True)
    if who == "ours":
        subprocess.run([CLI,"render",src,"--format","pdf","--outdir",d], capture_output=True)
    else:
        subprocess.run([SOFFICE,"-env:UserInstallation=file://"+out+"/prof-"+who,"--headless",
                        "--norestore","--convert-to","pdf","--outdir",d,src], capture_output=True)
    stem = os.path.splitext(os.path.basename(src))[0]
    p = os.path.join(d, stem + ".pdf")
    return p if os.path.exists(p) else None
