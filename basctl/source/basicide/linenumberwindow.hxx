/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef BASCTL_LINENUMBERWINDOW_HXX
#define BASCTL_LINENUMBERWINDOW_HXX

#include <vcl/window.hxx>

namespace basctl
{

class ModulWindow;

class LineNumberWindow : public Window
{
private:
    ModulWindow* m_pModulWindow;
    int m_nWidth;
    long m_nCurYOffset;
    int m_nBaseWidth;
    virtual void DataChanged (DataChangedEvent const& rDCEvt) SAL_OVERRIDE;

protected:
    virtual void Paint( const Rectangle& ) SAL_OVERRIDE;

public:
    LineNumberWindow (Window* pParent, ModulWindow* pModulWin);
    ~LineNumberWindow();

    void DoScroll( long nHorzScroll, long nVertScroll );

    bool SyncYOffset();
    long& GetCurYOffset();

    int GetWidth();
};

} // namespace basctl

#endif // BASCTL_LINENUMBERWINDOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
