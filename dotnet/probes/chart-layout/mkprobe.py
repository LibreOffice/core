import re, shutil, sys, zipfile
from pathlib import Path

BASE = Path('/home/user/sample-files/slides/chartset-002/pptx/001_advanced_powerpoint_bar.pptx')

def build(out: Path, labels):
    out.parent.mkdir(parents=True, exist_ok=True)
    zin = zipfile.ZipFile(BASE)
    with zipfile.ZipFile(out, 'w', zipfile.ZIP_DEFLATED) as zout:
        for item in zin.infolist():
            data = zin.read(item.filename)
            if item.filename == 'ppt/charts/chart1.xml':
                s = data.decode('utf-8')
                def rep(m):
                    body = m.group(0)
                    for i, lab in enumerate(labels):
                        body = re.sub(
                            r'(<c:pt idx="%d">\s*<c:v>)[^<]*(</c:v>)' % i,
                            lambda mm, l=lab: mm.group(1) + l + mm.group(2), body)
                    return body
                s = re.sub(r'<c:cat>.*?</c:cat>', rep, s, flags=re.S)
                data = s.encode('utf-8')
            zout.writestr(item, data)
    zin.close()

if __name__ == '__main__':
    build(Path(sys.argv[1]), sys.argv[2:])
