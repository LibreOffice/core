#!/usr/bin/env python3
# Drives a Collabora Online server over its websocket protocol: loads a WOPI
# document, dispatches a share-location Python macro the way the
# CallPythonScript postMessage API does, and checks that the macro ran.
#
# Exit codes: 0 = macro executed (marker text found), 1 = macro did not run,
#             2 = document failed to load / protocol error.
import sys
import time
import urllib.parse

import websocket

SERVER = sys.argv[1]  # e.g. ws://127.0.0.1:9980
WOPISRC = sys.argv[2]  # e.g. http://172.17.0.1:9981/wopi/files/test.odt
TIMEOUT = int(sys.argv[3]) if len(sys.argv) > 3 else 60

MARKER = "PYMACRO_TRACKED_OK"
SCRIPT_CMD = ("uno vnd.sun.star.script:macro_test.py$enable_track_changes"
              "?language=Python&location=share")

docurl = WOPISRC + "?access_token=test"
ws_url = (SERVER + "/cool/" + urllib.parse.quote(docurl, safe="")
          + "/ws?WOPISrc=" + urllib.parse.quote(WOPISRC, safe="") + "&compat=/ws")

log = open("wsclient.log", "w")


def trace(direction, msg):
    log.write("%.3f %s %s\n" % (time.time(), direction, msg))
    log.flush()


ws = websocket.create_connection(ws_url, timeout=10)
ws.settimeout(2)


def send(msg):
    trace(">>>", msg)
    ws.send(msg)


now = str(int(time.time() * 1000))
send("coolclient 0.1 " + now + " " + now)
send("load url=" + urllib.parse.quote(docurl, safe="") + " deviceFormFactor=desktop")

loaded = False
script_sent = False
trackchanges_on = False
marker_found = False
last_poll = 0.0
deadline = time.time() + TIMEOUT

while time.time() < deadline:
    try:
        frame = ws.recv()
    except websocket.WebSocketTimeoutException:
        frame = None
    except (websocket.WebSocketConnectionClosedException, ConnectionError):
        trace("---", "connection closed by server")
        break

    if frame is not None:
        msg = frame.decode("utf-8", "replace") if isinstance(frame, bytes) else frame
        first_line = msg.split("\n", 1)[0]
        trace("<<<", first_line[:400])

        if first_line.startswith("error:"):
            if "kind=faileddocloading" in first_line or "cmd=load" in first_line:
                print("FAIL: document load error: " + first_line)
                sys.exit(2)
        elif first_line.startswith("status:"):
            loaded = True
        elif first_line.startswith("statechanged: .uno:TrackChanges="):
            trackchanges_on = first_line.endswith("=true")
        elif first_line.startswith("textselectioncontent:"):
            if MARKER in msg:
                marker_found = True
                break

    if loaded and not script_sent:
        send(SCRIPT_CMD)
        script_sent = True
        last_poll = time.time()

    # Poll the document body for the marker the macro inserts.
    if script_sent and time.time() - last_poll >= 2:
        send("uno .uno:SelectAll")
        send("gettextselection mimetype=text/plain;charset=utf-8")
        last_poll = time.time()

ws.close()

if not loaded:
    print("FAIL: document never loaded (no status: message)")
    sys.exit(2)
if marker_found:
    print("PASS: python macro executed (marker found, TrackChanges=%s)"
          % trackchanges_on)
    sys.exit(0)
print("FAIL: python macro did not execute within %ss (TrackChanges=%s)"
      % (TIMEOUT, trackchanges_on))
sys.exit(1)
