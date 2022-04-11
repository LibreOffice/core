#!/usr/bin/python2
#
# Generate a custom linker script/map file for the --enabled-mergedlibs merged library
# which reduces the startup time and enables further optimisations with --enable-lto because 60% or more
# of the symbols become internal only.
#

import subprocess
import sys
import re
import multiprocessing

exported_symbols = set()
imported_symbols = set()


# Copied from solenv/gbuild/extensions/pre_MergedLibsList.mk
# TODO there has to be a way to run gmake and get it to dump this list for me
merged_libs = { \
    "avmedia" \
    ,"basctl" \
    ,"basprov" \
    ,"basegfx" \
    ,"canvasfactory" \
    ,"canvastools" \
    ,"comphelper" \
    ,"configmgr" \
    ,"cppcanvas" \
    ,"crashreport)" \
    ,"dbtools" \
    ,"deployment" \
    ,"deploymentmisc" \
    ,"desktopbe1)" \
    ,"desktop_detector)" \
    ,"drawinglayer" \
    ,"editeng" \
    ,"expwrap" \
    ,"filterconfig" \
    ,"fsstorage" \
    ,"fwk" \
    ,"helplinker)" \
    ,"i18npool" \
    ,"i18nutil" \
    ,"lng" \
    ,"localebe1" \
    ,"msfilter" \
    ,"mtfrenderer" \
    ,"opencl" \
    ,"package2" \
    ,"sax" \
    ,"sb" \
    ,"simplecanvas" \
    ,"sfx" \
    ,"sofficeapp" \
    ,"sot" \
    ,"spl" \
    ,"stringresource" \
    ,"svl" \
    ,"svt" \
    ,"svx" \
    ,"svxcore" \
    ,"tk" \
    ,"tl" \
    ,"ucb1" \
    ,"ucbhelper" \
    ,"ucpexpand1" \
    ,"ucpfile1" \
    ,"unoxml" \
    ,"utl" \
    ,"uui" \
    ,"vcl" \
    ,"xmlscript" \
    ,"xo" \
    ,"xstor" }

# look for symbols exported by libmerged
subprocess_nm = subprocess.Popen("nm -D instdir/program/libmergedlo.so", stdout=subprocess.PIPE, shell=True)
with subprocess_nm.stdout as txt:
    # We are looking for lines something like:
    # 0000000000036ed0 T flash_component_getFactory
    line_regex = re.compile(r'^[0-9a-fA-F]+ T ')
    for line in txt:
        line = line.strip()
        if line_regex.match(line):
            exported_symbols.add(line.split(" ")[2])
subprocess_nm.terminate()

# look for symbols imported from libmerged
subprocess_find = subprocess.Popen("(find instdir/program/ -type f; ls ./workdir/LinkTarget/CppunitTest/*.so) | xargs grep -l mergedlo",
        stdout=subprocess.PIPE, shell=True)
with subprocess_find.stdout as txt:
    for line in txt:
        sharedlib = line.strip()
        s = sharedlib[sharedlib.find("/lib") + 4 : len(sharedlib) - 3]
        if s in merged_libs: continue
        # look for imported symbols
        subprocess_objdump = subprocess.Popen("objdump -T " + sharedlib, stdout=subprocess.PIPE, shell=True)
        with subprocess_objdump.stdout as txt2:
            # ignore some header bumpf
            txt2.readline()
            txt2.readline()
            txt2.readline()
            txt2.readline()
            # We are looking for lines something like (noting that one of them uses spaces, and the other tabs)
            # 0000000000000000      DF *UND*  0000000000000000     _ZN16FilterConfigItem10WriteInt32ERKN3rtl8OUStringEi
            for line2 in txt2:
                line2 = line2.strip()
                if line2.find("*UND*") == -1: continue
                tokens = line2.split(" ")
                sym = tokens[len(tokens)-1].strip()
                imported_symbols.add(sym)
        subprocess_objdump.terminate()
subprocess_find.terminate()

intersec_symbols = exported_symbols.intersection(imported_symbols)
print("no symbols exported from libmerged   = " + str(len(exported_symbols)))
print("no symbols that can be made internal = " + str(len(intersec_symbols)))

# Now look for classes where none of the class symbols are imported,
# i.e. we can mark the whole class as hidden

def extract_class(sym):
    filtered_sym = subprocess.check_output(["c++filt", sym]).strip()
    if filtered_sym.startswith("vtable for "):
        classname = filtered_sym[11:]
        return classname
    if filtered_sym.startswith("non-virtual thunk to "):
        filtered_sym = filtered_sym[21:]
    elif filtered_sym.startswith("virtual thunk to "):
        filtered_sym = filtered_sym[17:]
    i = filtered_sym.find("(")
    if i != -1:
        i = filtered_sym.rfind("::", 0, i)
        if i != -1:
            classname = filtered_sym[:i]
            return classname
    return ""

pool = multiprocessing.Pool(multiprocessing.cpu_count())
classes_with_exported_symbols = set(pool.map(extract_class, list(exported_symbols)))
classes_with_imported_symbols = set(pool.map(extract_class, list(imported_symbols)))

# Some stuff is particular to Windows, so won't be found by a Linux analysis, so remove
# those classes.
can_be_private_classes = classes_with_exported_symbols - classes_with_imported_symbols;
can_be_private_classes.discard("SpinField")

with open("bin/find-mergedlib-can-be-private.classes.results", "wt") as f:
    for sym in sorted(can_be_private_classes):
        if sym.startswith("std::") or sym.startswith("void std::"): continue
        f.write(sym + "\n")
