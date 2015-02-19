/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_COMPHELPER_LIBRARYNAME_HXX
#define INCLUDED_COMPHELPER_LIBRARYNAME_HXX

#include <sal/types.h>
#include <rtl/string.hxx>

class MergedLibsSet
{
    std::set<OString> m_aMergedLibs;

public:
    MergedLibsSet()
    {
        /* keep in sync with solenv/gbuild/extensions/pre_MergedLibsList.mk */
        m_aMergedLibs.insert("avmedia");
        m_aMergedLibs.insert("basebmp");
        m_aMergedLibs.insert("basegfx");
        m_aMergedLibs.insert("canvastools");
        m_aMergedLibs.insert("configmgr");
        m_aMergedLibs.insert("cppcanvas");
        m_aMergedLibs.insert("dbtools");
        m_aMergedLibs.insert("deployment");
        m_aMergedLibs.insert("deploymentmisc");
        m_aMergedLibs.insert("desktopbe1");
        m_aMergedLibs.insert("desktop_detector");
        m_aMergedLibs.insert("drawinglayer");
        m_aMergedLibs.insert("editeng");
        m_aMergedLibs.insert("filterconfig");
        m_aMergedLibs.insert("fsstorage");
        m_aMergedLibs.insert("fwe");
        m_aMergedLibs.insert("fwi");
        m_aMergedLibs.insert("fwk");
        m_aMergedLibs.insert("helplinker");
        m_aMergedLibs.insert("i18npool");
        m_aMergedLibs.insert("i18nutil");
        m_aMergedLibs.insert("lng");
        m_aMergedLibs.insert("localebe1");
        m_aMergedLibs.insert("mcnttype");
        m_aMergedLibs.insert("msfilter");
        m_aMergedLibs.insert("package2");
        m_aMergedLibs.insert("sax");
        m_aMergedLibs.insert("sb");
        m_aMergedLibs.insert("sfx");
        m_aMergedLibs.insert("sofficeapp");
        m_aMergedLibs.insert("sot");
        m_aMergedLibs.insert("spl");
        m_aMergedLibs.insert("svl");
        m_aMergedLibs.insert("svt");
        m_aMergedLibs.insert("svx");
        m_aMergedLibs.insert("svxcore");
        m_aMergedLibs.insert("tk");
        m_aMergedLibs.insert("tl");
        m_aMergedLibs.insert("tubes");
        m_aMergedLibs.insert("ucb1");
        m_aMergedLibs.insert("ucpexpand1");
        m_aMergedLibs.insert("ucpfile1");
        m_aMergedLibs.insert("unoxml");
        m_aMergedLibs.insert("utl");
        m_aMergedLibs.insert("uui");
        m_aMergedLibs.insert("vcl");
        m_aMergedLibs.insert("vclplug_svp");
        m_aMergedLibs.insert("xmlscript");
        m_aMergedLibs.insert("xo");
        m_aMergedLibs.insert("xstor");
    }

    bool Contains(const OString& rName)
    {
        if (rName.endsWith("lo"))
            return Contains(rName.copy(0, rName.getLength() - 2));

        return m_aMergedLibs.find(rName) != m_aMergedLibs.end();
    }
};

OString GetLibraryName(const OString& rLib)
{
    static MergedLibsSet aMergedLibsSet;

    OStringBuffer sLibraryName;
#ifdef SAL_DLLPREFIX
    sLibraryName.append(SAL_DLLPREFIX);
#endif
    if (aMergedLibsSet.Contains(rLib))
        sLibraryName.append("mergedlo");
    else
        sLibraryName.append(rLib);

    sLibraryName.append(SAL_DLLEXTENSION);

    return sLibraryName.makeStringAndClear();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
