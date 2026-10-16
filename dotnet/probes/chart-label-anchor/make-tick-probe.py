import re, sys, os
SRC="/home/user/wt-charts/dotnet/tests/corpus/features/chart-bar-sheet.fods"
OUT=sys.argv[1] if len(sys.argv)>1 else None
src=open(SRC).read()

# The eight data cells, in document order: B2 C2 B3 C3 B4 C4 B5 C5 (values 120 88 95 132 143 101 168 121)
# Replace with a range whose automatic scale differs between N=8 and N=9:
#   min 68, max 78  ->  N<=8 gives 60..80 step 5, N>=9 gives 62..80 step 2.
NEW = [68, 78, 70, 72, 74, 71, 76, 69]
def cell(v): return f'<table:table-cell office:value-type="float" office:value="{v}"><text:p>{v}</text:p></table:table-cell>'
i=0
def sub(m):
    global i
    v=NEW[i % len(NEW)]
    i+=1
    return cell(v)
src=re.sub(r'<table:table-cell office:value-type="float" office:value="[^"]*"><text:p>[^<]*</text:p></table:table-cell>', sub, src)
assert i in (8,16), i

def variant(height_cm, size_pt, family):
    """Only the *value* axis' text size moves; the category band and both titles stay put."""
    s=src
    s=s.replace('svg:width="12cm" svg:height="7cm"', f'svg:width="12cm" svg:height="{height_cm:.3f}cm"')
    fam = f' fo:font-family="{family}"' if family else ''
    # A second chart style, used by the y axis alone.
    marker = '</style:style>\n         </office:automatic-styles>'
    assert marker in s
    s = s.replace(marker,
        '</style:style>\n'
        '          <style:style style:name="chAxisY" style:family="chart">\n'
        '           <style:chart-properties chart:display-label="true" chart:logarithmic="false"\n'
        '               chart:reverse-direction="false" text:line-break="false"/>\n'
        f'           <style:text-properties fo:font-size="{size_pt}pt"{fam}/>\n'
        '          </style:style>\n'
        '         </office:automatic-styles>')
    s = s.replace('<chart:axis chart:dimension="y" chart:name="primary-y" chart:style-name="chAxis">',
                  '<chart:axis chart:dimension="y" chart:name="primary-y" chart:style-name="chAxisY">')
    return s

if __name__ == "__main__":
    os.makedirs(OUT, exist_ok=True)
    plan=[]
    for tag,size,family in (("libs10",10,"Liberation Sans"),("libs20",20,"Liberation Sans"),("dejavu11",11,"DejaVu Sans")):
        for h in [round(x*0.1,3) for x in range(25, 96)]:
            name=f"{tag}-h{h:05.2f}"
            open(os.path.join(OUT,name+".fods"),"w").write(variant(h,size,family))
            plan.append(name)
    print(len(plan))
