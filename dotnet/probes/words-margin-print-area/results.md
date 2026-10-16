# What a vertically margin-relative frame is centred *in*

*Measured 2026-09-06 in the container described at the top of `dotnet/CLAUDE.md`. **Both** installed
references were used and they agree on every figure below: the distro's **24.2.7.2** at
`/usr/bin/soffice` and the TDF tarball's **26.2.4.2** at `/opt/libreoffice26.2` with its bundled Latin
faces moved aside. XRay.Office at `agent/draw-inline`, base `6bf527227`.*

## Why this exists

`probes/words-vml-fontwork/results.md` filed it: on
`words/done-015/docx/DOA_Template_Form_Type_Certification_Programme.docx` the watermark is drawn at
the right size and the right horizontal position and **too high**, and an isolated one-page probe put
the gap at exactly half of one empty header line. That round changed nothing, on the grounds that the
rule reaches every frame anchored `relativeFrom="margin"` vertically and wanted its own measurement.
This is it.

## The rule, from the source

`wp:positionV/@relativeFrom="margin"` imports as `RelOrientation::PAGE_PRINT_AREA`.
`SwAnchoredObjectPosition::GetVertAlignmentValues`
(`sw/source/core/objectpositioning/anchoredobjectposition.cxx`:336-361) takes the page's print area
and then walks the page frame's lowers:

```cpp
if( pPrtFrame->IsHeaderFrame() )
{
    nHeight -= pPrtFrame->getFrameArea().Height();
    nOffset += pPrtFrame->getFrameArea().Height();
}
else if( pPrtFrame->IsFooterFrame() )
    nHeight -= pPrtFrame->getFrameArea().Height();
```

So the area runs from the **header frame's bottom** to the **footer frame's top**. Those are
`w:top` and `pageHeight − w:bottom` exactly while the running heads fit the room those margins
reserve, because Writer's DOCX import makes the page's own top margin `w:header` and gives the header
frame `w:top − w:header` as a *dynamic* height with dynamic spacing
(`SectionPropertyMap::PrepareHeaderFooterProperties`, `dmapper/PropertyMap.cxx`:1148). They part
company when a head outgrows it.

**And that is the same quantity the paginator already computes.** `Paginator.PushedDownBy` and
`PulledUpBy` move the body's own rectangle by exactly this, so `LaidOutPage.BodyArea` *is* the print
area and nothing new has to be measured.

The horizontal case has no such rule: the identical walk at :824 is guarded by
`aRectFnSet.IsVert()`, so it applies to a vertical writing mode only. Our horizontal position was
already right on the corpus document, which agrees.

## The measurement

`makeprobe.py` builds one-page A4 fixtures carrying a 200 × 50 pt black band anchored
`relativeFrom="margin"` and centred both ways, plus a `BODYLINE` run so the body's own top can be
read off the same page. `w:top` = `w:header` = 708 twips throughout except where stated, so the
header has **no room reserved at all** — which is `DOA_Template`'s own shape and Word's default.

Band centre and body top, in PDF points. `24.2` and `26.2` are identical on every row, so one column:

| fixture | reference centre | ours, before | ours, after | body top, ref / ours |
|---|---:|---:|---:|---|
| `none` — no running heads | 402.62 | 402.75 | 402.75 | 35.91 / 35.91 |
| `hdr-empty` — one empty header paragraph | 409.38 | 402.75 | **408.50** | 49.36 / 47.46 |
| `hdr-1line` | 409.50 | 402.75 | **409.50** | 49.71 / 49.71 |
| `hdr-3line` | 423.38 | 402.75 | **423.25** | 77.31 / 77.31 |
| `hdr-roomy` — one line, `w:top` 2000 | 435.00 | 435.00 | 435.00 | 100.51 / 100.51 |
| `ftr-3line` | 400.25 | 402.75 | **400.25** | 35.91 / 35.91 |
| `hdr3-ftr3` | 421.00 | 402.75 | **421.00** | 77.31 / 77.31 |
| `inhdr-3line` — the band is *in* the header | 430.00 | 402.75 | **429.25** | 90.76 / 88.86 |

Four things it settles:

1. **The area follows the header frame, both ways.** A three-line header moves it 20.76 pt down; a
   three-line footer moves it 2.37 pt up; both together move it 18.38 down. Every one of those is
   half the corresponding overflow, which is what centring in a rectangle whose one edge moved means.
2. **`hdr-roomy` is the control that says it is the frame and not the content.** A one-line header
   inside a 64.6 pt reservation moves the band by **0.00** on both references. A rule reading "the
   area starts below the header's *text*" fits every other row here and fails this one.
3. **It is not about where the frame is anchored.** `inhdr-3line` puts the band in the header itself
   and the reference positions it against the same page rectangle.
4. **The residual on the two `*-empty*` rows is a different defect.** `hdr-empty` is 0.88 pt out and
   `inhdr-3line` 0.75, and in both the *body top* is 1.90 pt higher than the reference's — our empty
   header paragraph is shorter than Writer's by that much. The band tracks our body faithfully in
   both; it is the body that is out. Untouched here, and it reaches any document with an empty
   paragraph in its running head.

   > **Closed 2026-09-06 in `probes/words-empty-paragraph-height/`, and the reach sentence above is
   > wrong.** It is not the empty paragraph and not the header: a DOCX import starts at Calibri 11 pt
   > and is reset to Times New Roman 10 pt the moment a `w:docDefaults/w:rPrDefault` element is seen
   > at all (`DomainMapper.cxx`:182-193 and `StyleSheetTable.cxx`:2161-2167). These fixtures declare
   > no styles part, so they were the 11 pt case and we gave them 10. **No corpus document reaches
   > it** — all 272 words-track DOCX declare a `w:rPrDefault`. After the fix `hdr-empty`'s body top
   > is 49.36 against 49.36 and its centre 0.12 pt out rather than 0.88; `inhdr-3line` is exact at
   > 430.00.

## On the corpus document

`DOA_Template_Form_Type_Certification_Programme.docx`, whose header is a three-row table. The
watermark is `#_x0000_t136` silver at 50% over white, so its fill is exactly `#DFDFDF` in the raster
and separable from everything else on the page; band centre at 150 dpi, in points:

| page | before | after | 24.2 / 26.2 |
|---:|---:|---:|---:|
| 1 | 401.52 | 401.52 | 401.52 |
| 2 | 342.72 | **359.52** | 359.28 |
| 3 | 404.64 | 404.64 | 405.36 |
| 4 | 342.72 | **359.52** | 359.28 |
| 5 | 342.24 | **359.52** | 359.28 |
| 6 | 342.72 | **359.52** | 359.28 |
| 7 | 342.72 | **359.52** | 359.28 |
| 8 | 342.72 | **359.04** | 359.04 |

**16.56 pt closed to 0.24, on six of its first eight pages.** Pages 1 and 3 draw a different header
whose content fits, so nothing moves there and nothing should.

*The 34.7 pt that `words-vml-fontwork/results.md` records is a figure from that round's tree and no
longer reproduces: at `6bf527227` the gap is 16.56 pt. The mechanism is the one it named; the number
had decayed, which is `dotnet/CLAUDE.md`'s "a stored figure is evidence about an environment"
arriving on a figure four days old.*

## Reproducing

```sh
export XRAY_OFFICE_CLI=.../tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli
python3 makeprobe.py /abs/scratch/mpa
python3 measure.py   /abs/scratch/mpa /abs/scratch/mpaout
```

`measure.py` needs Pillow and `pdftotext`. It renders every fixture through `/usr/bin/soffice`,
`/opt/libreoffice26.2/program/soffice` and `$XRAY_OFFICE_CLI`, so the version question is re-checked
on every run rather than taken from this file.
