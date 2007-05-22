/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DragMethod_Base.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:07:26 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef CHART2_DRAGMETHOD_BASE_HXX
#define CHART2_DRAGMETHOD_BASE_HXX

#include "DrawViewWrapper.hxx"

#ifndef _SVX_ACTIONDESCRIPTIONPROVIDER_HXX
#include <svx/ActionDescriptionProvider.hxx>
#endif
// header for class SdrDragMethod
#ifndef _SVDDRGMT_HXX
#include <svx/svddrgmt.hxx>
#endif

// header for class WeakReference
#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif

namespace chart
{

class DragMethod_Base : public SdrDragMethod
{
public:
    DragMethod_Base( DrawViewWrapper& rDrawViewWrapper, const rtl::OUString& rObjectCID
        , const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xChartModel
        , ActionDescriptionProvider::ActionType eActionType = ActionDescriptionProvider::MOVE );
    virtual ~DragMethod_Base();

    virtual rtl::OUString getUndoDescription() const;

    virtual void TakeComment(String& rStr) const;
    virtual FASTBOOL IsMoveOnly() const;
    virtual Pointer GetPointer() const;
    virtual void Brk();

protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > getChartModel() const;

protected:
    DrawViewWrapper&    m_rDrawViewWrapper;
    rtl::OUString       m_aObjectCID;
    ActionDescriptionProvider::ActionType m_eActionType;

private:
    ::com::sun::star::uno::WeakReference< ::com::sun::star::frame::XModel > m_xChartModel;
};

} //  namespace chart

// CHART2_DRAGMETHOD_BASE_HXX
#endif
