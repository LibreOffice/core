/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_NOTEBOOKBAR_HXX
#define INCLUDED_VCL_NOTEBOOKBAR_HXX

#include <vcl/builder.hxx>
#include <vcl/ctrl.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/EnumContext.hxx>
#include <com/sun/star/ui/XContextChangeEventListener.hpp>

/// This implements Widget Layout-based notebook-like menu bar.
class VCL_DLLPUBLIC NotebookBar : public Control, public VclBuilderContainer
{
friend class NotebookBarContextChangeEventListener;
public:
    NotebookBar(Window* pParent, const OString& rID, const OUString& rUIXMLDescription, const css::uno::Reference<css::frame::XFrame> &rFrame);
    virtual ~NotebookBar();
    virtual void dispose() SAL_OVERRIDE;

    virtual Size GetOptimalSize() const SAL_OVERRIDE;
    virtual void setPosSizePixel(long nX, long nY, long nWidth, long nHeight, PosSizeFlags nFlags = PosSizeFlags::All) SAL_OVERRIDE;

    void SetIconClickHdl(Link<NotebookBar*, void> aHdl);

    virtual void StateChanged(StateChangedType nType) override;

    const css::uno::Reference<css::ui::XContextChangeEventListener>& getContextChangeEventListener() const { return m_pEventListener; }
private:
    css::uno::Reference<css::ui::XContextChangeEventListener> m_pEventListener;
    VclPtr<NotebookbarTabControl> m_pTabControl;
};


#endif // INCLUDED_VCL_NOTEBOOKBAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
