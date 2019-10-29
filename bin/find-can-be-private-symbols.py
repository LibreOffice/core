#!/usr/bin/python
#
# Find exported symbols that can be made private.
#
# Noting that (a) parsing these commands is a pain, the output is quite irregular and (b) I'm fumbling in the
# dark here, trying to guess what exactly constitutes an "import" vs an "export" of a symbol, linux linking
# is rather complex.

import subprocess
import sys
import re

exported_symbols = set()
imported_symbols = set()

subprocess_find = subprocess.Popen("find ./instdir -name *.so", stdout=subprocess.PIPE, shell=True)
with subprocess_find.stdout as txt:
    for line in txt:
        sharedlib = line.strip()
        # look for exported symbols
        subprocess_nm = subprocess.Popen("nm -D " + sharedlib, stdout=subprocess.PIPE, shell=True)
        with subprocess_nm.stdout as txt2:
            # We are looking for lines something like:
            # 0000000000036ed0 T flash_component_getFactory
            line_regex = re.compile(r'^[0-9a-fA-F]+ T ')
            for line2 in txt2:
                line2 = line2.strip()
                if line_regex.match(line2):
                    exported_symbols.add(line2.split(" ")[2])
        # look for imported symbols
        subprocess_objdump = subprocess.Popen("objdump -T " + sharedlib, stdout=subprocess.PIPE, shell=True)
        with subprocess_objdump.stdout as txt2:
            # ignore some header bumpf
            txt2.readline()
            txt2.readline()
            txt2.readline()
            txt2.readline()
            # We are looking for lines something like:
            # 0000000000000000      DF *UND*  0000000000000000     _ZN16FilterConfigItem10WriteInt32ERKN3rtl8OUStringEi
            for line2 in txt2:
                line2 = line2.strip()
                tokens = line2.split(" ")
                if len(tokens) < 7 or not(tokens[7].startswith("*UND*")): continue
                sym = tokens[len(tokens)-1]
                imported_symbols.add(sym)


# look for imported symbols in executables
subprocess_find = subprocess.Popen("find ./instdir -name *.bin", stdout=subprocess.PIPE, shell=True)
with subprocess_find.stdout as txt:
    for line in txt:
        executable = line.strip()
        # look for exported symbols
        subprocess_nm = subprocess.Popen("nm -D " + executable + " | grep -w U", stdout=subprocess.PIPE, shell=True)
        with subprocess_nm.stdout as txt2:
            # We are looking for lines something like:
            # U sal_detail_deinitialize
            for line2 in txt2:
                line2 = line2.strip()
                sym = line2.split(" ")[1]
                imported_symbols.add(sym)

diff = exported_symbols - imported_symbols
print("exported = " + str(len(exported_symbols)))
print("imported = " + str(len(imported_symbols)))
print("diff     = " + str(len(diff)))
# todo process these with c++filt
#for sym in diff:
#    if "Sd" in sym:
#        print sym
