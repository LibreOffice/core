#!/usr/bin/env python3
"""Which of a value axis' tick labels decide its automatic interval.

`027_Simple_personal_cash_flow_statement`'s savings chart (`xl/charts/chart44.xml`) runs a
currency value axis along the bottom over 0..12000 and states no `c:majorUnit`. The only edit
each variant makes is to that axis' `c:numFmt/@formatCode`, so the geometry, the data and the
category labels are identical throughout and the *width of some subset of the tick labels* is
the single variable.

  k=0   [<5000]"$"#,##0;[>=5000]"$"#,##0;General               the control - two identical arms
  k>0   [<5000]"$"#,##0"X"*k;[>=5000]"$"#,##0;General          only the ticks BELOW 5000 widen,
                                                               which at the reference's own
                                                               interval is exactly {0, 1, 2}
  late  [<5000]"$"#,##0;[>=5000]"$"#,##0"WWWWWWWW";General     only the ticks from 6000 up widen

If the cap were the widest label on the axis, `late` would coarsen it and `k=1` would barely
move it. `MaxLabelTickIter` (`chart2/source/view/axes/VCartesianAxis.cxx`:455-511) predicts the
opposite, and the opposite is what 26.2.4.2 draws.

    label-set-027.py <outdir>
    soffice --headless --convert-to pdf --outdir <outdir> <outdir>/027_*.xlsx
    python3 -c "..."   # read the turned/upright money labels off page 6
"""
import sys
import zipfile
from pathlib import Path

SRC = ("/home/user/sample-files/sheets/chartset-014/xlsx/"
       "027_Simple_personal_cash_flow_statement_675c6584.xlsx")
CHART = "xl/charts/chart44.xml"
PLAIN = 'formatCode="&quot;$&quot;#,##0"'

out = Path(sys.argv[1] if len(sys.argv) > 1 else ".")
out.mkdir(parents=True, exist_ok=True)


def variant(name, formatcode):
    with zipfile.ZipFile(SRC) as zin:
        chart = zin.read(CHART).decode("utf-8")
        assert chart.count(PLAIN) == 1, "the value axis' format code moved"
        path = out / ("027_%s.xlsx" % name)
        with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as zout:
            for item in zin.infolist():
                data = zin.read(item.filename)
                if item.filename == CHART:
                    data = chart.replace(PLAIN, formatcode).encode("utf-8")
                zout.writestr(item.filename, data)
    print(path)


def conditional(early_pad="", late_pad=""):
    def arm(pad):
        return '&quot;$&quot;#,##0' + ('&quot;%s&quot;' % pad if pad else "")

    return ('formatCode="[&lt;5000]%s;[&gt;=5000]%s;General"'
            % (arm(early_pad), arm(late_pad)))


for k in range(0, 9):
    variant("k%d" % k, conditional(early_pad="X" * k))

variant("lateWide", conditional(late_pad="WWWWWWWW"))
variant("earlyWide", conditional(early_pad="WWWWWWWW"))
