#!/usr/bin/env python3
"""Census: public entry points in Paperless.Core / Paperless.Text that no consumer references.

A 'consumer' is any .cs file outside the declaring project. Tests are counted separately,
because a type referenced only by its own unit test is exactly the 'two halves with nothing
joining them' shape.
"""
import re, sys, os, collections

ROOT = "/home/user/libreoffice-core/.claude/worktrees/words-r38/dotnet"
PROJECTS = ["src/Paperless.Core", "src/Paperless.Text"]

TYPE_RE = re.compile(
    r'^\s*public\s+(?:static\s+|sealed\s+|abstract\s+|partial\s+|readonly\s+|ref\s+)*'
    r'(class|struct|record|interface|enum)\s+(?:class\s+|struct\s+)?([A-Za-z_]\w*)')
MEMBER_RE = re.compile(
    r'^\s{4}public\s+(?:static\s+|virtual\s+|override\s+|sealed\s+|abstract\s+|readonly\s+|new\s+|partial\s+|async\s+|extern\s+|unsafe\s+)*'
    r'[\w<>\[\],\.\?\s]+?\s([A-Za-z_]\w*)\s*(?:\(|\{|=>|$)')

def collect():
    decls = {}   # name -> (kind, project, file)
    members = collections.defaultdict(list)  # (project,file,typename) -> [member names]
    for proj in PROJECTS:
        for dirpath, _, files in os.walk(os.path.join(ROOT, proj)):
            if '/obj/' in dirpath or '/bin/' in dirpath: continue
            for f in files:
                if not f.endswith('.cs'): continue
                p = os.path.join(dirpath, f)
                cur_type = None
                for line in open(p, encoding='utf-8', errors='replace'):
                    m = TYPE_RE.match(line)
                    if m:
                        cur_type = m.group(2)
                        decls.setdefault(cur_type, (m.group(1), proj, p))
                        continue
                    mm = MEMBER_RE.match(line)
                    if mm and cur_type:
                        nm = mm.group(1)
                        if nm in ('get','set','init','if','return','new','var','where','when'): continue
                        members[(proj, cur_type)].append((nm, p))
    return decls, members

def all_cs():
    out = []
    for base in ('src','tests','tools'):
        for dirpath, _, files in os.walk(os.path.join(ROOT, base)):
            if '/obj/' in dirpath or '/bin/' in dirpath: continue
            for f in files:
                if f.endswith('.cs'):
                    out.append(os.path.join(dirpath, f))
    return out

def main():
    decls, members = collect()
    files = all_cs()
    texts = {p: open(p, encoding='utf-8', errors='replace').read() for p in files}

    print("=== public TYPES declared in Core/Text with no consumer outside their project ===")
    for name, (kind, proj, declfile) in sorted(decls.items()):
        pat = re.compile(r'\b' + re.escape(name) + r'\b')
        prod, test, ownproj = [], [], []
        for p, t in texts.items():
            if p == declfile: continue
            if not pat.search(t): continue
            rel = os.path.relpath(p, ROOT)
            if rel.startswith(proj.replace('src/','src/')) and rel.startswith(proj):
                ownproj.append(rel)
            elif rel.startswith('tests/'):
                test.append(rel)
            else:
                prod.append(rel)
        if not prod:
            tag = "TESTS-ONLY" if test else ("OWN-PROJECT-ONLY" if ownproj else "NOBODY")
            print(f"{tag}\t{kind}\t{name}\t{os.path.relpath(declfile, ROOT)}\towntouches={len(ownproj)} tests={len(test)}")

if __name__ == '__main__':
    main()
