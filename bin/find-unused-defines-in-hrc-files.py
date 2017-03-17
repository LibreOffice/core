#!/usr/bin/python

# Search for unused constants in .hrc files.
#
# Note that sometimes these constants are calculated, so some careful checking of the output is necessary.
#
# Takes about 4 hours to run this on a fast machine with an SSD
#

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
    "RID_SUBSETSTR_",
    "ANALYSIS_",
    "FLD_DOCINFO_CHANGE",
    "FLD_EU_",
    "FLD_INPUT_",
    "FLD_PAGEREF_",
    "FLD_STAT_",
    "FMT_AUTHOR_",
    "FMT_CHAPTER_",
    "FMT_DBFLD_",
    "FMT_FF_",
    "FMT_GETVAR_",
    "FMT_MARK_",
    "FMT_REF_",
    "FMT_SETVAR_",
    "STR_AUTH_FIELD_ADDRESS_",
    "STR_AUTH_TYPE_",
    "STR_AUTOFMTREDL_",
    "STR_CONTENT_TYPE_",
    "STR_UPDATE_ALL",
    "STR_UPDATE_INDEX",
    "STR_UPDATE_LINK",
    "BMP_PLACEHOLDER_",
    "STR_RPT_HELP_",
    "STR_TEMPLATE_NAME",
    "UID_BRWEVT_",
    "HID_EVT_",
    "HID_PROP_",
    ])


def in_exclusion_set( a ):
    for f in exclusionSet:
        if a.startswith(f):
            return True;
    return False;

a = subprocess.Popen("git grep -hP '^#define\s+\w+\s+' -- *.hrc | sort -u", stdout=subprocess.PIPE, shell=True)

with a.stdout as txt:
    for line in txt:
        idx1 = line.find("#define ")
        idx2 = line.find(" ", idx1 + 9)
        idName = line[idx1+8 : idx2].strip()
        # the various _START and _END constants are normally unused outside of the .hrc and .src files, and that's fine
        if idName.endswith("_START"): continue
        if idName.endswith("_BEGIN"): continue
        if idName.endswith("_END"): continue
        if idName.startswith("RID_"):
            if idName == "RID_GROUPS_SFXOFFSET": continue
            if idName == "RID_SVX_FIRSTFREE": continue
        if in_exclusion_set(idName): continue
        # search for the constant
        b = subprocess.Popen(["git", "grep", "-w", idName], stdout=subprocess.PIPE)
        found_reason_to_exclude = False
        with b.stdout as txt2:
            cnt = 0
            for line2 in txt2:
                line2 = line2.strip() # otherwise the comparisons below will not work
                # check if we found one in actual code
                if not ".hrc:" in line2 and not ".src:" in line2: found_reason_to_exclude = True
                if idName.startswith("RID_"):
                        # is the constant being used as an identifier by entries in .src files?
                        if ".src:" in line2 and "Identifier = " in line2: found_reason_to_exclude = True
                        # is the constant being used by the property controller extension or reportdesigner inspection,
                        # which use macros to declare constants, hiding them from a search
                        if "extensions/source/propctrlr" in line2: found_reason_to_exclude = True
                        if "reportdesign/source/ui/inspection/inspection.src" in line2: found_reason_to_exclude = True
                if idName.startswith("HID_"):
                        # is the constant being used as an identifier by entries in .src files
                        if ".src:" in line2 and "HelpId = " in line2: found_reason_to_exclude = True
                # is it being used as a constant in an ItemList  in .src files?
                if ".src:" in line2 and ";> ;" in line2: found_reason_to_exclude = True
                # these are used in calculations in other .hrc files
                if "sw/inc/rcid.hrc:" in line2: found_reason_to_exclude = True
                # calculations
                if "sw/source/uibase/inc/ribbar.hrc:" in line2 and "ST_" in idName: found_reason_to_exclude = True
                if "sw/source/uibase/inc/ribbar.hrc:" in line2 and "STR_IMGBTN_" in idName: found_reason_to_exclude = True
                if "sw/source/core/undo/undo.hrc:" in line2: found_reason_to_exclude = True
                if "sw/inc/poolfmt.hrc:" in line2: found_reason_to_exclude = True
                # not sure about these, looks suspicious
                if "sd/source/ui/app/strings.src:" in line2 and idName.endswith("_TOOLBOX"): found_reason_to_exclude = True
                # used via a macro that hides them from search
                if "dbaccess/" in line2 and idName.startswith("PROPERTY_ID_"): found_reason_to_exclude = True
                if "reportdesign/" in line2 and idName.startswith("HID_RPT_PROP_"): found_reason_to_exclude = True
                if "reportdesign/" in line2 and idName.startswith("RID_STR_"): found_reason_to_exclude = True
                if "forms/" in line2 and idName.startswith("PROPERTY_"): found_reason_to_exclude = True
                # if we see more than 2 lines then it's probably one of the BASE/START/BEGIN things
                cnt = cnt + 2
                if cnt > 3: found_reason_to_exclude = True
        if not found_reason_to_exclude:
            sys.stdout.write(idName + '\n')
            # otherwise the previous line of output will be incorrectly mixed into the below git output, because of buffering
            sys.stdout.flush()
            # search again, so we log the location and filename of stuff we want to remove
            subprocess.call(["git", "grep", "-wn", idName])

