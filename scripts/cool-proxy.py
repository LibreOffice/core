#!/usr/bin/env python3
# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# Copyright the Collabora Online contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

"""Proxy for coolwsd that drops websockets, to stress-test reconnection.

Usage: cool-proxy.py -- ./coolwsd <coolwsd args>

Runs coolwsd, reads its port from the 'Ready to accept connections' line,
and listens on the first free port from 9980. The URLs coolwsd prints are
re-written to point at the proxy.

Connections are passed through unchanged. A connection whose first request
is a websocket upgrade is aborted 10 seconds after the first 'status:' that
coolwsd sends down it, ie. once the document is loaded.

When coolwsd has SSL, the proxy serves TLS with the same certificate, and
talks TLS to coolwsd too.
"""

import asyncio
import re
import signal
import ssl
import sys
import time

FIRST_PORT = 9980
DROP_AFTER_SECS = 10
BUFFER_SIZE = 65536
STATUS = b"status:"


def log(message):
    stamp = time.strftime("%H:%M:%S")
    print(f"[cool-proxy {stamp}] {message}", file=sys.stderr, flush=True)


def is_websocket(head):
    """Does this request head ask for a websocket upgrade?"""
    for line in head.split(b"\r\n")[1:]:
        name, _, value = line.partition(b":")
        if name.strip().lower() == b"upgrade" and value.strip().lower() == b"websocket":
            return True
    return False


async def pump(reader, writer, on_data=None):
    """Copy bytes from reader to writer until either side closes."""
    try:
        while True:
            data = await reader.read(BUFFER_SIZE)
            if not data:
                break
            if on_data:
                on_data(data)
            writer.write(data)
            await writer.drain()
    except (ConnectionError, asyncio.CancelledError):
        pass
    finally:
        # Pass the end of stream on, so a half-closed HTTP connection still completes.
        try:
            if writer.can_write_eof():
                writer.write_eof()
        except (OSError, RuntimeError):
            pass


class Proxy:
    def __init__(self, upstream_tls):
        self.upstream_tls = upstream_tls
        self.upstream_port = None
        self.next_id = 0

    async def handle(self, client_reader, client_writer):
        self.next_id += 1
        connection_id = self.next_id
        upstream_writer = None
        try:
            try:
                head = await client_reader.readuntil(b"\r\n\r\n")
            except asyncio.IncompleteReadError:
                return
            upstream_reader, upstream_writer = await asyncio.open_connection(
                "127.0.0.1", self.upstream_port, ssl=self.upstream_tls)
            upstream_writer.write(head)
            if is_websocket(head):
                await self.forward_websocket(connection_id, head, client_reader, client_writer,
                                             upstream_reader, upstream_writer)
            else:
                await asyncio.gather(pump(client_reader, upstream_writer),
                                     pump(upstream_reader, client_writer))
        except (OSError, ssl.SSLError, asyncio.LimitOverrunError) as error:
            log(f"#{connection_id} connection failed: {error}")
            client_writer.transport.abort()
        finally:
            for writer in (client_writer, upstream_writer):
                if writer:
                    writer.close()

    async def forward_websocket(self, connection_id, head, client_reader, client_writer,
                                upstream_reader, upstream_writer):
        log(f"#{connection_id} websocket {head.split(b' ')[1][:100].decode('latin-1')}")

        # Just look for 'status:' in the byte stream, without parsing websocket frames.
        seen = asyncio.Event()
        tail = b""

        def watch(data):
            nonlocal tail
            if not seen.is_set() and STATUS in tail + data:
                seen.set()
            tail = data[-(len(STATUS) - 1):]

        tasks = [
            asyncio.create_task(pump(client_reader, upstream_writer)),
            asyncio.create_task(pump(upstream_reader, client_writer, watch)),
        ]

        waiter = asyncio.create_task(seen.wait())
        await asyncio.wait(tasks + [waiter], return_when=asyncio.FIRST_COMPLETED)
        if seen.is_set():
            log(f"#{connection_id} status: seen - drop in {DROP_AFTER_SECS}s")
            done, _ = await asyncio.wait(tasks, timeout=DROP_AFTER_SECS)
            if len(done) < len(tasks):
                log(f"#{connection_id} dropping websocket")
                # Abort discards any unsent data and closes at once, like a lost network.
                client_writer.transport.abort()
                upstream_writer.transport.abort()
        else:
            waiter.cancel()

        await asyncio.gather(*tasks, return_exceptions=True)
        log(f"#{connection_id} websocket closed")


def option_value(command, name):
    """Return the value of --name=value in the coolwsd command line, or None."""
    for arg in command:
        if arg.startswith(name + "="):
            return arg[len(name) + 1:]
    return None


def setup_tls(command):
    """Steal coolwsd's certificate to serve TLS, and make a client context to talk to coolwsd.

    Returns (server context, client context), or (None, None) without SSL."""
    cert = option_value(command, "--o:ssl.cert_file_path")
    key = option_value(command, "--o:ssl.key_file_path")
    disabled = ("--disable-ssl" in command
                or option_value(command, "--o:ssl.enable") == "false")
    if disabled or not cert or not key:
        return None, None
    server = ssl.create_default_context(ssl.Purpose.CLIENT_AUTH)
    server.load_cert_chain(cert, key)
    client = ssl.create_default_context()
    client.check_hostname = False
    client.verify_mode = ssl.CERT_NONE  # it's our own coolwsd on loopback
    return server, client


async def listen(handler, tls_context, taken):
    """Listen on the first free port from FIRST_PORT upwards, skipping 'taken'."""
    for port in range(FIRST_PORT, FIRST_PORT + 100):
        if port == taken:
            continue
        try:
            server = await asyncio.start_server(handler, "0.0.0.0", port, ssl=tls_context)
            return server, port
        except OSError:
            pass
    log(f"no free port in {FIRST_PORT}-{FIRST_PORT + 99}")
    return None, None


async def main():
    command = sys.argv[1:]
    if command and command[0] == "--":
        command = command[1:]
    if not command:
        sys.exit(__doc__)
    tls_context, upstream_tls = setup_tls(command)

    # coolwsd gets its own session, so a Ctrl-C reaches it once - from us.
    process = await asyncio.create_subprocess_exec(
        *command, stdout=asyncio.subprocess.PIPE, stderr=asyncio.subprocess.STDOUT,
        start_new_session=True, limit=1 << 20)
    loop = asyncio.get_running_loop()
    for signum in (signal.SIGINT, signal.SIGTERM, signal.SIGHUP):
        loop.add_signal_handler(signum, process.send_signal, signum)

    proxy = Proxy(upstream_tls)
    server = None
    rewrite = None
    ready = re.compile(rb"Ready to accept connections on port (\d+)")

    while True:
        line = await process.stdout.readline()
        if not line:
            break
        if rewrite:
            line = rewrite.sub(rb"\g<1>:%d" % port, line)
        sys.stdout.buffer.write(line)
        sys.stdout.buffer.flush()

        match = ready.search(line) if not server else None
        if match:
            proxy.upstream_port = int(match.group(1))
            server, port = await listen(proxy.handle, tls_context, proxy.upstream_port)
            if not server:
                process.send_signal(signal.SIGTERM)
                continue
            # Swap coolwsd's port for ours in any URL it prints from now on.
            rewrite = re.compile(rb"(://(?:[^/@\s]*@)?[^/:\s]+):%d(?=[/\s?]|$)"
                                 % proxy.upstream_port)
            log(f"listening on port {port}, forwarding to coolwsd on port "
                f"{proxy.upstream_port}, dropping websockets {DROP_AFTER_SECS}s after status:")

    if server:
        server.close()
    code = await process.wait()
    return 128 - code if code < 0 else code


if __name__ == "__main__":
    sys.exit(asyncio.run(main()))
