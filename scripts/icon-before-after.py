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
"""Generate a before/after HTML comparison for an icon-update Gerrit change.

Usage:
    scripts/icon-before-after.py <gerrit-url> [--online-path DIR] [--out FILE]
                                 [--open] [--no-usage]

The script:
  1. cd's into the online checkout and runs scripts/git-review-fetch <URL>.
  2. Walks the files touched by FETCH_HEAD, keeping only image files under
     image/icon/cursor directories.
  3. For each, extracts the pre- and post-change blobs (FETCH_HEAD^ vs
     FETCH_HEAD) and embeds them in an HTML page (inline SVG markup for
     .svg; base64 data URI for bitmaps).
  4. Looks up where each icon is used in the browser/ sources (CSS class,
     url() reference, .uno: command sites) and lists those file:line hits
     under the icon. Pass --no-usage to skip this.
  5. Writes the HTML and prints its absolute path on the final stdout line.

By default the online checkout is the repository this script lives in, and
the HTML is written to the current directory.
"""

import argparse
import base64
import html
import re
import subprocess
import sys
from pathlib import Path
from typing import List, Optional, Tuple

ICON_EXTS = {'.svg', '.png', '.jpg', '.jpeg', '.gif'}
ICON_DIR_HINTS = ('image', 'icon', 'cursor')
# COOL icon files are named after a uno command with a theme prefix, e.g.
# lc_autosum.svg -> .uno:AutoSum. Strip the prefix to get the stem we grep for.
ICON_PREFIXES = ('lc_', 'sc_', 'cmd_')
# How many usage hits to list per icon before collapsing into "+N more".
USAGE_CAP = 12

# This script lives in <online>/scripts/, so the checkout root is two levels up.
DEFAULT_ONLINE_PATH = Path(__file__).resolve().parent.parent
# A bare change number is expanded against this base (the online project on
# Collabora's Gerrit) so git-review-fetch, which needs host + project, can fetch it.
DEFAULT_GERRIT_BASE = 'https://gerrit.collaboraoffice.com/c/online/+/'


def run(cmd, cwd=None, check=True, capture=True):
    return subprocess.run(
        cmd, cwd=str(cwd) if cwd else None, check=check,
        stdout=subprocess.PIPE if capture else None,
        stderr=subprocess.PIPE if capture else None,
    )


def fetch_change(online_path: Path, url: str) -> None:
    script = online_path / 'scripts' / 'git-review-fetch'
    if not script.exists():
        sys.exit(f'error: {script} not found; pass --online-path to point at a valid online checkout')
    proc = run([str(script), url], cwd=online_path, check=False)
    if proc.returncode != 0:
        sys.stderr.write(proc.stderr.decode('utf-8', errors='replace'))
        sys.exit(f'error: git-review-fetch failed for {url}')


def changed_files(online_path: Path):
    """Return list of (status, path) for files in FETCH_HEAD.

    status is 'A' (added), 'D' (deleted), 'M' (modified), 'R'/'C' etc.
    """
    proc = run(['git', 'diff-tree', '--no-commit-id', '--name-status', '-r', 'FETCH_HEAD'],
               cwd=online_path)
    out = proc.stdout.decode('utf-8', errors='replace')
    results = []
    for line in out.splitlines():
        if not line.strip():
            continue
        parts = line.split('\t')
        status = parts[0][0]
        path = parts[-1]
        results.append((status, path))
    return results


def is_icon_path(path: str) -> bool:
    p = Path(path)
    if p.suffix.lower() not in ICON_EXTS:
        return False
    parts_lower = [part.lower() for part in p.parts[:-1]]
    return any(any(hint in part for hint in ICON_DIR_HINTS) for part in parts_lower)


def git_show_blob(online_path: Path, ref: str, path: str) -> Optional[bytes]:
    proc = run(['git', 'show', f'{ref}:{path}'], cwd=online_path, check=False)
    if proc.returncode != 0:
        return None
    return proc.stdout


def inline_svg(blob: bytes) -> str:
    s = blob.decode('utf-8', errors='replace')
    s = re.sub(r'<\?xml[^?]*\?>', '', s)
    s = re.sub(r'<!DOCTYPE[^>]*>', '', s)
    return s.strip()


def data_uri(blob: bytes, suffix: str) -> str:
    mime = {
        '.png':  'image/png',
        '.jpg':  'image/jpeg',
        '.jpeg': 'image/jpeg',
        '.gif':  'image/gif',
    }.get(suffix.lower(), 'application/octet-stream')
    b64 = base64.b64encode(blob).decode('ascii')
    return f'<img alt="" src="data:{mime};base64,{b64}">'


def render_cell(blob: Optional[bytes], suffix: str, missing_label: str) -> str:
    if blob is None:
        return f'<span class="missing">{missing_label}</span>'
    if suffix.lower() == '.svg':
        return inline_svg(blob)
    return data_uri(blob, suffix)


def pretty_name(path: str) -> str:
    p = Path(path)
    name = p.name
    # Drop leading "lc_" prefix that's standard on COOL icons; keep extension stripped.
    stem = p.stem
    if stem.startswith('lc_'):
        stem = stem[3:]
    return stem


def icon_stem(path: str) -> str:
    """The command-ish base name of an icon, e.g. lc_autosum.svg -> 'autosum'."""
    stem = Path(path).stem
    for prefix in ICON_PREFIXES:
        if stem.startswith(prefix):
            return stem[len(prefix):]
    return stem


_usage_cache = {}


def find_usage(online_path: Path, stem: str) -> List[Tuple[str, str, str]]:
    """Return [(file, lineno, text)] of browser-side references to an icon.

    These icons are a *browser* asset, so a reference only counts if it lives
    under browser/ - that single scoping is what removes the noise: an
    unscoped grep for "autosum" drags in all the engine/ C++ that implements
    the AutoSum *feature*, none of which uses this SVG. We also:
      - word-bound the match (-w) so 'autosum' does not swallow 'autosummenu';
      - exclude browser/images/ (the icon files themselves) and browser/po/
        (translation catalogues, which mention every command label).
    Result is cached per stem so light + dark variants share one grep.
    """
    if not stem:
        return []
    if stem in _usage_cache:
        return _usage_cache[stem]
    proc = run(['git', 'grep', '-niw', stem, '--',
                'browser/', ':!browser/images/*', ':!browser/po/*'],
               cwd=online_path, check=False)
    hits: List[Tuple[str, str, str]] = []
    if proc.returncode == 0:  # grep returns 1 (no error output) when nothing matches
        for line in proc.stdout.decode('utf-8', errors='replace').splitlines():
            parts = line.split(':', 2)  # path:lineno:text  (text may contain colons)
            if len(parts) == 3:
                hits.append((parts[0], parts[1], parts[2].strip()))
    _usage_cache[stem] = hits
    return hits


def normalize_change(arg: str) -> Tuple[str, str]:
    """Accept either a full Gerrit change URL or a bare change number.

    Returns (url, change_number). A bare number like "1885" is expanded to the
    canonical online change URL, since git-review-fetch needs the host and
    project to fetch.
    """
    arg = arg.strip()
    if arg.isdigit():
        return DEFAULT_GERRIT_BASE + arg, arg
    m = re.search(r'/\+/(\d+)', arg)  # https://gerrit.../c/<repo>/+/<N>[/...]
    if not m:
        sys.exit(f'error: expected a Gerrit change URL or a bare change number, got {arg!r}')
    return arg, m.group(1)


def build_html(change_number: str, url: str, light_rows: List[tuple], dark_rows: List[tuple], online_path: Path, subject: str) -> str:
    def render_usage(usage):
        if usage is None:
            return ''  # --no-usage: omit the block entirely
        if not usage:
            return '<div class="usage usage-none">No browser-side references found</div>'
        items = []
        for f, ln, text in usage[:USAGE_CAP]:
            loc = html.escape(f'{f}:{ln}')
            items.append(f'<li><span class="loc">{loc}</span> <code class="src">{html.escape(text)}</code></li>')
        if len(usage) > USAGE_CAP:
            items.append(f'<li class="usage-more">+{len(usage) - USAGE_CAP} more</li>')
        return f'<div class="usage"><div class="usage-h">Used in ({len(usage)})</div><ul>{"".join(items)}</ul></div>'

    def render_rows(rows):
        out = []
        for status, path, before_html, after_html, usage in rows:
            badge = ''
            if status == 'A':
                badge = '<span class="badge badge-new">new</span>'
            elif status == 'D':
                badge = '<span class="badge badge-rm">removed</span>'
            out.append(f"""
    <tr>
      <td class="name"><code>{pretty_name(path)}</code>{badge}<div class="path">{path}</div>{render_usage(usage)}</td>
      <td class="icon"><div class="ico">{before_html}</div></td>
      <td class="icon"><div class="ico">{after_html}</div></td>
    </tr>""")
        return ''.join(out)

    def section(rows, dark):
        if not rows:
            return ''
        cls = 'dark' if dark else 'light'
        label = 'Dark theme' if dark else 'Light theme'
        body = render_rows(rows)
        return f"""
    <section class="{cls}">
      <h2>{label} <span class="count">({len(rows)})</span></h2>
      <table>
        <thead>
          <tr><th>Icon</th><th>Before</th><th>After</th></tr>
        </thead>
        <tbody>{body}
        </tbody>
      </table>
    </section>"""

    title = f'Gerrit {change_number} - {subject}' if subject else f'Gerrit {change_number}'
    return f"""<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <title>{title}</title>
  <style>
    :root {{
      font-family: system-ui, -apple-system, "Segoe UI", Roboto, sans-serif;
      color-scheme: light;
    }}
    body {{ margin: 0; padding: 2rem; background: #fafafa; color: #1a1a1a; }}
    header {{ max-width: 64rem; margin: 0 auto 2rem; }}
    header h1 {{ margin: 0 0 0.5rem; font-size: 1.4rem; }}
    header p {{ color: #555; font-size: 0.95rem; margin: 0.25rem 0; }}
    header a {{ color: #0061a8; }}
    section {{ max-width: 64rem; margin: 0 auto 3rem; padding: 1.5rem; border-radius: 12px; }}
    section.light {{ background: #ffffff; border: 1px solid #e3e3e3; }}
    section.dark  {{ background: #1f1f1f; color: #f0f0f0; border: 1px solid #333; }}
    section.dark a {{ color: #79b8ff; }}
    h2 {{ margin: 0 0 1rem; font-size: 1.15rem; }}
    .count {{ font-weight: normal; opacity: 0.6; font-size: 0.9rem; }}
    table {{ width: 100%; border-collapse: collapse; }}
    th, td {{ padding: 0.7rem 0.8rem; text-align: left; vertical-align: middle; }}
    thead th {{ font-size: 0.75rem; text-transform: uppercase; letter-spacing: 0.05em; opacity: 0.6; border-bottom: 1px solid currentColor; }}
    tbody tr + tr td {{ border-top: 1px solid rgba(127,127,127,0.2); }}
    td.name code {{ font-family: ui-monospace, "SF Mono", Menlo, Consolas, monospace; font-size: 0.9rem; font-weight: 600; }}
    td.name .path {{ font-family: ui-monospace, "SF Mono", Menlo, Consolas, monospace; font-size: 0.7rem; opacity: 0.55; margin-top: 0.15rem; word-break: break-all; }}
    td.icon {{ width: 96px; text-align: center; }}
    .ico {{ display: inline-flex; align-items: center; justify-content: center; width: 48px; height: 48px; }}
    .ico svg, .ico img {{ width: 100%; height: 100%; }}
    .missing {{ font-style: italic; opacity: 0.5; font-size: 0.85rem; }}
    .badge {{ display: inline-block; margin-left: 0.5rem; padding: 0 0.4rem; border-radius: 4px; font-size: 0.7rem; vertical-align: middle; }}
    .badge-new {{ background: #e0f5e0; color: #1d6b1d; }}
    .badge-rm  {{ background: #f7dada; color: #8b1f1f; }}
    section.dark .badge-new {{ background: #1d4d1d; color: #b6efb6; }}
    section.dark .badge-rm  {{ background: #4d1d1d; color: #f0bdbd; }}
    .usage {{ margin-top: 0.6rem; font-size: 0.72rem; }}
    .usage-h {{ font-weight: 600; opacity: 0.7; margin-bottom: 0.2rem; }}
    .usage ul {{ list-style: none; margin: 0; padding: 0; }}
    .usage li {{ margin: 0.1rem 0; line-height: 1.3; }}
    .usage .loc {{ font-family: ui-monospace, "SF Mono", Menlo, Consolas, monospace; color: #0061a8; }}
    section.dark .usage .loc {{ color: #79b8ff; }}
    .usage .src {{ font-family: ui-monospace, "SF Mono", Menlo, Consolas, monospace; opacity: 0.65; }}
    .usage-more {{ font-style: italic; opacity: 0.55; }}
    .usage-none {{ font-style: italic; opacity: 0.5; }}
  </style>
</head>
<body>
  <header>
    <h1>{title}</h1>
    <p>Change: <a href="{url}">{url}</a></p>
    <p>Online checkout: <code>{online_path}</code>. Inline SVGs rendered at 48x48; dark variants shown on a dark card.</p>
  </header>
  {section(light_rows, dark=False)}
  {section(dark_rows,  dark=True)}
</body>
</html>
"""


def main():
    ap = argparse.ArgumentParser(description='Build a before/after HTML for an icon-update Gerrit change.')
    ap.add_argument('change', help='Gerrit change URL (https://gerrit.collaboraoffice.com/c/online/+/1813) '
                                    'or a bare change number (1813)')
    ap.add_argument('--online-path', type=Path, default=DEFAULT_ONLINE_PATH,
                    help=f'path to the online checkout (default: the repo this script lives in, {DEFAULT_ONLINE_PATH})')
    ap.add_argument('--out', type=Path, default=None,
                    help='output HTML path (default: ./icon-changes-<N>.html in the current directory)')
    ap.add_argument('--open', dest='do_open', action='store_true',
                    help='open the generated HTML in the default browser via xdg-open')
    ap.add_argument('--no-usage', dest='usage', action='store_false',
                    help='skip the "Used in" code-reference lookup for each icon')
    args = ap.parse_args()

    online_path = args.online_path.resolve()
    if not online_path.is_dir():
        sys.exit(f'error: online checkout not found at {online_path}')

    url, change_number = normalize_change(args.change)

    print(f'fetching change {change_number} into {online_path} ...', file=sys.stderr)
    fetch_change(online_path, url)

    subject = run(['git', 'log', '-1', '--pretty=%s', 'FETCH_HEAD'], cwd=online_path).stdout.decode('utf-8').strip()

    files = changed_files(online_path)
    icons = [(s, p) for (s, p) in files if is_icon_path(p)]
    print(f'change touches {len(files)} files; {len(icons)} look like icons', file=sys.stderr)
    if not icons:
        sys.exit('error: no icon files (svg/png/jpg/gif under image/icon/cursor dirs) found in this change')

    light_rows = []
    dark_rows = []
    for status, path in icons:
        suffix = Path(path).suffix
        before_blob = git_show_blob(online_path, 'FETCH_HEAD^', path) if status != 'A' else None
        after_blob  = git_show_blob(online_path, 'FETCH_HEAD',  path) if status != 'D' else None
        before_html = render_cell(before_blob, suffix, '(new)')
        after_html  = render_cell(after_blob,  suffix, '(removed)')
        usage = find_usage(online_path, icon_stem(path)) if args.usage else None
        row = (status, path, before_html, after_html, usage)
        if '/dark/' in path or path.startswith('dark/'):
            dark_rows.append(row)
        else:
            light_rows.append(row)

    out_path = args.out
    if out_path is None:
        out_path = Path.cwd() / f'icon-changes-{change_number}.html'
    out_path = out_path.resolve()
    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text(build_html(change_number, url, light_rows, dark_rows, online_path, subject), encoding='utf-8')

    if args.do_open:
        subprocess.Popen(['xdg-open', str(out_path)],
                         stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
                         start_new_session=True)

    # Last line of stdout is the absolute path of the generated file.
    print(out_path)


if __name__ == '__main__':
    main()
