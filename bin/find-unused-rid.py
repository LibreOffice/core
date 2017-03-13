#!/usr/bin/python

# Search for unused RID_ constants.
#
# Note that sometimes RID constants are calculated, so some careful checking of the output is necessary.
#
# Takes about 30min to run this on a fast machine.

import subprocess
import sys

exclusionSet = set([
    # List of RID constants where we compute a value using a base before calling one of the RESSTR methods
    # Found with: git grep -P 'RID_\w+\s*\+' -- :/ ':!*.hrc' ':!*.src' ':!*.java' ':!*.py' ':!*.xba'
    "RID_SVXSTR_KEY_",
    "RID_SVXITEMS_SHADOW_",
    "RID_SVXITEMS_BREAK_",
    "RID_SVXITEMS_FRMDIR_",
    "RID_SVXITEMS_COLOR",
    "RID_SVXITEMS_HORJUST_",
    "RID_SVXITEMS_VERJUST_",
    "RID_SVXITEMS_JUSTMETHOD_",
    "RID_SVXITEMS_ADJUST_",
    "RID_SVXITEMS_WEIGHT_",
    "RID_SVXITEMS_UL_",
    "RID_SVXITEMS_OL_",
    "RID_SVXITEMS_STRIKEOUT_",
    "RID_SVXITEMS_CASEMAP_",
    "RID_SVXITEMS_ESCAPEMENT_",
    "RID_SVXITEMS_EMPHASIS_",
    "RID_SVXITEMS_RELIEF_",
    "RID_SVXITEMS_FRMDIR_",
    "RID_UPDATE_BUBBLE_TEXT_",
    "RID_UPDATE_BUBBLE_T_TEXT_",
    "RID_SVXSTR_TBLAFMT_",
    "RID_BMP_CONTENT_",
    "RID_DROPMODE_",
    "RID_BMP_LEVEL",
    "RID_SVXSTR_BULLET_DESCRIPTION",
    "RID_SVXSTR_SINGLENUM_DESCRIPTION",
    "RID_SVXSTR_OUTLINENUM_DESCRIPTION",
    "RID_SVXSTR_RULER_",
    "RID_GALLERYSTR_THEME_",
    "RID_SVXITEMS_ORI_",
    "RID_SVXITEMS_PAGE_NUM_",
    "RID_SVXSTR_BULLET_DESCRIPTION",
    "RID_SVXSTR_SINGLENUM_DESCRIPTION",
    "RID_SVXSTR_OUTLINENUM_DESCRIPTION",
    # doing some weird stuff in svx/source/unodraw/unoprov.cxx involving mapping of UNO api names to translated names and back again
    "RID_SVXSTR_GRDT",
    "RID_SVXSTR_HATCH",
    "RID_SVXSTR_BMP",
    "RID_SVXSTR_DASH",
    "RID_SVXSTR_LEND",
    "RID_SVXSTR_TRASNGR",
    # other places doing calculations
    "RID_SVXSTR_DEPTH",
    "RID_SUBSETSTR_"
    ])


def startswith_one_of( a, aset ):
    for f in aset:
        if a.startswith(f):
            return True;
    return False;

a = subprocess.Popen("git grep -P '^#define\s+RID_\w+\s+' -- *.hrc | sort -u", stdout=subprocess.PIPE, shell=True)

with a.stdout as txt:
    for line in txt:
        idx1 = line.find("#define ")
        idx2 = line.find(" ", idx1 + 9)
        ridName = line[idx1+8 : idx2]
        # the various _START and _END constants are normally unused outside of the .hrc and .src files, and that's fine
        if ridName.endswith("_START"): continue
        if ridName.endswith("_BEGIN"): continue
        if ridName.endswith("_END"): continue
        if ridName == "RID_GROUPS_SFXOFFSET": continue
        if ridName == "RID_SVX_FIRSTFREE": continue
        if startswith_one_of(ridName, exclusionSet): continue
        # search for the constant
        b = subprocess.Popen(["git", "grep", "-w", ridName], stdout=subprocess.PIPE)
        # check if we found one in actual code
        found_in_code = False
        # check that the constant is not being used as an identifier by MenuItem entries in .src files
        found_menu_identifier = False
        # check that the constant is not being used by the property controller extension or report inspection, which use macros
        # to declare constants, hiding them from a search
        found_property_macros = False
        with b.stdout as txt2:
            for line2 in txt2:
                if not line2.endswith(".hrc") and not line2.endswith(".src"): found_in_code = True
                if line2.find("Identifier = ") != -1: found_menu_identifier = True
                if line2.find("extensions/source/propctrlr") != -1: found_property_macros = True
                if line2.find("reportdesign/source/ui/inspection/inspection.src") != -1: found_property_macros = True
        if not found_in_code and not found_menu_identifier and not found_property_macros:
            sys.stdout.write(ridName + '\n')
            # otherwise the previous line of output will be incorrectly mixed into the below git output, because of buffering
            sys.stdout.flush()
            # search again, so we log the location and filename of stuff we want to remove
            subprocess.call(["git", "grep", "-wn", ridName])

