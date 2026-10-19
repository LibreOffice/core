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

## Running on CI

A run needs what a developer's desktop has anyway:

- A logged-in graphical session for the user the job runs as, with
  the screen unlocked. The driver types into the open panel with
  system key events, and those go to the frontmost window of that
  session.
- Accessibility permission for coda-driver.app, under System Settings
  -> Privacy & Security -> Accessibility. The grant is tied to the
  identity bundle.sh signs the driver with, so it survives rebuilds.
  A run without it stops right away, with an "accessibility-denied"
  message in the console.
- A build of the app, as above, or `CODA_APP` set.

What a run leaves behind is in `logs/`: the WebDriver command log
(`wdio-0-0.log`), the app's own log (`coda-app.log`) and the native
driver's (`coda-driver.log`). Keep the directory as a build artifact.
A failure is worked out from these three, and the console only shows
the last lines of the app's log.

`npm ci` wants a `package-lock.json`, which this directory does not
track, so install the dependencies with `npm install` on CI too.
