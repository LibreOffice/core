#!/usr/bin/python3
#
# Find exported symbols that can be made non-exported.
#
# Noting that (a) parsing these commands is a pain, the output is quite irregular and (b) I'm fumbling in the
# dark here, trying to guess what exactly constitutes an "import" vs an "export" of a symbol, linux linking
# is rather complex.
#
# Takes about 5min to run on a decent machine.
#
# The standalone function analysis is reasonable reliable, but the class/method analysis is less so
#   (something to do with destructor thunks not showing up in my results?)
#
# Also, the class/method analysis will not catch problems like
#    'dynamic_cast from 'Foo' with hidden type visibility to 'Bar' with default type visibility'
#    but loplugin:dyncastvisibility will do that for you
#

import subprocess
import sys
import re

exported_symbols = set()
imported_symbols = set()
# standalone functions that are exported but not imported
unused_function_exports = set()
classes_with_exported_symbols = set()
classes_with_imported_symbols = set()
# all names that exist in the source code
all_source_names = set()


subprocess_find_all_source_names = subprocess.Popen("git grep -oh -P '\\b\\w\\w\\w+\\b' -- '*.h*' | sort -u", stdout=subprocess.PIPE, stderr=subprocess.STDOUT, shell=True)
with subprocess_find_all_source_names.stdout as txt:
    for line in txt:
        line = line.strip()
        all_source_names.add(line)
subprocess_find_all_source_names.terminate()

# find all our shared libs
subprocess_find = subprocess.Popen("find ./instdir -name *.so && find ./workdir/LinkTarget/CppunitTest -name *.so", stdout=subprocess.PIPE, shell=True)
with subprocess_find.stdout as txt:
    for line in txt:
        sharedlib = line.strip()
        # look for exported symbols
        subprocess_nm = subprocess.Popen(b"nm -D " + sharedlib, stdout=subprocess.PIPE, shell=True)
        with subprocess_nm.stdout as txt2:
            # We are looking for lines something like:
            # 0000000000036ed0 T flash_component_getFactory
            line_regex = re.compile(r'^[0-9a-fA-F]+ T ')
            for line2_bytes in txt2:
                line2 = line2_bytes.strip().decode("utf-8")
                if line_regex.match(line2):
                    sym = line2.split(" ")[2]
                    exported_symbols.add(sym)
        # look for imported symbols
        subprocess_objdump = subprocess.Popen(b"objdump -T " + sharedlib, stdout=subprocess.PIPE, shell=True)
        with subprocess_objdump.stdout as txt2:
            # ignore some header bumpf
            txt2.readline()
            txt2.readline()
            txt2.readline()
            txt2.readline()
            # We are looking for lines something like:
            # 0000000000000000      DF *UND*  0000000000000000     _ZN16FilterConfigItem10WriteInt32ERKN3rtl8OUStringEi
            for line2_bytes in txt2:
                line2 = line2_bytes.strip().decode("utf-8")
                tokens = line2.split(" ")
                if len(tokens) < 7 or not(tokens[7].startswith("*UND*")): continue
                sym = tokens[len(tokens)-1]
                imported_symbols.add(sym)
subprocess_find.terminate()

# look for imported symbols in executables
subprocess_find = subprocess.Popen("find ./instdir -name *.bin", stdout=subprocess.PIPE, shell=True)
with subprocess_find.stdout as txt:
    for line in txt:
        executable = line.strip()
        # look for exported symbols
        subprocess_nm = subprocess.Popen(b"nm -D " + executable + b" | grep -w U", stdout=subprocess.PIPE, shell=True)
        with subprocess_nm.stdout as txt2:
            # We are looking for lines something like:
            # U sal_detail_deinitialize
            for line2_bytes in txt2:
                line2 = line2_bytes.strip().decode("utf-8")
                sym = line2.split(" ")[1]
                imported_symbols.add(sym)
subprocess_find.terminate()

diff = exported_symbols - imported_symbols
print("exported = " + str(len(exported_symbols)))
print("imported = " + str(len(imported_symbols)))
print("diff     = " + str(len(diff)))

for sym in exported_symbols:
    filtered_sym = subprocess.check_output(["c++filt", sym]).strip().decode("utf-8")
    if filtered_sym.startswith("non-virtual thunk to "): filtered_sym = filtered_sym[21:]
    elif filtered_sym.startswith("virtual thunk to "): filtered_sym = filtered_sym[17:]
    i = filtered_sym.find("(")
    i = filtered_sym.rfind("::", 0, i)
    if i != -1:
        classname = filtered_sym[:i]
        # find classes where all of the exported symbols are not imported
        classes_with_exported_symbols.add(classname)
    else:
        func = filtered_sym
        # find standalone functions which are exported but not imported
        if not(sym in imported_symbols): unused_function_exports.add(func)

for sym in imported_symbols:
    filtered_sym = subprocess.check_output(["c++filt", sym]).strip().decode("utf-8")
    if filtered_sym.startswith("non-virtual thunk to "): filtered_sym = filtered_sym[21:]
    elif filtered_sym.startswith("virtual thunk to "): filtered_sym = filtered_sym[17:]
    i = filtered_sym.find("(")
    i = filtered_sym.rfind("::", 0, i)
    if i != -1:
        classname = filtered_sym[:i]
        classes_with_imported_symbols.add(classname)

def extractFunctionNameFromSignature(sym):
    i = sym.find("(")
    if i == -1: return sym
    return sym[:i]

with open("bin/find-can-be-private-symbols.functions.results", "wt") as f:
    for sym in sorted(unused_function_exports):
        # Filter out most of the noise.
        # No idea where these are coming from, but not our code.
        if sym.startswith("CERT_"): continue
        elif sym.startswith("DER_"): continue
        elif sym.startswith("FORM_"): continue
        elif sym.startswith("FPDF"): continue
        elif sym.startswith("HASH_"): continue
        elif sym.startswith("Hunspell_"): continue
        elif sym.startswith("LL_"): continue
        elif sym.startswith("LP_"): continue
        elif sym.startswith("LU"): continue
        elif sym.startswith("MIP"): continue
        elif sym.startswith("MPS"): continue
        elif sym.startswith("NSS"): continue
        elif sym.startswith("NSC_"): continue
        elif sym.startswith("PK11"): continue
        elif sym.startswith("PL_"): continue
        elif sym.startswith("PQ"): continue
        elif sym.startswith("PBE_"): continue
        elif sym.startswith("PORT_"): continue
        elif sym.startswith("PRP_"): continue
        elif sym.startswith("PR_"): continue
        elif sym.startswith("PT_"): continue
        elif sym.startswith("QS_"): continue
        elif sym.startswith("REPORT_"): continue
        elif sym.startswith("RSA_"): continue
        elif sym.startswith("SEC"): continue
        elif sym.startswith("SGN"): continue
        elif sym.startswith("SOS"): continue
        elif sym.startswith("SSL_"): continue
        elif sym.startswith("VFY_"): continue
        elif sym.startswith("_PR_"): continue
        elif sym.startswith("_"): continue
        elif sym.startswith("ber_"): continue
        elif sym.startswith("bfp_"): continue
        elif sym.startswith("ldap_"): continue
        elif sym.startswith("ne_"): continue
        elif sym.startswith("opj_"): continue
        elif sym.startswith("pg_"): continue
        elif sym.startswith("pq"): continue
        elif sym.startswith("presolve_"): continue
        elif sym.startswith("sqlite3_"): continue
        # dynamically loaded
        elif sym.endswith("get_implementation"): continue
        elif sym.endswith("component_getFactory"): continue
        elif sym == "CreateDialogFactory": continue
        elif sym == "CreateUnoWrapper": continue
        elif sym == "CreateWindow": continue
        elif sym == "ExportDOC": continue
        elif sym == "ExportPPT": continue
        elif sym == "ExportRTF": continue
        elif sym == "GetSaveWarningOfMSVBAStorage_ww8": continue
        elif sym == "GetSpecialCharsForEdit": continue
        elif sym.startswith("Import"): continue
        elif sym.startswith("Java_com_sun_star_"): continue
        elif sym.startswith("TestImport"): continue
        elif sym.startswith("getAllCalendars_"): continue
        elif sym.startswith("getAllCurrencies_"): continue
        elif sym.startswith("getAllFormats"): continue
        elif sym.startswith("getBreakIteratorRules_"): continue
        elif sym.startswith("getCollationOptions_"): continue
        elif sym.startswith("getCollatorImplementation_"): continue
        elif sym.startswith("getContinuousNumberingLevels_"): continue
        elif sym.startswith("getDateAcceptancePatterns_"): continue
        elif sym.startswith("getForbiddenCharacters_"): continue
        elif sym.startswith("getIndexAlgorithm_"): continue
        elif sym.startswith("getLCInfo_"): continue
        elif sym.startswith("getLocaleItem_"): continue
        elif sym.startswith("getOutlineNumberingLevels_"): continue
        elif sym.startswith("getReservedWords_"): continue
        elif sym.startswith("getSTC_"): continue
        elif sym.startswith("getSearchOptions_"): continue
        elif sym.startswith("getTransliterations_"): continue
        elif sym.startswith("getUnicodeScripts_"): continue
        elif sym.startswith("lok_"): continue
        # UDK API
        elif sym.startswith("osl_"): continue
        elif sym.startswith("rtl_"): continue
        elif sym.startswith("typelib_"): continue
        elif sym.startswith("typereg_"): continue
        elif sym.startswith("uno_"): continue
        # remove things we found that do not exist in our source code, they're not ours
        if not(extractFunctionNameFromSignature(sym) in all_source_names): continue
        f.write(sym + "\n")

with open("bin/find-can-be-private-symbols.classes.results", "wt") as f:
    for sym in sorted(classes_with_exported_symbols - classes_with_imported_symbols):
        # externals
        if sym.startswith("libcdr"): continue
        elif sym.startswith("libabw"): continue
        elif sym.startswith("libebook"): continue
        elif sym.startswith("libepubgen"): continue
        elif sym.startswith("libfreehand"): continue
        elif sym.startswith("libmspub"): continue
        elif sym.startswith("libpagemaker"): continue
        elif sym.startswith("libqxp"): continue
        elif sym.startswith("libvisio"): continue
        elif sym.startswith("libzmf"): continue
        elif sym.startswith("lucene::"): continue
        elif sym.startswith("Sk"): continue
        f.write(sym + "\n")
