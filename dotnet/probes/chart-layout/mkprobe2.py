"""A bar chart with N categories, so the vertical category axis' thinning rhythm can be swept."""
import re, sys, zipfile
from pathlib import Path

BASE = Path('/home/user/sample-files/slides/chartset-002/pptx/001_advanced_powerpoint_bar.pptx')


def cat_block(labels):
    pts = "".join(f'<c:pt idx="{i}"><c:v>{l}</c:v></c:pt>' for i, l in enumerate(labels))
    return ('<c:cat><c:strRef><c:f>Sheet1!$A$2:$A$9</c:f><c:strCache>'
            f'<c:ptCount val="{len(labels)}"/>{pts}</c:strCache></c:strRef></c:cat>')


def val_block(n):
    pts = "".join(f'<c:pt idx="{i}"><c:v>{10 + (i % 7) * 10}</c:v></c:pt>' for i in range(n))
    return ('<c:val><c:numRef><c:f>Sheet1!$B$2:$B$9</c:f><c:numCache>'
            f'<c:formatCode>General</c:formatCode><c:ptCount val="{n}"/>{pts}'
            '</c:numCache></c:numRef></c:val>')


def build(out: Path, labels, size=None):
    n = len(labels)
    zin = zipfile.ZipFile(BASE)
    out.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(out, 'w', zipfile.ZIP_DEFLATED) as zout:
        for item in zin.infolist():
            data = zin.read(item.filename)
            if item.filename == 'ppt/charts/chart1.xml':
                s = data.decode('utf-8')
                s = re.sub(r'<c:cat>.*?</c:cat>', lambda m: cat_block(labels), s, flags=re.S)
                s = re.sub(r'<c:val>.*?</c:val>', lambda m: val_block(n), s, flags=re.S)
                # keep only the first series, so the bars stay legible
                s = re.sub(r'<c:ser>(?:(?!</c:ser>).)*</c:ser>\s*(?=<c:ser>)', '', s, flags=re.S)
                if size is not None:
                    s = re.sub(r'(<c:catAx>.*?<c:txPr>.*?)sz="\d+"',
                               lambda m: m.group(1) + f'sz="{size}"', s, flags=re.S)
                data = s.encode('utf-8')
            zout.writestr(item, data)
    zin.close()


if __name__ == '__main__':
    n = int(sys.argv[2])
    size = int(sys.argv[3]) if len(sys.argv) > 3 else None
    build(Path(sys.argv[1]), [f"Cat{i:03d}" for i in range(n)], size)
