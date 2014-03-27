/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SVX_SIDEBAR_PANELLAYOUT_HXX
#define INCLUDED_SVX_SIDEBAR_PANELLAYOUT_HXX

#include <svx/svxdllapi.h>

#include <vcl/builder.hxx>
#include <vcl/ctrl.hxx>
#include <vcl/timer.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XFrame.hpp>

/// This class is the base for the Widget Layout-based sidebar panels.
class SVX_DLLPUBLIC PanelLayout : public Control, public VclBuilderContainer
{
private:
    Timer m_aPanelLayoutTimer;
    bool m_bInClose;
    bool hasPanelPendingLayout() const;

    DECL_DLLPRIVATE_LINK( ImplHandlePanelLayoutTimerHdl, void* );

public:
    PanelLayout(Window* pParent, const OString& rID, const OUString& rUIXMLDescription,
            const com::sun::star::uno::Reference<com::sun::star::frame::XFrame> &rFrame);
    virtual ~PanelLayout();

    virtual Size GetOptimalSize() const SAL_OVERRIDE;
    virtual void setPosSizePixel(long nX, long nY, long nWidth, long nHeight, sal_uInt16 nFlags = WINDOW_POSSIZE_ALL) SAL_OVERRIDE;
    virtual void queue_resize() SAL_OVERRIDE;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
