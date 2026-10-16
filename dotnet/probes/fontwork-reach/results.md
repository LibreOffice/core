# Two WordArt reach censuses, and what they settle

Round `agent/fontwork2`. Environment: this container, corpus at `/home/user/sample-files`
(945 documents). `census.py` beside this file reproduces both.

## 1. Binary DOC/PPT Escher text path — 5 shapes, 4 documents

Censused by scanning every `.doc .dot .ppt .pot .pps .xls .xlt .rtf` for an `msofbtSp` record
(fbt `0xF00A`, length 8) whose *instance* — the shape type — is in the WordArt range 136 to 175
(`include/svx/msdffdef.hxx:412-451`). 181 binary files scanned.

| document | shapes | type |
|---|---:|---|
| `words/done-011/doc/135.doc` | 1 | 136 `mso_sptTextPlainText` |
| `words/done-014/doc/644730BRI0mna000BOX361539B00public0.doc` | 2 | 136 |
| `slides/done-008/ppt/8.16_AOD_FINAL_Provider_Training_Presentation_9_2009.ppt` | 1 | 144 `mso_sptTextArchUpCurve` |
| `slides/ceiling-002/ppt/pres_ioc_phuket.ppt` | 1 | 136 |
| **total** | **5** | |

For comparison: DOCX VML holds 15, DrawingML holds 29 warped bodies on the words side and ten arch
occurrences on two decks. **The binary path is the smallest of the three.**

All four documents are far out for reasons that have nothing to do with WordArt — 100 dpi mean
absolute grey difference against 24.2.7.2 over the whole document: `135.doc` **8.25**,
`644730BRI…` **23.29**, the AOD deck **3.75** over 94 slides, `pres_ioc_phuket` **4.58** over 26.
One shape each would be invisible in those figures, which is the second half of the answer: even
implemented, this would not be measurable on the corpus.

### The two knobs, settled

The round was briefed expecting `gtextFSameHeights` → `SameLetterHeights` and `gtextFStretch` →
`TextPathScaleX` to become reachable through the VML path. **They do not**, and they are not
exercised by the binary path either.

- **VML hard-codes both false.** `oox/source/vml/vmlformatting.cxx:966-975` writes `ScaleX` and
  `SameLetterHeights` as literal `false` into every `v:textpath`'s `CustomShapeGeometry`, whatever
  the shape type. The authored fixture's `V005 same letter heights` case
  (`v-same-letter-heights:t` in the textpath style) confirms it: the reference draws it
  identically to `V001`.
- **The binary path reads them and the corpus never sets them.** `msdffimp.cxx:2516-2600` takes
  `SameLetterHeights` from bit `0x80` of `DFF_Prop_gtextFStrikethrough` and `ScaleX` from bit
  `0x40`, and `IsHardAttribute(DFF_Prop_gtextFStretch)` from property 245. Read out of all five
  shapes' own `msofbtOPT` records:

  | shape | property 255 | SameLetterHeights (0x80) | ScaleX (0x40) | property 245 present |
  |---|---|---|---|---|
  | `135.doc` | `0xc0804000` | no | no | no |
  | `644730BRI…` #1 | `0x57305730` | no | no | no |
  | `644730BRI…` #2 | `0xd7305720` | no | no | no |
  | AOD deck | `0xf2bb5200` | no | no | no |
  | `pres_ioc_phuket` | `0xffff5700` | no | no | no |

  All five state `textpath` on (bit `0x4000`), three state `fitshape`, none states `ScaleX` or
  `SameLetterHeights`, and **none hard-sets `gtextFStretch`**, so even the fallback branch at
  `msdffimp.cxx:2531-2545` takes the shape-type default.

So `SameLetterHeights` is unreachable from OOXML VML by construction and unset in every binary
Escher WordArt shape the corpus holds. There is no document in this corpus, in any format, that
would render differently if it were implemented.

## 2. CFF/OTTO faces — confirmed, and no Type 2 interpreter is needed

`GlyphOutlines` reads `glyf` and answers null otherwise, so a warp set in a CFF face draws nothing.
Installed faces, by outline format:

| | files |
|---|---:|
| TrueType `glyf` | 45 |
| CFF / OTTO | 11 |
| Type 1 (`.pfb`/`.pfa`) | 8 |
| other | 1 |

The eleven CFF faces are `Loma{,-Bold,-Oblique,-BoldOblique}.otf` (Thai) and seven Unifont
variants. **Nothing WordArt resolves to.** Every family named in a part that carries a real warp —
counted across the whole corpus, which over-counts because it includes the unwarped runs in the
same part — resolves to a `glyf` face:

| family | resolves to | outlines |
|---|---|---|
| Arial (292, and 26 more from VML) | LiberationSans-Regular | `glyf` |
| Perpetua Titling MT (80) | DejaVuSans | `glyf` |
| Kristen ITC (68) | DejaVuSans | `glyf` |
| Times New Roman (40) | LiberationSerif-Regular | `glyf` |
| Calibri (10, and 12 from VML) | Carlito-Regular | `glyf` |
| Arial Black (6) | DejaVuSans | `glyf` |
| Corpid E1s SCd Regular (6) | DejaVuSans | `glyf` |
| Papyrus (4) | DejaVuSans | `glyf` |
| Informal Roman (4) | DejaVuSans | `glyf` |

**A Type 2 charstring interpreter would change nothing about this corpus's output**, and it is left
unwritten. The measurement is what is recorded instead. It is worth re-running before that decision
is relied on elsewhere: `fc-match` answers for the installed font set, and both the set and the
answers move with the container.
