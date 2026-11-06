# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4; fill-column: 100 -*-
#
# Copyright the Collabora Online contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

"""The crashtest session client: for each document, open a session as a browser does, recalculate
a spreadsheet, wait until the kit reports idle, save in each format of its application, close.
Then reload each export in its own session and wait until idle.

The results go in results.tsv, one line per pass: document, pass (first or reload), outcome (ok,
load-failed, save-failed, kit-died, timeout, refused), seconds, detail. The detail of a
save-failed pass starts with the format of the export that failed.
"""

import base64
import concurrent.futures
import http.client
import json
import os
import socket
import struct
import time
import urllib.parse

FORMATS = {'text': ['odt', 'doc', 'docx', 'rtf'], 'spreadsheet': ['ods', 'xls', 'xlsx'],
           'presentation': ['odp', 'ppt', 'pptx'], 'drawing': ['odg']}
# 60 seconds to load, 30 more for a document other than a spreadsheet to settle, 180 seconds per
# export.
LOAD_SECONDS, IDLE_SECONDS, SAVE_SECONDS = 60, 30, 180


class Closed(Exception):
    pass


class Refused(Exception):
    pass


class WebSocket:
    def __init__(self, host, port, path):
        self.sock = socket.create_connection((host, port), timeout=10)
        key = base64.b64encode(os.urandom(16)).decode()
        self.sock.sendall((f'GET {path} HTTP/1.1\r\nHost: {host}:{port}\r\nUpgrade: websocket\r\n'
                           f'Connection: Upgrade\r\nOrigin: http://{host}:{port}\r\n'
                           f'Sec-WebSocket-Key: {key}\r\nSec-WebSocket-Version: 13\r\n\r\n')
                          .encode())
        head = b''
        while b'\r\n\r\n' not in head:
            chunk = self.sock.recv(4096)
            if not chunk:
                raise Closed('no handshake answer')
            head += chunk
        status, self.buffer = head.split(b'\r\n\r\n', 1)
        if b' 101 ' not in status.split(b'\r\n', 1)[0]:
            raise Refused(status.split(b'\r\n', 1)[0].decode(errors='replace'))

    def send(self, text):
        self._send_frame(0x81, text.encode())

    def _send_frame(self, first, data):
        mask = os.urandom(4)
        n = len(data)
        head = bytes([first])
        if n < 126:
            head += bytes([0x80 | n])
        elif n < 65536:
            head += bytes([0x80 | 126]) + struct.pack('>H', n)
        else:
            head += bytes([0x80 | 127]) + struct.pack('>Q', n)
        self.sock.sendall(head + mask + bytes(b ^ mask[i % 4] for i, b in enumerate(data)))

    def _read(self, n, deadline):
        while len(self.buffer) < n:
            self.sock.settimeout(max(0.1, deadline - time.monotonic()))
            try:
                chunk = self.sock.recv(65536)
            except socket.timeout:
                if time.monotonic() >= deadline:
                    raise TimeoutError
                continue
            if not chunk:
                raise Closed('connection closed')
            self.buffer += chunk
        data, self.buffer = self.buffer[:n], self.buffer[n:]
        return data

    def receive(self, deadline):
        """Next complete message as text, or None for a binary one. Answers pings, as coolwsd
        closes a session that leaves them unanswered for a minute."""
        message, opcode = b'', None
        while True:
            b0, b1 = self._read(2, deadline)
            n = b1 & 0x7f
            if n == 126:
                n = struct.unpack('>H', self._read(2, deadline))[0]
            elif n == 127:
                n = struct.unpack('>Q', self._read(8, deadline))[0]
            payload = self._read(n, deadline)
            if b0 & 0x0f == 8:
                raise Closed('close frame')
            if b0 & 0x0f == 9:
                self._send_frame(0x8a, payload)
                continue
            if b0 & 0x0f == 10:
                continue
            if b0 & 0x0f:
                opcode = b0 & 0x0f
            message += payload
            if b0 & 0x80:
                return message.decode(errors='replace') if opcode == 1 else None

    def wait_for(self, prefixes, seconds):
        deadline = time.monotonic() + seconds
        while True:
            message = self.receive(deadline)
            if message is None:
                continue
            if message.startswith('error:') and 'kind=' in message and 'cmd=internal' in message:
                raise Closed(message.split('\n', 1)[0])
            for prefix in prefixes:
                if message.startswith(prefix):
                    return message

    def close(self):
        try:
            self.sock.close()
        except OSError:
            pass


def file_url(path):
    return 'file://' + urllib.parse.quote(os.path.abspath(path))


def download(host, port, url, download_id, target):
    path = '/cool/' + urllib.parse.quote(url, safe='') + '/download/' + download_id
    conn = http.client.HTTPConnection(host, port, timeout=SAVE_SECONDS)
    try:
        conn.request('GET', path, headers={'Origin': f'http://{host}:{port}'})
        response = conn.getresponse()
        if response.status != 200:
            return False
        with open(target, 'wb') as f:
            f.write(response.read())
        return True
    finally:
        conn.close()


def wait_until_idle(ws, doctype, load_deadline):
    """A spreadsheet recalculates and must be idle within the load limit. Another document gets
    IDLE_SECONDS to settle, and the exports start anyway when it does not."""
    if doctype == 'spreadsheet':
        ws.send('uno .uno:CalculateHard')
    ws.send('uno .uno:ReportWhenIdle ' + json.dumps({'idleID': {'type': 'string', 'value': '1'}}))
    hard = doctype == 'spreadsheet'
    deadline = load_deadline if hard else time.monotonic() + IDLE_SECONDS
    try:
        while True:
            answer = ws.wait_for(['unocommandresult:'], max(0.1, deadline - time.monotonic()))
            try:
                body = json.loads(answer.split(':', 1)[1])
            except ValueError:
                continue
            if body.get('commandName') == '.uno:ReportWhenIdle':
                return
    except TimeoutError:
        if hard:
            raise


def session(host, port, path, formats, outdir):
    """Returns (outcome, detail, exported paths)."""
    url = file_url(path)
    ws = WebSocket(host, port, '/cool/' + urllib.parse.quote(url, safe='') + '/ws')
    exported = []
    try:
        ws.send('coolclient 0.1')
        ws.send('load url=' + url)
        load_deadline = time.monotonic() + LOAD_SECONDS
        status = ws.wait_for(['status:', 'error:'], LOAD_SECONDS)
        if status.startswith('error:'):
            return 'load-failed', status.split('\n', 1)[0], exported
        try:
            doctype = json.loads(status.split(':', 1)[1]).get('type', 'other')
        except ValueError:
            doctype = 'other'
        wait_until_idle(ws, doctype, load_deadline)
        if formats is None:
            formats = FORMATS.get(doctype, [])
        stem = os.path.splitext(os.path.basename(path))[0]
        for fmt in formats:
            target = os.path.join(outdir, stem + '.' + fmt)
            # The export a user downloads: the kit saves into the jail and coolwsd serves it.
            ws.send(f'downloadas name={urllib.parse.quote(stem + "." + fmt)} id=export '
                    f'format={fmt} options=')
            answer = ws.wait_for(['downloadas:', 'error:'], SAVE_SECONDS)
            if answer.startswith('error:'):
                return 'save-failed', fmt + ' ' + answer.split('\n', 1)[0], exported
            download_id = answer.split('downloadid=', 1)[1].split()[0]
            if not download(host, port, url, download_id, target):
                return 'save-failed', fmt + ' download failed', exported
            exported.append(target)
        return 'ok', '', exported
    finally:
        ws.close()


def run_pass(host, port, path, formats, outdir):
    start = time.monotonic()
    try:
        outcome, detail, exported = session(host, port, path, formats, outdir)
    except TimeoutError:
        outcome, detail, exported = 'timeout', '', []
    except Refused as e:
        outcome, detail, exported = 'refused', str(e), []
    except (Closed, OSError) as e:
        outcome, detail, exported = 'kit-died', str(e), []
    return outcome, detail, exported, time.monotonic() - start


def one_document(host, port, path, outroot):
    """The passes of one document: the source, then each of its exports. The exports go under
    outroot/current followed by the directory of the source document."""
    outdir = os.path.join(outroot, 'current') + os.path.dirname(os.path.abspath(path))
    os.makedirs(outdir, exist_ok=True)
    rows = []
    outcome, detail, exported, seconds = run_pass(host, port, path, None, outdir)
    rows.append((path, 'first', outcome, f'{seconds:.1f}', detail))
    for export in exported:
        outcome, detail, _, seconds = run_pass(host, port, export, [], outdir)
        rows.append((export, 'reload', outcome, f'{seconds:.1f}', detail))
    return rows


def run(host, port, paths, outroot, workers):
    """Runs the documents in paths, workers at a time, and writes outroot/results.tsv."""
    os.makedirs(outroot, exist_ok=True)
    with open(os.path.join(outroot, 'results.tsv'), 'w') as results, \
            concurrent.futures.ThreadPoolExecutor(workers) as pool:
        jobs = [pool.submit(one_document, host, port, path, outroot) for path in paths]
        for job in jobs:
            for row in job.result():
                results.write('\t'.join(row) + '\n')
            results.flush()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
