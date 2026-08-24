#!/usr/bin/env python3
# Minimal WOPI host for testing: serves one document, accepts saves.
import json
import os
import sys
from http.server import BaseHTTPRequestHandler, HTTPServer

PORT = int(sys.argv[1])
DOCPATH = sys.argv[2]


class WopiHandler(BaseHTTPRequestHandler):
    def log_message(self, fmt, *args):
        sys.stderr.write("wopi: %s\n" % (fmt % args))

    def _send(self, code, body=b"", ctype="application/json"):
        self.send_response(code)
        self.send_header("Content-Type", ctype)
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def do_GET(self):
        path = self.path.split("?")[0]
        if path.endswith("/contents"):
            with open(DOCPATH, "rb") as f:
                self._send(200, f.read(), "application/octet-stream")
        elif "/wopi/files/" in path:
            info = {
                "BaseFileName": os.path.basename(DOCPATH),
                "Size": os.path.getsize(DOCPATH),
                "OwnerId": "owner",
                "UserId": "tester",
                "UserFriendlyName": "Macro Tester",
                "UserCanWrite": True,
                "PostMessageOrigin": "*",
            }
            self._send(200, json.dumps(info).encode())
        else:
            self._send(404)

    def do_POST(self):
        # PutFile / PutRelativeFile - accept and discard.
        length = int(self.headers.get("Content-Length", 0))
        self.rfile.read(length)
        self._send(200, b"{}")

    do_PUT = do_POST


HTTPServer(("0.0.0.0", PORT), WopiHandler).serve_forever()
