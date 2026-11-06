#!/usr/bin/env python3
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

"""Crashtesting through coolwsd: load each document of a list, recalculate, export it to the
formats of its application and reload each export, through the coolwsd and kits of this build
tree.

usage: crashtest.py [options] <list> <outdir>

The list is a file with one document path per line, or a directory, whose files are taken in
os.walk order, which stays the same from run to run on the same file system. The documents are
numbered into tasks of --task-size documents in that order, counted from 0. The documents run in chunks, each with a fresh coolwsd, in
outdir/chunk-NNNN. A chunk with a "done" file is complete, so a rerun with the same arguments
carries on where an earlier run stopped. At the end, outdir holds crashlog.txt, exportCrash.txt
and crashes.tsv for the whole list, and backtraces/ with one gdb backtrace per kit crash whose
core was found.

Build the tree with --enable-debug, so that a document which makes the kit reach outside its
sandbox aborts it (SAL_ABORT_ON_FORBIDDEN). Cores need a core_pattern that writes to a
directory, passed as --cores, or systemd-coredump.
"""

import argparse
import http.client
import os
import resource
import signal
import subprocess
import sys
import time

import client
import report

TREE = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
READY_SECONDS, STOP_SECONDS = 120, 60


def coolwsd_command(port, workers):
    # No capabilities: the kits run under landlock rather than in a chroot, and can dump core.
    return [os.path.join(TREE, 'coolwsd'), f'--port={port}', '--unattended',
            '--disable-cool-user-checking',
            '--o:sys_template_path=' + os.path.join(TREE, 'systemplate'),
            '--o:child_root_path=' + os.path.join(TREE, 'jails'),
            '--o:cache_files.path=' + os.path.join(TREE, 'cache'),
            '--o:storage.filesystem[@allow]=true', '--o:ssl.enable=false',
            '--o:security.capabilities=false', f'--o:num_prespawn_children={workers}',
            '--o:per_document.max_concurrency=2',
            '--o:mount_jail_tree=false', '--o:logging.level=warning',
            '--o:logging.file[@enable]=false',
            '--o:admin_console.username=admin', '--o:admin_console.password=admin']


def allow_cores():
    resource.setrlimit(resource.RLIMIT_CORE, (resource.RLIM_INFINITY, resource.RLIM_INFINITY))


def wait_until_ready(wsd, port):
    deadline = time.monotonic() + READY_SECONDS
    while time.monotonic() < deadline:
        if wsd.poll() is not None:
            sys.exit(f'coolwsd exited early with {wsd.returncode}')
        conn = http.client.HTTPConnection('localhost', port, timeout=5)
        try:
            conn.request('GET', '/hosting/discovery')
            if conn.getresponse().status == 200:
                return
        except OSError:
            pass
        finally:
            conn.close()
        time.sleep(1)
    wsd.kill()
    sys.exit(f'coolwsd not ready after {READY_SECONDS} seconds')


def stop(wsd):
    """Stops coolwsd cleanly, so that it saves nothing half done and its kits exit."""
    wsd.send_signal(signal.SIGTERM)
    try:
        wsd.wait(STOP_SECONDS)
    except subprocess.TimeoutExpired:
        print(f'coolwsd ignored SIGTERM for {STOP_SECONDS} seconds, killing it', file=sys.stderr)
        wsd.kill()
        wsd.wait()


def list_documents(source):
    if os.path.isdir(source):
        return [os.path.join(directory, name)
                for directory, _, names in os.walk(source) for name in names]
    return [line.strip() for line in open(source) if line.strip()]


def remove_good_exports(chunk):
    """Removes the exports that reloaded fine. The exports whose reload failed stay, as they
    reproduce the failure."""
    for line in open(os.path.join(chunk, 'results.tsv')):
        path, stage, outcome = line.split('\t')[:3]
        if stage == 'reload' and outcome == 'ok' and os.path.exists(path):
            os.remove(path)
    for directory, _, _ in os.walk(os.path.join(chunk, 'current'), topdown=False):
        try:
            os.rmdir(directory)
        except OSError:
            pass


def run_chunk(chunk, tasks, args):
    os.makedirs(chunk, exist_ok=True)
    with open(os.path.join(chunk, 'list.tsv'), 'w') as f:
        for task, path in tasks:
            f.write(f'{task}\t{path}\n')
    with open(os.path.join(chunk, 'coolwsd.log'), 'w') as log:
        wsd = subprocess.Popen(coolwsd_command(args.port, args.workers), cwd=TREE,
                               stdout=log, stderr=subprocess.STDOUT, preexec_fn=allow_cores)
        try:
            wait_until_ready(wsd, args.port)
            client.run('localhost', args.port, [path for _, path in tasks], chunk, args.workers)
        finally:
            stop(wsd)
    crashes = report.report(chunk, args.cores, os.path.join(args.outdir, 'backtraces'))
    if not args.keep_exports:
        remove_good_exports(chunk)
    open(os.path.join(chunk, 'done'), 'w').close()
    return crashes


def collect(outdir, chunks):
    """Joins the report files of the chunks into outdir."""
    for name in ('crashlog.txt', 'exportCrash.txt', 'crashes.tsv', 'results.tsv'):
        with open(os.path.join(outdir, name), 'w') as out:
            for chunk in chunks:
                with open(os.path.join(chunk, name)) as part:
                    out.write(part.read())


def main():
    parser = argparse.ArgumentParser(
        description='Crashtest a list of documents through the coolwsd of this build tree.')
    parser.add_argument('--workers', type=int, default=os.cpu_count(),
                        help='documents in progress at once (default: the number of CPUs)')
    parser.add_argument('--chunk', type=int, default=2000,
                        help='documents per coolwsd run (default: 2000)')
    parser.add_argument('--task-size', type=int, default=100,
                        help='documents per task number (default: 100)')
    parser.add_argument('--port', type=int, default=9990, help='coolwsd port (default: 9990)')
    parser.add_argument('--cores', help='directory that core_pattern writes cores into')
    parser.add_argument('--keep-exports', action='store_true',
                        help='keep the exports that reloaded fine')
    parser.add_argument('list', help='file with one document path per line, or a directory')
    parser.add_argument('outdir', help='output directory')
    args = parser.parse_args()
    args.outdir = os.path.abspath(args.outdir)

    if not os.path.exists(os.path.join(TREE, 'coolwsd')):
        sys.exit(f'no coolwsd in {TREE}: build the tree first')
    with open(os.path.join(TREE, 'config.h')) as f:
        if '#define ENABLE_DEBUG 1' not in f.read():
            print('warning: not a debug build, so access outside the sandbox does not abort',
                  file=sys.stderr)

    tasks = [(n // args.task_size, path)
             for n, path in enumerate(list_documents(args.list))]
    os.makedirs(args.outdir, exist_ok=True)
    chunks = []
    for start in range(0, len(tasks), args.chunk):
        chunk = os.path.join(args.outdir, f'chunk-{start // args.chunk:04d}')
        chunks.append(chunk)
        if os.path.exists(os.path.join(chunk, 'done')):
            continue
        began = time.monotonic()
        part = tasks[start:start + args.chunk]
        crashes = run_chunk(chunk, part, args)
        print(f'{os.path.basename(chunk)}: {len(part)} documents, {crashes} kit crashes, '
              f'{time.monotonic() - began:.0f} seconds', flush=True)
    collect(args.outdir, chunks)


if __name__ == '__main__':
    main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
