# Emoji and symbol AutoCorrect shortcodes

Typing a shortcode between colons replaces it with a character as soon as the
closing colon is typed:

    :rocket:  -> 🚀        :1/2:  -> ½        :-->:  -> →
    :tada:    -> 🎉        :^2:   -> ²        :alpha: -> α

`emoji.txt` holds the shortcodes, `bin/gen-autocorr-emoji.py` turns them into
the AutoCorrect replacement list the office reads.

## The names are international, not translated

The shortcodes are the GitHub/Slack names, which people already know from chat
and issue trackers. They are the same in every language, so the list is
generated once into `../lang/und/DocumentList.xml` — the `und` (undetermined)
language, which is what the AutoCorrect dialog shows as `[All]`.

`SvxAutoCorrect::SearchWordsInList` (`editeng/source/misc/svxacorr.cxx`) looks
in the document language's list first and then **always** in the `und` list,
even when the language list exists and did not match. So one generated file
covers every language and no per-language file has to be generated or
translated.

Before this, the names were translated through `emoji.ulf` and baked into each
language's `DocumentList.xml` by hand. That meant `:rocket:` did nothing in a
German document — you had to know it was `:Rakete:` there — and the generator
lived on one person's machine, so the last regeneration was 2018 while
translators kept working on the strings. Both are gone: there is no `.ulf` any
more, so nothing is extracted for translation, and the generator is in the tree.

## Editing

Each line of `emoji.txt` is four tab separated fields:

    codepoints <TAB> character <TAB> shortcodes <TAB> Unicode name

    1F680	🚀	rocket	ROCKET
    1F389	🎉	tada, party	PARTY POPPER

The first shortcode is the canonical one; the rest are aliases, kept so that
names from before the change still work. The codepoints must spell the
character — the generator checks it — and a shortcode may only mean one thing
in the whole file.

After editing, regenerate and rebuild:

    ./bin/gen-autocorr-emoji.py
    make extras

`./bin/gen-autocorr-emoji.py --check` verifies the generated file is current
without writing anything, which is what to call from a build or a CI job.

## Adding the emoji of a newer Unicode version

The shortcode set comes from the [gemoji](https://github.com/github/gemoji)
database, which is what GitHub and Slack use, restricted to emoji that people
actually use according to the
[Unicode emoji frequency](https://home.unicode.org/emoji/emoji-frequency/)
ranking. To refresh it, take `db/emoji.json` from gemoji and add a line per new
emoji with its `aliases` as the shortcodes. Do not add the long tail: an
unused emoji under a name nobody guesses costs a lookup on every word typed
and buys nothing.

Deliberately kept although GitHub has no name for them: the typographic and
mathematical entries — superscripts and subscripts, Greek letters, arrows,
fractions, dashes, currency, musical notation. They are the part of this list
that an office suite needs and an emoji picker does not offer, and they were
never language dependent to begin with.

## Why not use the emoji picker of the operating system?

For emoji alone that is usually the better tool, and LibreOffice removed its own
emoji widget in 2022 for that reason (tdf#151197). This list stays because it
is faster than any picker for the handful of characters people type over and
over, and because the symbol half of it has no picker at all.
