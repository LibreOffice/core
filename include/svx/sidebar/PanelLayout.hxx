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
#include <vcl/idle.hxx>
#include <vcl/vclptr.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XFrame.hpp>

/// This class is the base for the Widget Layout-based sidebar panels.
class SVX_DLLPUBLIC PanelLayout : public Control, public VclBuilderContainer
{
private:
    Idle m_aPanelLayoutIdle;
    bool m_bInClose;
    bool hasPanelPendingLayout() const;

    DECL_DLLPRIVATE_LINK_TYPED( ImplHandlePanelLayoutTimerHdl, Idle*, void );

public:
    PanelLayout(vcl::Window* pParent, const OString& rID, const OUString& rUIXMLDescription,
            const com::sun::star::uno::Reference<com::sun::star::frame::XFrame> &rFrame);
    virtual ~PanelLayout();
    virtual void dispose() SAL_OVERRIDE;

    virtual Size GetOptimalSize() const SAL_OVERRIDE;
    virtual void setPosSizePixel(long nX, long nY, long nWidth, long nHeight, PosSizeFlags nFlags = PosSizeFlags::All) SAL_OVERRIDE;
    virtual void queue_resize(StateChangedType eReason = StateChangedType::Layout) SAL_OVERRIDE;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
