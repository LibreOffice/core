# The Duden Korrekturserver, a remote sentence checker

The checker sends each paragraph to a Duden Korrekturserver and underlines
what it reports: spelling, grammar and style, for German above all, and
English, French, Italian and Spanish. It also spell checks, so for the
languages a reachable server has, the local spell checker steps aside.

The server is licensed software from EPC Consulting und Software GmbH; its
own documentation is at `<server>/doc/index.html`. The HTTP code the
checker shares with the LanguageTool and Grammalecte checkers is in
`../../lingutil/remotechecker.cxx`.

## The pieces

| Path | What it is |
|---|---|
| `dudenimp.cxx` | builds the request and reads the answer |
| `Duden.component` | `cpo.lingu.Duden` |

Settings: `/org.openoffice.Office.Linguistic/GrammarChecking/Duden`
(`IsEnabled`, `BaseURL`, `ApiKey`, `SSLCertVerify`, `SpellcheckingLevel`,
`OrthographyStandard`, `MessageLevel`, `PropertySets`, `Dictionaries`, and
the `Style` group). The languages are its `GrammarCheckers` entry in
`../../../config/Linguistic-lingucomponent-grammarchecker.xcu`.

On the wire: `POST <BaseURL>/check` with a JSON body, the API token in an
`access_token` header; whether the server is there is asked with
`GET <BaseURL>/version`, and the correction profiles and dictionaries it
offers with `GET /propset` and `GET /dictionaries`.

## Testing it by hand

1. A server and a token: there is no free one. The token is created in the
   server's user accounts page.

2. Online: start it with the checker on, without editing coolwsd.xml,
   which `make` regenerates from coolwsd.xml.in:

       make run RUN_EXTRA="--o:duden.enabled=true --o:duden.base_url=https://<server>/api --o:duden.api_key=<token>"

   The desktop app: Options, Remote Sentence Checkers, Duden.

3. Type `Das ist ein Tset.` in a German (Germany) paragraph and right-click
   `Tset`: the popup shows the server's message, such as "Dieses Wort
   korrigieren?", its proposals and the Duden icon.

Without a server, `make CppunitTest_linguistic_restprotocol` runs the
checker against a mock server of its own.

## When nothing shows

The kit prints `Remote checker duden has not answered; keeping the local
spell checker` on stderr when the server did not answer. Run with
`SAL_LOG="+WARN+INFO.linguistic+INFO.lingucomponent"` to see each checker
that declined a paragraph's language (`... does not support the required
locale`), and the
`warn:lingucomponent.remote` line of a failed request with its curl code and
HTTP status.
