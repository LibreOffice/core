---
name: page-vision
description: Use your own vision on a rendered page — how to get a page in front of a reader at a resolution where the defect survives, how to delegate the reading to an uncontaminated reviewer, and what a visual reading can and cannot establish. Use when looking at a failing page, reviewing a rendering against the LibreOffice reference, or when a metric says a page is wrong and you need to know *what* is wrong. Covers the viewer's pixel budget, when to crop, pairing two renderings into one reviewable image, and the contamination traps that make a fluent reading worthless.
---

# Looking at a page

`render-comparison` says *look at the page* and is right. This skill is the mechanics of
actually doing it: getting the pixels in front of a reader at a size where the thing you are
hunting is still visible, and keeping the reading honest enough to act on.

Three separate failures this addresses, all of which produce a confident and useless report:

- the page arrives too small to show the defect, and the reviewer reports the resolution;
- the reviewer already knows what is supposed to be wrong, and finds it;
- the reading is treated as a diagnosis rather than as an observation.

## Delegate the reading. It is the only real control.

> **A reader IS available in this container, and the six rounds that reported otherwise were
> looking at the wrong tool.** Measured 2026-09-27: the in-process **`Agent`** tool with
> `subagent_type: "general-purpose"` spawns a subagent in *this* container. It opens
> `/home/user/...` and reads the PNG, and its report comes back as a hand-back message. Six
> readings were delegated that way in one session and all six returned usable descriptions —
> one of them caught a dropped dot leader that the numbers did not show, and four independently
> named a one-page offset that turned out to be the round's whole finding.
>
> What is genuinely unavailable is the **sibling session**,
> `mcp__Claude_Code_Remote__create_session`: that runs in its own container, cannot see your
> PNG, and has no channel back. The earlier rounds checked for that and for a tool called
> `Task`, found neither, and concluded the section was dead. **It is `Agent`, and it works.**
>
> So: compose the pair, then send the path with the brief below. Launch several at once — they
> are independent, and a class that shows up in three unrelated readings is worth far more than
> one that shows up in three pages a single reader looked at in sequence.
>
> **If `Agent` is genuinely absent in some future container**, then and only then:
> say so in your write-up, treat every
> reading as your own and contaminated, and corroborate anything you lean on with arithmetic that
> does not depend on the reading — content-stream operators, drawing dumps, the gate columns, the
> reference's own flat ODF. The round that found the Calc drawing clip did exactly that: the
> composed pair gave direction and kind and could not decide between *displaced*, *rescaled* and
> *clipped*; an authored probe's clip rectangle settled it. **A contaminated reading that names
> candidate causes and then defers to an instrument is worth a great deal.** What it is not worth
> is being quoted as though a blind reader had confirmed it.
>
> The parent session driving those rounds *does* have the tool, so everything below applies to it
> — and delegating one page costs it very little.

**Send the page to a fresh subagent and let it describe the page before you look at it
yourself.** Everything else in this file is secondary to that.

The reason is measured, not stylistic. While calibrating this skill I read one page at 150
dpi, then at 72, then at 36. The 36 dpi read came out fluent and detailed — and it was
worthless, because by then I was reciting the page rather than reading it. **You cannot
un-see a page.** Any second look at a page you have already seen is recall wearing the
costume of observation, and it will agree with whatever you already believed.

A subagent that has never seen the document, has not read the round's brief, and is
forbidden from grepping the repo is the only reader in the loop whose agreement means
anything.

**In this container a round cannot do it — only the parent session can.** Two rounds reported,
independently, that a subagent has no subagent tool here: it can be given an image but it cannot
spawn the reader. So the delegation this section calls the only real control is not available to
the agent that most needs it. The working arrangement is therefore: **a round names the page and
says which of its conclusions rest on its own reading; the parent spawns the reviewer.** Do not
let a round spend time rediscovering this.

**A worked example, because the payoff is easy to disbelieve.** `Thailand17` page 11 had been
worked by three rounds, and the last of them had closed its character-size half exactly the same
day — 548 alphanumerics at 11.99 pt against the reference's 548. A reviewer given the composed
pair and nothing else, not told what to look for, reported in its first paragraph that our table's
strokes were two to three times heavier than the reference's. Measured out of the PDFs afterwards:
**1.0 and 2.25 pt against 0.40 and 0.95**, same item counts, both black. Nobody had measured
stroke width because nobody had a reason to; the rounds were counting characters and row heights.

The same reading also *declined* to see the thing it was sent for — it put the row-height
difference at "under 2 % of the table's height, spread over nine rows" and called it
"indistinguishable rather than a difference". That was the correct answer, and it is worth as much
as the finding: it says the seat behind that page is real but sub-visual, which is a fact about
how to spend the next round.

```bash
export XRAY_OFFICE_CLI=<the tree you mean to measure>/dotnet/tools/…/XRay.Office.Cli
export REF_SOFFICE=/opt/libreoffice26.2/program/soffice   # PATH is 24.2.7.2; they disagree
.claude/skills/page-vision/scripts/pair.sh "<doc>__xlsx" --worst --outdir /abs/pairs
```

That prints one path. Hand the path to a subagent with a brief that:

- names which half is which **only as a fallback** — the image labels itself, see below;
- **forbids** reading project documentation, source, `results.md`, or running any command.
  A reviewer that greps for the answer is no longer a control;
- asks for the two halves to be described **separately, before** they are compared;
- demands **direction** — "the reference breaks lines earlier", not "line breaks differ";
- asks explicitly for what looks *identical*, which is what rules causes out;
- asks for the candidate causes the image **cannot decide between**, and what measurement
  would separate them.

Run several in parallel, one page each. They are cheap and independent, and a class that
shows up in three unrelated readings is worth more than one that shows up in three pages a
single reader looked at in sequence.

### Do not brief a reviewer with the numbers

Telling a reviewer "this page has 945 words against the reference's 431" buys a report about
word counts. The gate numbers are what you are trying to *explain*; feeding them in gets you
a story that fits them. Give the reviewer the image and nothing else, and compare their
reading against the numbers afterwards — when a blind reading and an independent measurement
land on the same mechanism, that agreement is evidence. When you supplied the answer, it is not.

## The viewer has a fixed pixel budget: about 2000 px on the long edge

This is the mechanical fact everything else follows from, and it is measured. A page rendered
at 600 dpi — 4961×7016 — came back annotated:

```
[Image: original 4961x7016, displayed at 1414x2000. Multiply coordinates by 3.51 …]
```

So an image larger than the budget is **downscaled before anyone sees it**, and the viewer
says so when it happens. Two consequences:

- **For a full A4 portrait page, anything above ~170 dpi is thrown away.** 600 dpi costs 40×
  the pixels of 150 dpi and shows you exactly what 150 dpi shows you.
- **If you see that "displayed at" line, you wasted the render.** It is a receipt for pixels
  you paid for and did not receive.

Landscape 16:9 slides have a shorter long edge relative to their content, so the same budget
buys them ~150 dpi.

## What resolution do you actually need? Work in pixels per em

The variable that decides legibility is not dpi, it is **how many pixels tall one em is**:

```
px_per_em = point_size × dpi / 72
```

Measured on a real 10.00 pt table (blind reads, scored token-for-token against `pdftotext`):

| dpi | px/em | page pixels | blind read against ground truth |
|---:|---:|---|---|
| 150 | 20.8 | 1241×1754 | **221/221 tokens**, 3 differences, all line-join artefacts |
| 72 | 10.0 | 596×842 | fully legible |
| 60 | 8.3 | 497×702 | **249/249 tokens, 0 missed, 0 invented** — on a page never seen before |
| 36 | 5.0 | 298×421 | structure only; and see the contamination warning above |

**About 8 px per em reads reliably.** So the dpi you need is `576 / point_size` — 58 dpi for
10 pt text, 96 dpi for 6 pt.

Put that beside the 2000 px budget and the useful conclusion is: **a full page at the budget
resolves text far smaller than any document actually uses.** Legibility is therefore almost
never the reason to crop, and "I could not read it" almost always means the image was built
wrongly rather than that the page needed more pixels.

## A thin rule cannot be judged from a full page — and a false *presence* is the dangerous half

The section above says to confirm a reviewer's "it is absent" against the PDF's own operators
before believing it. The mirror case is worse and cost a reading here: **a reviewer will report a
thin mark as *present* when it is not there at all**, and that closes an investigation instead of
opening one.

Measured, on the same block, by two fresh readers of the same quality:

| | mark | reading |
|---|---|---|
| full page, side by side, **117 dpi** (the budget's maximum for this page) | a 0.51 pt underline = **0.83 px** | *"the blue underline … is present in both"* — **it is in neither half of ours** |
| **600 dpi** crop of that block | the same underline = **4.25 px** | *"nothing in the top half and one solid underline in the bottom half … roughly 4–5 px thick"* — exactly right, with the right candidate causes named |

The arithmetic is the px-per-em rule applied to a rule rather than a glyph. About **3 px** is
needed to call a hairline present or absent, so

    dpi ≈ 216 / width_in_points

and a full page is capped at 117–170 dpi by the 2000 px budget. **Any rule thinner than roughly
1.3 pt is therefore unjudgeable from a full-page pair.** Crop the block, or do not put the
question to a reviewer at all — a fluent "present in both" is indistinguishable from a real one.

Note the 117 dpi reader *did* warn that "all hairlines are one pixel at this scale" and that a
small missing mark "would need a crop at several times this resolution". A reviewer that states
its own resolution limit is telling you its presence claims are void; read that sentence before
the findings.

## What cropping *is* for: ink finer than a glyph

Crop when the question is about geometry below the level of a letter:

- hairline width; **doubled or overlapping strokes** where table borders meet;
- glyph shape and ligatures;
- antialiasing, colour at edges, gradient banding;
- whether two nearly-equal lengths are actually equal.

Measured: at 150 dpi the word "Aircraft" is simply the word "Aircraft". In a 600 dpi crop it
is visibly set with an **`ft` ligature**, and the cell separators resolve into distinct
hairlines. Neither fact is recoverable from the full page at any dpi, because the full page
cannot exceed the budget.

`pdftoppm` crops in device pixels at the dpi you ask for, which is exactly what you want:

```bash
pdftoppm -r 600 -f 5 -l 5 -png -x 600 -y 780 -W 1900 -H 700 ref.pdf crop
```

Keep `-W`/`-H` inside the budget or the crop is downscaled too and you are back where you
started.

## Pairing two renderings

**By default, do not composite at all.** Two separate images each get their own 2000 px
budget; one composite splits a single budget between both halves. Reading them as two images
is strictly higher resolution.

Composite when the question is **alignment** — is this block displaced, does this column
start in the same place, has the whole page drifted — because that is the one question two
separate images answer badly.

```bash
.claude/skills/page-vision/scripts/compose.py ours.png ref.png -o pair.png
```

`compose.py` does three things worth knowing:

- **It picks the arrangement that costs the least resolution.** Two portrait pages side by
  side have a long edge of 2×595 pt; stacked, 2×842. So side-by-side is the higher-resolution
  arrangement for portrait pages, and stacking is higher for 16:9 slides. `--layout auto`
  computes this. Note this cuts against the older advice to always stack — stacking is right
  when you need the same region to land under itself, and it costs resolution to get that.
- **It labels the halves inside the image** — blue band "OURS (PAPERLESS)" on the left/top,
  orange "REFERENCE (LIBREOFFICE)" on the right/bottom. A reviewer told which side is which
  only in the prompt will occasionally report them the wrong way round, and a swapped reading
  inverts the sign of every conclusion in it. The label travels with the pixels.
- **It warns when the two halves differ in size.** Two halves rendered at different dpi
  compose into an image whose most striking feature is the scale difference, and the reviewer
  duly reports that our text is tiny. That defect lives in the compositor. If both halves
  *were* rendered at the same dpi, then the page geometry genuinely differs and that is the
  finding — which is why this warns rather than refuses.

It reduces by taking the **darkest pixel of each block**, and the reason is a scar.

It used to reduce with nearest-neighbour, with a comment claiming that preserved a
one-pixel hairline. It does not: nearest *samples* one source pixel per destination pixel,
so a rule thinner than the sampling step falls between samples and disappears. Composing at
80% silently dropped a real underline — a 245 px solid black run at y=802 in the 150 dpi
render, present as a fill in the PDF itself — and two independent reviewers then correctly
reported that the composed image had no underline in it. That was relayed onward as a
renderer defect. It was a compositor defect.

Averaging is not the fix either: it turns a doubled hairline — a defect this project *is*
hunting — into one grey smudge indistinguishable from a single line. Darkest-of-block keeps
any hairline at full strength, keeps two of them distinguishable while a pale row separates
them, and never invents ink. It biases towards showing marks, which is the right bias when
the question is "is this drawn or not".

**The general rule, which this file should have taken from `render-comparison` and did
not:** before believing that a reviewer's "it is absent" is a fact about the document, check
it is not a fact about the pipeline that built their image. Confirm absence in the PDF's own
operators — `pdf-ops.py dump`, or a thin-wide-fill scan — not in a downscaled raster. A
missing mark and a mark your instrument threw away look identical to the reader.

**`look.py` renders the reference itself when the bank has no copy, so `REF_SOFFICE` decides
which binary you are reading against.** It hard-coded PATH — 24.2.7.2 — until round 180, while
the tree is calibrated to 26.2.4.2, and the two genuinely paginate real documents differently.
It now prints the binary it resolved; read that line before reading the page.

`pair.sh` chains `look.py` and `compose.py` so the two halves cannot be rendered at
mismatched dpi by hand. About 20 s per document.

## What a reading establishes, and what it does not

**Reliable from an image:** presence and absence; position and displacement; relative size;
colour; the *direction* of a difference; the *kind* of element involved.

**Not reliable — measure it:** exact values; counts of many similar items; whether two nearly
equal lengths are equal; font identity; and anything that requires knowing what *should* be
there.

### Independent readers agreeing does not make a detail true

Corroboration across uncontaminated readers is the strongest signal this method produces, and it
is still only evidence about *direction and kind*, never about a detail one of them measured by
eye.

Measured: five reviewers read five unrelated `.odp` pages blind. Four independently reported that
the body text "wraps at the same words in both halves" and used it to argue that text measurement
was correct and only paragraph spacing was wrong. **The wrap claim was false** on at least one of
the pages — the reference breaks a hyperlink at any character, because a `text:a` in slide text is
an EditEngine field, and fits a URL segment onto a line we end early. Their *conclusion* was right;
their stated reason was not.

So: take the class from the agreement, then measure the detail the argument rests on. Four readers
agreeing that a thing looks identical is a good reason to go and check whether it is, and a bad
reason to record that it is.

**Never available from an image: cause.** An image cannot separate a picture bullet from a
character bullet in a substituted symbol font from an autonumber. The discipline is to *name
the candidates the image cannot decide between* and then go and measure. A reading that
promotes itself into a diagnosis is worse than no reading, because it is cited later as though
it were one.

### Readers who DISAGREE are doing the job — and a reader who bounds itself is worth more than one that does not

Two results from a ten-reader run, both worth more than any single reading in it.

**Disagreement is a finding, not noise.** Two readers, on two different dense spreadsheet
pages, both reported a text-weight difference — and reported it in **opposite directions**:
one said ours was darker, the other said the reference was. Three readers independently named
*downsampling in the compositor* as a candidate cause. A property that two uncontaminated
readers describe with opposite signs is almost certainly in the instrument, not in either
renderer. Had one reader seen both pages, it would have reported a consistent story and been
believed. Run them separately and let them contradict each other; that contradiction is the
control paying out.

**Ask every reader to state its own resolution limit, and to say which of its claims that
voids.** Every reader in that run did, unprompted beyond the instruction, and it changed what
was usable:

- one computed ~115 dpi and voided its own "no header rule / no footer rule / no image border"
  claims as unresolvable, *and then argued past the limit for the one claim that survived*: the
  reference half plainly renders a one-pixel header rule, so the rasterization **can** show
  hairlines there, so ~43 missing table-of-contents underlines are a real absence and not a
  thin-line dropout. That reasoning is what made it the strongest finding of the ten;
- one refused to state a direction at all on row heights, because two of its own measurements
  contradicted each other and both sat inside its error bar. "No direction established" is a
  better answer than a coin flip dressed as an observation;
- one noted its corner estimates carried ±5-8 px because the shape's gradient faded toward the
  background at exactly the corners it needed, and insisted the ratio be recomputed from the
  operators before anyone used it. It was right to: the stored adjustment settled the geometry
  exactly, and the eyeballed ratio would not have.

So put both in the brief: **state your resolution limit, and say explicitly which of your own
claims it voids.** A reader that marks half its observations unreliable has made the other half
usable.

### `pdftotext -bbox` reports an ink box, not a baseline

Its `yMin` is derived from the **font descriptor**, not from the text-positioning operator. So two
renderings whose baselines are identical to the hundredth of a point can report a constant `yMin`
offset, and it looks exactly like a vertical layout defect.

Measured: every Caladea heading in `exhibit-06---technical-architecture-template.docx` showed a
flat **2.1 pt** offset against the reference — which is precisely `usWinAscent − sTypoAscent` for
that face. The `Td` baselines were the same on both sides. A round nearly filed a font-metric bug
on it.

**When the question is vertical position, read the baseline out of the content stream.** Use
`-bbox` for *which* words are on a page and roughly where; use the operators for where a line
actually sits.

### Ink and the text layer are different things, and may legitimately disagree

You read ink. `pdftotext` reads the text layer. Measured on the calibration page: the rendered
line ends `L410 UVP-E-` and continues `LW`, with a visible hyphen — and the text layer contains
no hyphen at all, joining it as `UVP-ELW`. Neither is wrong.

So when your reading and `pdftotext` disagree, that is not automatically a misreading. It can
be a finding *about the text layer* — which is the half of the output the word-count gate
actually scores.

## Order of work

1. **Render both sides at the same dpi.** `pair.sh` if you want one image, `look.py` if two.
2. **Get it read by someone who has not seen it** — a subagent, in parallel across pages.
3. **Collect direction and kind**, plus what looked identical.
4. **Name the candidate causes** the image could not decide between.
5. **Only now consult the record** — `results.md`, the TODOs, the source. Reading blind first
   and looking up second is a control on the reading, and it works: a gradient description
   produced that way matched a diagnosis made a week earlier from source with no chance of
   having been led to it.
6. **Measure the named cause.** The reading told you where to point the instrument; it is not
   the instrument.

## Look at pages that PASS

Inherited from `render-comparison` and repeated because it is the highest-yield habit here:
the failing set is picked over, and the gate — page count, words within max(2%, 3), unembedded
fonts — is blind to most real defects. A track can be 163 of 163 page-exact while three
passing pages opened at random yield three findings. Rank the *passing* documents by `|ink|%`
and send the worst to a reviewer.
