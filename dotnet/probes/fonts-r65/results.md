# fonts-r65 — the script item decides glyph fallback, and the deciding half is the language

Two defects, in the brief's order, and a third that was in the way of both.

Everything below is measured against **LibreOffice 26.2.4.2** from the TDF tarball with all three
font confounds moved aside — the metric-compatible duplicates, the Latin `NotoSans-*`/`NotoSerif-*`
and `opens___.ttf`. `/usr/bin/soffice` (24.2.7.2) is not used anywhere in this round.

---

## 1. The script-specific font item

### The mechanism, and it is not the one the brief names

Writer keeps `RES_CHRATR_FONT`, `RES_CHRATR_CJK_FONT` and `RES_CHRATR_CTL_FONT` side by side and
`SwScriptInfo::WhichFont` selects one per script item of the text
(`sw/source/core/text/porlay.cxx`:879-901, via `lcl_ScriptToFont` at :879-890). The classification is
`i18nutil::GetScriptClass`'s block table (`i18nutil/source/utility/scriptclass.cxx`:56-141); a
**weak** character — every symbol, dingbat, arrow and punctuation mark — takes the script of the text
around it, or the one `w:rFonts/@w:hint` names, and nothing else can move it
(`i18nutil/source/utility/scriptchangescanner.cxx`:246-268;
`DomainMapper::lcl_attribute`:969-988 turns the hint into `RES_CHRATR_SCRIPT_HINT`).

**The class never reaches the other two items at all.** `LN_CT_Fonts_ascii` inserts
`PROP_CHAR_FONT_FAMILY`; `LN_CT_Fonts_eastAsia` and `LN_CT_Fonts_cs` insert the *name* and nothing
else (`sw/source/writerfilter/dmapper/DomainMapper.cxx`:436-508). So those two items keep the pool
default's family type, and `OutputDevice::GetDefaultFont` sets `FAMILY_SYSTEM` for `CJK_TEXT` and
`CTL_TEXT` — *"don't care, but don't use font subst config later…"* (`vcl/source/outdev/font.cxx`,
in the `switch (nType)` over `DefaultFontType`) — which appends no generic to the pattern at all,
because `FontConfigManager::Substitute`'s switch has arms for `FAMILY_ROMAN` and `FAMILY_SWISS` only
(`vcl/unx/generic/font/fontconfig.cxx`:1075-1088).

**And each item carries its own language, which is what actually decides the answer.**
`SwDoc::SwDoc` resolves the document's three default languages through
`MsLangId::resolveSystemLanguageByScriptType` (`sw/source/core/doc/docnew.cxx`:383-398), which
answers `LANGUAGE_ENGLISH_US` for the western item, **`LANGUAGE_CHINESE_SIMPLIFIED`** for the Asian
one and **`LANGUAGE_HINDI`** for the complex one
(`i18nlangtag/source/isolang/mslangid.cxx`:135-165). `Substitute` puts it in the pattern as
`FC_LANG` (`fontconfig.cxx`:1092, 1118-1119) and `fcmatch.c` scores `PRI_LANG` **above**
`PRI_FAMILY_WEAK`. `mapToFontConfigLangTag` (`fontconfig.cxx`:936-970) then reduces the tag to what
`FcGetLangs()` knows — `hi-IN` is not a member and `hi` is; `en-US` is not and `en` is; `zh-CN`
**is**.

`isImpossibleCodePointForLang` only clears the language for Oriya, Telugu and Bengali
(`fontconfig.cxx`:979-1023), so a Hebrew or Thai character asked under `hi` keeps `hi`.

### The probe

`gen-scriptitem.py` builds 25 one-run DOCX, the family always `Calibri` so that every cell falls
back, varying the slot that names it, the `w:hint`, the declared class and the character. Faces read
out of the PDFs 26.2.4.2 produced:

| run | 26.2.4.2 draws | the pattern that explains it |
|---|---|---|
| western, `U+2610` / `U+2713` | FreeSerif; **DejaVu Sans** under `swiss` | `Calibri,serif:lang=en:charset=…` |
| `w:hint="eastAsia"`, `U+2610` or `U+2713` | **Unifont** | `Calibri:lang=zh-cn:charset=…` |
| complex, `U+05D0`; `w:hint="cs"`, `U+2610` | **FreeSans** | `Calibri:lang=hi:charset=…` |
| complex, `U+0E01` or `U+0627` | **FreeSerif** | `Calibri:lang=hi:charset=…` |
| asian, `U+4E00` | WenQuanYi Zen Hei | `…:lang=zh-cn:charset=4e00` |
| weak character, no hint | FreeSerif — the *western* item | the application language is Latin |

**The declared class moves none of the CJK or CTL rows.** `roman`, `swiss` and no font table at all
give the same answer in every one of them, which is what `dotnet/CLAUDE.md` got wrong when it said
those items have "their own family and their own class". They have their own *language*; the class is
simply absent.

**A document that states `w:lang` overrides those defaults, and Word writes one into `docDefaults`
for nearly every file.** `<w:lang w:val="en-US" w:eastAsia="en-US" w:bidi="ar-SA"/>` is what both
corpus witnesses carry, which is why their `w:hint="eastAsia"` runs answer **DejaVu Sans** and not
the Unifont a document stating no language gets. Round 64 measured the answer and inferred the wrong
cause from it.

### The seat

- **`XRay.Office.Text.Fonts.WriterScripts`** — `GetScriptClass`'s block table ported to code points,
  and `ForRun`, which reduces the paragraph-wide scanner to one item per run: the first character
  with a script of its own decides, and the hint decides only where *every* character is weak. That
  reduction is the conservative half of the rule and is deliberate — a run mixing a hinted symbol
  with prose keeps the western item rather than dragging its Latin text onto the East Asian one.
- **`WriterScripts.DefaultLanguage`** — `en-US`, `zh-CN`, `hi-IN`, from
  `resolveSystemLanguageByScriptType`.
- **`WordParagraphFormats`** — reads `w:rFonts/@w:hint` (`HintOf`) and `w:lang/@w:eastAsia` and
  `@w:bidi`; `WordTextStyle.OnScript` selects the item from the run's text and `ItemLanguage` and
  `FontItem` express it. A run naming *no* family states no item at all, which is the same
  distinction `WordFallbackClass.ForDeclared` already carried.
- **`WordFallbackClass.ForScript`** — the western item's answer for a western run and
  `FontFamilyClass.Unknown` for the other two.
- **`FontLanguages`** — a model of `FcCompareLang`. fontconfig derives a face's language set from an
  orthography per language compiled into the library, which the configuration this tree parses does
  not publish; what is asked instead is whether the face covers one exemplar character of the
  language's script. Checked against `fc-list :lang=X` over 25 languages, comparing the families each
  answers: **24 agree face for face** and the twenty-fifth (Gurmukhi) names two fewer. A Latin-script
  language is deliberately unmodelled — every text face covers Latin, so `en` can only demote a face
  fontconfig would have kept.
- **`SystemFontResolver`** — `Preferred` now ranks `PRI_CHARSET`, then `PRI_LANG`, then the generic's
  preference list, then the merged order; the `SymbolFallbackFor` path is untouched.

### The item has to travel with the *run*, and that was a defect of its own

*(Its own commit: `fix(fonts): a run's font item travels with the run, not with the face it chose`.)*


Round 64 recorded the generic against the **face** the request resolved to, first writer winning. In
a word-processing document the first request to reach a face is the paragraph mark's — so a run on
any other item silently took the paragraph's. **With the item recorded against the face, every one of
the 25 cells answered exactly as it had before the item existed**, including the ones the item was
built to fix. It also hid a row the tree was supposed to have: `west-swiss-2713` answered FreeSerif
rather than DejaVu Sans, because the package's own no-`docDefaults` Calibri had already claimed
Carlito for `serif`.

So `FontItem` is passed down through `FontItemiser.Split` and `IGlyphFallbackResolver.FallbackFor`,
carried on `FormattedRun`, `PageRun` and `PageParagraph`. A caller that supplies none — a slide, a
sheet, a metafile, and the DOC, RTF and ODF readers — keeps the face-keyed lookup it had.

---

## 2. `rMissingCodes` is a set

### The mechanism

`OutputDevice::ImplGlyphFallbackLayout` walks the layout's unmapped runs and appends every one of
their code units to a single `OUString`, then hands it to
`PhysicalFontCollection::GetGlyphFallbackFont` (`vcl/source/outdev/font.cxx`). That reaches
`FcGlyphFallbackSubstitution::FindFontSubstitute` (`vcl/unx/generic/font/fontsubst.cxx`:171-184) and
`FontConfigManager::Substitute`, which puts **every code point of it into one `FcCharSet`**
(`fontconfig.cxx`:1092-1116). `FcCompareCharSet` scores a candidate by `FcCharSetSubtractCount` —
how many of the set it is *missing* — and `PRI_CHARSET` is fontconfig's highest priority, above the
family list and above the language.

The answer is then **subtracted** from the set: `Substitute` ends by rebuilding `rMissingCodes` from
the code points the chosen face's charset does not hold (`fontconfig.cxx`:1229-1245), and the next
of `MAX_FALLBACK` levels asks again with the remainder. So a face further down the family list wins
when it covers more of the run, and which face draws one character depends on what else the run was
missing.

### The seat

`FontItemiser.Split` gathers the range's distinct missing code points in text order before it walks
the characters, resolves them in levels through the new
`IGlyphFallbackResolver.FallbackFor(IReadOnlyList<int>, …)`, and then assigns from the map. A level
that covers nothing new ends the loop rather than repeating on the same remainder, which is the
guard the C++ writes as *"ignore fallback font if it is the same as the original font"*.
`SystemFontResolver` relaxes the coverage requirement one character at a time rather than loading and
scoring every installed face, so a single missing character — which is nearly every call — costs
exactly what it did before the set existed.

A **pi face is still asked per character**, because the list that answers for one is indexed by
fallback level and not by a charset: `GetGlyphFallbackFont` returns `(*mpFallbackList)[nFallbackLevel]`
without consulting `rMissingCodes` at all. The two rules the previous rounds established therefore
both hold: a pi face is never handed to fontconfig, and a fontconfig-resolved fallback is ranked by
one generic's own list.

---

## What this did not close

- **The DOC, RTF and ODF readers state no item.** The WW8 reader builds an `SvxFontItem` per font
  from the `FFN` and has its own three items to model; nothing here changes what it does, and its
  runs keep the face-keyed generic. It is the obvious next step and it is a reader change, not a
  resolver one.
- **`FontLanguages` is a proxy.** It agrees with `fc-list` on 24 of 25 languages here and cannot be
  exact without fontconfig's orthography data.
- **Colour bitmap faces still do not paint.** Unchanged from round 64, and it belongs to
  `agent/colourglyphs`.
- **Writer's default list-bullet font.** `AAC-AD-No-2021-01…doc` and others draw their bullets in
  OpenSymbol in the reference and in the paragraph's own face here. Found by round 64; still open.

---

## Reach

### The probes

| probe | before | after |
|---|---:|---:|
| `gen-scriptitem.py`, 25 cells | 15/25 | **25/25** |
| round 64's `gen-generic.py`, 72 cells | 64/72 | **72/72** |

**Round 64 reported 65/72 and the re-measure at `260611dae` says 64.** Its stated residual is wrong
in two ways, and both are recorded here rather than corrected silently: it named *Thai under swiss*,
which agrees — `swiss__0E01` answers FreeSerif on both sides, by the accident that the complex item's
Hindi answer for Thai and the western serif list's answer are the same face — and it missed
**`swiss__2713` and `swiss__27A2`**, which are the round-64 defect showing in its own probe. Those
cells declare `Calibri` swiss, but the package has no `docDefaults`, so the paragraph mark resolves
Calibri undeclared — roman — and claims Carlito for `serif` before the swiss run asks. The eight
disagreeing cells at base are Hebrew `U+05D0` under all six classes, `swiss__2713` and `swiss__27A2`.

### The corpus

`ourfaces.sh` renders all 947 corpus documents through our own binary and records each face set;
`facediff.py` reports which sets differ between two such sweeps, and only the documents that moved are
worth a `soffice` run. **4 of 947 moved**, all words DOCX, and screened against 26.2.4.2 with
`reffaces.sh`:

| document | before | after | |
|---|---:|---:|---|
| `ESPN-R - MCF - Manual…docx` | 1 | **0** | exact match |
| `150-5370-10H.docx` | 2 | **1** | closer |
| `PI-doc.-no.-2E-Technical-Review-Report.docx` | 2 | **1** | closer |
| `AC-150-5370-10G-updated-201604.docx` | 2 | 2 | unchanged |
| total | **7** | **4** | |

Nothing got worse and no document lost a render (`(no render)` is 0 in both sweeps).

**Over round 64's own fourteen movers the symmetric difference goes 19 → 16**, which is the figure
that is comparable with the 19 that round left. `distance.py` reproduces the 19 at `260611dae`
exactly, which is what calibrates the instrument.

**Page counts on all four movers are unchanged** — 12, 696, 35 and 727 before and after — so the
extra runs the uniform-paragraph fold now keeps cost no pagination.

### What the sweeps say about the shape of the change

Only words DOCX moved. Slides, sheets and the DOC, RTF and ODF readers state no font item and keep
the face-keyed lookup, and the set changes an answer only where no single face covers everything a
run is missing *and* the ranking differs — which on this machine is rare, because the first face on
the generic's list usually covers the whole set.
