# -*- encoding: UTF-8 -*-
import sys, os, zipfile

distname = 'librelogo-0.2.oxt'
z = zipfile.ZipFile(distname, mode='w', compression = zipfile.ZIP_DEFLATED)

for i in ["LibreLogo/LibreLogo.py", "pythonpath/en.properties", "pythonpath/hu.properties", \
    "META-INF/manifest.xml", "description.xml", "Addons.xcu", "README", \
    "Office/UI/WriterWindowState.xcu", "Office/UI/StartModuleWindowState.xcu", \
    "help/en-US/LibreLogo/LibreLogo.xhp", "help/en-US/help.tree", "pythonpath/librelogodummy_path.py", \
    "description-en.txt", "description-hu.txt", "LibreLogoDummy.py" ] + \
"""icons/lc_arrowshapes.circular-arrow.png
icons/lc_arrowshapes.circular-leftarrow.png
icons/lc_arrowshapes.down-arrow.png
icons/lc_arrowshapes.up-arrow.png
icons/lc_basicstop.png
icons/lc_editglossary.png
icons/lc_navigationbarleft.png
icons/lc_newdoc.png
icons/lc_runbasic.png
icons/sc_arrowshapes.circular-arrow.png
icons/sc_arrowshapes.circular-leftarrow.png
icons/sc_arrowshapes.down-arrow.png
icons/sc_arrowshapes.up-arrow.png
icons/sc_basicstop.png
icons/sc_editglossary.png
icons/sc_navigationbarleft.png
icons/sc_newdoc.png
icons/sc_runbasic.png""".split("\n"):
    z.writestr(i, open(i, "r").read())
