# Round 138 — O89: a worksheet shape whose own fill is a picture

Worked solo; the parallel rounds this was to have shared a wave with died on a rate limit and
their branches are gone. Only **O89** is closed here. **O92** (the number format's colour clause)
was in the same brief and is **not** touched — it keeps its seat.

## 0. The environment this was measured in, because it was not the one the brief assumed

The container was rebuilt between the brief and the round. The dotnet tree built clean at 0
warnings while `/opt/libreoffice26.2`, poppler, pymupdf, pillow, Carlito, Caladea and the
LibreOffice application packages were all absent, and `/usr/bin/soffice` survived at 24.2.7.2 —
the binary this project explicitly does not measure against. Everything below was taken after
restoring the reference to **26.2.4.2, build hash `0229ac93fcf0d7cbc6376066c6f35021cef002dc`**,
which matches the recorded reference exactly. `MISSING_PACKAGES.md` carries the restore procedure
and the four traps found doing it.

## 1. The seat reproduces, with one correction

| | seat | measured here |
|---|---|---|
| workbooks with an `xdr:sp` carrying `a:blipFill` in its own `xdr:spPr` | 10 of **241** | 10 of **240** |
| such shapes | 10 | 10 |
| reference draws on the witness | `147.912 0 0 25.483 213.25 682.213 cm /Im100 Do` | identical |
| this tree draws | nothing | nothing |

The denominator is 240, not 241. Everything else is exact.

**All ten are the same shape**: `prstGeom` `rect`, `a:stretch`, unrotated, nine of ten also
stating `a:srcRect`. Not one tiles.

## 2. An instrument that reported absence rather than failure

The first measurement of both sides came back **0 image draws for the reference as well as for
us**, which reads as agreement. A PDF's content streams are Flate-compressed, so a raw-bytes
`grep` for `/Im\d+ Do` finds nothing whether or not the image is drawn. Inflating each
`stream…endstream` with `zlib.decompress` gives the reference's single placement immediately.

This is the same class as the pikepdf incident and as `timeout` exiting 0 for a command that does
not exist: **an instrument that is not reading the right layer reports absence, and absence looks
like agreement.** Both are now in `MISSING_PACKAGES.md`.

## 3. The change

`XlsxDrawings.ReadAnchor` returned early for a shape because its `picture` is the `xdr:pic`
element and there is none. It now takes the shape's own `spPr/a:blipFill` as the source for the
existing picture path — the blip choice, `a:alphaModFix` and `a:srcRect` are all already done
there — rather than teaching `XlsxShapeInk.Ink`, which carries colours, about bitmaps.

**Only `a:stretch`.** A stretched bitmap fills the shape's rectangle, which is exactly what the
picture path draws. `a:tile` is a repeat at the blip's own size with its own offsets, is a
different geometry, and is left unfilled as before. `XlsxShapeInk`'s remarks already made the
argument and it still holds: *a tile painted stretched would be a confident wrong answer where
today there is an absent one.*

## 4. Result on the ten

Every one now matches the reference's image count, worst placement delta **0.11 pt** over all six
numbers of the `cm` matrix:

| document | ours | ref | max abs delta (pt) |
|---|--:|--:|--:|
| 091 Colored_Background | 1 | 1 | 0.0563 |
| 092 Complete_Guide | 1 | 1 | 0.0568 |
| 093 Customizable_Format | 1 | 1 | 0.0852 |
| 094 Editable_Format | 2 | 2 | 0.0566 |
| 095 Fillable_Format | 1 | 1 | 0.1133 |
| 096 Green_Theme | 1 | 1 | 0.0846 |
| 097 Professional_Format | 2 | 2 | 0.0283 |
| 098 Simple_Layout | 1 | 1 | 0.0572 |
| 099 Stylish_Format | 1 | 1 | 0.0568 |
| 100 Tabular_Format | 1 | 1 | 0.0614 |

Three carry two placements because they hold an `xdr:pic` as well; both are drawn.

## 5. Confinement

Whole `xlsx` track rendered twice, our half only, `SOURCE_DATE_EPOCH` pinned:

- **10 of 240 renderings move** — exactly the ten that carry the construct.
- **230 byte-identical.**
- **0 page counts move. 0 alphanumeric counts move.** A fill adds no glyph and no page, which is
  why no gate column can see any of this, before or after.

**The first attempt at this reported 240 of 240 moved**, with every page and character count
identical — the signature `dotnet/CLAUDE.md` names for a forgotten `SOURCE_DATE_EPOCH`. Both
sweeps were discarded and re-run pinned. A reach figure equal to the whole set is not a finding.

## 6. Tests

`SheetShapePictureFillTests`, two facts, against a new fixture
`features/sheet-shape-picture-fill.xlsx` built by `make-fixture.py` in this directory — 3468
bytes, one anchor, a 2×2 PNG.

**The fixture is validated against the reference, not just against us.** 26.2.4.2 places its
image once at `143.972 0 0 71.972 100.998 686.324 cm`; this tree places it at
`144 0 0 72 101.9622 685.2858` — extent agreeing to 0.03 pt, origin to the two writers' own
constant offset.

**Pinned by mutation**: restoring the single-condition early return makes
`AShapeWhoseOwnFillIsAPictureCarriesThatPicture` fail. `ThePictureFillsTheShapesOwnRectangle`
survives the mutation and is a supporting assertion about the anchor, not the pin — said here
rather than implied, because a test that cannot fail is not evidence.

## 7. Suite

Every project green at **0 skipped**; `XRay.Office.Fidelity.Tests` 552 discovered, 542 passed, 10
failed — the four documented families left failing on purpose, the same ten by name as before the
round. Spreadsheets 1389 (1387 + the two new), WordProcessing 2007.

## 8. Not done

- **O92** is untouched and keeps its seat.
- **`a:tile`** is refused rather than approximated, and nothing in the corpus exercises it.
- The `.xls` and `.ods` halves of the picture-fill question are **not censused**. `OdsShapeInk`
  and the BIFF reader may or may not have the same hole; this round did not look.
