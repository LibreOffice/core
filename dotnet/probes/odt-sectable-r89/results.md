# `odt-sectable-r89` — CUT OFF, scripts only

***Settled by `probes/odt-sectable-r92`.*** The nesting this round's filenames pointed at is the
rule, and `xhist.py`'s own docstring — a baseline crosses a column, so a histogram that merges
every span of one baseline reads a two-column page as one column — is what refutes the two claims
it was sent to build on. Both of them are withdrawn there; read that file rather than this one.

**This round did not finish and wrote down no conclusions.** Its agent was terminated by a rate
limit partway through, immediately after saying it was about to *"implement the nested-section
rule"* — so it had reached a rule by that name, and what the rule says is not recorded anywhere.
The scripts below are banked so the next attempt knows what was already built and does not spend
its first hour rebuilding it. **Treat every one of them as unvalidated**: none of their output was
checked into evidence, and no measurement from this round has been reproduced.

## What it was asked to settle

Two seats left by `odt-startx-r88` (merged at `a83359a68`):

1. **A table inside a columned `text:section` is laid out against the page rather than against its
   column** — measured there at 3 of the 33 documents stating a columned section.
2. **`absrc-pac-01-info-note-en`, the one verdict that round cost.** 26.2.4.2 draws its section in
   one column although the file states two. Established by r88: six one-attribute variants leave
   the rendering identical; a probe carrying the document's *exact* section style **is** drawn in
   two columns at the positions this tree computes; and replacing the section's table *and* its
   table-of-contents with paragraphs restores the two columns. So the arithmetic is right and the
   condition is about the section's content.

## The scripts, and what each was evidently for

| script | apparent purpose |
|---|---|
| `absrc-content.py` | reduce `absrc-pac-01-info-note-en`'s section content toward the condition |
| `content-variants.py` | generate content variants of a columned section |
| `nested-variants.py` | generate variants around nesting — the "nested-section rule" it named |
| `census-nested.py` | count nested sections across the converted `.odt` |
| `pairs.py`, `xhist.py` | pair renderings and histogram line-start differences |

The word *nested* appearing in two of the five is the strongest surviving hint: the condition it
found is more likely about a section inside another section than about the table or the
table-of-contents that r88 had identified as the suspects. That is an inference from filenames and
nothing more — verify it before building on it.

## Where to start instead

Re-derive from r88's three established facts above rather than from anything here. They were
measured and written down; this round's were not.
