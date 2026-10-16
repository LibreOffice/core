"""Probe 4: the witness with driver cells blanked / turned into text, so the
empty-cell and text arms run through the real conditional-format path."""
import re, shutil, zipfile, os
SRC = '/home/user/libreoffice-core/dotnet/probes/cfsem-r144/072_Gantt_project_planner_dde00e33.xlsx'
DST = '/home/user/libreoffice-core/dotnet/probes/cfsem-r144/probe4.xlsx'

zin = zipfile.ZipFile(SRC)
sheet = zin.read('xl/worksheets/sheet11.xml').decode('utf-8')

# F12 -> empty; D14 -> empty; G16 already 0; C20 -> empty; E24 -> text; G27 -> empty
DROP = ['F12', 'D14', 'C20', 'F25']
for ref in DROP:
    sheet = re.sub(r'<c r="%s"[^>]*?(?:/>|>.*?</c>)' % ref, '', sheet, flags=re.S)
# E24 becomes an inline string
sheet = re.sub(r'<c r="E24"[^>]*?(?:/>|>.*?</c>)',
               '<c r="E24" t="inlineStr"><is><t>n/a</t></is></c>', sheet, flags=re.S)
# C22 becomes an inline string too (tests $C1>0 against text)
sheet = re.sub(r'<c r="C22"[^>]*?(?:/>|>.*?</c>)',
               '<c r="C22" t="inlineStr"><is><t>TBD</t></is></c>', sheet, flags=re.S)

with zipfile.ZipFile(DST, 'w', zipfile.ZIP_DEFLATED) as zo:
    for it in zin.infolist():
        data = zin.read(it.filename)
        if it.filename == 'xl/worksheets/sheet11.xml':
            data = sheet.encode('utf-8')
        zo.writestr(it, data)
print(DST, 'blanked:', DROP, 'text: E24, C22')
