#!/usr/bin/python3

# Look for CXX files that are not referenced by any makefile

import subprocess

sourceFiles = set()

a = subprocess.Popen("git ls-files", stdout=subprocess.PIPE, shell=True, encoding='utf8')
with a.stdout as txt:
    for filename in txt:
        if filename.find(".cxx") != -1 \
            and filename.find("precompiled") == -1 \
            and filename.find("/workben") == -1 \
            and not filename.startswith("odk/examples/") \
            and not filename.startswith("bridges/") \
            and not filename.startswith("compilerplugins/") \
            and filename.find("/qa/") == -1 \
            and filename.find("/test/") == -1 \
            and not filename.startswith("testtools/") \
            and not filename.startswith("vcl/") \
            and not filename.startswith("cli_ure/"):
            sourceFiles.add(filename.strip())

a = subprocess.Popen("git ls-files */*.mk", stdout=subprocess.PIPE, shell=True, encoding='utf8')
with a.stdout as txt:
    for makefilename in txt:
        makefilename = makefilename.strip()
        with open(makefilename, "r") as makefile:
            moduleName = makefilename[:makefilename.find("/")]
            state = 0
            for line in makefile:
                line = line.strip()
                if state == 0 and "_add_exception_objects" in line:
                    state = 1
                elif state == 1 and line != "))":
                    s = line.replace("\\","").replace(")", "").strip()
                    # parse line like: $(call gb_Helper_optional,AVMEDIA,svx/source/sidebar/media/MediaPlaybackPanel) \
                    idx = s.rfind(",")
                    if idx != -1:
                        s = s[idx+1:].strip()
                    sourceFiles.discard(s + ".cxx")
                elif state == 1:
                    state = 0




print("files not listed in makefile")
print("----------------------------")
for x in sorted(sourceFiles):
    print(x)
