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
    "STR_VOBJ_MODE_",
    "STR_COND_",
    "SCSTR_CONTENT_",
    "DATE_FUNCDESC_",
    "DATE_FUNCNAME_",
    "DATE_DEFFUNCNAME_",
    "PRICING_DEFFUNCNAME_",
    "PRICING_FUNCDESC_",
    "PRICING_FUNCNAME_",
    "STR_ItemValCAPTION",
    "STR_ItemValCIRC",
    "STR_ItemValEDGE",
    "STR_ItemValFITTOSIZE",
    "STR_ItemValMEASURE_",
    "STR_ItemValMEASURETEXT_",
    "STR_ItemValTEXTANI_",
    "STR_ItemValTEXTHADJ",
    "STR_ItemValTEXTVADJ",
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
                if idName.startswith("SID_"):
                    if not ".hrc:" in line2 and not ".src:" in line2 and not ".sdi:" in line2: found_reason_to_exclude = True
                else:
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
                if ".src:" in line2 and (";> ;" in line2 or "; >;" in line2): found_reason_to_exclude = True
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
                if "svx/source/tbxctrls/extrusioncontrols.hrc:" in line2 and idName.startswith("DIRECTION_"): found_reason_to_exclude = True
                if "svx/source/tbxctrls/extrusioncontrols.hrc:" in line2 and idName.startswith("FROM_"): found_reason_to_exclude = True
                # if we see more than a few lines then it's probably one of the BASE/START/BEGIN things
                cnt = cnt + 1
                if cnt > 4: found_reason_to_exclude = True
        if not found_reason_to_exclude:
            sys.stdout.write(idName + '\n')
            # otherwise the previous line of output will be incorrectly mixed into the below git output, because of buffering
            sys.stdout.flush()
            # search again, so we log the location and filename of stuff we want to remove
            subprocess.call(["git", "grep", "-wn", idName])

