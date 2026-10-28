#!/usr/bin/env python3
#
# Copyright the Collabora Online contributors.
#
# SPDX-License-Identifier: Apache-2.0
#
"""Compare and synchronize the open-sourced branding with online-branding.

During the transition the Collabora Online, CODE and Collabora Office
branding lives both here (browser/branding/) and in the online-branding
repository.  This tool knows which files correspond and how the two copies
are allowed to differ:

- SVG files here carry no "(C) Collabora Productivity, All Rights Reserved"
  notice,
- the helper scripts here start with a license header.

Everything is compared after removing those differences, so the reported
changes are real ones.  Pair the branches: main here with master there,
distro/collabora/co-XX.YY with the branch of the same name.

  sync-online-branding.py [-b DIR] status             list differing files
  sync-online-branding.py [-b DIR] diff [PATH...]     show the differences
  sync-online-branding.py [-b DIR] pull [-n]          online-branding -> here
  sync-online-branding.py [-b DIR] push [-n]          here -> online-branding
  sync-online-branding.py [-b DIR] compare-output THEME
                     run both brand.sh scripts and compare what they generate

DIR defaults to $ONLINE_BRANDING_DIR.  pull and push only touch the mapped
files; a file that exists on one side only is copied, never deleted, and
reported so that the deletion can be done by hand if it was intended.
"""

import argparse
import difflib
import fnmatch
import os
import re
import shutil
import subprocess
import sys
import tempfile

TREE_ROOT = os.path.normpath(
    os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..")
)

# (directory in online-branding, directory here, include globs, exclude globs,
# globs of files that may exist in online-branding only)
# The globs match paths relative to the mapped directory.  The Nextcloud and
# the customer brands stay in online-branding only, and so does the unused
# CODE stylesheet.  The CODA theme (themes/CODA) exists only here.
#
# Only the images that the generated CSS references are kept here, so an image
# that exists in online-branding only is not reported; brand.sh copies the
# referenced images, and compare-output shows when one is missing here.
MAPPING = [
    (
        "sass",
        "browser/branding/sass",
        ["*", ".stylelintrc"],
        [
            "def/_CODE.scss",
            "def/_NC-theme.scss",
            "def/_NC-icons.scss",
            "def/Magentacloud-icons.scss",
            "def/_ClaroDrive-theme-1.1.scss",
            "def/_ContentEditor-theme.scss",
            "*.bak",
        ],
        [],
    ),
    (
        "themes/collabora-online-brand",
        "browser/branding/themes/collabora-online-brand",
        [
            "branding.js",
            "branding-unsupported.js",
            "images/*",
            "styleguide/x-office-*.svg",
        ],
        ["images/es/*", "*.bak"],
        ["images/*"],
    ),
    ("themes/CODE", "browser/branding/themes/CODE", ["branding.js"], [], []),
    (
        ".",
        "browser/branding",
        ["generate-document-icons.sh"],
        [],
        [],
    ),
]

NOTICE_RE = re.compile(
    rb"<!-- \(C\) Collabora Productivity (\d{4}), All Rights Reserved -->"
)
NOTICE_LINE_RE = re.compile(
    rb"^[ \t]*<!-- \(C\) Collabora Productivity \d{4}, All Rights Reserved -->[ \t]*\r?\n",
    re.MULTILINE,
)

# The license header of a shell or Python script here, right after the
# shebang line.
SCRIPT_HEADER_RE = re.compile(
    rb"\A(#![^\n]*\n)#\n# Copyright the Collabora Online contributors\.\n#\n"
    rb"# SPDX-License-Identifier: [^\n]*\n#\n"
)


def strip_notice(data):
    """Remove the copyright notice from an SVG."""
    data = NOTICE_LINE_RE.sub(b"", data)
    return NOTICE_RE.sub(b"", data)


def strip_script_header(data):
    return SCRIPT_HEADER_RE.sub(rb"\1", data, count=1)


def is_script(path):
    return path.endswith((".sh", ".py"))


def to_tree(path, data, current):
    """Turn online-branding content into what is committed here.

    current is the file here (or None); its license header is kept."""
    if path.endswith(".svg"):
        return strip_notice(data)
    if is_script(path) and current is not None:
        match = SCRIPT_HEADER_RE.match(current)
        if match and data.startswith(match.group(1)):
            return match.group(0) + data[len(match.group(1)) :]
    return data


def to_branding(path, data, current):
    """Turn content from here into online-branding content.

    current is the file there (or None); its notice is kept, so that a push
    only brings over the real change."""
    if is_script(path):
        return strip_script_header(data)
    if path.endswith(".svg") and current is not None:
        match = NOTICE_RE.search(current)
        if match and strip_notice(current) == data:
            return current
        if match and b"</svg>" in data:
            head, _, tail = data.rpartition(b"</svg>")
            return head + b"\n" + match.group(0) + b"\n</svg>" + tail
    return data


def normalize(path, data):
    """The form in which the two sides must be equal."""
    if path.endswith(".svg"):
        return strip_notice(data)
    if is_script(path):
        return strip_script_header(data)
    return data


def matches(rel, includes, excludes):
    if any(fnmatch.fnmatch(rel, pattern) for pattern in excludes):
        return False
    return any(fnmatch.fnmatch(rel, pattern) for pattern in includes)


def mapped_files(root, base, includes, excludes):
    """Relative paths under root/base that the mapping covers."""
    top = os.path.join(root, base)
    found = set()
    if not os.path.isdir(top):
        return found
    for dirpath, dirnames, filenames in os.walk(top):
        dirnames[:] = [d for d in dirnames if d not in (".git", "node_modules")]
        for name in filenames:
            rel = os.path.relpath(os.path.join(dirpath, name), top)
            rel = rel.replace(os.sep, "/")
            if matches(rel, includes, excludes):
                found.add(rel)
    return found


def read(path):
    try:
        with open(path, "rb") as f:
            return f.read()
    except FileNotFoundError:
        return None


def write(path, data):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "wb") as f:
        f.write(data)


def compare(branding):
    """Yield (state, branding path, tree path) for every mapped file."""
    for src, dst, includes, excludes, optional in MAPPING:
        left = mapped_files(branding, src, includes, excludes)
        right = mapped_files(TREE_ROOT, dst, includes, excludes)
        for rel in sorted(left | right):
            bpath = os.path.normpath(os.path.join(branding, src, rel))
            tpath = os.path.normpath(os.path.join(TREE_ROOT, dst, rel))
            if rel not in right:
                if any(fnmatch.fnmatch(rel, pattern) for pattern in optional):
                    continue
                yield "only-branding", bpath, tpath
            elif rel not in left:
                yield "only-tree", bpath, tpath
            else:
                a = normalize(bpath, read(bpath))
                b = normalize(tpath, read(tpath))
                yield ("same" if a == b else "differ"), bpath, tpath


def git_branch(directory):
    try:
        return subprocess.run(
            ["git", "-C", directory, "rev-parse", "--abbrev-ref", "HEAD"],
            capture_output=True,
            text=True,
            check=True,
        ).stdout.strip()
    except (subprocess.CalledProcessError, FileNotFoundError):
        return "?"


def check_branches(branding):
    here, there = git_branch(TREE_ROOT), git_branch(branding)
    if here == "main":
        expected = "master"
    elif here.startswith("distro/"):
        expected = here
    else:
        return
    if there != expected:
        print(
            f"warning: comparing {here} here with {there} in online-branding, "
            f"expected {expected} there",
            file=sys.stderr,
        )


def cmd_status(args):
    check_branches(args.branding)
    labels = {
        "differ": "M",
        "only-branding": "B",
        "only-tree": "T",
    }
    count = 0
    for state, bpath, tpath in compare(args.branding):
        if state == "same":
            continue
        count += 1
        print(f"{labels[state]} {os.path.relpath(tpath, TREE_ROOT)}")
    if count:
        print(
            "\nM: differs, B: only in online-branding, T: only here",
            file=sys.stderr,
        )
    return 1 if count else 0


def cmd_diff(args):
    check_branches(args.branding)
    wanted = [os.path.abspath(p) for p in args.paths]
    for state, bpath, tpath in compare(args.branding):
        if state == "same":
            continue
        if wanted and not any(
            p in (bpath, tpath) or tpath.startswith(p + os.sep) for p in wanted
        ):
            continue
        a = normalize(bpath, read(bpath) or b"")
        b = normalize(tpath, read(tpath) or b"")
        rel_b = os.path.relpath(bpath, args.branding)
        rel_t = os.path.relpath(tpath, TREE_ROOT)
        try:
            lines_a = a.decode("utf-8").splitlines(keepends=True)
            lines_b = b.decode("utf-8").splitlines(keepends=True)
        except UnicodeDecodeError:
            print(f"Binary files online-branding/{rel_b} and {rel_t} differ")
            continue
        sys.stdout.writelines(
            difflib.unified_diff(
                lines_a, lines_b, f"online-branding/{rel_b}", rel_t
            )
        )
    return 0


def cmd_pull(args):
    check_branches(args.branding)
    for state, bpath, tpath in compare(args.branding):
        rel = os.path.relpath(tpath, TREE_ROOT)
        if state == "differ":
            print(f"{'would update' if args.dry_run else 'update'} {rel}")
            if not args.dry_run:
                write(tpath, to_tree(bpath, read(bpath), read(tpath)))
        elif state == "only-branding":
            print(f"only in online-branding, not touched: {rel}")
        elif state == "only-tree":
            print(f"only here, not touched: {rel}")
    return 0


def cmd_push(args):
    check_branches(args.branding)
    for state, bpath, tpath in compare(args.branding):
        rel = os.path.relpath(bpath, args.branding)
        if state in ("differ", "only-tree"):
            print(f"{'would update' if args.dry_run else 'update'} {rel}")
            if not args.dry_run:
                write(bpath, to_branding(bpath, read(tpath), read(bpath)))
        elif state == "only-branding":
            print(f"only in online-branding, not touched: {rel}")
    return 0


HEADER_RE = re.compile(rb"\A/\*[^\n]*\*/\n\n?")


def normalize_output(path, data):
    """Generated files differ in their first-line header comment only."""
    if path.endswith((".css", ".js")):
        return HEADER_RE.sub(b"", data, count=1)
    if path.endswith(".svg"):
        return strip_notice(data)
    return data


def list_tree(top):
    found = set()
    for dirpath, _, filenames in os.walk(top):
        for name in filenames:
            found.add(os.path.relpath(os.path.join(dirpath, name), top))
    return found


def cmd_compare_output(args):
    check_branches(args.branding)
    sass_bin = os.path.join(TREE_ROOT, "browser", "node_modules", ".bin")
    env = dict(os.environ)
    if os.path.exists(os.path.join(sass_bin, "sass")):
        env["PATH"] = sass_bin + os.pathsep + env["PATH"]
    with tempfile.TemporaryDirectory() as tmp:
        # brand.sh in online-branding edits its own sass files, run a copy
        copy = os.path.join(tmp, "online-branding")
        shutil.copytree(
            args.branding,
            copy,
            ignore=shutil.ignore_patterns(".git", "node_modules"),
        )
        results = {}
        for side, cmd, cwd, with_instdir in (
            ("online-branding", ["bash", "./brand.sh"], copy, True),
            (
                "tree",
                ["bash", os.path.join(TREE_ROOT, "browser", "branding", "brand.sh")],
                tmp,
                False,
            ),
        ):
            instdir = os.path.join(tmp, side, "instdir")
            outdir = os.path.join(tmp, side, "dist")
            os.makedirs(instdir)
            os.makedirs(outdir)
            argv = cmd + ([instdir] if with_instdir else []) + [outdir, args.theme]
            run = subprocess.run(
                argv,
                cwd=cwd,
                env=env,
                capture_output=True,
                text=True,
            )
            if run.returncode != 0:
                sys.stdout.write(run.stdout)
                sys.stderr.write(run.stderr)
                print(f"brand.sh failed in {side}", file=sys.stderr)
                return 2
            results[side] = (instdir, outdir)

        differences = 0
        for kind in (0, 1):
            left = results["online-branding"][kind]
            right = results["tree"][kind]
            lfiles, rfiles = list_tree(left), list_tree(right)
            for rel in sorted(lfiles | rfiles):
                label = ("instdir/" if kind == 0 else "dist/") + rel
                if rel not in rfiles:
                    print(f"only from online-branding: {label}")
                elif rel not in lfiles:
                    print(f"only from the tree: {label}")
                else:
                    a = normalize_output(rel, read(os.path.join(left, rel)))
                    b = normalize_output(rel, read(os.path.join(right, rel)))
                    if a == b:
                        continue
                    print(f"differs: {label}")
                differences += 1
        if differences:
            print(f"{differences} difference(s) for theme {args.theme}")
            return 1
        print(f"identical output for theme {args.theme}")
        return 0


def main():
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    parser.add_argument(
        "-b",
        "--branding",
        default=os.environ.get("ONLINE_BRANDING_DIR"),
        help="online-branding checkout (default: $ONLINE_BRANDING_DIR)",
    )
    sub = parser.add_subparsers(dest="command", required=True)
    sub.add_parser("status").set_defaults(func=cmd_status)
    p = sub.add_parser("diff")
    p.add_argument("paths", nargs="*")
    p.set_defaults(func=cmd_diff)
    for name, func in (("pull", cmd_pull), ("push", cmd_push)):
        p = sub.add_parser(name)
        p.add_argument("-n", "--dry-run", action="store_true")
        p.set_defaults(func=func)
    p = sub.add_parser("compare-output")
    p.add_argument("theme", help="CO-theme, CODE or CODA")
    p.set_defaults(func=cmd_compare_output)
    args = parser.parse_args()
    if not args.branding or not os.path.isdir(args.branding):
        parser.error("point --branding or $ONLINE_BRANDING_DIR at an online-branding checkout")
    args.branding = os.path.abspath(args.branding)
    return args.func(args)


if __name__ == "__main__":
    sys.exit(main())
