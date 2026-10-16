#!/usr/bin/env python3
"""One flat ODS per `chart:chart/@chart:class`, with everything else left alone.

The whole point is that exactly one attribute moves. The first cut of this experiment
replaced *every* `chart:class` in the file, series included, and reported that
`chart:gantt` and `chart:histogram` behaved differently from the rest -- which was the
series-level attribute talking, not the chart-level one. Only the `<chart:chart>`
element's own attribute is rewritten here; the series stay at `chart:bar`.

    ./variants.py <outdir>

`base.fods` is 26.2.4.2's own `--convert-to fods` of
`sheets/chartset-001/xlsx/001_advanced_excel_bar.xlsx` -- a plain clustered bar chart, so
any class that falls back to bars must reproduce it exactly.
"""
import re
import sys
from pathlib import Path

# The five `chart:`-namespace classes LibreOffice cannot draw, and the nine chart2 service
# names `servicenames_charttypes.hxx` declares that `VSeriesPlotter::createSeriesPlotter`
# has no plotter for -- written bare, as a document that bound no `ooo` prefix would.
CLASSES = [
    "chart:bar",                                   # the control: the file as it stands
    "chart:histogram",
    "chart:gantt",
    "chart:donut",                                 # not `chart:ring`, which IS drawn
    "chart:pyramid",
    "chart:add-in",
    "com.sun.star.chart2.HistogramChartType",
    "com.sun.star.chart2.TreemapChartType",
    "com.sun.star.chart2.SunburstChartType",
    "com.sun.star.chart2.WaterfallChartType",
    "com.sun.star.chart2.FunnelChartType",
    "com.sun.star.chart2.BoxWhiskerChartType",
    "com.sun.star.chart2.ParetoLineChartType",
    "com.sun.star.chart2.RegionMapChartType",
    "com.sun.star.chart2.ClusteredColumnChartType",

    # The add-in branch: the same service name with the prefix LibreOffice's own exporter
    # writes. `SchXMLChartContext::ParseAttributes` takes this one verbatim.
    "ooo:com.sun.star.chart2.ClusteredColumnChartType",
    "ooo:com.sun.star.chart2.HistogramChartType",
]

ELEMENT = re.compile(r"<chart:chart\b[^>]*>")


def main() -> int:
    out = Path(sys.argv[1])
    out.mkdir(parents=True, exist_ok=True)
    source = (Path(__file__).parent / "base.fods").read_text(encoding="utf-8")

    opening = ELEMENT.search(source)
    if opening is None:
        print("no <chart:chart> in base.fods", file=sys.stderr)
        return 1

    for stated in CLASSES:
        patched = re.sub(
            r'chart:class="[^"]*"',
            'chart:class="%s"' % stated,
            opening.group(0),
            count=1,
        )
        name = stated.replace(":", "_").replace(".", "_")
        (out / ("k-%s.fods" % name)).write_text(
            source[: opening.start()] + patched + source[opening.end() :],
            encoding="utf-8",
        )

    print("%d variants in %s" % (len(CLASSES), out))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
