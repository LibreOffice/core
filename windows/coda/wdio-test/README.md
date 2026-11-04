# WebdriverIO tests for CODA-W

This runs the shared specs in `qt/test/specs`, which CODA-Q and CODA-M
also run, against CODA-W.

The specs use two drivers:

- `browser.webEngine` is msedgedriver, attached to the WebView2
  browser of CODA-W through its remote debugging port, as
  WebEngineDriver is attached to QtWebEngine on CODA-Q.
- `browser.native` is coda-driver, in `windows/coda/coda-driver`. It
  drives the native user interface, such as the Open dialog, through
  UI Automation, as `macos/coda-driver` does on CODA-M.

A service in `lib/coda-windows.service.ts` starts both drivers and
CODA-W before the specs run. CODA-W gets a temporary folder for its
data and its documents, through the environment variables
CODA_TEST_DATA_DIR and CODA_TEST_DOCUMENTS_DIR, so a run leaves the
settings and documents of the user alone. The fixtures from
`qt/test/fixtures` are copied into the documents folder.

## What a run needs

- A build of CODA-W.
- Node.js, for WebdriverIO.
- The .NET 8 SDK, for building coda-driver. The service builds it with
  `dotnet build` at the start of each run, unless CODA_DRIVER_EXE
  names an executable to use as it is.
- msedgedriver.exe of the same major version as the WebView2 runtime
  on the machine. The service compares the two at the start of a run,
  and says which version it needs when they differ.
- A logged-in desktop session. The run drives windows on the screen,
  so leave the mouse and keyboard alone while it runs.

## Running the tests

    cd windows\coda\wdio-test
    npm install             # This is needed just once
    $env:CODA_APP = '<build>\windows\coda\x64\Debug\program\<APP_NAME>.exe'
    $env:MSEDGEDRIVER = '<folder>\msedgedriver.exe'
    npm test

The ports are 9222 for remote debugging, 9515 for msedgedriver and
4723 for coda-driver. REMOTE_DEBUGGING_PORT, EDGEDRIVER_PORT and
NATIVE_UI_PORT change them.

A run leaves the WebDriver command log in `logs/`. The output of CODA-W
and of both drivers goes to the console.

## Which specs run

The list is in `wdio.conf.ts`. A spec that reaches native user
interface needs a CODA_PLATFORM=windows branch in its selectors before
it can be added there.
