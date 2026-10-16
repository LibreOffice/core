# Round 139 — O92: a number format's colour clause

Worked solo. **O92 is closed for the OOXML readers.** The ODF twin is measured, still wrong, and
seated separately rather than bundled in — see §6.

## 1. The seat reproduces exactly on the figure it said to quote

| | seat | measured here |
|---|---|---|
| `xlsx` stating a colour clause | 23 of 241 | **14 of 240** |
| ...using one on a numeric cell | 20 (13 771 cells) | **12 (12 479 cells)** |
| **...where a value selects the coloured section** | **8 documents, 137 cells** | **8 documents, 137 cells** — exact, same eight |
| `069`'s styles | 39 `cellXfs`, 10 at `numFmtId=38`, one custom | exact |

The headline — *"8 documents and 137 cells, which is the figure to quote"* — reproduces to the
document. My two intermediate counts are lower than the seat's; the seat counted clauses stated in
`styles.xml`, this counts clauses reachable through a `cellXf`. The difference does not touch the
figure either of us said to use.

Breakdown of the 137 by colour, which the seat did not have: **RED 115, COLOR10 18, BLUE 4**.

## 2. The witness, before and after

`069_Blue_modern_balance_sheet`, span colours read with PyMuPDF:

| | ours before | ours after | 26.2.4.2 |
|---|---|---|---|
| `(100)`, `(85)` | `#262626` | **`#ff0000`** | `#ff0000` |
| `(185)` | `#000000` | **`#ff0000`** | `#ff0000` |
| `500` (blue), `700` (red) | correct | correct | correct |

The last row is the discriminator the blind reader supplied and it is why this was never "font
colour is dropped": the same row already drew a blue 500 and a red 700 on both sides.

## 3. The rule, from LibreOffice's own source and confirmed at its binary

`ImpSvNumberformatScan::GetColor` (`svl/source/numbers/zforscan.cxx`:546-659) matches the
upper-cased word against ten keywords and indexes `StandardColor` (`:98-111`, `:145-147`):

| keyword | BLACK | BLUE | GREEN | CYAN | RED | MAGENTA | BROWN | GREY | YELLOW | WHITE |
|---|---|---|---|---|---|---|---|---|---|---|
| value | 000000 | **0000FF** | 00FF00 | 00FFFF | FF0000 | FF00FF | 808000 | 808080 | FFFF00 | FFFFFF |

**`BLUE` is `COL_LIGHTBLUE`, not `COL_BLUE`** — the fixture states a blue section for exactly that
reason, and 26.2.4.2 draws it `#0000ff`. Confirmed at the binary for all three colours the corpus
uses: `[Red]` → `#ff0000`, `[Blue]` → `#0000ff`, `[Color10]` → `#dddddd`.

## 4. `[COLOR n]` is refused, and the reason is an installation asset

`GetColor`:630-637 sends a numeric index to `GetUserDefColor(n-1)`, which Calc wires to
`XColorList::CreateStdColorList()` (`sc/source/core/data/documen9.cxx`:246-259) — the palette loaded
from `SvtPathOptions().GetPalettePath()`. That is the running installation's own
`share/palette/standard.soc`, 120 entries, user-configurable; its index 9 is `#dddddd` "Light Gray
4", which is exactly what 26.2.4.2 paints for `[Color10]`.

Reproducing it would mean vendoring one installation's UI palette into a document renderer and
pinning to it — the same class of coupling as the bundled-font confound this project already
records. **Refused cleanly instead**, leaving those cells the colour they had. Reach of the gap:
**18 cells in one document** (`018_Weight_Loss_Chart`), which also uses `[Red]` and `[Blue]` and
gets both of those right.

## 5. Confinement

`SheetTextLayout` is the only reader of the new property, so the blast radius is the sheets track.
Rendered all **307** `xlsx`/`xls`/`xlsm`/`xlsb` twice under `SOURCE_DATE_EPOCH`:

- **7 renderings move.** 300 byte-identical.
- The seven are exactly seven of the eight documents holding a selecting cell.

**The eighth does not move and should not.** `certification-type-certificates-…-MAdB-Light-Prop`
draws its red from explicit cell font colours: this tree already matched 26.2.4.2 there exactly —
**4838 red spans, 686 black, 343 pages on both sides** — and its two format-coloured cells are at
`AN5267`/`AN5295`, which the printed block does not reach. An expected mover that does not move is
worth chasing to an answer rather than rounding off.

## 6. The ODF twin is still wrong, for a larger reason — seated

Converting the witness to `.ods` with 26.2.4.2 and rendering both sides:

```
ours : (100)=#262626  (85)=#262626  (185)=#000000
ref  : (100)=#ff0000  (85)=#ff0000  (185)=#ff0000
```

Two gaps, and the colour is the smaller one. ODF states the colour as
`<style:text-properties fo:color="#ff0000"/>` — a **`style:`-namespace child** that
`OdfNumberFormat.Code`'s loop skips, since it walks `number:` children only. But ODF also splits
the subformats across *separate* `number:number-style` elements linked by `style:map`:

```xml
<number:number-style style:name="N142">
  <style:text-properties fo:color="#ff0000"/>
  <number:text>(</number:text><number:number …/><number:text>)</number:text>
  <style:map style:condition="value()&gt;=0" style:apply-style-name="N142P0"/>
</number:number-style>
```

**`style:map` is not followed for number styles anywhere in this tree** (`git grep` over
`OdfNumberFormat` and `OdsCellFormats` finds no handling), so the ODF path compiles one section and
loses the others. That is the bigger defect and has to be fixed first or with it. Seated as **O96**
rather than bundled here: it is an ODF section-assembly question, not a colour one, and the
`.ods`/`.fods` corpus needed to size it does not exist in this container.

## 7. Tests

`SheetNumberFormatColourTests`, **23 facts**, against a new 2038-byte fixture
`features/sheet-numfmt-colour.xlsx` (`make-fixture.py` here) stating `0.0;[Red]-0.0;[Blue]0.0`.

**Validated against the reference, not just against us**: 26.2.4.2 renders the fixture
`1.5`=#000000, `-2.5`=#ff0000, `0.0`=#0000ff, `-2.50`=#000000, and this tree now agrees span for
span.

The fourth cell is the one that earns its place: it holds the **same −2.5** as the red cell under a
*colourless* format, so an implementation that coloured by the sign of the value rather than by the
selected subformat fails on it.

**Pinned by mutation, twice**: making the draw site ignore the format colour fails 2 of 23; making
the parser never recognise a colour fails 16 of 23.

## 8. Suite

Every project green at **0 skipped**. Spreadsheets **1412** (1389 + 23). Fidelity 552 discovered,
542 passed, 10 failed — the four documented families, same by name.

## 9. Not done

- **`[COLOR n]`** — refused, §4, 18 cells in one document.
- **The ODF path** — §6, seated as O96.
- **`.xls`/`.xlsb`** carry the same code through the same draw site and would be covered by the
  same rule, but **no corpus `.xls` states a colour clause**, so there is nothing to measure and
  nothing is claimed for them.
