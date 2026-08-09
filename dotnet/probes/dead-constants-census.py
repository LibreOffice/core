#!/usr/bin/env python3
"""Census: named record/sprm/element constants that nothing consumes.

The shape this looks for is the one that hid `w:sym` and `sprmCSymbol` for a corpus's
worth of documents: a reader *declares* it knows about a thing — a sprm identifier, a
record id, a model property — and no code anywhere reads the declaration. The parse looks
like support, and the constant's own doc comment reads like a specification.

`sprmCFSpec` was found this way: declared in `Ww8Sprm.Ids`, referenced by no file at all.
"""
import re, os, sys, collections

ROOT = sys.argv[1] if len(sys.argv) > 1 else \
    "/home/user/libreoffice-core/.claude/worktrees/words-r38/dotnet"

CONST = re.compile(
    r'^\s*(?:public|internal|private)\s+const\s+(?:ushort|uint|short|int|byte|string|char)\s+'
    r'([A-Za-z_]\w*)\s*=')

def files():
    out = []
    for base in ('src', 'tests', 'tools'):
        for dp, _, fns in os.walk(os.path.join(ROOT, base)):
            if '/obj/' in dp or '/bin/' in dp:
                continue
            out.extend(os.path.join(dp, f) for f in fns if f.endswith('.cs'))
    return out

def main():
    paths = files()
    texts = {p: open(p, encoding='utf-8', errors='replace').read() for p in paths}

    decls = collections.defaultdict(list)   # name -> [file]
    for p, t in texts.items():
        for line in t.splitlines():
            m = CONST.match(line)
            if m:
                decls[m.group(1)].append(p)

    # One pass over every file, counting every identifier once, instead of one regex
    # scan per constant: the naive form is O(constants x files) and does not finish.
    word = re.compile(r'[A-Za-z_]\w*')
    cref = re.compile(r'cref="([A-Za-z_]\w*)"')
    counts = collections.Counter()
    own = collections.Counter()
    for p, t in texts.items():
        here = collections.Counter(word.findall(t))
        counts.update(here)
        for name, w in decls.items():
            if w[0] == p and name in here:
                own[name] += 1 + len(re.findall('cref="' + re.escape(name) + '"', t))

    dead = []
    for name, where in sorted(decls.items()):
        if len(where) > 1:
            continue                        # a name declared twice is ambiguous to count
        if counts[name] - own.get(name, 0) <= 0:
            dead.append((name, os.path.relpath(where[0], ROOT)))

    print(f"constants declared once and referenced nowhere: {len(dead)}")
    by_lib = collections.Counter(d[1].split('/')[1] for d in dead)
    for lib, n in by_lib.most_common():
        print(f"  {lib:28s} {n}")
    print()
    for name, where in dead:
        print(f"{name}\t{where}")

if __name__ == '__main__':
    main()
