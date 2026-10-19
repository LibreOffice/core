# Grammalecte, a remote sentence checker

The checker sends each French paragraph to a Grammalecte server and
underlines the grammar and typography errors it reports. It does not spell
check: the local spell checker keeps French.

The HTTP code it shares with the LanguageTool and Duden checkers is in
`../../lingutil/remotechecker.cxx`.

## The pieces

| Path | What it is |
|---|---|
| `grammalecteimp.cxx` | builds the request and reads the answer |
| `Grammalecte.component` | `cpo.lingu.Grammalecte` |

Settings: `/org.openoffice.Office.Linguistic/GrammarChecking/Grammalecte`
(`IsEnabled`, `BaseURL`, `SSLCertVerify`, and the `Options` group, one switch
per rule family, with Grammalecte's own Writer defaults). The language is
its `GrammarCheckers` entry in
`../../../config/Linguistic-lingucomponent-grammarchecker.xcu`.

On the wire: `POST <BaseURL>/gc_text/fr` with `text` and `options` form
fields; whether the server is there is asked with
`GET <BaseURL>/get_options/fr`. The server counts its offsets in code
points from the start of each line, which the checker turns into UTF-16
offsets into the paragraph.

## Testing it by hand

1. A server: `docker/grammalecte/` in the online tree builds one; or unpack
   the release archive from grammalecte.net and run
   `python3 grammalecte-server.py -ht 127.0.0.1 -p 8080`.

2. Online: start it with the checker on, without editing coolwsd.xml,
   which `make` regenerates from coolwsd.xml.in:

       make run RUN_EXTRA="--o:grammalecte.enabled=true --o:grammalecte.base_url=http://127.0.0.1:8080"

   The desktop app: Options, Remote Sentence Checkers, Grammalecte.

3. Type `Il sont parti.` in a French paragraph and right-click `sont`: the
   popup shows Grammalecte's message on the conjugation, the suggestion
   `est` and the Grammalecte icon.

Without a server, `make CppunitTest_linguistic_grammalecte` runs the
checker against a mock server of its own.

## When nothing shows

Run with `SAL_LOG="+WARN+INFO.linguistic+INFO.lingucomponent"`. A checker
that is off or has no server URL declines the paragraph's language
(`... does not support the required locale`), and a failed request leaves a
`warn:lingucomponent.remote` line with its curl code and HTTP status.
