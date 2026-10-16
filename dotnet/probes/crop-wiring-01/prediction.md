# crop-wiring-01 — prediction, committed before any reach measurement

Written after the corpus census (`crop-census.py`) and after reading both call sites, and
**before** either sweep leg was rendered. Unedited afterwards.

## 0. The brief's central claim, checked rather than assumed

> `XlsDrawing.cs:346` and `Ww8DocumentReader.Drawings.cs:200` are **each one call from
> `EscherPicture.Cropped`**.

**Refuted, and identically on both paths.** Both lines are the same statement —
`uint pib = shape.Properties.Value(EscherPropertyIds.Picture);` — inside a method
(`PictureOf`) whose only parameter is an `EscherShape`. `EscherPicture.Cropped(properties,
destination)` needs a `destination`, and **there is no rectangle in scope at either site**,
because on both tracks the rectangle is not known when the picture is read:

- sheets — `SheetDrawing` carries a *cell anchor*, and `SheetPageGraphics.Place` turns it into
  a rectangle at paint time, once the column widths and row heights of the page are known.
  A two-cell anchor has no size at all until then.
- words — `PageFrame` carries a `Size` and an anchor, and `FrameLayout` resolves the
  rectangle; `PageDrawing.DrawFrame` receives it as `PlacedFrame.Area`.

And there is a second, larger reason, which is the one that would have shipped a regression:

- **Neither painter clips a picture.** `SheetPageGraphics` calls `sink.DrawImage(image, box)`
  and `PageDrawing.DrawFrame` calls `sink.DrawImage(image, frame.Area)`, both bare. The
  `.ppt` path only worked because `SlideDrawing.DrawPicture` already wrapped every picture in
  `Save`/`ClipPath(shape.Outline)`/`Restore` for its own reasons. A larger destination with
  no clip does not crop a picture — it **spills the whole of it across the page**.

So each site is three changes, not one: carry the crop fractions from the reader to the
painter, apply `PictureCrop.Uncropped` where the rectangle exists, and add the clip that
makes a larger destination read as a crop. The arithmetic is indeed shipped and tested; the
plumbing is not.

## 1. Reach

Census by binary record walk over the Escher property table, never by regex. The instrument
was run against a known answer first: on the 51 `.ppt` decks it reports **16 documents / 100
cropped shapes**, which is `slides-b-01`'s figure to the digit.

Its own first answer for words was **0**, and that was the instrument and not the corpus: a
`.doc` keeps an inline picture's `OfficeArtSpContainer` in the **Data** stream behind a
`sprmCPicLocation`, not in the `fcDggInfo` blob, so a walker that reads only `fcDggInfo`
reaches 0 shapes in 0 documents. The control column — shapes reached, pictures reached — is
what caught it. After the Data-stream scan:

| track | documents reached | Escher shapes | with a `pib` | **crop** | **crop + `pib`** |
|---|---:|---:|---:|---:|---:|
| words | 23 | 101 | 87 | 7 docs / 32 shapes | **5 docs / 30 shapes** |
| sheets | 26 | 617 | 60 | **0** | **0** |

The five: `150_5300_13_chg10.doc` (20 shapes), `150_5335_5a.doc` (6), `150_5300_13_chg12.doc`
(2), `150_5300_13_chg8.doc` (1), `RMI_Document_Repository_Public-Reprts_GettingOffOil.doc` (1).
Two more carry a crop on a shape with **no** picture — `644730BRI0mna000BOX361539B00public0.doc`
and `SFSP_2013-02_Bulletin.doc` — and cannot move a rendering.

| prediction | value |
|---|---|
| **words renderings changed** | **4 or 5 of 200** — the ceiling is 5, and the slides round's ceiling overshot by one |
| **sheets renderings changed** | **0 of 171** — there is no crop to wire on that track at all |
| **slides renderings changed** | **0 of 163** — nothing on the `.ppt` or `.pptx` path is touched |

The sheets zero is the awkward one and is stated plainly: the `XlsDrawing` half of this round
is **dead code on this corpus**. It is still worth shipping — the arithmetic is right and the
next `.xls` with a cropped logo gets it — but it buys nothing measurable here, and no amount
of care in the wiring will make it.

## 2. Verdicts

**Zero movement, both tracks.** The gate asks how many pages, how many extractable words, and
whether the fonts are embedded. A crop changes where a picture's edges fall inside a rectangle
whose size does not change; it moves no line, no page break and no glyph. Words stays wherever
it is and sheets stays wherever it is.

## 3. Direction

Of the changed pages, **none worse**. LibreOffice reaches the same picture by baking the crop
into the bitmap (`msdffimp.cxx`'s `pSet` branch) rather than by a larger destination plus a
clip, but both draw the same pixels in the same place, so every changed page should move
towards the reference rather than away. A page that got *worse* would mean the fractions are
being applied in the wrong direction or the clip is the wrong rectangle.

## 4. What must not be ported, restated

The `+ 1` in `lcl_ApplyCropping` runs in a bitmap's pixel space and is not carried across.
Nothing in this round reintroduces it.

## 5. What this round does not close

`Escher picture cropping, implemented nowhere in the word path` closes. What does **not** close
is cropping in general:

- `a:srcRect` on a `.docx` picture — 11 documents / 14 instances in the words track — is read
  into `DrawingFill.SourceRect` and dropped by `DocxFrames`.
- ODF's `fo:clip` is not read at all.
- Word's `PICF` `dxaCropLeft`/`Top`/`Right`/`Bottom`, which is the *other* place a `.doc`
  states an inline crop, is not read; this round takes the Escher properties only.

Those are separate items and are not claimed.
