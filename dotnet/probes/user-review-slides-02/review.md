# User review of the slides track — 30 decks, verbatim

Taken against the review sheet built at commit `445f253bd0f` (slides 144/163 under the gate's word
check, **163 of 163 page-exact**), one page per deck — the page with the highest `|ink|%` over the
common prefix. Every deck below is page-exact, so **not one of these observations is visible to any
gate column**.

The project's standing rule applies: **the user's visual reports are primary evidence.** Two of
their earlier observations outperformed every metric here — "some cells are taller" identified the
real axis of a 14-document cluster *after* a column-width hypothesis had been formally refuted, and
their page-split calls turned a 3-document lead into 14. Where a brief has contradicted one of
these, the brief has been wrong.

Recorded verbatim, then classified. **The classification is mine and is the part to distrust**; the
observations are the evidence.

---

## Verbatim

**8_P-Pavese_AIRBUS-ATB-journee-CRATB** — Chart missing background. Chart fonts are wrong. Chart
vertical axis is off 7000% where it should have been 100%. Text overlay on top of chart is missing
or wrong color or drawn behind.

**RRM-training-syllabus-Chapter-3-Teamwork-TC-Towing-Incident-Analysis-Dec-2009** — Font is
different, everything else seems fine.

**N2_E_Maestroni_Swarm_COP** — Vertical axis order is inverted. Text rendering for the vertical axis
is also very different. LibreOffice doesn't render all vertical axis labels.

**Ramp Up Campaign - French** — Wrong font, maybe not using correctly embedded characters or fonts?

**Thailand17** — Minor spacing difference above the word "evaluation". Header missing shadow?

**iep-amount-frequency-for-webinar** — Minor differences in text placement and spacing.

**1-secretariat** — Image rendering without transparent background — maybe using the PowerPoint
transparent color feature? Second line on header has wrong placement (maybe tabs spacing used to
place it).

**16 - UTM - (NASA)** — Title missing underline. Text is less dense vertically on X-Ray.Office.

**architecture6** — Table cell sizes are different.

**Demick_JetBlue** — Missing a curve/line in the top, chart sizes and axis labels not matching.

**W3_Case_Study_of_a_Tsunami_Warning_Simulation_Exercise_Ed** — Text sizes are different.

**ITE106-Chapter 4** — Text sizes are different.

**solog_orientation_august_2019** — Text sizes are different.

**Aerospace_Journey_of_Flight_Chapter_BCB1637572DA6** — Text sizes are different, title missing
shadow.

**pres_ioc_phuket** — Missing transparent color handling for many images.

**manufacturing_process_simulation_working_group_overview_2023** — Very different font sizes.

**010605Vul** — Text sizes are different.

**Framing Europe** — Text sizes are different.

**Stakeholders-v08052017 - v5** — Font size or spacing different, link missing underline.

**ws_prod-g-doc-Events-industrymeeting18112004-European-Safety-Strategy-Initiative** — Shapes are
not matching and drawn in different order. Custom bullets missing.

**2014BSA_Sunday_Killion** — Text sizes are different.

**7-Zulkefli_Part147n66_IKMAS** — Text sizes are different.

**WiGr_2021W_1_Angebot-Nachfrage-Elastizität-211017-171222** — Row in the top height is bigger.

**southern-classic-kennesaw-state-university-final** — Charts rendering different, text over bars
wrong color.

**Fundamentals_Module_1_basics** — Text sizes are different.

**OnTrac_StarCertificationProgram-3Day** — Image crop off by 1 pixel.

**NWD-GLA-Community-Outreach-Day-Oct-2025** — Text missing or in wrong color.

**Sylva introduction session** — Text rendered in the wrong direction.

**FAAAIandtheArtandScienceofV&Vfinal** — Minor vertical placement difference for text.

---

## Classification

### 1. Text metrics on the slide text path — 17 of 30 decks

**This is the largest single finding on the track and possibly on the project.**

Bare *"text sizes are different"*: `W3_Case_Study`, `ITE106-Chapter 4`, `solog_orientation`,
`Aerospace_Journey`, `010605Vul`, `Framing Europe`, `2014BSA_Sunday_Killion`, `7-Zulkefli`,
`Fundamentals_Module_1_basics` — **nine decks, identical wording**.

The same class in other words: `manufacturing_process` ("very different font sizes"),
`Stakeholders` ("font size or spacing different"), `RRM-training` ("font is different, everything
else seems fine" — note *everything else seems fine*, which makes it a clean single-variable case),
`Ramp Up Campaign` ("wrong font, maybe not using correctly embedded characters"), `16 - UTM`
("text is **less dense vertically**" — a leading rather than a size reading),
`iep-amount-frequency` and `Thailand17` and `FAAAI` (spacing/vertical placement), and `WiGr`
("row in the top height is bigger" — a text-height consequence in a table).

**What makes this urgent rather than merely large**: every one of these decks is **page-exact and
passes or nearly passes the word gate**, so no gate column can see it, and it has therefore been
invisible to every round this project has run. It also sits directly on top of a fact established
independently this week — `dotnet/probes/fidelity-01/results.md` measured a real **~0.1% advance
divergence** with tab stops exact to 0.0000 pt and **LibreOffice kerning 19% harder** — and on the
removal of the false claim that "advance widths agree by construction".

**Do not assume the fidelity finding is the same defect.** A 0.1% advance drift is not what a
reviewer calls "text sizes are different" at a glance; that reads like a whole-point or
percentage-scale error. Both may be real and separate.

### 2. Charts — 4 decks, at least four distinct defects

- `8_P-Pavese_AIRBUS` — **"vertical axis is off 7000% where it should have been 100%"**. A
  percentage axis rendered as a raw ratio ×100 twice over, or a `c:dispUnits`/percent-format miss.
  This is the most concrete numeric claim in the whole review and the deck is the worst page on the
  sheet (53% `|ink|`, signed −44.8%). Also: missing chart background, wrong chart fonts, and a text
  overlay missing / wrong colour / drawn behind.
- `N2_E_Maestroni_Swarm_COP` — **vertical axis order inverted**. Concrete and testable.
  Note the user also says LibreOffice fails to render all its axis labels — i.e. **we are better
  there**; do not "fix" toward the reference on that half.
- `Demick_JetBlue` — a missing curve/line at the top, chart sizes and axis labels not matching.
  The *colour* half of this deck was fixed in `slides-e-01`; this is the remainder.
- `southern-classic-kennesaw-state-university-final` — charts rendering different, **text over bars
  in the wrong colour**.

### 3. Image transparency — 2 decks

`1-secretariat` ("image rendering without transparent background — maybe using the PowerPoint
transparent color feature") and `pres_ioc_phuket` ("missing transparent color handling for many
images"). PowerPoint's *Set Transparent Color* is `a:clrChange` in DrawingML and a
`pictureTransparent` property in Escher; the user naming the feature is a strong pointer.

### 4. Two-deck clusters worth one round together

- **Missing shadow**: `Thailand17` (header), `Aerospace_Journey` (title).
- **Missing underline**: `16 - UTM` (title), `Stakeholders` (link).

### 5. Singletons

`architecture6` table cell sizes; `ws_prod-g-doc` shape order and missing custom bullets;
`Sylva introduction session` **text rendered in the wrong direction** (vertical/rotated text);
`NWD-GLA` text missing or wrong colour — note a previous round measured this deck as a genuine
**109-word loss** against a 0.99 pt subtitle placeholder we emit no text record for, which is
plausibly the same thing; `OnTrac` image crop off by one pixel — this deck's alpha and position
were fixed in `slides-c-01`, so a one-pixel crop residue is a good sign rather than a bad one;
`1-secretariat` second header line placement (the user suggests tab spacing).
