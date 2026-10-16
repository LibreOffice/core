#!/usr/bin/env python3
"""Adds this round's cause classification to the two-reference screen.

Reads `screen26.tsv` (the ours / 24.2 / 26.2 comparison) and emits the round's
deliverable TSV: every one of the 87 gate mismatches with a cause and a one-line
observation.  A document is only given a cause the round actually established;
where it was screened but not read, the cause says so rather than guessing.
"""
import pathlib
import sys

SRC = pathlib.Path("/home/user/mismatch-work/screen26.tsv")

# Keyed on the document's basename. Only the 38 rows that still fail against
# 26.2.4.2 need an entry; everything else is the version gap by measurement.
CAUSE = {
    # ---- the two references disagree with each other: read, do not score ----
    "001_Contextures_chart_sample_b089bc34.xlsx": (
        "refs-disagree",
        "26.2 paginates 11 pages where 24.2 and we make 15; glyphs agree with 26.2 to 0.6%"),
    "02_mcar_part-2_and_IS_v2.10.docx": (
        "refs-disagree",
        "26.2 makes 200 pages against 24.2's 312 and our 314 - a 112-page reference split"),
    "SPA-02_mcar_part-2_and_IS_v2.9.docx": (
        "refs-disagree",
        "26.2 makes 205 pages against 24.2's 266 and our 268; same shape as its v2.10 sibling"),
    "150_5300_13_chg10.doc": (
        "refs-disagree",
        "77 ours, 76 on 24.2, 78 on 26.2 - we sit between the two references"),
    "ABCD-FE-01-00 Flight Envelope - v1 08.03.16.docx": (
        "refs-disagree",
        "14/15/16 pages; our glyphs are within 0.4% of 26.2 and 3.8% of 24.2"),
    "070_Equipment_inventory_list_Use_this_template_fd524c8a.xlsx": (
        "refs-disagree",
        "943 glyphs on 24.2, 1028 on 26.2, 1128 ours; a chart-fallback notice one ref draws"),
    "033_Event_planning_tracker_Use_this_template_f29a848e.xlsx": (
        "refs-disagree",
        "2783 on 24.2, 2650 on 26.2, 2720 ours - inside the band against neither"),

    # ---- the reference draws a picture or an outline where we draw text ----
    "Demick_JetBlue.pptx": ("ceiling-raster", "listed in TODO.raster-ceiling.md, verdict ceiling"),
    "W3_Case_Study_of_a_Tsunami_Warning_Simulation_Exercise_Ed.ppt": (
        "ceiling-raster",
        "listed in TODO.raster-ceiling.md; shares its +500 glyph surplus token for token with "
        "Thailand17.ppt, which carries the same slide"),
    "Thailand17.ppt": (
        "ceiling-raster",
        "listed in TODO.raster-ceiling.md; same slide as W3_Case_Study, same +500"),
    "OnTrac_StarCertificationProgram-3Day.pptx": ("ceiling-raster", "listed in TODO.raster-ceiling.md"),
    "N2_E_Maestroni_Swarm_COP.pptx": ("ceiling-raster", "listed in TODO.raster-ceiling.md"),
    "16 - UTM - (NASA).pptx": ("ceiling-raster", "listed in TODO.raster-ceiling.md"),
    "8_P-Pavese_AIRBUS-ATB-journee-CRATB.pptx": ("ceiling-raster", "listed in TODO.raster-ceiling.md"),

    # ---- the reference recalculates; we print the file's cached values ----
    "040_Blood_pressure_tracker_872b6833.xlsx": (
        "volatile-recalc",
        "ours 11/6/2022 x4 where both references print today's date; nothing else differs"),
    "045_Check_register_with_chart_4becb8a0.xlsx": (
        "volatile-recalc", "ours 02/27/2023, references 09/06/2026 - the render date"),
    "047_Date_tracker_Gantt_chart_bf34f3a8.xlsx": (
        "volatile-recalc", "ours 1/10/2024, references 2026 - a Gantt keyed off TODAY()"),
    "065_Weight_loss_tracker_ff1c89af.xlsx": (
        "volatile-recalc",
        "dates, plus two of our own: an unformatted serial (44790) and the day-name format "
        "code 'aaaa' printed literally where the reference prints Sunday"),
    "030_Basic_balance_sheet_Use_this_template_10ed9144.xlsx": (
        "volatile-recalc", "ours 2022/2021, references 2025/2026 - a YEAR(TODAY()) heading"),
    "sistem-rekod-markah-srm-_-rekod-master.xlsx": (
        "volatile-recalc",
        "1395 cached 5s against the references' 1395 #N/A - a lookup they re-evaluate"),

    # ---- closed this round ----
    "062_Run_chart_cb7476ea.xlsx": (
        "chartsheet-scale",
        "CLOSED: chart sheet not fitted to one page; 3 pages -> 2, 680 glyphs -> 643 (ref 645)"),
    "057_Simple_balance_sheet_Use_this_template_e2d4cbb2.xlsx": (
        "chartsheet-scale",
        "PARTLY CLOSED: 4 pages -> 3, pages,words -> words. The residual is the outlining "
        "ceiling - the reference's chart page yields 112 alnum characters to pdftotext "
        "against our 398, because it draws the rotated category labels as vector outlines"),

    # ---- singletons, each read or measured, each its own mechanism ----
    "024_Unit_Circle_Chart_Colorful_Circles_7c92601e.docx": (
        "docx-smartart",
        "we draw an empty frame where the reference draws a five-node SmartArt diagram. "
        "Diagram support exists for PPTX (XRay.Office.Presentations/Ooxml/PptxDiagram) and not "
        "for DOCX. Three corpus docx carry a diagram; the other two pass inside the band"),
    "Template Pilot Logbook JAR-FCL V3.0.xls": (
        "number-format-time",
        "124 cells read 00:00 where the reference reads 0:00, and the date cells differ in the "
        "same way - a leading zero our h:mm formatter adds"),
    "042_Business_monthly_budget_4e4d092f.xlsx": (
        "number-format-unapplied",
        "we print 500, 1000, -100 where the reference prints 500.00, 1,000.00, (100.00)"),
    "014_Contextures_chart_sample_991ecfc5.xls": (
        "chart-broken",
        "read: our chart plots ~20 #N/A categories, has no secondary axis, and its bars run "
        "off the top of the plot; value axis reads $1/$0 against the reference's $800/$720"),
    "068_Blue_inventory_list_Use_this_template_f9908489.xlsx": (
        "missing-text",
        "the reference draws INVENTORY LIST x3, PICK, BIN, LOOKUP that we omit entirely; "
        "-58 glyphs accounts for the whole gap"),
    "omrIMInterpretiveGuideLine.doc": (
        "missing-text",
        "six words the reference draws and we do not - Mental Retardation Program Appropriate "
        "Regional Managers; -51 glyphs is exactly those six"),
    "UG.CAO.00006 Foreign Part 145 approvals - User Guide for Applicants & Approval Holders.docx": (
        "unclassified",
        "+1384; we draw 23 extra 'note' and much other body text, the reference draws TOC "
        "leader lines we do not. Screened, not read"),
    "053_Personal_asset_inventory_5446d84b.xlsx": (
        "unclassified",
        "4 pages against 2, +40 glyphs of which the surplus is two extra Page n of 4 footers; "
        "read: our theme colour is blue where the reference's is teal, on both the heading and "
        "the line art, and our value axis steps 100k where the reference steps 50k"),
    "055_Project_timeline_with_milestones_Use_this_template_546cecc0.xlsx": (
        "unclassified", "+166; chart text on both sides, shattered into fragments. Screened, not read"),
    "029_Annual_budget_Use_this_template_30324a97.xlsx": (
        "unclassified", "+29; chart text fragments on both sides. Screened, not read"),
    "048_Expense_trends_budget_18d1e8ba.xlsx": (
        "unclassified", "+102; instruction text differs in wrapping. Screened, not read"),
    "076_Inventory_list_accessibility_guide_Use_this_template_f43dab19.xlsx": (
        "unclassified",
        "+167; we draw 'Inventory list' x5 and 'Navigation links' x3 the reference does not. "
        "Screened, not read"),
    "1447.doc": (
        "pagination-fill",
        "4 pages against 3 with identical glyph counts and an empty token diff; we fit less on "
        "every page (p1 1570 against 1928) and spill 103 glyphs onto a fourth"),
    "AAC-AD-No-2021-01-Boeing-737-8-and-737-9-MAX.doc": (
        "pagination-break",
        "21 pages against 20; our page 3 holds MORE than the reference's (3266 against 3117) "
        "and is then followed by a 245-glyph page 4 the reference has no counterpart for. "
        "Every later page is exactly one behind"),
    "absrc-pac-01-info-note-en.doc": (
        "pagination-break",
        "6 pages against 7: the reference splits its page 1 into 411 + 146 glyphs where we "
        "keep 555 together, and every later page is one behind. A break it takes and we do "
        "not - the mirror of AAC-AD, where we take one it does not"),
    "OM template for non-complex NCC operators_August 2016.docx": (
        "pagination-fill", "166 pages against 165, glyphs agree to 0.03%"),
}

rows = []
head = None
for line in SRC.read_text().splitlines():
    if line.startswith("#"):
        continue
    f = line.split("\t")
    if f[0] == "path":
        head = f
        continue
    base = f[0].rsplit("/", 1)[-1]
    if f[9] == "match":
        cause, note = "version-gap", "matches 26.2.4.2 under the gate's own rule"
    elif base in CAUSE:
        cause, note = CAUSE[base]
    else:
        cause, note = "UNASSIGNED", ""
        print(f"no cause for {base}", file=sys.stderr)
    rows.append(f + [cause, note])

print("# The 87 documents the whole-corpus gate scored MISMATCH at X-Ray.Office 2f4709c08.")
print("# ours   = XRay.Office.Cli @ 2f4709c08, XRAY_OFFICE_BUNDLED_FONTS unset (installed faces win)")
print("# ref24  = /usr/bin/soffice, LibreOffice 24.2.7.2 420(Build:2) - what batch-check.sh measures")
print("# ref26  = /opt/libreoffice26.2/program/soffice, LibreOffice 26.2.4.2 0229ac93fcf0d7cb,")
print("#          with the eight Latin duplicate faces moved aside (.duplicates-aside/.noto-aside)")
print("# fonts  = system fontconfig: Carlito, Caladea, Liberation, DejaVu, WenQuanYi, IPAGothic")
print("# rule   = batch-check.sh of 2026-09-05: page count, then max(2%, 15) alphanumeric characters")
print("# host   = /home/user/sample-files corpus, 947 documents; measured 2026-09-06")
print("\t".join(head + ["cause", "note"]))
for r in rows:
    print("\t".join(r))
