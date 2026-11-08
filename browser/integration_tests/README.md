# Integration tests

Mocha tests that connect to a running coolwsd over WebSocket and check
the COOL protocol from Node. They sit between the mocha unit tests in
browser/mocha_tests, which need no server, and the cypress tests, which
drive a real browser against a real server.

## Running

To start coolwsd, run the tests and stop coolwsd again, also when a
test fails, run this from this folder:

    make check-with-server

To keep one server running while you work on the tests:

    make start-coolwsd       # in the background, pid file in workdir/coolwsd.pid
    make check-integration   # run as often as you like
    make stop-coolwsd

coolwsd listens on port 9980. When another server already listens
there, start-coolwsd stops with an error. Pass another port to every
step then, such as make check-with-server COOLWSD_PORT=9990.

make check at the top of the tree runs these tests from the test
folder, which comes after coolwsd is built, through make
check-on-free-port. That target runs check-with-server with coolwsd on
a port that no server listens on. make check in the browser folder
does not run them, since coolwsd may not be built yet.

The tests are compiled into workdir in the build tree. The tests in
the renderer folder draw what the engine writes with the browser
renderer, so they build into one script with the browser sources, the
same way the mocha_tests bundle builds.

## Environment

- COOLWSD_URL: the WebSocket URL of the server that check-integration
  talks to. The default is ws://localhost:9980.
- COOL_TEST_SAMPLES: the folder of the sample documents. make sets it
  to test/samples of the source tree.
- COOL_TEST_DOC: the absolute path of one document that every test
  loads instead of its own sample.
- COOL_TEST_WORKDIR: the folder that each test copies its document to
  before it loads it, since coolwsd saves a document that a test
  changed. make sets it to workdir/documents. The default is a new
  folder in the temporary folder of the system.
- COOL_TEST_DEBUG: any value other than an empty one logs every message
  sent and received, which helps when writing a new test.
- COOL_VECTOR_PRIMITIVE_SAMPLES: set in the environment of coolwsd, any
  value makes its kits answer .uno:VectorPrimitiveSample with the
  primitive samples of the engine. start-coolwsd sets it. A coolwsd
  started some other way needs it for the tests of the samples.
