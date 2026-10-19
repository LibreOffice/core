# LanguageTool, a remote sentence checker

The checker sends each paragraph to a LanguageTool server and underlines
what it reports: spelling, grammar and style, in some thirty languages. It
also spell checks, so for the languages a reachable server has, the local
spell checker steps aside.

The HTTP code it shares with the Duden and Grammalecte checkers is in
`../../lingutil/remotechecker.cxx`.

## The pieces

| Path | What it is |
|---|---|
| `languagetoolimp.cxx` | builds the request and reads the answer |
| `LanguageTool.component` | `org.openoffice.lingu.LanguageToolGrammarChecker` |

Settings: `/org.openoffice.Office.Linguistic/GrammarChecking/LanguageTool`
(`IsEnabled`, `BaseURL`, `Username`, `ApiKey`, `SSLCertVerify`). The
languages are its `GrammarCheckers` entry in
`../../../config/Linguistic-lingucomponent-grammarchecker.xcu`.

On the wire: `POST <BaseURL>/check` with `text` and `language` form fields,
plus `username` and `apiKey` for the premium service; whether the server is
there is asked with `GET <BaseURL>/languages`.

## Testing it by hand

1. A server: `docker run --rm -p 8010:8010 collabora/languagetool`
   (see `docker/languagetool/` in the online tree), or the public one at
   `https://api.languagetool.org/v2`, which limits how much it takes.

2. Online: start it with the checker on, without editing coolwsd.xml,
   which `make` regenerates from coolwsd.xml.in:

       make run RUN_EXTRA="--o:languagetool.enabled=true --o:languagetool.base_url=http://127.0.0.1:8010/v2"

   The desktop app: Options, Remote Sentence Checkers, LanguageTool.

3. Type `The catt sat down.` in an English (USA) paragraph and right-click
   `catt`: the popup says "Possible spelling mistake found." with the
   LanguageTool icon and the server's suggestions.

## When nothing shows

The kit prints `Remote checker languagetool has not answered; keeping the
local spell checker` on stderr when the server did not answer. Run with
`SAL_LOG="+WARN+INFO.linguistic+INFO.lingucomponent"` to see each checker
that declined a paragraph's language (`... does not support the required
locale`), and the
`warn:lingucomponent.remote` line of a failed request with its curl code and
HTTP status.
