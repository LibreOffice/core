# An unrecognised ODF `chart:class` is a bar chart, not nothing — r186

Measured 2026-09-27 against **LibreOffice 26.2.4.2** (`/opt/libreoffice26.2/program/soffice`,
`0229ac93fcf0d7cbc6376066c6f35021cef002dc`). The C++ read below is this checkout, which is
27.2.0.0.alpha0+ and **not** the reference binary's source; every arm is confirmed a second
time against 26.2.4.2's own output.

## 0. The question

`OdfChartPlot.Read` began:

```csharp
if (KindOf(Attribute(chart, OdfNamespaces.Chart, "class")) is not { } kind) return null;
```

with a comment saying that a class we cannot draw "yields null and the frame goes back to
drawing nothing … rather than being drawn as some other type". That is a choice about what
*we* do. The question this probe asks is what **26.2.4.2** does, and the answer is the
opposite: it draws bars.

## 1. The rule, in the importer

`SchXMLChartContext::ParseAttributes` (`xmloff/source/chart/SchXMLChartContext.cxx`:372-402)
sets `maChartTypeServiceName` in exactly two branches:

* the class resolves to the **`chart:` namespace** *and* `SchXMLTools::GetChartTypeEnum`
  knows the local name (`:378-394`);
* the class resolves to the **`ooo:` namespace** — the add-in branch, `:395-402`, whose own
  comment is *"service is taken from add-in-name attribute"*. The name is taken verbatim
  and `bHasAddin` is set.

Everything else leaves `aOldChartTypeName` empty, and `SchXMLChartContext::EndElement`
(`:470-478`) then fills it in:

```cpp
if( aOldChartTypeName.isEmpty() )
{
    SAL_WARN("xmloff.chart", "need a charttype to create a diagram" );
    //set a fallback value:
    const OUString& aChartClass_Bar( GetXMLToken(XML_BAR ) );
    …
}
```

So *"a class LibreOffice cannot draw"* and *"no chart"* are different things. The nine
service names in `chart2/source/inc/servicenames_charttypes.hxx` that
`VSeriesPlotter::createSeriesPlotter` (`chart2/source/view/charttypes/VSeriesPlotter.cxx`
:2882-2907) has no plotter for reach `UnsupportedChart` only when the *service name* is set
— which the bar fallback prevents.

## 2. The measurement

`variants.py` writes one flat ODS per class, rewriting **only** the `<chart:chart>`
element's own `chart:class`. The series stay at `chart:bar` throughout.

> **The first cut of this experiment rewrote every `chart:class` in the file**, series
> included, and reported that `chart:gantt` and `chart:histogram` behaved differently from
> the rest. That was the *series*-level attribute talking. The control changed the
> conclusion; keep the one-attribute discipline.

`base.fods` is 26.2.4.2's own `--convert-to fods` of
`sheets/chartset-001/xlsx/001_advanced_excel_bar.xlsx`, a plain clustered bar chart — so a
class that falls back must reproduce it exactly.

`measure.sh` renders each variant both ways and `countpaths.py` counts the PDF's own
path-painting operators (`f f* s S B B* b b*`) over the whole document. A path count rather
than `|ink|%` because the question is binary: *is a chart drawn at all*. `paths.tsv`:

| `chart:chart/@chart:class` | reference | ours before | ours after |
|---|---:|---:|---:|
| `chart:bar` *(control)* | 126 | **156** | 156 |
| `chart:histogram` | 126 | 6 | **156** |
| `chart:gantt` | 126 | 6 | **156** |
| `chart:donut` | 126 | 6 | **156** |
| `chart:pyramid` | 126 | 6 | **156** |
| `chart:add-in` | 126 | 6 | **156** |
| `com.sun.star.chart2.HistogramChartType` | 126 | 6 | **156** |
| `…TreemapChartType` | 126 | 6 | **156** |
| `…SunburstChartType` | 126 | 6 | **156** |
| `…WaterfallChartType` | 126 | 6 | **156** |
| `…FunnelChartType` | 126 | 6 | **156** |
| `…BoxWhiskerChartType` | 126 | 6 | **156** |
| `…ParetoLineChartType` | 126 | 6 | **156** |
| `…RegionMapChartType` | 126 | 6 | **156** |
| `…ClusteredColumnChartType` | 126 | 6 | **156** |
| `ooo:com.sun.star.chart2.ClusteredColumnChartType` | 22 | 6 | 6 |
| `ooo:com.sun.star.chart2.HistogramChartType` | 22 | 6 | 6 |

Three things to read out of it.

* **The reference's 126 is constant across all fifteen non-`ooo` rows**, and it is
  `chart:bar`'s own 126. Fifteen classes, one answer: the fallback is real.
* **6 is what "no chart drawn" looks like in this file** — the sheet's own furniture —
  and it is what we produced for fourteen of the fifteen. The `ooo:` rows, which are
  deliberately unchanged, are the internal control for that reading.
* **156 is not 126**, and never was: the control row `chart:bar` is 156 before and after,
  so the residual 30 paths are how this tree draws a bar chart rather than anything this
  change introduced. It is the same 30 on every row.

## 3. Why the `ooo:` prefix is left alone

The add-in branch sets the service name, so the bar fallback never fires — and what
26.2.4.2 then draws is 22 paths rather than 126, which is not a bar chart. Making such a
class a bar chart would be inventing a chart the reference does not draw.

The corpus's two witnesses, both
`chart:class="ooo:com.sun.star.chart2.ClusteredColumnChartType"`, agree with that direction
whole-page:

| document | reference | ours |
|---|---:|---:|
| `corpus-odf/ods/…054_Problem_analysis_with_Pareto_chart` | 1 | 8 |
| `corpus-odf/ods/…051_Manufacturer_defect_analysis` | 46 | 191 |

Those two counts include each sheet's own ink, so they are context and not the argument;
the argument is the branch in `ParseAttributes`. What they do establish is the sign — the
reference already draws **less** on both pages than we do, so adding bars there could only
make it worse.

## 4. Reach on real files: nil, and that is the finding

`census.sh` reads every embedded `Object*/content.xml` of the converted-ODF corpus and
counts the `chart:chart/@chart:class` values actually stated:

```
 54 ods  chart:bar          10 ods  chart:line        9 ods  chart:scatter
  9 ods  chart:area          7 ods  chart:circle      6 odt  chart:circle
  5 ods  chart:ring          5 ods  chart:bubble      4 ods  chart:radar
  2 odt  chart:bar           1 odt  chart:scatter     1 odt  chart:ring
  2 ods  ooo:com.sun.star.chart2.ClusteredColumnChartType
```

Every class a real document states is one this reader already drew, and the only two
exceptions are the `ooo:` pair this change deliberately does not touch. **So no corpus
rendering moves.** The change is correctness against the reference's own rule for files the
corpus does not contain — an ODF writer that states a chart type LibreOffice has no plotter
for, which is exactly the case a reach census cannot witness because the corpus holds no
hand-written ODF at all.

## Files

| file | what it is |
|---|---|
| `base.fods` | 26.2.4.2's `--convert-to fods` of `001_advanced_excel_bar.xlsx` |
| `variants.py` | one variant per class, rewriting only `<chart:chart>`'s attribute |
| `measure.sh` | renders both ways and prints the path counts |
| `countpaths.py` | path-painting operators per PDF, out of the content stream |
| `census.sh` | which classes the converted-ODF corpus actually states |
| `paths.tsv` | §2's table, as measured |
