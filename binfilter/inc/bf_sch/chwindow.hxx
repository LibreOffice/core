/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SCH_SDWINDOW_HXX
#define _SCH_SDWINDOW_HXX


#include <tools/gen.hxx>
#include <vcl/window.hxx>
#include <vcl/event.hxx>
#include <bf_svtools/transfer.hxx>
namespace com { namespace sun { namespace star {
    namespace accessibility {
        class XAccessible;
    }
}}}
namespace binfilter {

class SchViewShell;
class ChartModel;

//STRIP008 namespace com { namespace sun { namespace star {
//STRIP008 	namespace accessibility {
//STRIP008 		class XAccessible;
//STRIP008 	}
//STRIP008 }}}

namespace accessibility 
{
    class AccessibleDocumentView;
}

/*************************************************************************
|*
|* SchWindow beinhaltet den eigentlichen Arbeitsbereich von
|* SchViewShell
|*
\************************************************************************/
class SchWindow : public Window, public DropTargetHelper
{
protected:
    Point	aWinPos;
    Point	aViewOrigin;
    Size	aViewSize;

    SchViewShell* pViewShell;


    // for quick help

    /** Create an accessibility object that makes this window accessible.
        
        @return
            The returned reference is empty if no accessible object could be
            created.
    */

private:
    // for quick help

    /// attention: this pointer is only valid, if the weak reference below is valid
    ::binfilter::accessibility::AccessibleDocumentView * m_pAccDocumentView;
    ::com::sun::star::uno::WeakReference< ::com::sun::star::accessibility::XAccessible >
        m_xAccDocumentView;

public:
    SchWindow(Window* pParent);
    virtual ~SchWindow();

    void SetViewShell(SchViewShell* pViewSh) { pViewShell=pViewSh; };


    void SetZoom(long nZoom);
    long SetZoomFactor(long nZoom);

    long GetZoom() const
        { return GetMapMode().GetScaleX().GetNumerator() * 100L /
                 GetMapMode().GetScaleX().GetDenominator(); }

    void SetWinPos(const Point& rPos) { aWinPos = rPos; }
    const Point& GetWinPos() { return aWinPos; }
    void SetViewOrigin(const Point& rPos) { aViewOrigin = rPos; }
    const Point& GetViewOrigin() { return aViewOrigin; }
    void SetViewSize(const Size& rSize) { aViewSize = rSize; }
    const Size& GetViewSize() { return aViewSize; }


    // DropTargetHelper



};

} //namespace binfilter
#endif		// _SCH_SDWINDOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
