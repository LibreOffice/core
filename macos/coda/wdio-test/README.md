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

    cd macos/coda/wdio-test
    npm install             # This is needed just once

    export CODA_APP='/Users/<you>/Library/Developer/Xcode/DerivedData/coda-<hash>/Build/Products/Debug/Collabora Office.app'
    npm test

## NOTE

The Collabora Online that is started will complain that it cannot
open the document; just click 'hello.odt' in the backstage before
the actual test attempts to run (so that it opens), and the test
will pass. This is a bug that will be fixed in follow-up work.
