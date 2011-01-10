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

#ifndef SVX_TABLE_CELLEDITSOURCE_HXX
#define SVX_TABLE_CELLEDITSOURCE_HXX

#include "cell.hxx"
#include "celltypes.hxx"
#include "editeng/unoedsrc.hxx"

class SvxTextForwarder;
class SdrObject;
class SdrModel;
class SdrView;
class Window;

namespace sdr { namespace table {

class CellEditSourceImpl;

class CellEditSource : public SvxEditSource, public SvxViewForwarder
{
public:
    CellEditSource( const CellRef& xCell );

    /** Since the views don't broadcast their dying, make sure that
        this object gets destroyed if the view becomes invalid

        The window is necessary, since our views can display on multiple windows
     */
    CellEditSource( const CellRef& xCell, SdrView& rView, const Window& rViewWindow );
    virtual ~CellEditSource();

    virtual SvxEditSource*          Clone() const;
    virtual SvxTextForwarder*       GetTextForwarder();
     virtual SvxViewForwarder*      GetViewForwarder();
     virtual SvxEditViewForwarder*  GetEditViewForwarder( sal_Bool bCreate = sal_False );
    virtual void                    UpdateData();

    virtual void addRange( SvxUnoTextRangeBase* pNewRange );
    virtual void removeRange( SvxUnoTextRangeBase* pOldRange );
    virtual const SvxUnoTextRangeBaseList& getRanges() const;

    virtual SfxBroadcaster&         GetBroadcaster() const;

    void lock();
    void unlock();

    // the SvxViewForwarder interface
    virtual sal_Bool        IsValid() const;
    virtual Rectangle   GetVisArea() const;
    virtual Point       LogicToPixel( const Point&, const MapMode& ) const;
    virtual Point       PixelToLogic( const Point&, const MapMode& ) const;

    void ChangeModel( SdrModel* pNewModel );

private:
    CellEditSource( CellEditSourceImpl* pImpl );

    CellEditSourceImpl* mpImpl;
};

} }

#endif
