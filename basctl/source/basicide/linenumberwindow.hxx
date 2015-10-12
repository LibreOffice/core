/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_BASCTL_SOURCE_BASICIDE_LINENUMBERWINDOW_HXX
#define INCLUDED_BASCTL_SOURCE_BASICIDE_LINENUMBERWINDOW_HXX


namespace basctl
{

class ModulWindow;

class LineNumberWindow : public vcl::Window
{
private:
    VclPtr<ModulWindow> m_pModulWindow;
    int m_nWidth;
    long m_nCurYOffset;
    int m_nBaseWidth;
    virtual void DataChanged (DataChangedEvent const& rDCEvt) override;

protected:
    virtual void Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect) override;

public:
    LineNumberWindow (vcl::Window* pParent, ModulWindow* pModulWin);
    virtual ~LineNumberWindow();
    virtual void dispose() override;

    void DoScroll( long nHorzScroll, long nVertScroll );

    bool SyncYOffset();
    long& GetCurYOffset() { return m_nCurYOffset;}

    int GetWidth() { return m_nWidth;}
};

} // namespace basctl

#endif // INCLUDED_BASCTL_SOURCE_BASICIDE_LINENUMBERWINDOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
