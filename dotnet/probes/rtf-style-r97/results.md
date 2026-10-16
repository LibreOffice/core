# `rtf-style-r97` — O24: the other route into *Standard*, what the intermediates add, and a reach of four

Measured 2026-09-11 in `/home/user/wt-rtfstyle` (branch `agent/rtfstyle`, base `61bc19e03`).
Reference **`/opt/libreoffice26.2/program/soffice` — LibreOffice 26.2.4.2
`0229ac93fcf0d7cbc6376066c6f35021cef002dc`**. Nothing here is read off a page by eye, and there is
no `Task`/subagent tool in this container to delegate a blind reading to.

**This round was interrupted by a container restart and resumed.** `f7f5572aa` is what was on disk
at that instant; it was never built, never run and never reviewed. Everything below has been
re-measured from the beginning, and §6 lists what the salvaged draft got wrong — including one
claim whose evidence file was *empty*, and one measuring instrument that was reporting the
opposite of the truth on a third of its rows.

`words-close-r95` §2 closed O9: `Body Text` and `caption` inherit the document's own `Normal`,
because *Standard* is a **reference** rather than a constant, and its reach is **nil** because both
corpus entries state their own `\fs` and a style's own `\fs` reaches no paragraph. It left six
names out — `header`, `footer`, `toc 1`…`toc 3` and a bare `Heading` — on the stated grounds that
they reach `COLL_STANDARD` through an *intermediate* pool style whose own properties the import
does not reset. This round measures those intermediates, finds a seventh and eighth name the map
cannot see, and measures the reach the way O9's closure demands.

---

## 1. The two routes, and the census of the second

`StyleSheetTable::ApplyStyleSheets` converts the entry's name and then asks `xStyles->hasByName`
(`sw/source/writerfilter/dmapper/StyleSheetTable.cxx`:1099-1101 — r95's citation, verified line for
line). **Both halves of that sentence decide the question, and only the first had been censused.**

For a name `ConvertStyleName`'s map does not hold, the function returns it **unchanged**
(`:2083-2113`) — unless the name collides with one of the map's own *values*, when it gains a
` (WW)` suffix and then matches nothing.

**What the second lookup accepts is narrower than "either spelling", and the first cut of the
census had it wrong.** `SwXStyleFamily::hasByName` (`sw/source/core/unocore/unostyle.cxx`:1030-1039)
is `FillUIName` followed by `SwDocStyleSheetPool::Find`, and `FillUIName`
(`sw/source/core/doc/SwStyleNameMapper.cxx`:306-335) has three arms:

* the name **is** a programmatic pool name → it becomes that pool style's UI name and `Find` hits;
* the name is not programmatic but **is** a pool style's UI name → `rFillName = rFillName + " (user)"`
  (`:327`) and `Find` misses;
* neither → the name goes through unchanged, and `Find` matches only a style physically in the
  document.

`Find` itself does hit a pool style that has not been created yet, because `FillStyleSheet`'s
`FillOnlyName` arm falls back to `GetPoolIdFromUIName` when no collection exists
(`sw/source/uibase/app/docstyle.cxx`:2389-2416) and `Find` returns the sheet whenever that
answered (`:3113`). So the reachable set is *the programmatic names of Writer's paragraph pool
styles*, plus whatever the import has already created.

`writer-names.tsv` is the pool table, generated from the reference's own two arrays — the
`STR_POOLCOLL_*` order in `DocumentStylePoolManager.cxx` against `sw/inc/strings.hrc` for the UI
names and `SwStyleNameMapper`'s `Get*ProgNameArray` for the programmatic ones. **126 paragraph pool
styles, 252 spellings, of which the programmatic column is what route two can reach.**

`hasbyname-census.py` runs both rules over the 338 converted `.rtf`, on r88's selection rule (a
style counts as *used* only when a `\sN` outside `{\stylesheet}` names it) and r95's condition (no
resolvable `\sbasedon`, because that is when the pool parent decides anything). It also prints the
difference between the strict rule and the rejected lenient one:

| | |
|---|---:|
| `.rtf` scanned | **338** |
| distinct names applied without a resolvable `\sbasedon` | **84** |
| of those, names that reach an existing Writer paragraph style | **17** |
| of the 17, names that reach it by `hasByName` rather than by the map | **3** |
| names the lenient UI-spelling rule would have added and `FillUIName` rejects | **0** |

The three are **`Figure`, `Heading` and `Text`**, one document each, and all three are names whose
programmatic and UI spellings are identical, which is why the correction costs nothing here.
**The brief is wrong about `Figure`**: it is in `ConvertStyleName`'s map for neither spelling, so it
is in the same class as the bare `Heading` the brief separated it from. `Text` is a name nobody had
named — it is `COLL_LABEL_FRAME`, and `bulletin.rtf` applies it to 17 paragraphs.

**Every Writer paragraph pool style has `COLL_STANDARD` at the root of its chain.** That falls out
of transcribing `GetPoolParent` (`sw/source/core/doc/poolfmt.cxx`:169-298) group by group, which the
census does: `COLL_HTML_BITS` answers Standard outright; `COLL_LISTS_BITS`, `COLL_REGISTER_BITS` and
`COLL_DOC_BITS` funnel through one base style each; and in `COLL_EXTRA_BITS` and `COLL_TEXT_BITS`
every id that exists is handled and every branch terminates there. So *"does the walk reach
Standard"* is never the question. *"What does the intermediate contribute"* is the whole of it.

**And the branch that would cut the chain cannot fire for RTF.** `ApplyStyleSheets`:1113-1119 clears
a matched built-in style's parent when the entry states no `\sbasedon` — but only under
`m_bHasImportedDefaultParaProps`, which is set at `:667` from `LN_CT_PPrDefault_pPr` /
`LN_CT_DocDefaults_pPrDefault` alone. `git grep -n "PPrDefault\|DocDefaults" sw/source/writerfilter/rtftok/`
returns **nothing**, so no RTF import ever sets it and the pool parent always stands.

The control family is unchanged and now has two more members. `Quote`, `List Paragraph` and
`Normal (Web)` are in the map with an **empty** Writer name, so `ConvertStyleName` passes them
through and no pool style answers; `Marginalia` and `Text body indent` are Writer names that are
also map *values* (`:1710-1711`, `:1786`), so the ` (WW)` suffix takes them out of reach. Measured:
all five keep `\pard\plain`'s twelve points at 26.2.4.2 (§2).

## 2. What the intermediates contribute — 116 probes, and the instrument that had to be fixed first

`genpool2.py` and `genpool3.py` write 116 one-name probes: a style named *X* with a forward
`\sbasedon` that cannot resolve, applied to one paragraph between two `\fs20` controls, each arm ×
the two `Normal` sizes that tell inheritance from a constant. `genpool2` adds an arm where the entry
states its own `\fs28` and an arm whose paragraph holds a `\tab`; `genpool3` adds an arm where the
document also declares a `caption` of its own. `readfodt.py` resolves them out of
`soffice --convert-to fodt`, which is ~0.15 s a document when a directory goes to one process
against ~8 s for a PDF.

**`readfodt.py` as salvaged read only the paragraph-style chain, and on the `own` arm that is the
wrong answer.** When a stylesheet entry states its own `\fs`, the reference writes RTF's reset back
over the run as *direct* character formatting — an automatic `style:family="text"` style on a
`text:span` inside the paragraph — which no walk up `style:parent-style-name` can see. The style
object keeps its 14 pt and only the span says the run is 12. Uncorrected, the instrument reported
14 pt on all twenty `own` rows and would have refuted r95's rule on its own evidence; corrected, it
reproduces r95's rule **20 of 20**. The docstring records it, and this is the round's sharpest
instance of *the measurement was measuring the instrument*.

`pool-resolved-2.txt` and `pool-resolved-3.txt` are the corrected readings, and **they are the
reference's side only** — the flat ODF is an instrument this tree has no counterpart for. The
common channel is the raster, and `measurepool.py` is it: both sides rendered to PDF, HEAD's drawn
size, face and the baseline gaps either side read out with PyMuPDF.

| | agree | differ |
|---|---:|---:|
| `genpool2.py` — 10 names × 3 arms × 2 `Normal` sizes (`both-sides-2.txt`) | **60** | **0** |
| `genpool3.py` — 14 names × 2 arms × 2 `Normal` sizes (`both-sides-3.txt`) | **52** | **4** |
| total | **112** | **4** |

**The four are `List Indent`** — `p_listindent_{plain,withcap}_{20,28}`, the `COLL_TEXT`
intermediate this round deliberately does not model (§5) — and they are reported as differing
rather than dropped. Beside the raster, `RtfStyleFormattingTests` carries **36 cases** over these
names, 28 of them added by this round; §7 shows which fail at the base.

At 26.2.4.2:

| intermediate | names measured under it | what it adds to the paragraph |
|---|---|---|
| `COLL_HEADERFOOTER` — *Header and Footer* | `header`, `footer`, `Header`, `Footer` | **nothing** — no size, no spacing, **no tab stops** |
| `COLL_REGISTER_BASE` — *Index* | `toc 1`, `toc 2`, `toc 3`, `TOC 1`, `Index 1` | **nothing** |
| none — a bare `Heading` **is** `COLL_HEADLINE_BASE` | `Heading` | **nothing** |
| none — `COLL_COMMENT`, `COLL_SIGNATURE` are under Standard directly | `Comment`, `Signature` | **nothing** |
| `COLL_LABEL` — *Caption* | `Figure`, `Illustration`, `Table`, `Drawing`, `Text` | **italic, 12 pt, 6 pt above and below** |
| `COLL_TEXT` — *Text body* | `List Indent` | 7 pt below and 115 % line spacing — **not modelled**, §5 |
| — (control) | `Quote`, `Marginalia`, `Text body indent` | nothing on the chain at all |
| — (control) | `heading 4`, `Body Text` | r95's and r87's answers, reproduced |

**The `COLL_HEADERFOOTER` row is a measurement, not a reading, and it had to be**:
`DocumentStylePoolManager.cxx`:913-940 gives *Header and Footer* two tab stops and switches line
numbering off, and after an RTF import it has neither — the `tab` arm of `genpool2` puts a `\tab` in
the paragraph and the reference draws no tab stop on any of the twelve header/footer rows.
`intermediates.py` is the control that separates *the pool style has none* from *the export does not
write them*: the same binary and the same filter over a natively-imported document, where both stops
are present (`intermediates.txt`).

**The line that decides it is `DomainMapper.cxx`:141**, *"Don't load the default style definitions
to avoid weird mix"* — `SetDocumentSettingsProperty("StylesNoDefault", true)` at the start of the
import, put back to false at `:259`. That flag is `bNoDefault` in
`DocumentStylePoolManager::GetTextCollFromPool` (`:676-677`), and it skips the whole per-style
`switch` that would have given a newly created pool style its properties — while leaving the
`GetPoolParent` linkage above it (`:660-674`) intact. So the rule is **when the pool style was
created**, not which one it is:

* `COLL_HEADLINE_BASE`, `COLL_NUMBER_BULLET_BASE`, `COLL_LABEL` and `COLL_REGISTER_BASE` are created
  by `SwDocShell::InitNew` (`sw/source/uibase/app/docshini.cxx`:224-289) before the filter runs, and
  keep everything;
* `COLL_HEADERFOOTER`, `COLL_HEADER`, `COLL_FOOTER`, `COLL_COMMENT` and `COLL_SIGNATURE` are created
  *during* the import, when the entry's name is looked up, and get nothing.

**That same `InitNew` loop is where *Caption*'s 12 pt comes from, and it is not the `PT_10` in
`COLL_LABEL`'s own block** (`DocumentStylePoolManager.cxx`:978). The loop overwrites each of those
four styles' size with the configured default — `FONTSIZE_DEFAULT` 240 for `FONT_CAPTION`,
`FONTSIZE_OUTLINE` 280 for `FONT_OUTLINE` (`sw/source/uibase/inc/fontcfg.hxx`:51, :54,
`SwStdFontConfig::GetDefaultHeightFor`, `fontcfg.cxx`:251-274) — and writes it only when it differs
from what the style already carries (`docshini.cxx`:279-288). That is why the four behave
differently and why the difference is arithmetic rather than a special case:

| pool style | its own block states | `InitNew` wants | result |
|---|---|---|---|
| `COLL_LABEL` | `PT_10` = 200 | 240 | **written — 12 pt** |
| `COLL_HEADLINE_BASE` | `PT_14` = 280 | 280 | equal, not written — 14 pt either way |
| `COLL_REGISTER_BASE` | no size (inherits Standard's 240) | 240 | equal, not written — **no size** |
| `COLL_NUMBER_BULLET_BASE` | no size | 240 | equal, not written — no size |

The 280 is the 14 pt this tree has given `HeadingPool` since round 87 from the other citation; the
two agree. The `COLL_REGISTER_BASE` row is what makes *Index* transparent, and it is measured as
well as derived: `p_index1_plain_20` reads `10pt@Standard` and `_28` reads `14pt@Standard`.

**A bare `Heading` answers *Standard*, and that is the trap in the name.** `Heading` *is*
`COLL_HEADLINE_BASE`, so the very style that supplies `heading 1`…`heading 9` with their 14 pt, 12/6
and keep-with-next is the one `SetPropertiesToDefault` resets when a document declares a style of
that name. Measured on six probes: `p_headingbare_plain_20` is 10 pt with no spacing and no keep,
`_28` is 14 pt, and the `heading 4` control beside it is 14 pt at both `Normal` sizes with
0.1665 in / 0.0835 in and `keep-with-next always`.

## 3. *Caption* is a reference too, and the corpus takes that arm

`SetPropertiesToDefault` (`StyleSheetTable.cxx`:305-331, called at `:1111`) resets the style the
entry **matched**, and the entry's own properties are then written back over it. A document that
declares a `caption` of its own therefore *replaces* the intermediate — and both corpus documents
that apply a `COLL_LABEL_*` name do exactly that:

```
{\s344\sbasedon0\snext0\sb120\sa60\keepn\rtlch\ab\ltrch\f4\fs20\b caption;}   231164_SystemDesignDocument
{\s593\sbasedon0\snext0\rtlch\afs20\ab\ltrch\f0\fs20\b caption;}              bulletin
```

`genpool3.py` measures both arms over fourteen names × two `Normal` sizes. The `plain` arm gives
12 pt italic with 6/6 for all five `COLL_LABEL_*` names at both `Normal` sizes; the `withcap` arm
gives **10 pt bold, upright, no added spacing** for all five, at both. The bold is read out of
`fo:font-weight`, which the salvaged `readfodt.py` collected and did not print — so the
"bold 10 pt" the draft asserted was, until this run, unmeasured.

The two corpus documents' own flat ODF confirms the same thing on the real files rather than on a
probe. `corpusfodt.py` is `readfodt.py`'s sibling for a document with no marker in it — it selects
paragraphs by their style *chain* and collapses identical rows — and `corpusfodt.txt` is its
reading of both:

```
231164   1x  Figure>Caption>Standard  size=12pt@Figure  weight=bold@Caption  keep=always@Figure
bulletin 7x  P23>Text>Caption>Standard  size=11pt@Text  weight=bold@Caption
```

**The size in both rows is the entry's own and reaches the paragraph as direct formatting**: every
one of `bulletin`'s seventeen `\s606` paragraphs restates `\fs22` inline, and `231164`'s `\s347`
restates `\fs24`, so nothing about the size distinguishes the two trees. **The weight does**: the
paragraphs state no `\b`, and `bold@Caption` is the pool parent's contribution and nothing else.

So `RtfPoolParent.Caption` answers the document's own `caption` entry when there is one and Writer's
pool *Caption* when there is not, exactly as `RtfPoolParent.Standard` answers the document's own
`Normal`.

## 4. Reach: **4 of 338**, and it is not the twelve that apply a name

**A document that declares and applies one of these names is a candidate, not a mover.** That is the
whole of O9's closure — `Body Text` is applied by 2 of 338 and neither moves — and
`reach-census.py`, which counts the candidates, is kept only as the shortlist this section explains.
Its answer is *12 apply a name this round adds*, and it is not a reach figure.

`reach-sweep.py` is the measurement, and `reach-base.tsv` / `reach-head.tsv` are its two legs —
one row per document, `name\thash\tsize`. Every one of the **338 converted `.rtf`** in
`/home/user/corpus-odf/words` is rendered by `XRay.Office.Cli render --format pdf` under a pinned
`SOURCE_DATE_EPOCH`, hashed, and the PDF deleted immediately — so the sweep costs one PDF of disk
rather than 338, which is what made it affordable at 3.3 GB free. Determinism was checked before the
sweep rather than assumed: two renders of `bulletin.rtf` by the same binary are byte-identical.
Both legs rendered 338 of 338 with no failures.

**The corpus is 338 `.rtf` and 336 of them have a banked 26.2.4.2 reference**
(`/home/user/gate-odf-r80/ref`, 336 `__rtf.pdf`). Both numbers are in circulation and they are not
interchangeable: 338 is what we render, 336 is what is scoreable. This sweep is 338, because it
compares our two legs against each other.

| | |
|---|---:|
| `.rtf` rendered at `61bc19e03` and at this round's HEAD | **338** |
| renderings that differ by a byte | **4** |
| documents that apply a name this round adds | 12 |
| of those twelve, documents whose rendering does not move | **8** |

The four are `150-5370-10H`, `19-06 Assistive Technology TAB Final - 508`,
`AC-150-5370-10G-updated-201604` and `bulletin`.

**A byte is not a pixel, and one of the four is only a byte.** Rasterised page by page at 150 dpi,
`19-06 Assistive Technology TAB Final - 508` is **identical on all eight pages** — its PDF is
eleven bytes shorter and draws the same ink, because what it inherits from `Normal` is `\cf0`,
a colour index whose resolved value is the black it already had. `bulletin` differs on **4 of its
15 pages**, page 4 by 25.01 of mean grey. So the honest headline is **4 of 338 renderings change and
3 of 338 change what is drawn.**

Scored against the banked 26.2.4.2 (`/home/user/gate-odf-r80/ref`) by `mover-ink.py`, mean
per-page absolute grey difference at 150 dpi — a *distance*, so it can fall as well as rise
(`mover-ink.txt`):

| document | pages ref/ours | before | after | |
|---|---|---:|---:|---|
| `150-5370-10H` | 746/746 | 23.0532 | **22.9818** | better |
| `19-06 Assistive Technology TAB Final - 508` | 8/8 | 24.3159 | 24.3159 | level (pixel-identical) |
| `AC-150-5370-10G-updated-201604` | 667/665 | 17.1045 | **16.8734** | better |
| `bulletin` | 15/15 | 3.7463 | **1.7038** | better |
| `231164_SystemDesignDocument` (control, does not move) | 21/21 | 13.5668 | 13.5668 | level |

**Three of the four improve and none worsens**, which is not something the byte comparison could
have told me: `bulletin` more than halves, from 3.7463 to 1.7038, because seventeen paragraphs
the reference draws bold were drawn light. `AC-150`'s 667/665 page difference is unchanged by
this round and predates it.

**The whole of the gap between twelve and four is one rule, and it is O9's.** `ContributionOf`
answers the *inherited* value only where the named style states nothing for that property, so a
pool parent changes an answer exactly when the entry leaves a property unstated and the parent
states it. `whichproperty.py` applies that by hand to all seventeen (document, style) pairs the
twelve documents contribute, against `Normal` for a `Standard`-parented name and against the
document's own `caption` entry over `Normal` for a `Caption`-parented one — `whichproperty.txt`:

| document | style | what the pool parent newly supplies | rendering |
|---|---|---|---|
| `150-5370-10H` | `s3154 header` | align, colour, space before | **moves, and improves** |
| `AC-150-5370-10G-updated-201604` | `s1239 header` | align, colour, space before | **moves** |
| `19-06 Assistive Technology TAB Final - 508` | `s121 header` | colour | bytes only |
| `bulletin` | `s606 Text` | **bold**, from the `caption` entry | **moves, 4 pages of 15** |
| `231164_SystemDesignDocument` | `s347 Figure` | **bold**, from the `caption` entry | does not move |
| the other twelve pairs, in nine documents | `header`, `footer`, `toc 1`…`toc 3`, `Heading` | **nothing** | do not move |

So **five pairs can move and four renderings do**, and the two that fall out are each explained
rather than shrugged at:

* **`231164_SystemDesignDocument`**'s `Figure` paragraph holds a `\pict` and a bookmark and **no
  text at all** (`\pard\plain \s347…{{\*\bkmkstart _GoBack}{\pict…`), so a character property has
  nothing to change. Byte-identical before and after.
* **`19-06`** gains a foreground colour index of **0**, whose resolved value is the black the run
  was already drawn in — hence a shorter content stream and eight pixel-identical pages.

And the twelve pairs that gain nothing are the interesting ones, because they are what a
declaration census counts and a reach figure must not. `CSCRS_…-with-instructions`, the only
document applying a bare `Heading`, states its own `\fs24`, `\qc`, `\caps`, `\b`, `\sb0\sa0` and
`\cf0`: every property its pool parent could have supplied.

`whichproperty.py` is a reimplementation of `ContributionOf` and is therefore evidence about the
model rather than about the reference. It is here because it *explains* the sweep exactly — five
pairs predicted to be able to move, four documents measured to move — and not in place of it.

`ANameWriterHasNoStyleForKeepsTheResetSize` and `AnIntermediateBelowTextBodyIsNotModelledYet` are
the controls that must not move, and the 334 unchanged renderings are the corpus-scale form of the
same control: **`RtfStyles.cs` is reached by every one of the 338, and 334 of them are byte-identical
across the change.** That is the confinement measured rather than assumed.

## 5. What is not modelled, and why

**`COLL_TEXT` as an intermediate.** `List Indent` is `COLL_CONFRONTATION`, whose pool parent is
`COLL_TEXT` — *Text body*, which states 7 pt below and 115 % proportional line spacing
(`DocumentStylePoolManager.cxx`:693-703) and which `InitNew` does not create but which the corpus
never reaches either. 26.2.4.2 answers the document's own `Normal` size with `0in`/`0.0972in` from
*Text body*; this tree answers `\pard\plain`'s twelve points and no spacing.
`AnIntermediateBelowTextBodyIsNotModelledYet` pins it, and it is left open for two reasons:
`RtfStyleFormatting` carries no proportional line spacing, and the reach is **nil** — 0 of 338 apply
any `COLL_TEXT`-parented name. It is `N17` in the register. Four of the 116 both-sides probes are
these and are **the only four that differ**; `both-sides-3.txt` shows the reference at 10/14 pt with
20.25/20.89 pt below against this tree's 12 pt and 11.52.

**The rest of `ConvertStyleName`'s map and the rest of the 126 pool styles.** `PoolParentOf` holds
the names this round measured plus the numeric siblings of measured families. Specifically not
modelled and specifically nil on this corpus: the no-space spellings `TOC1`…`TOC9`
(`StyleSheetTable.cxx`:1695-1703), `Index Heading`, `Envelope Address`, `Table of Figures` and the
other named map entries whose Writer style exists. None of the 84 applied names is one of them.

**`RtfPoolParent.Heading` does not walk into `Normal`.** `HeadingPool` is folded in as a constant and
the walk stops, so a `Normal` stating bold or a colour does not reach a heading-parented style
although §1 establishes that `COLL_HEADLINE_BASE`'s parent is `COLL_STANDARD`. That is r87's and
r95's model unchanged; this round did not measure it and deliberately did not change it, because the
nine headings are 12 of 338 documents — three times this round's candidate set — and a wrong answer
there is far more expensive than the two this round is about. **It is the obvious next seat, and it
is `O30` in the register.**

**A candidate residual that turned out not to be one, recorded because the first reading of it was
wrong.** `corpusfodt.txt` shows 26.2.4.2 giving `231164`'s `Figure` paragraph
`above=0.0835in@Figure`, which reads like the `Figure` style's own `\sb120` reaching the paragraph
and contradicting r80's rule that `ContributionOf` implements. It is not: **that paragraph restates
`\sb120` itself** (`\pard\plain \s347\ql\keep\widctlpar\sb120\sa0\keepn…`), so it is direct
paragraph formatting and both trees honour it. The flat ODF cannot tell a style's contribution from
direct formatting that the reference folded into the style, so **a `@StyleName` in that column is
not by itself evidence about inheritance** — check the paragraph's own control words before reading
one as a residual.

## 6. What the salvaged draft got wrong

The container restart's commit is `f7f5572aa` and none of it had been run. Re-measuring found five
things, in descending order of how much they mattered:

1. **`pool-resolved-2.txt` was empty** — a header line and no rows. The draft's table of "60 agree,
   0 differ" for `genpool2` had no evidence on disk at all, and neither did its "112 agree, 4
   differ" over both generators; `measurepool.py` had produced no output. Re-run, both of those
   numbers turn out to be **right** — which is the least comfortable of the five findings, because
   a claim that is correct and unsupported is indistinguishable from one that is neither until
   somebody runs it. The 60 fodt rows and the 116 raster rows now exist.
2. **`readfodt.py` was reading the wrong thing on the `own` arm** (§2). Left uncorrected it reports
   that a style's own `\fs` *does* reach the paragraph, contradicting O9 on eight probes.
3. **The reach figure was a count of documents that declare a name.** `reach-census.txt` says
   *12 apply a name this round adds*; the measured reach is **4**, and four of the twelve
   non-movers are non-movers for reasons worth stating.
4. **`hasByName` does not accept a pool style's UI spelling** (§1). The census's rule 3 was wrong;
   on this corpus the strict and lenient rules give the same 17 names, so no conclusion changes,
   but the script now prints the difference rather than assuming it is empty.
5. **Four citations were off and one pointed at the wrong statement.** `poolfmt.cxx`'s
   `GetPoolParent` is `:169-298` not `:169-296`; `COLL_LABEL_*` is `:247-252` not `:248-252`;
   *Header and Footer* is `:913-940` not `:914-937`; *Caption*'s block is `:971-984` not `:971-982`;
   and **`DocumentStylePoolManager.cxx`:964 is a different case's `PT_10`** — `COLL_LABEL`'s is
   `:978`.

One more thing was wrong in the *code* rather than in the write-up: `IsNumbered` compared the whole
prefix with `OrdinalIgnoreCase`, so `HEADING 1` and `TOc 1` would have answered *Standard* for names
Writer has no pool style for. It now compares the two exact spellings the map holds.

## 7. Validation

* `dotnet build XRay.Office.slnx` — **0 warnings, 0 errors**, from a clean tree (`bin`/`obj` had been
  deleted to recover disk after the restart, so this is a full rebuild and not an incremental one).
* The ten non-fidelity projects, run one at a time: Containers, Core 521, Markup 259, OpenDocument
  146, Presentations 1044, Rendering 164, Spreadsheets 1264, Text 728, Vector 309, WordProcessing
  1913 — **all `Passed!`, 0 failed, 0 skipped**.
* **`XRay.Office.Fidelity.Tests`: `Failed: 10, Passed: 542, Skipped: 0, Total: 552`**, and the ten are
  exactly the known names — `PageDrawingComparisonTests.EveryLineIsDrawnWhereLibreOfficeDrawsIt`
  ×4 (`paginated.rtf/.fodt/.doc/.docx`),
  `TabStopComparisonTests.AListLabelsTabAdvancesToLibreOfficesStop` ×4
  (`list-label-overrun.docx/.odt/.fodt/.doc`),
  `SheetDrawingComparisonTests.APictureIsDrawnWhereLibreOfficeDrawsIt(sheet-rich-text.xlsx)` and
  `JustificationShrinkComparisonTests.TheParagraphBreaksWhereLibreOfficeBreaksIt(justify-shrink-2013.docx)`.
  No eleventh.
* **The tests fail at the base.** `RtfStyles.cs` reverted to `61bc19e03` and the test project
  rebuilt: `RtfStyleFormattingTests` is **24 failed / 28 passed of 52**, and at HEAD **52 of 52**.
  The 24 are every case that asserts a pool parent's new contribution; the 28 that pass at both are
  r95's own five, the controls, `AnIntermediateBelowTextBodyIsNotModelledYet` and
  `ABareHeadingIsStandardRatherThanTheHeadingPool` — that last one *should* pass at the base, since
  it asserts that a bare `Heading` does **not** get the heading constants, which was already true;
  what pins the change for that name is the two `APoolStyleUnderStandardTakesTheDocumentsOwnNormal`
  size cases, and both are in the 24.
* Corpus reach, `.rtf` column of `/home/user/corpus-odf/words`: **338 rendered on each leg, 4
  differ, 3 of the 4 change a pixel, all four are level or better against the reference** (§4).
  The HEAD leg was rendered **twice** — once before the `IsNumbered` correction and the comment
  edits and once after — and the two are identical document for document, so the figure belongs to
  the code that is committed.

**One test run in this round was wrong and is reported rather than hidden.** The first
`XRay.Office.WordProcessing.Tests` run after the rebuild reported `Passed: 1389` and a
`[FATAL ERROR] … exit code 137` — an OOM kill under contention, with three other rounds live and
4 GB free. Re-run alone it is 1913, matching the run before it. That is `CLAUDE.md`'s *a truncated
run reports success* arriving as a truncated run that at least admitted it.
