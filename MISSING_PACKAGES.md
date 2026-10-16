# Packages this environment needs and did not have

Install these when provisioning a container for the Paperless parity work. Each one below was
found missing by measurement, not by guesswork, and each entry says what broke and how the
breakage presented — because in every case so far the symptom looked like something else.

```sh
apt-get update && apt-get install -y fonts-dejavu-core
```

That list was wrong, and the way it was wrong is the point: it recorded the one package a
*test failure* pointed at, and missed everything whose absence produces no failure at all.

A container rebuild on 2026-09-16 came up with the dotnet tree building cleanly, 0 warnings, and
the whole reference toolchain gone:

| what | state after rebuild | how the absence presented |
|---|---|---|
| `/opt/libreoffice26.2/program/soffice` | **absent** | `timeout: failed to run command … No such file or directory`, exit status **0** |
| `poppler-utils` (`pdfinfo`, `pdftotext`, `pdftoppm`) | absent | `pdfinfo: command not found`, and an empty page count that reads as a broken PDF |
| `pymupdf`, `pillow` | absent | a probe script fails on import, or silently returns nothing |
| `fonts-dejavu-core` | present this time | — |
| `/usr/bin/soffice` | **present, 24.2.7.2** | this is the trap, see below |

**The dangerous one is the third row of that table combined with the last.** With
`/opt/libreoffice26.2` gone, `/usr/bin/soffice` still answers, still converts, still produces a
plausible PDF — and it is **24.2.7.2**, a different renderer whose output this project has
measured to differ from the reference on page counts, word counts and font fallback. A script
that falls back to `soffice` on `$PATH`, or a person who types it from memory, gets a complete,
confident, wrong reference half with nothing anywhere reporting a problem. **Check the version,
not the existence.**

Two failure modes made this worse than it needed to be:

- **`timeout` exits 0 when the command does not exist.** `timeout 300 /opt/…/soffice … ; echo $?`
  printed `exit=0`. A conversion loop testing `$?` concludes every document converted.
- **A raw-bytes `grep` for `/Im1 Do` in a PDF finds nothing even when the image is drawn**, because
  content streams are Flate-compressed. It returned 0 for the reference *and* for us, which looks
  like agreement. Inflate the streams — `zlib.decompress` per `stream…endstream` — or use the
  project's own `pdf-ops.py`. This is the same class as the pikepdf incident: an instrument that
  is not installed, or not reading the right layer, reports absence rather than failure.

### Restoring the reference

The exact build is archived and the download works through the proxy:

```sh
curl -O https://downloadarchive.documentfoundation.org/libreoffice/old/26.2.4.2/deb/x86_64/LibreOffice_26.2.4.2_Linux_x86-64_deb.tar.gz
tar xzf LibreOffice_26.2.4.2_Linux_x86-64_deb.tar.gz
cd LibreOffice_26.2.4.2_Linux_x86-64_deb/DEBS
mkdir -p /tmp/stage && for d in *.deb; do dpkg-deb -x "$d" /tmp/stage; done
rm -rf /opt/libreoffice26.2 && mv /tmp/stage/opt/libreoffice26.2 /opt/
apt-get update && apt-get install -y poppler-utils fonts-dejavu-core \
    fonts-crosextra-carlito fonts-crosextra-caladea fonts-liberation fonts-liberation2 \
    libreoffice-writer libreoffice-calc libreoffice-impress
pip install pymupdf pillow

# The bundled faces must then be moved aside, or the harness refuses the reference outright.
D=/opt/libreoffice26.2/share/fonts/truetype
mkdir -p $D/.duplicates-aside $D/.noto-aside
mv $D/Carlito*.ttf $D/Caladea*.ttf $D/Liberation*.ttf $D/DejaVu*.ttf $D/opens___.ttf $D/.duplicates-aside/
mv $D/NotoSans-*.ttf $D/NotoSerif-*.ttf $D/.noto-aside/
fc-cache -f
```

**Four more packages than the first version of this said, and each was invisible in a different
way.** After restoring only the reference binary and poppler, `dotnet build` was clean at 0
warnings and the suite reported **85 spreadsheet failures, 8 word-processing failures, 158 text
skips and 490 fidelity skips** — which reads as a catastrophic regression and is entirely the
environment:

- **`fonts-crosextra-carlito` and `fonts-crosextra-caladea`** were absent, so `fc-match Calibri`
  answered DejaVu Sans. `dotnet/CLAUDE.md` already says every OOXML comparison is meaningless
  without them; what it does not say is that the *failure* is 93 red tests in two projects that
  have nothing to do with fonts by name. Installing them took the whole suite green in one step.
- **`libreoffice-writer`, `-calc` and `-impress`** were absent while `libreoffice-core` was
  present, which is the case `dotnet/CLAUDE.md` describes exactly: `soffice` starts, reports a
  version, and converts nothing. `LibreOfficeRunner.IsAvailable` converts a probe file rather
  than looking for the binary, so this shows up as **472 skipped**, not as an error.
- **The bundled-font shadow check refuses the reference**, and a freshly unpacked tarball trips
  it by construction: the harness compares the tarball's `share/fonts` against the system's byte
  for byte and skips every comparison while any face differs. The recipe in `dotnet/CLAUDE.md`
  covers Carlito, Caladea, Liberation, DejaVu and the Latin Noto — **it does not cover
  `opens___.ttf`**, and OpenSymbol is installed by `libreoffice-core`, so restoring the distro
  packages *creates* the last clash. One file, and it holds 472 tests hostage.

**The skip reason is printed and it names the file.** Do not guess at this: run one fidelity test
with `--logger "console;verbosity=detailed"` and read the line — it says which faces shadow and
what to do. Three guesses were spent here before that was done.

Restored-environment baseline, for comparison after any future rebuild: every project green with
**0 skipped**, and `Paperless.Fidelity.Tests` at **552 discovered, 542 passed, 10 failed** — the
four documented families (`paginated.*`, `list-label-overrun.*`, `justify-shrink-2013.docx`,
`sheet-rich-text.xlsx`) that are left failing on purpose.

Extract with `dpkg-deb -x` rather than `dpkg -i`: the debs also install a `/usr/bin` launcher, and
overwriting the 24.2.7.2 there would destroy the one control this project has for telling a 26.2
behaviour apart from a long-standing one.

### The check to run at the start of every session

```sh
/opt/libreoffice26.2/program/soffice --version   # must end 0229ac93fcf0d7cbc6376066c6f35021cef002dc
fc-match "DejaVu Sans"                           # must say DejaVuSans.ttf, not wqy-zenhei.ttc
pdfinfo -v 2>&1 | head -1                        # must exist
python3 -c "import pymupdf, PIL"                 # must not raise
```

The version line's trailing hex **is** the identifier this project records as the reference's hash.
It is LibreOffice's own build id, printed by `--version` — it is not a `sha1sum` of the binary, and
checking it with `sha1sum` gives a different number and a false alarm.

---

That is the whole list at present. It is short and it is not trivial.

**Re-check it every session — the install does not survive.** This was installed and written
up as fixed, and a later session opened with `fc-match "DejaVu Sans"` answering
`wqy-zenhei.ttc` again and the package absent from `dpkg -l`. Every *other* font the reference
needs was still present, so nothing looks wrong until you check this one. The `apt-get update`
in the command above is load-bearing rather than habit: without it the container's stale index
answers `E: Package 'fonts-dejavu-core' has no installation candidate`, which reads as the
package having been withdrawn from the archive when it has not.

```sh
fc-match "DejaVu Sans"      # must say DejaVuSans.ttf, not wqy-zenhei.ttc
```

---

## `fonts-dejavu-core`

**Symptom without it:** two unit tests fail —
`Paperless.Spreadsheets.Tests.SheetColumnDigitsTests.ADigitWidthIsNeitherTruncatedNorRounded`
at `("DejaVu Sans", 11)` and `("DejaVu Sans", 12)` — and *nothing else in the suite reports a
problem*. Spreadsheets sits at 619/621 and the other nine projects are green.

**Why those two tests are the canary.** They are not incidental. The test pins DejaVu Sans at
1303/2048 of an em specifically because it straddles the rounding carry from the opposite side
to Carlito's 1038/2048, and its comment records that all four figures "were read out of the
`style:column-width` LibreOffice 24.2.7.2 wrote for a one-column probe workbook". So the
repository's own test suite is a statement that DejaVu was installed in the environment where
every stored figure on this project was measured. **The test suite is the specification of the
environment**, and this is the case that proves it.

**Why it matters far beyond two tests.** DejaVu sits ahead of WenQuanYi Zen Hei in fontconfig's
fallback chain. **267 of the 534 reference PDFs — half the corpus — fall back to
WenQuanYiZenHei when DejaVu is absent.**

Measured on the **reference half only**, by sweeping the whole corpus twice with LibreOffice held
constant at 26.2.4.2 and *only* the font set differing:

| track | documents | page counts changed | total \|Δpages\| | word counts changed | total \|Δwords\| |
|---|---:|---:|---:|---:|---:|
| words | 200 | **42** | 383 | 55 | 7029 |
| slides | 163 | **0** | 0 | 61 | 3676 |
| sheets | 171 | **11** | 43 | 36 | 3191 |
| **total** | **534** | **53** | **426** | **152** | **13 896** |

Slides moving zero pages is the expected structural result: a deck's page count is its slide
count, so only the text channel can move there, and it did (61 decks).

**Two corrections to the first reading of this table, each established by later measurement:**

- **The direction is not uniform.** This was first written up as "every page-count change is in
  the same direction — fewer pages with DejaVu, because the DejaVu fallback is narrower". That
  holds on words but is false on sheets, where **6 of the 11 gain pages with DejaVu and 5 lose
  them**. DejaVu is not uniformly narrower than the face it displaced, and it also restores a bold
  that had collapsed into WenQuanYi. A tidy directional story was reached for before the sign had
  been checked per document.
- **The font set is an input to *both* halves of the gate, not only the reference.** Paperless
  resolves faces through fontconfig as well, so our own column moves too: re-rendering the sheets
  track from the same source with DejaVu present moved **31 of 171** of our own documents. The
  practical rule is that a parity figure is valid only when **both** banks were rendered on the
  same font set. A mismatched pair is worse than a merely stale one, because it is silently
  internally inconsistent. One round withdrew its number as a mismatched pair and it was later
  shown to have been consistent after all — both halves had been rendered before the font landed,
  53 seconds ahead of the `dpkg` timestamp.

A missing font is therefore not a cosmetic gap. It moves glyph advances, and glyph advances
move wrapping, row heights, cropping and pagination — which is to say it moves the gate's first
two checks directly. Any parity figure measured without it is measuring the wrong environment.

A missing font is therefore not a cosmetic gap. It moves glyph advances, and glyph advances
move wrapping, row heights, cropping and pagination — which is to say it moves the gate's first
two checks directly. Any parity figure measured without it is measuring the wrong environment.

**The trap it set, recorded because it nearly worked.** This container also has LibreOffice
26.2.4.2 where the stored figures were taken against 24.2.7.2. It is very natural to attribute
all reference movement to the version bump and stop looking — a prior pass did exactly that, and
wrote up a whole-corpus movement table on that basis. **Two variables had changed, not one.**
Anything phrased as "the 24.2.7.2 → 26.2.4.2 effect" and measured before this font landed is
confounded and has to be re-taken.

The first census that looked for the problem also missed it: `grep -rl dejavu` over the corpus
returns **zero** documents, and grepping the reference PDFs for `DejaVu` or `WenQuanYi` returns
zero too, because PDF font names live inside compressed streams. Both readings say "no reach"
and both are wrong. `pdffonts` parses the file and gives the true answer — the right instrument
matters more than the thorough-looking sweep.

---

## Deliberately *not* installed

- **`ttf-mscorefonts-installer`** — LibreOffice suggests it, and it is correct that it is absent.
  The reference PDFs name `LiberationSans`/`LiberationSerif`/`LiberationMono` throughout and
  never `Arial`, `Times New Roman` or `Courier New`. Liberation is the metric-compatible
  substitute, so the substitution is already happening and is what every stored figure was
  measured against. Installing the real MS fonts would change the reference on a large fraction
  of the corpus and invalidate the baseline in the same way the missing DejaVu did.
- **`fonts-dejavu-extra`** — adds Condensed variants that would enter the fallback chain. Only
  `-core` (which pulls `-mono`) is evidenced by the test suite; adding more is an unforced
  change to the font environment.
- **`python3-pil` and `python3-numpy`** — these are now installed on this container, and that was
  **an unnecessary install, recorded so nobody repeats it**. They went in to build a one-off
  side-by-side review page. The project's own comparison tooling does **not** want them and says
  so in `pdf-image-diff.py`'s own header: poppler renders to PPM, which is a header and raw RGB,
  and PNG is zlib plus four chunks, so *"adding numpy or Pillow to read two rectangles of bytes
  would be a dependency for its own sake"*. It reads P6 directly with `struct` and diffs a page in
  about a tenth of a second.

  Nothing was broken by their absence and nothing is broken by their presence — they do not touch
  rendering. **Do not add them to a provisioning script**, and prefer `pdf-image-diff.py` to
  writing a second pixel comparator.
- **Anything shipping a `.otf`** — and this one is a genuine, measured gap that is still being
  left open on purpose. `fc-list | grep -c '\.otf'` is **0** here, so the single remaining skip
  in `Paperless.Rendering.Tests` never runs: it guards a poppler failure mode that once blanked
  **161 glyph runs**, and it needs a CFF-outline face to exercise. A guard that cannot run is
  worth roughly nothing, so this is a real loss.

  It is nonetheless the right call for now, because **installing a font is precisely what caused
  this project's worst confound**. Adding `fonts-dejavu-core` moved 53 of 534 reference page
  counts and 31 of 171 of our own sheets renderings; any new face may enter the fallback chain
  and invalidate the canonical reference bank, forcing a 534-document re-sweep and re-stating
  every scoreboard. **The cost is a re-baseline; the benefit is one test.** If it is ever taken,
  take it deliberately: install, re-sweep the reference, and diff the banks before believing any
  figure measured across the change.

## Already present, and required — do not remove

`fonts-liberation`, `fonts-crosextra-carlito`, `fonts-crosextra-caladea`, `fonts-opensymbol`,
plus the CJK faces `fonts-wqy-zenhei` and `fonts-ipafont-gothic`. A census of all 534 reference
renderings with `pdffonts` shows the corpus resolves to exactly these:

| face | reference PDFs |
|---|---:|
| LiberationSans / -Bold / -Italic / -BoldItalic | 338 / 282 / 115 / 76 |
| WenQuanYiZenHei | 268 |
| Carlito Regular / Bold / Italic / BoldItalic | 233 / 177 / 60 / 39 |
| OpenSymbol | 203 |
| LiberationSerif and variants | 153 / 109 / 59 / 29 |
| LiberationMono and variants | 41 / 11 / 6 / 2 |
| IPAGothic, IPAPGothic | 26 / 3 |
| Caladea Regular / Bold | 8 / 8 |

`Montserrat-Bold` (2) and `Verdana-Italic` (1) also appear; those are embedded by the documents
themselves and need nothing installed.

---

## How to check the environment before trusting a measurement

```sh
fc-match "DejaVu Sans"                  # must report DejaVuSans.ttf, not a fallback
cd dotnet && dotnet test tests/Paperless.Spreadsheets.Tests/Paperless.Spreadsheets.Tests.csproj
                                        # must be 621/621, 0 skipped
```

A fallback answer from `fc-match` is the tell. `fc-match` never fails — it always returns
*something*, and here it returned WenQuanYi Zen Hei for a request for DejaVu Sans, which reads
as success unless you look at what came back.


---

## `libreoffice-math` — not installed, and it silently changes the reference

Found 2026-08-14 while working the words `extra` group, and **not installed**, deliberately.

Without it **every reference in this container draws nothing for an OMML equation.** A
one-equation probe renders as `BEFOREEQUATION  AFTEREQUATION`, with the equation's space
reserved on the page and no ink in it.

**`ABCD-FE-01-00 Flight Envelope - v1 08.03.16.docx` is the corpus document this decides, and
it is not a defect of ours — it is this missing package.** Measured 2026-08-15: the document
holds **54 `m:oMath` elements** (the figure of 33 recorded here earlier counted `m:oMathPara`,
which is the wrapper and undercounts) carrying **121 whitespace-split tokens**. Against a word
delta of **+111**, and — the decisive test rather than the arithmetic — of the 45 *distinct*
equation strings, **we draw more occurrences than the reference for 37**, the other 8 being
strings that also occur in ordinary body text. So the reference is drawing essentially none of
them.

That makes it the same shape as a raster ceiling: **our output is the better one and the word
gate scores it as a failure.** It should be read as an environment ceiling for as long as
`libreoffice-math` is absent, and it is the one corpus document whose verdict would change if
the package were installed — which is the concrete argument for eventually taking that step, at
the cost of re-banking every equation-bearing reference.

This is the same class of problem as the missing `fonts-dejavu-core` above: **an input to the
gate that nothing in the harness declares.** The difference is that the font affected 267 of
534 references and this affects only equation-bearing documents — but the failure mode is
identical, and so is the way it hides. A document whose equations the reference cannot draw
looks like a document where we draw too much.

**It was not installed on the spot, and that was the right call.** Installing it changes the
reference for every equation-bearing document, and other agents were mid-round measuring
against the banked set at `/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/`. Doing it properly
means: install, re-bank the affected references, and re-baseline — as a deliberate step, when
no measurement is in flight.

```sh
apt-get update && apt-get install -y --no-install-recommends libreoffice-math
```

Until that happens, treat any word-count gap on a document containing `m:oMath` as suspect,
and check whether the reference drew the equation at all before attributing the gap to us.
