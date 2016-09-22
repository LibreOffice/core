#!/usr/bin/python

import sys
import io

definitionSet = set()
overridingSet = set()


with io.open("loplugin.unnecessaryvirtual.log", "rb", buffering=1024*1024) as txt:
    for line in txt:
        tokens = line.strip().split("\t")
        if tokens[0] == "definition:":
            clazzName = tokens[1]
            definitionSet.add(clazzName)
        elif tokens[0] == "overriding:":
            clazzName = tokens[1]
            overridingSet.add(clazzName)
            
with open("loplugin.unnecessaryvirtual.report", "wt") as f:
    for clazz in sorted(definitionSet - overridingSet):
        # external code
        if clazz.startswith("std::"): continue
        if clazz.startswith("icu_"): continue
        if clazz.startswith("__cxx"): continue
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
        f.write(clazz + "\n")
    # add an empty line at the end to make it easier for the removevirtuals plugin to mmap() the output file 
    f.write("\n")

