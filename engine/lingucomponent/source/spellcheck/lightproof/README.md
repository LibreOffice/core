# Lightproof, the built-in sentence checker

Lightproof checks a paragraph against a table of rules: a regular expression,
a replacement, a message, and an optional condition. It used to be a Python
extension shipped inside each dictionary; it is now native code, and no
Python is involved at run time.

This directory holds the checker and the two tools that feed it.

## How a rule reaches the reader

    <pkg>.dat   rule source, written by hand
    <pkg>.dlg   the options, their defaults and their titles
       |
       |  generator/lpgenerate.py          (by hand, when rules change)
       v
    lightproof_<pkg>.py                    the rule table, committed
    lightproof_opts_<pkg>.py               the options and their defaults
       |
       |  tools/lpcompile.py               (every build)
       v
    <pkg>.lpr                              installed to share/lightproof,
                                           read by the checker

The titles take a different road, and not this one: see step 5 of
*Adding a language*.

The middle two files are committed rather than generated at build time,
because one of the four packages has no rule source at all. See
*Where the sources are* below.

## The pieces

| Path | What it is |
|---|---|
| `lightproofimp.cxx` | the `XProofreader`, and the host functions the rules call |
| `lprfile.cxx` | reads a `.lpr`, which is mapped rather than loaded |
| `lpvm.cxx` | runs the bytecode a rule's condition compiles to |
| `tools/lpcompile.py` | rule table to `.lpr`; also emits the options schema and the dialog titles |
| `tools/lpdemo.py` | builds the demonstration document described below |
| `generator/lpgenerate.py` | rule source to rule table |
| `generator/lightproof_compile.py` | vendored from upstream Lightproof; see its header |

## Changing a rule

For a package that has a rule source:

    python3 lingucomponent/source/spellcheck/lightproof/generator/lpgenerate.py \
        dictionaries/en/lightproof

That rewrites `dictionaries/en/pythonpath/lightproof_en.py` and
`lightproof_opts_en.py`. Commit both; the build takes them from there.

Compile one package per run. The vendored compiler keeps its definitions in
module state, and compiling a second package in the same process inherits
the first one's.

For a package with no rule source, edit the rule table directly, and expect
it to be unpleasant: it is one long Python list of regular expressions.

## Adding a language

1. Write the rule source, or take a package from upstream, and put it in
   `dictionaries/<pkg>/lightproof/`.
2. Run `lpgenerate.py` to produce the two tables.
3. Add the package to `lingucomponent/CustomTarget_lightproof.mk` and its
   `.lpr` to `lingucomponent/Package_lightproof.mk`.
4. If the language has a bundled dictionary, name the service for its
   locale in that dictionary's `Linguistic.xcu`, as the others do. Nothing
   else is needed: the checker is found through its component registration
   and asked which locales it serves, and it answers from the rule files it
   reads out of the directory.
5. Add the options to the configuration and to the dialog:

       lpcompile.py --emit-schema dictionaries/<pkg> <pkg>
       lpcompile.py --emit-titles dictionaries/<pkg> <pkg>

   Both print a block to paste. Neither runs at build time, and both read
   the package's `.dlg`. The first block goes under
   `GrammarChecking/SentenceChecking` in
   `officecfg/registry/schema/org/openoffice/Office/Linguistic.xcs`, the
   second into `settingTitles` in
   `browser/admin/src/integrator/AdminIntegratorSettings.ts`, along with the
   package's defaults in `Xcu.ts`.

   The schema is committed because integrators set these keys through the
   xcu, so the names are an interface. It carries the English titles: the
   person reading it is configuring a deployment, not necessarily a speaker
   of the language the package checks.

   The dialog gets the titles in the package's own language instead, and
   they are not translated. An option names a notion of the grammar of one
   language, and whoever has a reason to set it is reading that language
   already; translating them into every interface language would mean
   inventing terms for notions the target language may not have. Only the
   panel titles -- the names of the languages -- are translated. Where a
   package gives an option no help of its own the label stands in for both.

   An option's default is written down three times over: in the package's own
   `lightproof_opts_<pkg>.py`, which is where it comes from, in the schema,
   and in `Xcu.ts`. The schema is the one that decides, because it is what
   configmgr answers with until somebody sets the key; the `.lpr` carries a
   copy too, which only shows through when a package is installed whose schema
   group is missing. Nothing keeps the three in step by itself, so
   `lingucomponent/qa/python/test_option_defaults.py` compares them and fails
   when they drift.

6. Name the checker for the language in
   `lingucomponent/config/Linguistic-lingucomponent-grammarchecker.xcu`,
   after the remote checker where that one offers the language too.

## Where the rules live

`share/lightproof`, and the checker takes the directory as the whole of the
registry: a file names the languages it serves, so nothing else has to say
where it is or what it is for.

They deliberately do not ride inside the dictionary extensions, the way the
spelling, hyphenation and thesaurus data do, for two reasons.

A build configured with `--without-myspell-dicts` leaves the whole
dictionaries module out, and the flatpak and the snap are both configured
that way. That is no loss for the other three, which fall back to whatever
the system has in `/usr/share`. There is no system package of sentence
checking rules, so a build that left ours out would have none at all and no
way to get any, which is why these have to be installed unconditionally.

And an entry in a dictionary's `dictionaries.xcu` is registered per user
profile, refreshed only when the extension directory looks newer than the
profile. It does not look newer when a build rewrites files inside it, so a
profile that registered an older build would keep its answer and never see a
newly added package. Online would not notice, because its forkit registers
the bundled extensions into a throwaway profile at startup and the kits
inherit the merged configuration across `fork()`; an app has no forkit and
keeps its profile. A directory of files has no registration to go stale.

## What the compiler will and will not accept

`lpcompile.py` fails the build for a rule it cannot lower, and drops one
that could never have run, warning either way. The distinction matters: the
first means the compiler needs work, the second that the rule names
something undefined, references a group its own pattern does not have, or is
not valid Python. Brazilian Portuguese had 172 of the second kind, all of
which raise in the Python too and so had never fired in any release. They are
repaired. See *Repairing Brazilian Portuguese* below.

Two things about patterns are worth knowing:

- ICU gives `[`, `&`, `{` and a leading `:` meanings inside a character class
  that Python does not, and a pattern that keeps one is rejected outright at
  run time, taking its rule with it. The translator escapes them and fails
  the build if one survives.
- A rule may name a word its pattern must contain, which the checker hashes
  into a per-paragraph filter so most rules are dismissed before their
  pattern is built. The filter must never reject a rule that could have
  matched, so only text outside every group counts: text inside one may sit
  in a branch of an alternation and need not appear at all.

## Checking it by hand

    python3 lingucomponent/source/spellcheck/lightproof/tools/lpdemo.py demo.odt

writes an ODT with a chapter per language and a row per rule: an example, what
the rule reports, and the suggestions it should offer. Open it in Writer, turn
on the options named in the headings, and look for the underlines.

The examples are generated from the patterns themselves, by walking each one
and building a string it matches, so a row appears only if the text really does
match its rule. Where a pattern asks for a run of letters the text comes from
the package's own spelling dictionary, so an example reads "a apple" and not
"a aahaah", and the pattern's optional parts are left out unless the match
needs them. Brazilian Portuguese gets one example per kind of report rather
than all 11875.

Two things the document cannot promise. A rule whose condition asks the
spelling dictionary or the morphology about the words it matched will not fire
on invented text, and those rows say "needs real words" rather than "always".
And one Russian rule can only match a paragraph with no words in it, which the
checker skips; that row says so too.

## Licensing

The checker itself is ours, MPL 2.0 like the rest of the tree. Everything
taken from upstream is MPL 1.1 / GPL / LGPL and is used under the MPL arm:
the rule compiler in `generator/`, which carries upstream's `COPYING`, and
the rule sources under `dictionaries/<pkg>/lightproof/`.

The record is in `readlicense_oo/license/license.xml`, which is what the
shipped licence file is generated from. The compiled rules are listed there
per language, under Dictionaries, and the compiler under Libraries as
`lightproof`. Change a rule source's copyright line and that file needs the
same change.

## Where the sources are

| Package | Rule source | State |
|---|---|---|
| en | `dictionaries/en/lightproof/` | regenerates the shipped table |
| ru_RU | `dictionaries/ru_RU/lightproof/` | regenerates the shipped table |
| hu_HU | `dictionaries/hu_HU/lightproof/` | regenerates the shipped table |
| pt_BR | `dictionaries/pt_BR/lightproof/` | titles only, see below |

Upstream was `git://anongit.freedesktop.org/libreoffice/lightproof`, and it is
gone. anongit no longer answers, the GitHub mirror it was published through
has been deleted, and the repository gerrit still lists under that name is
empty. It carried sources for en, hu_HU and ru_RU, under MPL 1.1 / GPL / LGPL;
ours are taken under the MPL arm. Its `doc/manual.txt` describes the rule
language.

What survives is a Software Heritage archive of the GitHub mirror, and that is
where all three of our sources came from:

    origin    https://github.com/freedesktop/libreoffice-lightproof
    snapshot  1d247105ba9e95eed20bf58258a71bb9c7d6a9e0   (visited 2020-06-12)
    master    c299ddd4b39395c5b2b0090fa627f05a89d387ee   (2019-09-11)

Fetch a file from it with

    https://archive.softwareheritage.org/api/1/content/sha1_git:<hash>/raw/

walking there from the revision above. The last commit is Laszlo Nemeth's, from
2019, so the repository ran five years past the 2014 mirror that is easier to
find and that only carries Hungarian 1.5.

Brazilian Portuguese has no source to chase. It came from VERO, the Brazilian
spelling project, written by Raimundo Santos Moura in 2013 and generated with
the Lightproof generator that was then on Launchpad. What was handed over was
the generated table, and nothing else: the first commit to the dictionaries
repository carries no rule source, neither does that repository today, neither
does the VERO project page, which publishes only built extensions and asks
contributors to mail in words rather than patches. Upstream has accepted this
and edits the table directly, most recently in 2026.

Nor would a rule source explain much if one turned up. The 11875 rules carry
190 distinct messages between them, 6194 of them the same one, so whatever
Raimundo worked from was a set of word lists and something to expand them, not
a file anyone wrote a rule at a time.

Our table was upstream's rule for rule until the repairs below. Take any
other change from upstream rather than editing ours, so the two stay
comparable, and send the repairs there.

### Where the Brazilian titles came from

The package reached us as a built extension: its author never published a rule
source, so there is no `.dat` for it and the rule table in
`pythonpath/lightproof_pt_BR.py` is the only copy of the rules there is.

Its `lightproof/pt_BR.dlg` is therefore not the author's file. It carries the
option titles only, recovered from the dialog resources the extension shipped
(`dialog/pt_BR_en_US.properties` and `dialog/pt_BR_pt_BR.properties`), which
are themselves generated: upstream's `make.py` built the extension's `.xdl`
and `.properties` out of a `.dlg` we do not have. The option list and the
defaults keep coming from the shipped `lightproof_opts_pt_BR.py`, so the file
has no option section at all.

The recovery is checkable rather than taken on trust: the shipped dialog
reproduces that option table exactly, same 26 options and same 24 on by
default, `minus` and `times` off in both.

Beware that pt_BR reuses option ids from the English package with entirely
different meanings. Its `metric` is Pleonasms, `nonmetric` is Cacophonous
sound, `gerund` is Gerundisms. Reading across from en would mislabel three
checkboxes.

### Repairing Brazilian Portuguese

172 rules could never run. They are not subtle: each one raises where it
stands, so it has never marked anything for anybody, in this engine or in the
Python extension before it. Four kinds, all repaired the same way, by reading
what the rules either side of the broken one do:

- 50 conditions had a stray plus after `not`, as in `not + " ou " + ...`,
  applying unary plus to a string. The clause immediately before it in the
  same condition has the correct form.
- 91 named a word list that lost the last letter of its name, `plvrFem` for
  `plvrFemu` and `plvrMasc` for `plvrMascu`. Every one of these rules is about
  a word ending in u or us, and the neighbouring rules for the same ending use
  the u lists.
- 17 asked for a group their own pattern does not have, mostly off by one or
  two after a renumbering: `Ptaz_26` where the pattern defines `Ptaz_24`.
- 14 were not Python at all: `m.group("m.group("Pivv_53")`, an unterminated
  string, a stray `s` after a group reference.

The last one needed a word list rather than a correction. It turns "A" into
"O" before a masculine noun ending in -es, and asked for an exception list
that exists under no spelling. Its three sibling rules show what such a list
is for: each holds the words that "a" may correctly precede, where "a" is a
preposition and not an article, `fim` for "a fim de", `meu` and `seu` for
"a meu ver", `propósito` and `tempo` for "a propósito" and "a tempo". For
this ending the exceptions are a different part of the grammar: `rês` is
feminine, and `cortês`, `descortês`, `montês` and `pedrês` are the adjectives
of this ending that keep one form for both genders, as in "cabra montês". So
`excplMasces` holds those five, and "A cortês resposta" is left alone while
"A inglês" is still corrected to "O inglês".

Each repaired rule was then run through the checker with a word taken from
the list its own condition consults. 127 of the 171 mark something, and the
suggestions are right: "onde a bantus" offers "onde as bantus", "centena de
abaulagem" offers "centena de abaulagens". The rest need more of their
condition satisfied than one word supplies.

They have never run, so they have never been read by a Portuguese speaker
either. `lpdemo.py` will show them; a native check is still worth having.

Hungarian is version 1.6.4, which is what the archive has, and it regenerates
the shipped table but for one rule: the old generator wrote a second "(?iu)"
into the middle of the pattern for the rule on guillemets, where the flag does
nothing and where Python 3.11 refuses it outright. Regenerating drops it.

Russian took two fixes in 2021 that went into the table alone, because at the
time there was nothing to regenerate it with. Both are now settled at the
source, and the table has been regenerated:

- the message read "Символ троеточия" in the source and "Символ
  многоточия" in the table. The table is the correct one: commit
  f206a4b5a330, which calls the source's word vernacular. The source now
  says the same.
- the apostrophe rule lost its case-insensitive flag in commit 9a771d3d10ef,
  "Fix Flags not at the start of the expression in ru-RU". The generator was
  putting "(?iu)" in the middle of the pattern, where Python rejects it, and
  deleting it was the fix available. It cost nothing: the rule's character
  class already spells out both cases, so the flag changes no match. The
  generator now emits it at the front instead, which is what the source asks
  for and what the other packages have.

A third fix, commit 8167fb5ecbd9, reformatted the table to one rule per
line. The generator writes it that way too, so regenerating keeps it.

Those two rules and one label are the whole of what our sources carry over the
archived ones: the ellipsis message above, and "typographica=Typographical" in
`ru_RU.dlg`, which the archive spells "Typographica". Everything else in all
three packages is byte for byte what upstream had, and each of the six tables
is byte for byte what its source generates.
