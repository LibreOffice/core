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

"""Turns the output of one crashtest chunk into the files the crashtest report reads.

usage: report.py [--cores DIR] <chunk directory>

Reads list.tsv, results.tsv and coolwsd.log in the chunk directory and writes there:
- crashlog.txt: "Crash:<path>" for each pass whose kit crashed. The path is the source document
  for the first pass and the export for a reload. A kit the debug build aborted for an access
  outside its sandbox is not a crash here, but a line of sandbox.tsv.
- exportCrash.txt: for each crash in the background save process, the path of the source
  document with the extension of the export.
- crashes.tsv: task, pid, stage (load or save), signal, document, pass, path.
- sandbox.tsv: task, pid, stage, document, pass, path, access (r, w or x), and the path the
  access resolved to, for each kit aborted for an access outside its sandbox.
- backtraces/task<N>-core.<pid>.backtrace.txt: gdb's "thread apply all backtrace full" for each
  crash whose core can be found, in DIR or else through coredumpctl.
"""

import argparse
import os
import re
import subprocess
import sys

FATAL = re.compile(r'^kit-(\d+)-\d+ \S+ \S+ \[ (\S+) \] SIG +Fatal signal received: (SIG[A-Z]+)?')
FORBIDDEN = re.compile(r'warn:sal\.osl:(\d+):\d+:\S+: access outside sandbox to ([rwx]):.* '
                       r'which is really (.*) forbidden')
LOAD_DOC = re.compile(r'load doc: \S*/([^/\s]+)$')
BACKTRACE = re.compile(r'^Backtrace (\d+) ')
TREE = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
GDB_SECONDS = 600


def kit_crashes(log):
    """(pid, stage, signal, file name, forbidden access) for each fatal signal in a kit. The
    signal is "?" when another process's output split its name. The forbidden access is
    (access, path) when the kit aborted on an access outside its sandbox, which it logs with its
    pid just before the signal, and None otherwise. The kit names its
    document a few lines after the signal, and its background save process runs in a thread
    whose name starts with kitbgsv. The kit's dump ends at "Backtrace <pid>". Two kits that
    crash together can interleave their dumps, so the file name is None when another kit's
    signal or backtrace comes first, or when more than one document is named."""
    crashes, lines = [], open(log, errors='replace').readlines()
    forbidden = {}
    for n, line in enumerate(lines):
        access = FORBIDDEN.search(line)
        if access:
            forbidden[access.group(1)] = (access.group(2), access.group(3))
        match = FATAL.match(line)
        if not match:
            continue
        pid, thread, signal = match.groups()
        # The log prefix pads the pid with zeros, and other messages print it without them.
        pid = str(int(pid))
        signal = signal or '?'
        sandbox = forbidden.pop(pid, None) if signal in ('SIGABRT', '?') else None
        names = set()
        for following in lines[n + 1:n + 40]:
            backtrace = BACKTRACE.match(following)
            if backtrace and int(backtrace.group(1)) == int(pid):
                break
            if backtrace or FATAL.match(following):
                names.clear()
                break
            found = LOAD_DOC.search(following.strip())
            if found:
                names.add(found.group(1))
        name = names.pop() if len(names) == 1 else None
        crashes.append((pid, 'save' if thread.startswith('kitbgsv') else 'load', signal, name,
                        sandbox))
    return crashes


def find_core(pid, cores, scratch):
    """The core file of pid: a file in cores whose name holds the pid as a whole number, or else
    the core systemd-coredump kept, written out into scratch. Returns (path, whether it is a
    copy in scratch)."""
    if cores:
        whole = re.compile(rf'(^|\D){pid}(\D|$)')
        for name in sorted(os.listdir(cores)):
            if whole.search(name):
                return os.path.join(cores, name), False
    target = os.path.join(scratch, f'core.{pid}')
    try:
        subprocess.run(['coredumpctl', 'dump', pid, '--output=' + target],
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=GDB_SECONDS)
    except (OSError, subprocess.TimeoutExpired):
        return None, False
    return (target, True) if os.path.exists(target) else (None, False)


def write_backtrace(core, target):
    with open(target, 'w') as out:
        try:
            subprocess.run(['gdb', '-batch', '-nx', '-ex', 'thread apply all backtrace full',
                            os.path.join(TREE, 'coolforkit-ns'), core],
                           stdout=out, stderr=subprocess.STDOUT, timeout=GDB_SECONDS)
        except subprocess.TimeoutExpired:
            out.write(f'\ngdb did not finish in {GDB_SECONDS} seconds\n')


def write_backtrace_of(pid, task, cores, chunk, backtraces):
    """Writes the backtrace of the core of pid into backtraces, when its core can be found."""
    os.makedirs(backtraces, exist_ok=True)
    core, copy = find_core(pid, cores, chunk)
    if core:
        write_backtrace(core, os.path.join(backtraces, f'{task}-core.{pid}.backtrace.txt'))
        if copy:
            os.remove(core)
    else:
        print(f'pid {pid}: no core found', file=sys.stderr)


def report(chunk, cores=None, backtraces=None):
    """Writes the report files of the chunk directory, and the backtraces into backtraces or else
    chunk/backtraces. Returns the number of kit crashes and of sandbox aborts."""
    task_of = {}
    for line in open(os.path.join(chunk, 'list.tsv')):
        task, path = line.rstrip('\n').split('\t', 1)
        task_of[path] = task

    # Each pass with the source document it belongs to: a first pass starts a document, and
    # its reloads follow it.
    rows, source = [], None
    for line in open(os.path.join(chunk, 'results.tsv')):
        path, stage, outcome, _, detail = (line.rstrip('\n').split('\t') + [''] * 5)[:5]
        if stage == 'first':
            source = path
        rows.append((source, path, stage, outcome, detail))

    crashes = kit_crashes(os.path.join(chunk, 'coolwsd.log'))
    backtraces = backtraces or os.path.join(chunk, 'backtraces')
    with open(os.path.join(chunk, 'crashlog.txt'), 'w') as crashlog, \
            open(os.path.join(chunk, 'exportCrash.txt'), 'w') as export_crash, \
            open(os.path.join(chunk, 'crashes.tsv'), 'w') as table, \
            open(os.path.join(chunk, 'sandbox.tsv'), 'w') as sandbox_table:
        for pid, stage, signal, name, sandbox in crashes:
            # A crash fails its pass, so the candidates are the failed passes of that file name.
            matches = [r for r in rows if os.path.basename(r[1]) == name and r[3] != 'ok']
            if len(matches) != 1:
                print(f'pid {pid}: {len(matches)} failed passes match {name}', file=sys.stderr)
                if sandbox:
                    sandbox_table.write('\t'.join(('?', pid, stage, '', '', name or '') + sandbox)
                                        + '\n')
                else:
                    table.write('\t'.join(('?', pid, stage, signal, '', '', name or '')) + '\n')
                continue
            source, path, which, _, detail = matches[0]
            task = 'task' + task_of.get(source, '?')
            if sandbox:
                sandbox_table.write('\t'.join((task, pid, stage, source, which, path) + sandbox)
                                    + '\n')
                write_backtrace_of(pid, task, cores, chunk, backtraces)
                continue
            crashlog.write('Crash:' + path + '\n')
            if stage == 'save':
                # The detail of a failed save starts with the format of the export.
                fmt = detail.split(' ', 1)[0]
                export_crash.write(os.path.splitext(source)[0] + '.' + fmt + '\n')
            table.write('\t'.join((task, pid, stage, signal, source, which, path)) + '\n')
            write_backtrace_of(pid, task, cores, chunk, backtraces)
    aborts = sum(1 for crash in crashes if crash[4])
    return len(crashes) - aborts, aborts


def main():
    parser = argparse.ArgumentParser(description='Write the crash report of a crashtest chunk.')
    parser.add_argument('--cores', help='directory that core_pattern writes cores into')
    parser.add_argument('chunk', help='chunk directory')
    args = parser.parse_args()
    crashes, aborts = report(args.chunk, args.cores)
    print(f'{crashes} kit crashes, {aborts} sandbox aborts')


if __name__ == '__main__':
    main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
