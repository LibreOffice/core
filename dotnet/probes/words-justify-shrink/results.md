# Word 2013's justification shrink: 26.2.4.2 weighs it, 24.2.7.2 took the maximum

Measured 2026-09-06, worktree `wt-frames` at `531e9a1f3`, against
`/opt/libreoffice26.2/program/soffice` (**26.2.4.2**, TDF tarball with its Latin font duplicates moved
aside) and `/usr/bin/soffice` (**24.2.7.2**). Corpus documents
`dotnet/tests/corpus/features/justify-shrink-{2013,2007}.docx` — one justified paragraph twice,
differing only in the `compatibilityMode` their `settings.xml` declares, 15 against 12.

## It is not the advance divergence, and the control says so first

`justify-shrink-2007.docx` — the same text, the same measure, shrinking off — **breaks identically in
both engines, all five lines, at every width tried.** So the break positions on this text are not
being moved by advance drift. The `justify-shrink-2013.docx` failure was ours setting it in **4**
lines against the reference's 5, and the first divergence was a whole word: line 2 took `road` where
the reference stopped at `long`. Eighteen points of word is two orders of magnitude outside a 0.1%
drift over a 482 pt line.

## The rule

Both engines shrink line 1 and both refuse to shrink line 2, and the difference between those two
lines is not how far the blanks would have to be squeezed — line 2's squeeze is the *shallower* of the
two. It is what the alternative costs. `sw/source/core/text/portxt.cxx`:769-805 compares two ratios of
the natural blank: how far the longer line squeezes (`z0 = 1/squeeze`), and how far the shorter line
stretches, discounted by `fExpansionWeight = 1/1.7` (`z1`). The longer line wins when `z1 >= z0`.

Measured on 26.2.4.2 by sweeping the corpus paragraph's text width and reading the first line's
decision off the mode-15 and the mode-12 rendering of each width — the mode-12 one being the un-shrunk
break, since shrinking is off there:

| text width, tw | stretch | squeeze | `z1 × squeeze` | 26.2.4.2 |
|---:|---:|---:|---:|---|
| 9638 | 1.552 | 0.839 | 1.112 | shrank |
| 9138 | 1.531 | 0.920 | 1.207 | shrank |
| 8738 | 2.295 | 0.991 | 1.746 | shrank |
| 8638 | 2.156 | 0.858 | 1.441 | shrank |
| 9038 | 1.391 | 0.795 | 0.978 | **did not** |
| the fixture's line 2 | 1.392 | 0.800 | 0.985 | **did not** |

Six for six. The other seven widths in the sweep are settled by the 75% floor alone — the next word
needs the blanks below 75% — so they say nothing either way.

The rule carries a second clause, *shrink when not shrinking would stretch the blanks past the maximum
word spacing*. **It did not discriminate on any measured line**, and the tree's value for that maximum
in this mode is 100%, which would shrink all six rows above and contradict two of them. The tree is
27.2.0.0.alpha0+ and the reference is 26.2.4.2; the clause is deliberately not implemented and the
region where it could differ — a stretch just over 1.5 with a squeeze between 0.75 and 0.78 — is
recorded here as unmeasured.

## After the port: 13 of 14 widths exact, and the fixture sits on the fourteenth

With the weighted rule in place, the corpus pair at three text widths, every line compared:

| right margin, tw | text width, tw | mode 15 | mode 12 |
|---:|---:|---|---|
| 1133 | 9639 | **all 5 lines match** | all 5 match |
| 1134 | 9638 | line 3 differs | all 5 match |
| 1135 | 9637 | **all 5 lines match** | all 5 match |

The fixture's own width is 9638, and 9638 is the one width in that neighbourhood where the reference
answers differently from *itself*. Isolating that line into its own paragraph and sweeping the width a
twip at a time, 26.2.4.2 takes the word `line` at 9631, 9632, 9633, 9634, 9635, 9636, 9637, 9639,
9640, 9641, 9642, 9643 and 9644 — and refuses it at **9638 alone**. We take it at all fourteen.

9638 twips is exactly the natural advance of that seventeen-word line as the reference itself measures
it: setting the string left-aligned on a page wide enough not to wrap, with a half-size run behind it
so the pen after the string is its own text record, gives **481.9000 pt = 9638 tw for 26.2.4.2 and
481.9400 pt for us**, against a room of 9638 tw. So the fixture's measure lands exactly on the width
at which the line's natural width equals its room, which is a tie, and at that tie the reference
contradicts its own behaviour one twip either side of it. No rule reproduces that, and 0.8 twips over
9638 — 0.008% — is what puts us on the other side of it.

**So the line-count failure was the rule and is closed; the residual is a tie in the reference plus
sub-twip advance, and it belongs with the advance divergence rather than here.** The fixture was left
where it is: moving its page width one twip would turn the test green and would stop it measuring the
thing it is for.

## Scripts

* `sweep.py` — the text-width sweep that gives the six decided rows.
* `boundary.py` — the twip-by-twip sweep that isolates the 9638 singleton.
* `advance-probe.py` — the exact natural advance of a candidate line in both engines.
* `pair-width.py` — the corpus pair at several widths, every line compared.
* `gaps.py` — per-line word count and mean word gap from a PDF.

## Reach

Thirty-nine corpus documents are DOCX with `compatibilityMode` 15 or more *and* justified text —
`reach-set.txt` is the census. Rendered before and after the change and compared against 26.2.4.2:

| | |
|---|---:|
| documents whose laid-out text moved | **18 of 39** |
| page count or word count moved | **0 of 39** |
| line breaks agreeing with 26.2.4.2: better / worse / level | **14 / 1 / 3** |

So the change is invisible to the gate's own columns and visible in the line breaking, which is what
it is about. The largest movements, agreeing line breaks against 26.2.4.2, before → after:

```
4400-91_Proposal_To_Lease_Space_10-2024.docx        251 -> 346  of 346   (every line)
Company-profile-2022-EN.docx                         74 -> 131  of 162
AUWG min 4-3-24 final.docx                           40 ->  49  of  50
slcc-architecture-uu-architecture.docx              145 -> 153  of 153   (every line)
1603642410-MoM-CASCOM-06-2020-draft04.docx          544 -> 560  of 564
```

The single document that moved the wrong way is `FAA 2025-26 Holdover Tables.docx`, 857 → 855 of
8542 — two lines in eight and a half thousand, and it is one of the two Holdover Tables, which are
carried as a known pagination pair.
