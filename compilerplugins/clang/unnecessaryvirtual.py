#!/usr/bin/python

import io
import re
import sys

definitionSet = set()
definitionToSourceLocationMap = dict()
overridingSet = set()


with io.open("loplugin.unnecessaryvirtual.log", "rb", buffering=1024*1024) as txt:
    for line in txt:
        tokens = line.strip().split("\t")
        if tokens[0] == "definition:":
            fullMethodName = tokens[1]
            sourceLocation = tokens[2]
            definitionSet.add(fullMethodName)
            definitionToSourceLocationMap[fullMethodName] = sourceLocation
        elif tokens[0] == "overriding:":
            fullMethodName = tokens[1]
            overridingSet.add(fullMethodName)
            
unnecessaryVirtualSet = set()

for clazz in (definitionSet - overridingSet):
    # windows-specific stuff
    if clazz.startswith("canvas::"): continue
    if clazz.startswith("psp::PrinterInfoManager"): continue
    if clazz.startswith("DdeTopic::"): continue
    if clazz == "basegfx::unotools::UnoPolyPolygon::void-modifying()const": continue
    if clazz == "SalLayout::_Bool-IsKashidaPosValid(int,)const": continue
    if clazz == "SalLayout::void-DisableGlyphInjection(_Bool,)": continue
    # Linux-TDF specific
    if clazz == "X11SalFrame::void-updateGraphics(_Bool,)": continue
    # OSX specific
    if clazz == "SalFrame::void-SetRepresentedURL(const class rtl::OUString &,)": continue
    if clazz == "SalMenu::_Bool-AddMenuBarButton(const struct SalMenuButtonItem &,)": continue
    if clazz == "SalMenu::class Rectangle-GetMenuBarButtonRectPixel(sal_uInt16,class SalFrame *,)": continue
    if clazz == "SalMenu::void-RemoveMenuBarButton(sal_uInt16,)": continue
    if clazz == "SalLayout::_Bool-DrawTextSpecial(class SalGraphics &,sal_uInt32,)const": continue
    # GTK < 3
    if clazz == "GtkSalDisplay::int-CaptureMouse(class SalFrame *,)": continue
    # some test magic
    if clazz.startswith("apitest::"): continue
    # ignore external code
    if definitionToSourceLocationMap[clazz].startswith("external/"): continue

    unnecessaryVirtualSet.add((clazz,definitionToSourceLocationMap[clazz] ))


# sort the results using a "natural order" so sequences like [item1,item2,item10] sort nicely
def natural_sort_key(s, _nsre=re.compile('([0-9]+)')):
    return [int(text) if text.isdigit() else text.lower()
            for text in re.split(_nsre, s)]

# sort results by name and line number
tmp1list = sorted(unnecessaryVirtualSet, key=lambda v: natural_sort_key(v[1]))

with open("compilerplugins/clang/unnecessaryvirtual.results", "wt") as f:
    for t in tmp1list:
        f.write( t[1] + "\n" )
        f.write( "    " + t[0] + "\n" )
    # add an empty line at the end to make it easier for the removevirtuals plugin to mmap() the output file
    f.write("\n")

