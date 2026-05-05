# WebDriverIO test runner for the macOS app

Collabora Office for macOS implements the W3C WebDriver protocol
internally: when launched with `--uitesting --testDriverPort=<port>`,
it starts an embedded HTTP server (`WebDriverServer`) that handles
WebDriver requests against its WKWebView. WebDriverIO connects to
this server directly, with no separate driver process needed.

This setup runs the shared specs from `qt/test/specs/` against the
macOS app via that embedded server. Multiremote is used so that the
specs can keep using `browser.webEngine` without modification.

A service in `lib/coda-macos.service.ts` launches the app with the
required arguments and waits for the WebDriverServer to respond
before tests start.

## Running the tests

Build the macOS app once in Xcode (Cmd+B), then:

    cd macos/coda/wdio-test
    npm install             # This is needed just once
    npm test

The setup auto-detects the app from Xcode's DerivedData. You can
override the location of Collabora Office by setting `CODA_APP`
before running `npm test`:

    export CODA_APP='/<path>/Collabora Office.app'

## NOTE

The Collabora Online that is started will complain that it cannot
open the document; just click 'hello.odt' in the backstage before
the actual test attempts to run (so that it opens), and the test
will pass. This is a bug that will be fixed in follow-up work.
