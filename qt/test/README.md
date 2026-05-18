coda-qt E2E Tests
==================

WebdriverIO tests with two drivers:

  browser.webEngine -- web content inside QtWebEngine (via WebEngineDriver)
  browser.native    -- native Qt widgets (via AT-SPI)


Setup
-----

1. Install system packages (Node.js and Qt 6 with qtpaths6 in PATH also required):

  Fedora:
    dnf install weston at-spi2-core python3-pyatspi \
        python3-gobject python3-lxml python3-numpy \
        qt6-qtwayland

  openSUSE:
    zypper in weston at-spi2-core python3-atspi \
        python3-gobject python3-lxml python3-numpy \
        qt6-wayland

  Ubuntu/Debian:
    apt install weston at-spi2-core python3-pyatspi \
        python3-gi python3-lxml python3-numpy \
        qt6-wayland

2. Clone and set up selenium-webdriver-at-spi somewhere you prefer:

    git clone https://invent.kde.org/sdk/selenium-webdriver-at-spi.git
    cd selenium-webdriver-at-spi
    python3 -m venv --system-site-packages venv
    source venv/bin/activate
    pip install -r requirements.txt

Running
-------

Configure with the test flags enabled:

    ./autogen.sh && ./configure \
      --enable-qtapp \
      --enable-qtapp-tests \
      --with-atspi-driver-path=/path/to/selenium-webdriver-at-spi/selenium-webdriver-at-spi.py \
      --with-lo-path=.../core-auto/instdir \
      --with-lokit-path=.../core-auto/include \
      --enable-debug

Build and run the tests:

    make -j$(nproc)
    make check-qtapp

`make check-qtapp` installs npm dependencies automatically on first run.

Troubleshooting
---------------

The tests run headless inside a nested Weston compositor, so the coda-qt
window is normally invisible.  If eg. coda-qt crashes right after
the test starts and you cannot tell why, set `CODA_QT_TEST_GUI=1`:

    CODA_QT_TEST_GUI=1 make check-qtapp

This opens the Weston window on your desktop so you can watch the test run
interactively, or even launch coda-qt by hand inside that window to
reproduce the crash without the harness.

Environment variables
---------------------

The following variables can still be set to override defaults when invoking
run-tests.sh directly (e.g. during development):

  AT_SPI_VENV             Path to the AT-SPI driver virtualenv (default: <AT_SPI_DRIVER_PATH>/venv)
  REMOTE_DEBUGGING_PORT   QtWebEngine CDP port (default: 9222)
  WEBENGINE_DRIVER_PORT   WebEngineDriver port (default: 9515)
  AT_SPI_PORT             AT-SPI Flask server port (default: 4723)


Writing tests
-------------

Test specs go in specs/. Both drivers are available:

  describe('My test', () => {
      it('does something', async function() {
          const title = await browser.webEngine.execute(() => document.title);

          const btn = await browser.native.$('name=OK');
          await btn.click();
      });
  });

