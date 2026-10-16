# Round 160's three failed reproductions, kept so they are not tried again

Chasing O110 (see `probes/tocstyle-r160/results.md`) the question was why 26.2.4.2 ignores
`<w:spacing w:after="0"/>` on one `Heading3` paragraph of `24-25_FAA_Holdover_Tables.docx`. Three
hypotheses were built as one-attribute probes and **all three are refuted**: in every arm below the
direct spacing survives, read out of 26.2.4.2's own `--convert-to fodt`.

- **`make-probe.py` — is it the shape of the `w:spacing`, or the style?** Eleven arms: `after=0`,
  `after=0` with `before=0`, `after=40`, `after=0` with a `w:line`, `w:contextualSpacing`, a
  built-in `heading 3`, a custom style with the same properties, and a built-in `heading 4`. All
  eleven keep what they state.
- **`make-break.py` — is it the deferred page break?** The witness's caption is preceded by an
  otherwise-empty paragraph whose only content is `<w:br w:type="page"/>`, and the reference's
  resolved caption carries `fo:break-before="page"`. Eight arms, including that exact shape, a
  break at the end of a paragraph with text, and `w:pageBreakBefore` stated directly. All keep
  their spacing.
- **`make-table.py` — is it the table in front of the break paragraph?** Six arms putting a
  `w:tbl`, an empty paragraph and a break paragraph in front of the heading in every combination.
  All keep their spacing.

What the three of them did establish is that the trigger is **not local to the paragraph**, which
is what sent the next step at the body rather than at the markup around the caption — and the body
bisect found a `TOC` field 1040 children earlier.
